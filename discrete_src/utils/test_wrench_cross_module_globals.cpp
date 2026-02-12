/**
 * Cross-Module Global Variable Tests
 *
 * These tests characterize how Wrench VM handles global variables
 * across imported module boundaries. The goal is to make definitive
 * statements about what works, what doesn't, and what corrupts.
 *
 * PROVEN SAFE (all patterns, after vm.cpp stackOffset fix):
 * 1. Global variables are per-context (isolated between modules)
 * 2. wr_getGlobalRef() only accesses the specified context's globals
 * 3. Same-named globals in different modules are independent
 * 4. Cross-module function calls read callee's globals, not caller's
 * 5. Globals (int, string, array) survive many cross-module call rounds
 * 6. 2-module A->B calls work reliably even with heavy traffic (100+ rounds)
 * 7. C++ library functions coexist with per-module globals
 * 8. 3-module chains (A->B->C) work after stackOffset fix
 * 9. Sequential cross-module calls from different source contexts work
 * 10. Heavy traffic (72+ iterations) through 3-module chains works
 *
 * The fix (in wrench vm.cpp CallFunctionByHash / CallFunctionByHashAndPop):
 * - Save/restore import->stackOffset around wr_callFunction calls
 * - Use absolute offset (stackTop - context->stack) instead of relative
 *   (stackTop - stackBase) so nested calls compute correct frame bases
 */

#include "doctest.h"
#include "wrench.h"
#include <cstring>
#include <string>
#include <vector>

namespace {

/**
 * Helper: extract string from WRValue using asString buffer.
 * Returns empty string if value is not a string.
 */
std::string wrValueToString(WRValue* val) {
    if (!val) return "";
    char buf[256];
    char* result = val->asString(buf, sizeof(buf));
    if (!result) return "";
    return std::string(result);
}

/**
 * Helper: compile source code into bytecode.
 * Returns true on success, false on compile error.
 */
bool compileSource(const char* source, unsigned char** bytecode, int* len) {
    WRError err = wr_compile(source, strlen(source), bytecode, len, nullptr,
                              WR_INCLUDE_GLOBALS);
    return err == WR_ERR_None;
}

/**
 * Helper: set up a two-module environment.
 * Creates a main (bootstrap) context, compiles and imports two modules.
 * Returns the WRContext* for each imported module.
 */
struct TwoModuleEnv {
    WRState* state = nullptr;
    WRContext* mainCtx = nullptr;
    WRContext* moduleA = nullptr;
    WRContext* moduleB = nullptr;

    ~TwoModuleEnv() {
        if (state) wr_destroyState(state);
    }

    bool setup(const char* sourceA, const char* sourceB) {
        state = wr_newState();
        if (!state) return false;

        // Bootstrap context (empty)
        unsigned char* bootBytecode = nullptr;
        int bootLen = 0;
        if (!compileSource("", &bootBytecode, &bootLen)) return false;
        mainCtx = wr_run(state, bootBytecode, bootLen);
        if (!mainCtx) return false;

        // Module A
        unsigned char* bytecodeA = nullptr;
        int lenA = 0;
        if (!compileSource(sourceA, &bytecodeA, &lenA)) return false;
        moduleA = wr_import(mainCtx, bytecodeA, lenA);
        if (!moduleA) return false;

        // Module B
        unsigned char* bytecodeB = nullptr;
        int lenB = 0;
        if (!compileSource(sourceB, &bytecodeB, &lenB)) return false;
        moduleB = wr_import(mainCtx, bytecodeB, lenB);
        if (!moduleB) return false;

        return true;
    }
};

/**
 * Helper: set up an N-module environment.
 */
struct MultiModuleEnv {
    WRState* state = nullptr;
    WRContext* mainCtx = nullptr;
    std::vector<WRContext*> modules;

    ~MultiModuleEnv() {
        if (state) wr_destroyState(state);
    }

    bool setup(const std::vector<std::string>& sources) {
        state = wr_newState();
        if (!state) return false;

        unsigned char* bootBytecode = nullptr;
        int bootLen = 0;
        if (!compileSource("", &bootBytecode, &bootLen)) return false;
        mainCtx = wr_run(state, bootBytecode, bootLen);
        if (!mainCtx) return false;

        for (const auto& src : sources) {
            unsigned char* bytecode = nullptr;
            int len = 0;
            if (!compileSource(src.c_str(), &bytecode, &len)) return false;
            WRContext* ctx = wr_import(mainCtx, bytecode, len);
            if (!ctx) return false;
            modules.push_back(ctx);
        }
        return true;
    }
};

} // anonymous namespace

TEST_SUITE("Cross-Module Global Variables") {

    // =========================================================================
    // SECTION 1: Basic Global Isolation
    // =========================================================================

    TEST_CASE("Globals are isolated per context - integer") {
        // Module A sets globalA = 100
        // Module B sets globalB = 200
        // Verify each module only sees its own globals
        TwoModuleEnv env;
        REQUIRE(env.setup(
            "var globalA = 100;\n"
            "function getGlobalA() { return globalA; }\n",

            "var globalB = 200;\n"
            "function getGlobalB() { return globalB; }\n"
        ));

        // Each function should return its own module's global
        WRValue* resultA = wr_callFunction(env.moduleA, "getGlobalA", nullptr, 0);
        REQUIRE(resultA != nullptr);
        CHECK(resultA->asInt() == 100);

        WRValue* resultB = wr_callFunction(env.moduleB, "getGlobalB", nullptr, 0);
        REQUIRE(resultB != nullptr);
        CHECK(resultB->asInt() == 200);
    }

    TEST_CASE("Globals are isolated per context - string") {
        TwoModuleEnv env;
        REQUIRE(env.setup(
            "var globalStr = \"hello from A\";\n"
            "function getStrA() { return globalStr; }\n",

            "var globalStr = \"hello from B\";\n"
            "function getStrB() { return globalStr; }\n"
        ));

        WRValue* resultA = wr_callFunction(env.moduleA, "getStrA", nullptr, 0);
        REQUIRE(resultA != nullptr);
        CHECK(wrValueToString(resultA) == "hello from A");

        WRValue* resultB = wr_callFunction(env.moduleB, "getStrB", nullptr, 0);
        REQUIRE(resultB != nullptr);
        CHECK(wrValueToString(resultB) == "hello from B");
    }

    TEST_CASE("Same-named globals in different modules are independent") {
        // Both modules define 'var counter = 0' and increment functions
        // Incrementing in one module should NOT affect the other
        TwoModuleEnv env;
        REQUIRE(env.setup(
            "var counter = 0;\n"
            "function incrementA() { counter += 1; return counter; }\n"
            "function getCounterA() { return counter; }\n",

            "var counter = 0;\n"
            "function incrementB() { counter += 1; return counter; }\n"
            "function getCounterB() { return counter; }\n"
        ));

        // Increment A three times
        for (int i = 0; i < 3; i++) {
            wr_callFunction(env.moduleA, "incrementA", nullptr, 0);
        }

        // Increment B once
        wr_callFunction(env.moduleB, "incrementB", nullptr, 0);

        // A's counter should be 3
        WRValue* resultA = wr_callFunction(env.moduleA, "getCounterA", nullptr, 0);
        REQUIRE(resultA != nullptr);
        CHECK(resultA->asInt() == 3);

        // B's counter should be 1 (independent)
        WRValue* resultB = wr_callFunction(env.moduleB, "getCounterB", nullptr, 0);
        REQUIRE(resultB != nullptr);
        CHECK(resultB->asInt() == 1);
    }

    // =========================================================================
    // SECTION 2: wr_getGlobalRef behavior
    // =========================================================================

    TEST_CASE("wr_getGlobalRef accesses only the specified context") {
        TwoModuleEnv env;
        REQUIRE(env.setup(
            "var myGlobal = 42;\n"
            "function dummy() { return 0; }\n",

            "var myGlobal = 99;\n"
            "function dummy2() { return 0; }\n"
        ));

        // Get global from module A
        WRValue* globalA = wr_getGlobalRef(env.moduleA, "myGlobal");
        REQUIRE(globalA != nullptr);
        CHECK(globalA->asInt() == 42);

        // Get same-named global from module B - should be different value
        WRValue* globalB = wr_getGlobalRef(env.moduleB, "myGlobal");
        REQUIRE(globalB != nullptr);
        CHECK(globalB->asInt() == 99);

        // They should be different memory locations
        CHECK(globalA != globalB);
    }

    TEST_CASE("wr_getGlobalRef returns null for globals in other modules") {
        TwoModuleEnv env;
        REQUIRE(env.setup(
            "var onlyInA = 42;\n"
            "function dummy() { return 0; }\n",

            "var onlyInB = 99;\n"
            "function dummy2() { return 0; }\n"
        ));

        // Module A should NOT see module B's global
        WRValue* crossRef = wr_getGlobalRef(env.moduleA, "onlyInB");
        CHECK(crossRef == nullptr);

        // Module B should NOT see module A's global
        WRValue* crossRef2 = wr_getGlobalRef(env.moduleB, "onlyInA");
        CHECK(crossRef2 == nullptr);
    }

    TEST_CASE("wr_getGlobalRef can modify a context's global") {
        TwoModuleEnv env;
        REQUIRE(env.setup(
            "var value = 10;\n"
            "function getValue() { return value; }\n",

            "function dummy() { return 0; }\n"
        ));

        // Modify global from C++
        WRValue* ref = wr_getGlobalRef(env.moduleA, "value");
        REQUIRE(ref != nullptr);
        CHECK(ref->asInt() == 10);

        wr_makeInt(ref, 999);

        // Wrench function should see the updated value
        WRValue* result = wr_callFunction(env.moduleA, "getValue", nullptr, 0);
        REQUIRE(result != nullptr);
        CHECK(result->asInt() == 999);
    }

    // =========================================================================
    // SECTION 3: Cross-module function calls and globals
    // =========================================================================

    TEST_CASE("Cross-module function call does NOT see caller's globals") {
        // Module A: defines globalA and calls getGlobalFromB()
        // Module B: tries to read globalA (defined in A) — should NOT work
        TwoModuleEnv env;
        REQUIRE(env.setup(
            // Module A
            "var globalA = 42;\n"
            "function callIntoB() { return readGlobalA(); }\n",

            // Module B - tries to access globalA which is in A's context
            "var globalA = 0;\n" // B has its own globalA, initialized to 0
            "function readGlobalA() { return globalA; }\n"
        ));

        // When A calls B's readGlobalA(), B reads its OWN globalA (0), not A's (42)
        WRValue* result = wr_callFunction(env.moduleA, "callIntoB", nullptr, 0);
        REQUIRE(result != nullptr);
        CHECK(result->asInt() == 0); // B's globalA, not A's
    }

    TEST_CASE("Cross-module function call: callee uses its own globals") {
        TwoModuleEnv env;
        REQUIRE(env.setup(
            // Module A - calls B's function
            "function callB() { return getValueFromB(); }\n",

            // Module B - has its own global and accessor
            "var bValue = 777;\n"
            "function getValueFromB() { return bValue; }\n"
        ));

        // A calls B's function, which reads B's global — should work
        WRValue* result = wr_callFunction(env.moduleA, "callB", nullptr, 0);
        REQUIRE(result != nullptr);
        CHECK(result->asInt() == 777);
    }

    TEST_CASE("Cross-module function can modify callee's own globals") {
        TwoModuleEnv env;
        REQUIRE(env.setup(
            // Module A
            "function callSetB() { setBValue(42); }\n"
            "function callGetB() { return getBValue(); }\n",

            // Module B
            "var bState = 0;\n"
            "function setBValue(v) { bState = v; }\n"
            "function getBValue() { return bState; }\n"
        ));

        // Initially B's global is 0
        WRValue* before = wr_callFunction(env.moduleB, "getBValue", nullptr, 0);
        REQUIRE(before != nullptr);
        CHECK(before->asInt() == 0);

        // Call through A to set B's global
        wr_callFunction(env.moduleA, "callSetB", nullptr, 0);

        // B's global should now be 42
        WRValue* after = wr_callFunction(env.moduleB, "getBValue", nullptr, 0);
        REQUIRE(after != nullptr);
        CHECK(after->asInt() == 42);

        // Also verify calling through A still reads correctly
        WRValue* viaA = wr_callFunction(env.moduleA, "callGetB", nullptr, 0);
        REQUIRE(viaA != nullptr);
        CHECK(viaA->asInt() == 42);
    }

    TEST_CASE("Passing globals as function arguments works cross-module") {
        // The recommended pattern: pass globals as args, not relying on
        // cross-module global access
        TwoModuleEnv env;
        REQUIRE(env.setup(
            // Module A - has a global, passes it to B as an argument
            "var myData = 123;\n"
            "function passToB() { return processData(myData); }\n",

            // Module B - receives value as parameter, not as global
            "function processData(d) { return d * 2; }\n"
        ));

        WRValue* result = wr_callFunction(env.moduleA, "passToB", nullptr, 0);
        REQUIRE(result != nullptr);
        CHECK(result->asInt() == 246);
    }

    // =========================================================================
    // SECTION 4: Global strings across module boundaries
    // =========================================================================

    TEST_CASE("String globals are isolated between modules") {
        TwoModuleEnv env;
        REQUIRE(env.setup(
            "var name = \"moduleA\";\n"
            "function getNameA() { return name; }\n"
            "function setNameA(n) { name = n; }\n",

            "var name = \"moduleB\";\n"
            "function getNameB() { return name; }\n"
            "function setNameB(n) { name = n; }\n"
        ));

        // Read initial values
        WRValue* rA = wr_callFunction(env.moduleA, "getNameA", nullptr, 0);
        REQUIRE(rA != nullptr);
        CHECK(wrValueToString(rA) == "moduleA");

        WRValue* rB = wr_callFunction(env.moduleB, "getNameB", nullptr, 0);
        REQUIRE(rB != nullptr);
        CHECK(wrValueToString(rB) == "moduleB");

        // Modify A's string
        WRValue arg;
        wr_makeString(env.moduleA, &arg, "changed_A");
        wr_callFunction(env.moduleA, "setNameA", &arg, 1);

        // A changed, B unchanged
        WRValue* rA2 = wr_callFunction(env.moduleA, "getNameA", nullptr, 0);
        REQUIRE(rA2 != nullptr);
        CHECK(wrValueToString(rA2) == "changed_A");

        WRValue* rB2 = wr_callFunction(env.moduleB, "getNameB", nullptr, 0);
        REQUIRE(rB2 != nullptr);
        CHECK(wrValueToString(rB2) == "moduleB");
    }

    TEST_CASE("Cross-module function reads callee's string global, not caller's") {
        TwoModuleEnv env;
        REQUIRE(env.setup(
            // Module A has a string global, calls B to read "name"
            "var name = \"from_A\";\n"
            "function readNameViaB() { return readName(); }\n",

            // Module B has its own "name" global
            "var name = \"from_B\";\n"
            "function readName() { return name; }\n"
        ));

        // When A calls B's readName(), B should read B's global
        WRValue* result = wr_callFunction(env.moduleA, "readNameViaB", nullptr, 0);
        REQUIRE(result != nullptr);
        CHECK(wrValueToString(result) == "from_B");
    }

    // =========================================================================
    // SECTION 5: Globals persist across multiple call rounds
    // =========================================================================

    TEST_CASE("Globals persist across multiple C++ call rounds") {
        TwoModuleEnv env;
        REQUIRE(env.setup(
            "var accumulator = 0;\n"
            "function addToAcc(v) { accumulator += v; return accumulator; }\n"
            "function getAcc() { return accumulator; }\n",

            "function dummy() { return 0; }\n"
        ));

        // Call addToAcc multiple times from C++
        for (int i = 1; i <= 10; i++) {
            WRValue arg;
            wr_makeInt(&arg, i);
            wr_callFunction(env.moduleA, "addToAcc", &arg, 1);
        }

        WRValue* result = wr_callFunction(env.moduleA, "getAcc", nullptr, 0);
        REQUIRE(result != nullptr);
        CHECK(result->asInt() == 55); // sum(1..10)
    }

    TEST_CASE("Globals persist across cross-module call rounds") {
        TwoModuleEnv env;
        REQUIRE(env.setup(
            // Module A calls B's increment function repeatedly
            "function callIncB() { return incB(); }\n"
            "function callGetB() { return getB(); }\n",

            // Module B has a counter
            "var counter = 0;\n"
            "function incB() { counter += 1; return counter; }\n"
            "function getB() { return counter; }\n"
        ));

        // Call A->B increment 5 times
        for (int i = 0; i < 5; i++) {
            WRValue* r = wr_callFunction(env.moduleA, "callIncB", nullptr, 0);
            REQUIRE(r != nullptr);
            CHECK(r->asInt() == i + 1);
        }

        // Final check through both paths
        WRValue* directB = wr_callFunction(env.moduleB, "getB", nullptr, 0);
        REQUIRE(directB != nullptr);
        CHECK(directB->asInt() == 5);

        WRValue* viaA = wr_callFunction(env.moduleA, "callGetB", nullptr, 0);
        REQUIRE(viaA != nullptr);
        CHECK(viaA->asInt() == 5);
    }

    // =========================================================================
    // SECTION 6: Three-module chains (A -> B -> C)
    // =========================================================================

    TEST_CASE("Three-module chain: direct reads from each module") {
        MultiModuleEnv env;
        REQUIRE(env.setup({
            // Module 0
            "var id = \"A\";\n"
            "function getIdA() { return id; }\n",

            // Module 1
            "var id = \"B\";\n"
            "function getIdB() { return id; }\n",

            // Module 2
            "var id = \"C\";\n"
            "function getIdC() { return id; }\n",
        }));

        // Direct reads - each module reads its own global
        WRValue* rA = wr_callFunction(env.modules[0], "getIdA", nullptr, 0);
        REQUIRE(rA != nullptr);
        CHECK(wrValueToString(rA) == "A");

        WRValue* rB = wr_callFunction(env.modules[1], "getIdB", nullptr, 0);
        REQUIRE(rB != nullptr);
        CHECK(wrValueToString(rB) == "B");

        WRValue* rC = wr_callFunction(env.modules[2], "getIdC", nullptr, 0);
        REQUIRE(rC != nullptr);
        CHECK(wrValueToString(rC) == "C");
    }

    TEST_CASE("Three-module chain: A->B cross-module string return") {
        MultiModuleEnv env;
        REQUIRE(env.setup({
            // Module 0
            "function chainToB() { return getIdB(); }\n",

            // Module 1
            "var id = \"B\";\n"
            "function getIdB() { return id; }\n",

            // Module 2 (unused but present)
            "function dummy() { return 0; }\n",
        }));

        // Chain: A -> B (cross-module string return)
        WRValue* chainAB = wr_callFunction(env.modules[0], "chainToB", nullptr, 0);
        REQUIRE(chainAB != nullptr);
        CHECK(wrValueToString(chainAB) == "B");
    }

    TEST_CASE("Three-module chain: B->C cross-module string return") {
        MultiModuleEnv env;
        REQUIRE(env.setup({
            // Module 0 (unused but present)
            "function dummy() { return 0; }\n",

            // Module 1
            "function chainToC() { return getIdC(); }\n",

            // Module 2
            "var id = \"C\";\n"
            "function getIdC() { return id; }\n",
        }));

        // Chain: B -> C (cross-module string return)
        WRValue* chainBC = wr_callFunction(env.modules[1], "chainToC", nullptr, 0);
        REQUIRE(chainBC != nullptr);
        CHECK(wrValueToString(chainBC) == "C");
    }

    TEST_CASE("Three-module: sequential A->B calls (same direction, no crash)") {
        MultiModuleEnv env;
        REQUIRE(env.setup({
            "function chainToB() { return getIdB(); }\n",
            "var id = \"B\";\n"
            "function getIdB() { return id; }\n",
            "function dummy() { return 0; }\n",
        }));

        // Two sequential A->B calls from C++
        WRValue* r1 = wr_callFunction(env.modules[0], "chainToB", nullptr, 0);
        REQUIRE(r1 != nullptr);
        CHECK(wrValueToString(r1) == "B");

        WRValue* r2 = wr_callFunction(env.modules[0], "chainToB", nullptr, 0);
        REQUIRE(r2 != nullptr);
        CHECK(wrValueToString(r2) == "B");
    }

    TEST_CASE("Three-module: sequential calls to DIFFERENT cross-module paths") {
        // Tests sequential cross-module calls from different source contexts:
        // C++ calls A->B (string return), then C++ calls B->C (int return).
        //
        // This previously crashed (SIGSEGV) or returned null due to stackOffset
        // corruption. Fixed by using absolute stack offset and save/restore.
        MultiModuleEnv env;
        REQUIRE(env.setup({
            "function chainToB() { return getIdB(); }\n",
            "var id = \"B\";\n"
            "function getIdB() { return id; }\n"
            "function chainToC() { return getValC(); }\n",
            "var valC = 99;\n"
            "function getValC() { return valC; }\n",
        }));

        // First: A -> B (returns string)
        WRValue* chainAB = wr_callFunction(env.modules[0], "chainToB", nullptr, 0);
        REQUIRE(chainAB != nullptr);
        CHECK(wrValueToString(chainAB) == "B");

        // Then: B -> C (returns int) - works after stackOffset fix
        WRValue* chainBC = wr_callFunction(env.modules[1], "chainToC", nullptr, 0);
        REQUIRE(chainBC != nullptr);
        CHECK(chainBC->asInt() == 99);
    }

    TEST_CASE("Three-module chain: deep A->B->C with int return") {
        // Tests nested cross-module call chain: A calls B, B calls C.
        // Previously crashed (SIGSEGV) due to stackOffset corruption.
        // Fixed by using absolute stack offset and save/restore.
        MultiModuleEnv env;
        REQUIRE(env.setup({
            "function deepChain() { return callB(); }\n",
            "function callB() { return callC(); }\n",
            "var val = 42;\n"
            "function callC() { return val; }\n",
        }));

        WRValue* result = wr_callFunction(env.modules[0], "deepChain", nullptr, 0);
        if (result != nullptr) {
            CHECK(result->asInt() == 42);
        } else {
            MESSAGE("Deep A->B->C chain with int return returned null");
            CHECK(false);
        }
    }

    TEST_CASE("Three-module: A->B then B->C sequential string returns") {
        // Tests sequential cross-module calls with string returns from
        // different source contexts. Previously crashed with SIGSEGV.
        // Fixed by using absolute stack offset and save/restore.
        MultiModuleEnv env;
        REQUIRE(env.setup({
            "function chainToB() { return getIdB(); }\n",
            "var id = \"B\";\n"
            "function getIdB() { return id; }\n"
            "function chainToC() { return getIdC(); }\n",
            "var id = \"C\";\n"
            "function getIdC() { return id; }\n",
        }));

        // First call: A->B string return (works)
        WRValue* chainAB = wr_callFunction(env.modules[0], "chainToB", nullptr, 0);
        REQUIRE(chainAB != nullptr);
        CHECK(wrValueToString(chainAB) == "B");

        // Second call: B->C string return (SIGSEGV here)
        WRValue* chainBC = wr_callFunction(env.modules[1], "chainToC", nullptr, 0);
        REQUIRE(chainBC != nullptr);
        CHECK(wrValueToString(chainBC) == "C");
    }

    // =========================================================================
    // SECTION 7: Heavy cross-module call traffic (corruption stress test)
    // =========================================================================

    TEST_CASE("Moderate cross-module call traffic - globals remain correct") {
        TwoModuleEnv env;
        REQUIRE(env.setup(
            // Module A - calls B's function in a loop
            "var aGlobal = \"stable_A\";\n"
            "function stressTest() {\n"
            "    var i;\n"
            "    for (i = 0; i < 20; i += 1) {\n"
            "        incCounter();\n"
            "    }\n"
            "    return aGlobal;\n"
            "}\n",

            // Module B - simple counter
            "var counter = 0;\n"
            "var bGlobal = \"stable_B\";\n"
            "function incCounter() { counter += 1; }\n"
            "function getCounter() { return counter; }\n"
            "function getBGlobal() { return bGlobal; }\n"
        ));

        // Run the stress test - A calls B 20 times, then reads its own global
        WRValue* result = wr_callFunction(env.moduleA, "stressTest", nullptr, 0);
        REQUIRE(result != nullptr);
        CHECK(wrValueToString(result) == "stable_A");

        // B's counter should have incremented correctly
        WRValue* counter = wr_callFunction(env.moduleB, "getCounter", nullptr, 0);
        REQUIRE(counter != nullptr);
        CHECK(counter->asInt() == 20);

        // B's string global should be intact
        WRValue* bStr = wr_callFunction(env.moduleB, "getBGlobal", nullptr, 0);
        REQUIRE(bStr != nullptr);
        CHECK(wrValueToString(bStr) == "stable_B");
    }

    TEST_CASE("Heavy cross-module call traffic - repeated C++ driven rounds") {
        // This test simulates the pattern that triggers corruption on hardware:
        // repeated C++ -> Module A -> Module B call chains
        TwoModuleEnv env;
        REQUIRE(env.setup(
            // Module A - has a global string, calls into B
            "var hidString = \"usb_host:TestDevice\";\n"
            "function processEvent(val) {\n"
            "    updateState(val);\n"
            "    return hidString;\n"
            "}\n",

            // Module B - state tracking
            "var state = 0;\n"
            "function updateState(v) { state = v; }\n"
            "function getState() { return state; }\n"
        ));

        // Simulate 100 event processing cycles (like MIDI CC events)
        bool allCorrect = true;
        std::string firstFailure;
        int failureIteration = -1;

        for (int i = 0; i < 100; i++) {
            WRValue arg;
            wr_makeInt(&arg, i);

            WRValue* result = wr_callFunction(env.moduleA, "processEvent", &arg, 1);
            if (result == nullptr) {
                allCorrect = false;
                firstFailure = "null result";
                failureIteration = i;
                break;
            }

            std::string str = wrValueToString(result);
            if (str != "usb_host:TestDevice") {
                allCorrect = false;
                firstFailure = str.empty() ? "(empty)" : str;
                failureIteration = i;
                break;
            }
        }

        if (!allCorrect) {
            MESSAGE("Global corruption detected at iteration " << failureIteration
                    << ": expected 'usb_host:TestDevice', got '" << firstFailure << "'");
        }
        CHECK(allCorrect);

        // Verify B's state is correct
        WRValue* state = wr_callFunction(env.moduleB, "getState", nullptr, 0);
        REQUIRE(state != nullptr);
        CHECK(state->asInt() == 99);
    }

    TEST_CASE("Heavy traffic with multiple modules - simulates real workload") {
        // Simulates the real Midian pattern:
        // C++ calls user script -> library function -> pickup functions
        // across 3+ modules with many iterations
        MultiModuleEnv env;
        REQUIRE(env.setup({
            // Module 0 - "user script" - calls library functions
            "var deviceHID = \"usb_host:Arturia\";\n"
            "function handleCC(cc, val) {\n"
            "    var result;\n"
            "    result = processCC(cc, val);\n"
            "    return deviceHID;\n"
            "}\n",

            // Module 1 - "library" - calls pickup module
            "var libState = \"library_ok\";\n"
            "function processCC(cc, val) {\n"
            "    var i;\n"
            "    for (i = 0; i < 3; i += 1) {\n"
            "        checkPickup(cc + i, val);\n"
            "    }\n"
            "    return libState;\n"
            "}\n",

            // Module 2 - "pickup" - leaf module with state
            "var pickupCount = 0;\n"
            "function checkPickup(cc, val) {\n"
            "    pickupCount += 1;\n"
            "}\n"
            "function getPickupCount() { return pickupCount; }\n",
        }));

        bool allCorrect = true;
        std::string firstFailure;
        int failureIteration = -1;

        // 50 rounds of: C++ -> user -> library -> pickup (3 times per round)
        for (int i = 0; i < 50; i++) {
            WRValue args[2];
            wr_makeInt(&args[0], i);      // cc number
            wr_makeInt(&args[1], i * 2);   // value

            WRValue* result = wr_callFunction(env.modules[0], "handleCC", args, 2);
            if (result == nullptr) {
                allCorrect = false;
                firstFailure = "null result";
                failureIteration = i;
                break;
            }

            std::string str = wrValueToString(result);
            if (str != "usb_host:Arturia") {
                allCorrect = false;
                firstFailure = str.empty() ? "(empty)" : str;
                failureIteration = i;
                break;
            }
        }

        if (!allCorrect) {
            MESSAGE("Global corruption at iteration " << failureIteration
                    << ": expected 'usb_host:Arturia', got '" << firstFailure << "'");
        }
        CHECK(allCorrect);

        // Verify pickup count: 50 rounds * 3 calls per round = 150
        WRValue* pCount = wr_callFunction(env.modules[2], "getPickupCount", nullptr, 0);
        REQUIRE(pCount != nullptr);
        CHECK(pCount->asInt() == 150);
    }

    TEST_CASE("Extreme traffic: 72+ iterations in loop (reproduces hardware pattern)") {
        // This is the specific pattern from MEMORY.md that triggers corruption
        // on hardware: user script -> library function -> 72 iterations
        // calling pickup:: functions
        MultiModuleEnv env;
        REQUIRE(env.setup({
            // Module 0 - user entry point
            "var hid = \"usb_host:KeyStep37\";\n"
            "function onCC(cc, val, ch) {\n"
            "    runPickupLoop(cc, val);\n"
            "    return hid;\n"
            "}\n",

            // Module 1 - library that loops heavily
            "var libName = \"pickup_lib\";\n"
            "function runPickupLoop(cc, val) {\n"
            "    var i;\n"
            "    for (i = 0; i < 72; i += 1) {\n"
            "        doPickup(i, val);\n"
            "    }\n"
            "    return libName;\n"
            "}\n",

            // Module 2 - pickup operations
            "var totalOps = 0;\n"
            "function doPickup(idx, val) {\n"
            "    totalOps += 1;\n"
            "}\n"
            "function getTotalOps() { return totalOps; }\n",
        }));

        // Run multiple rounds of the 72-iteration pattern
        bool allCorrect = true;
        std::string firstFailure;
        int failureRound = -1;

        for (int round = 0; round < 20; round++) {
            WRValue args[3];
            wr_makeInt(&args[0], round);      // cc
            wr_makeInt(&args[1], round * 2);   // val
            wr_makeInt(&args[2], 1);           // channel

            WRValue* result = wr_callFunction(env.modules[0], "onCC", args, 3);

            if (result == nullptr) {
                allCorrect = false;
                firstFailure = "null result";
                failureRound = round;
                break;
            }

            std::string got = wrValueToString(result);
            if (got != "usb_host:KeyStep37") {
                allCorrect = false;
                firstFailure = got.empty() ? "(empty)" : got;
                failureRound = round;
                break;
            }

            // Also check library's global is intact
            WRValue* libResult = wr_callFunction(env.modules[1], "runPickupLoop", args, 2);
            if (libResult != nullptr) {
                std::string libStr = wrValueToString(libResult);
                if (!libStr.empty() && libStr != "pickup_lib") {
                    allCorrect = false;
                    firstFailure = "lib global corrupted: " + libStr;
                    failureRound = round;
                    break;
                }
            }
        }

        if (!allCorrect) {
            MESSAGE("Corruption at round " << failureRound
                    << ": got '" << firstFailure << "'");
        }
        CHECK(allCorrect);

        // Verify total pickup operations: 20 rounds * 2 calls * 72 iterations = 2880
        WRValue* total = wr_callFunction(env.modules[2], "getTotalOps", nullptr, 0);
        REQUIRE(total != nullptr);
        CHECK(total->asInt() == 2880);
    }

    // =========================================================================
    // SECTION 8: Global arrays across modules
    // =========================================================================

    TEST_CASE("Global arrays are per-context") {
        TwoModuleEnv env;
        REQUIRE(env.setup(
            "var arr[] = { 10, 20, 30 };\n"
            "function getArrA(i) { return arr[i]; }\n"
            "function setArrA(i, v) { arr[i] = v; }\n",

            "var arr[] = { 100, 200, 300 };\n"
            "function getArrB(i) { return arr[i]; }\n"
        ));

        // Read from each module's array
        WRValue idx;
        wr_makeInt(&idx, 1);

        WRValue* rA = wr_callFunction(env.moduleA, "getArrA", &idx, 1);
        REQUIRE(rA != nullptr);
        CHECK(rA->asInt() == 20);

        WRValue* rB = wr_callFunction(env.moduleB, "getArrB", &idx, 1);
        REQUIRE(rB != nullptr);
        CHECK(rB->asInt() == 200);

        // Modify A's array
        WRValue setArgs[2];
        wr_makeInt(&setArgs[0], 1);
        wr_makeInt(&setArgs[1], 999);
        wr_callFunction(env.moduleA, "setArrA", setArgs, 2);

        // A changed, B unchanged
        WRValue* rA2 = wr_callFunction(env.moduleA, "getArrA", &idx, 1);
        REQUIRE(rA2 != nullptr);
        CHECK(rA2->asInt() == 999);

        WRValue* rB2 = wr_callFunction(env.moduleB, "getArrB", &idx, 1);
        REQUIRE(rB2 != nullptr);
        CHECK(rB2->asInt() == 200);
    }

    TEST_CASE("Cross-module accessor functions for arrays (recommended pattern)") {
        // This is the recommended workaround for the compile error:
        // Instead of Module B indexing Module A's array directly,
        // Module A exposes accessor functions
        TwoModuleEnv env;
        REQUIRE(env.setup(
            // Module A owns the array
            "var data[] = { 1, 2, 3, 4, 5 };\n"
            "function getData(i) { return data[i]; }\n"
            "function setData(i, v) { data[i] = v; }\n"
            "function getDataLen() { return 5; }\n",

            // Module B uses accessor functions (cross-module calls)
            "function sumAllData() {\n"
            "    var total = 0;\n"
            "    var len = getDataLen();\n"
            "    var i;\n"
            "    for (i = 0; i < len; i += 1) {\n"
            "        total += getData(i);\n"
            "    }\n"
            "    return total;\n"
            "}\n"
            "function doubleData(i) {\n"
            "    var val = getData(i);\n"
            "    setData(i, val * 2);\n"
            "}\n"
        ));

        // B can read A's array data via functions
        WRValue* sum = wr_callFunction(env.moduleB, "sumAllData", nullptr, 0);
        REQUIRE(sum != nullptr);
        CHECK(sum->asInt() == 15); // 1+2+3+4+5

        // B can modify A's array via accessor functions
        WRValue idx;
        wr_makeInt(&idx, 2); // double data[2] (value 3 -> 6)
        wr_callFunction(env.moduleB, "doubleData", &idx, 1);

        // Verify change
        WRValue* newSum = wr_callFunction(env.moduleB, "sumAllData", nullptr, 0);
        REQUIRE(newSum != nullptr);
        CHECK(newSum->asInt() == 18); // 1+2+6+4+5
    }

    // =========================================================================
    // SECTION 9: Library function (C++ callback) interaction with globals
    // =========================================================================

    TEST_CASE("C++ library function coexists with per-module globals") {
        // Register a C++ function, verify it doesn't interfere with
        // module globals
        WRState* state = wr_newState();
        REQUIRE(state != nullptr);

        // Register a C++ library function
        static int callCount = 0;
        callCount = 0;
        wr_registerLibraryFunction(state, "lib::track", [](WRValue* stackTop,
            const int argn, WRContext* c) {
            (void)argn;
            (void)c;
            callCount++;
            wr_makeInt(stackTop, callCount);
        });

        // Bootstrap
        unsigned char* bootBytecode = nullptr;
        int bootLen = 0;
        REQUIRE(compileSource("", &bootBytecode, &bootLen));
        WRContext* mainCtx = wr_run(state, bootBytecode, bootLen);
        REQUIRE(mainCtx != nullptr);

        // Module with a global and calls to the library function
        const char* src =
            "var myGlobal = \"test_value\";\n"
            "function callLib() {\n"
            "    var result;\n"
            "    result = lib::track();\n"
            "    return myGlobal;\n"
            "}\n"
            "function getGlobal() { return myGlobal; }\n";

        unsigned char* modBytecode = nullptr;
        int modLen = 0;
        REQUIRE(compileSource(src, &modBytecode, &modLen));
        WRContext* modCtx = wr_import(mainCtx, modBytecode, modLen);
        REQUIRE(modCtx != nullptr);

        // Call the function that uses both library call and global
        for (int i = 0; i < 10; i++) {
            WRValue* result = wr_callFunction(modCtx, "callLib", nullptr, 0);
            REQUIRE(result != nullptr);
            CHECK(wrValueToString(result) == "test_value");
        }

        CHECK(callCount == 10);

        wr_destroyState(state);
    }

    // =========================================================================
    // SECTION 10: Global mutation from different entry points
    // =========================================================================

    TEST_CASE("Global mutation is visible regardless of which context calls") {
        // When C++ calls a function in context A vs context B to invoke the
        // same target function, the target's globals should reflect correctly
        TwoModuleEnv env;
        REQUIRE(env.setup(
            // Module A - two entry points that call B
            "function routeToB_add() { return addB(10); }\n"
            "function routeToB_get() { return getB(); }\n",

            // Module B - has mutable state
            "var value = 0;\n"
            "function addB(n) { value += n; return value; }\n"
            "function getB() { return value; }\n"
        ));

        // Call from A to add
        WRValue* r1 = wr_callFunction(env.moduleA, "routeToB_add", nullptr, 0);
        REQUIRE(r1 != nullptr);
        CHECK(r1->asInt() == 10);

        // Call directly from B
        WRValue arg;
        wr_makeInt(&arg, 5);
        WRValue* r2 = wr_callFunction(env.moduleB, "addB", &arg, 1);
        REQUIRE(r2 != nullptr);
        CHECK(r2->asInt() == 15);

        // Read back through A
        WRValue* r3 = wr_callFunction(env.moduleA, "routeToB_get", nullptr, 0);
        REQUIRE(r3 != nullptr);
        CHECK(r3->asInt() == 15);

        // Read back directly from B
        WRValue* r4 = wr_callFunction(env.moduleB, "getB", nullptr, 0);
        REQUIRE(r4 != nullptr);
        CHECK(r4->asInt() == 15);
    }

    // =========================================================================
    // SECTION 11: Global type integrity
    // =========================================================================

    TEST_CASE("Global type changes don't leak across modules") {
        TwoModuleEnv env;
        REQUIRE(env.setup(
            // Module A - changes global type from int to string
            "var flexible = 42;\n"
            "function makeStringA() { flexible = \"now_a_string\"; return flexible; }\n"
            "function getFlexA() { return flexible; }\n",

            // Module B - has same-named global, stays as int
            "var flexible = 99;\n"
            "function getFlexB() { return flexible; }\n"
        ));

        // Initially both are ints
        WRValue* rA = wr_callFunction(env.moduleA, "getFlexA", nullptr, 0);
        REQUIRE(rA != nullptr);
        CHECK(rA->asInt() == 42);

        WRValue* rB = wr_callFunction(env.moduleB, "getFlexB", nullptr, 0);
        REQUIRE(rB != nullptr);
        CHECK(rB->asInt() == 99);

        // Change A's global to a string
        WRValue* rA2 = wr_callFunction(env.moduleA, "makeStringA", nullptr, 0);
        REQUIRE(rA2 != nullptr);
        CHECK(wrValueToString(rA2) == "now_a_string");

        // B's global should still be int 99
        WRValue* rB2 = wr_callFunction(env.moduleB, "getFlexB", nullptr, 0);
        REQUIRE(rB2 != nullptr);
        CHECK(rB2->asInt() == 99);
    }
}
