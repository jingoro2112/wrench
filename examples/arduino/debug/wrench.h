#define WRENCH_COMBINED
#ifndef _WRENCH_H
#define _WRENCH_H
/*******************************************************************************
Copyright (c) 2026 Curt Hartung -- curt.hartung@gmail.com

MIT Licence

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*******************************************************************************/

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define WRENCH_VERSION_MAJOR 7
#define WRENCH_VERSION_MINOR 0
#define WRENCH_VERSION_BUILD 2

struct WRState;

/************************************************************************
The compiler is not particularly memory or space efficient, for
small embedded systems it is strongly reccommeded (nay, required) that
only bytecode be executed. This flag explicitly removes the compiler
(although just not calling it should allow a smart linker to remove it
anyway)
*/
//#define WRENCH_WITHOUT_COMPILER


/***********************************************************************
Cause the interpreter to compile into the smallest program size
possible at the cost of some speed. This loss is from the removal of
unrolled loops and inlined functionality, also making use of some goto.

WRENCH_REALLY_COMPACT reduces size further by removing the jumptable
interpreter in favor of a giant switch(). This savings comes at the cost
of more speed so only use it if you need to.
*/
//#define WRENCH_COMPACT           // saves a lot, costs some speed
//#define WRENCH_REALLY_COMPACT    // saves a little more, costs more speed


/***********************************************************************
Some architectures (mostly embedded) really don't like reading more than
8 bits on an unaligned memory location and will bus fault! If your architecture
allows it (x86/64, Mac, most *nix etc..) this is a decent optimization
*/
//#define WRENCH_UNALIGNED_READS


/***********************************************************************
wrench automatically detects endian-ness and defines these three
macros for reading data from the code stream, if you have special
memory requirements, as some embedded systems do, you may re-define
them here, but they MUST match the endian-ness of the target
architecture, see vm.h for the current definitions
*/
//#define READ_32_FROM_PC( P ) 
//#define READ_16_FROM_PC( P )
//#define READ_8_FROM_PC( P )


/***********************************************************************
 **** EXPERIMENTAL **** only very basic support is in place */
//#define WRENCH_INCLUDE_DEBUG_CODE

// the debugger works over a serial link, define the architecture this
// side is being targeted for
//#define WRENCH_WIN32_SERIAL
//#define WRENCH_ARDUINO_SERIAL
//#define WRENCH_LINUX_SERIAL

/***********************************************************************/


/************************************************************************
The stack is statically allocated but only used for function
calls and locals, NOT structs/arrays/hashes which are malloc'ed. Unless your
applications uses lots of recursion+local variables, a modest size should
be more than enough.
To really reduce RAM footprint this can be lowered considerably
depending on usage. (consumes 8 bytes per stack entry)
*/
#ifndef WRENCH_DEFAULT_STACK_SIZE
#define WRENCH_DEFAULT_STACK_SIZE 64
#endif
// this costs a small bit of overhead whenever the stack is used, for
// most applications it is not necessary, but will protect against
// things like infinite recursion
//#define WRENCH_PROTECT_STACK_FROM_OVERFLOW


/************************************************************************
With this defined the VM will enforce a maximum number of instructions
before a yield is forced, to prevent infinite loops, this adds a small check to each
instruction!
*/
//#define WRENCH_TIME_SLICES

#ifdef WRENCH_TIME_SLICES
// how many instructions each call to the VM executes before yielding,
// set to '0' for unlimited (default)
void wr_setInstructionsPerSlice( WRState* w, const int instructions );
void wr_forceYield( WRState* w );  // for the VM to yield right NOW, (called from a different thread)
int wr_slicesUsedLastCall( WRState* w );  // how many time slices did the last call to the VM use?
#endif

/************************************************************************
if you WANT full sprintf support for floats (%f/%g) this adds it at
the cost of using the standard c library for it (stdlib.h), which can incur a
decent code-size penalty. Without this %f/%g is ignored.
*/
//#define WRENCH_FLOAT_SPRINTF


/************************************************************************
File operations: define ONE of these. If you use "Custom" then you
need to link in a source file with the functions defined in:

  /discrete_src/std_io_defs.h

examples are in

  /discrete_src/std_io_linux.cpp
  /discrete_src/std_io_win32.cpp
  /discrete_src/std_io_spiffs.cpp
  /discrete_src/std_io_littlefs.cpp

*/
//#define WRENCH_WIN32_FILE_IO
//#define WRENCH_LINUX_FILE_IO
//#define WRENCH_SPIFFS_FILE_IO
//#define WRENCH_LITTLEFS_FILE_IO


/************************************************************************
basic TCP/IP socket operations, define the architecture you are
compiling for here, then see wr_loadTCPLib(...)
*/
//#define WRENCH_WIN32_TCP
//#define WRENCH_LINUX_TCP


/************************************************************************
Wrench protects itself against divide-by-zero, but allows the result to
be '0'. Trapping the error requires a tiny bit of overhead(ie- every
result has to be checked to see if it was a divide by zero) so by
default wrench does not bother in favor of speed.
Define this to received the fatal error:  WR_ERR_division_by_zero
*/
//#define WRENCH_TRAP_DIVISION_BY_ZERO

/************************************************************************
for systems that need to know if they have run out of memory.

WARNING: This imposes a small if() check on EVERY INSTRUCTION so the
malloc failure is detected on the instruction it happens and guarantees
graceful exit
*/
//#define WRENCH_HANDLE_MALLOC_FAIL

/************************************************************************
Custom allocator:
by default wrench uses malloc/free but if you want to use your own
allocator it can be set up here
NOTE: used for all RETURNED MEMORY AS WELL, such as asMallocString(...)!!!!
*/
typedef void* (*WR_ALLOC)(size_t size);
typedef void (*WR_FREE)(void* ptr);
void wr_setGlobalAllocator( WR_ALLOC wralloc, WR_FREE wrfree );

//------------------------------------------------------------------------------

struct WRValue;
struct WRFunction;
struct WRContext;
class WRstr;

//------------------------------------------------------------------------------
// to minimize text segment size, only a minimum of strings
// are embedded in the code. error messages are very verbose enums
// also provided as an array of strings which are _not_ referenced
// by the VM or Compiler, so normally will not be linked in.
extern const char* c_errStrings[];
enum WRError
{
	WR_ERR_None = 0,

	WR_ERR_compiler_not_loaded,
	WR_ERR_function_not_found,
	WR_ERR_lib_function_not_found,
	WR_ERR_hash_not_found,
	WR_ERR_library_constant_not_loaded,
	WR_ERR_unknown_opcode,
	WR_ERR_unexpected_EOF,
	WR_ERR_unexpected_token,
	WR_ERR_bad_expression,
	WR_ERR_bad_label,
	WR_ERR_statement_expected,
	WR_ERR_unterminated_string_literal,
	WR_ERR_newline_in_string_literal,
	WR_ERR_bad_string_escape_sequence,
	WR_ERR_tried_to_load_non_resolvable,
	WR_ERR_break_keyword_not_in_looping_structure,
	WR_ERR_continue_keyword_not_in_looping_structure,
	WR_ERR_expected_while,
	WR_ERR_compiler_panic,
	WR_ERR_constant_redefined,
	WR_ERR_struct_in_struct,
	WR_ERR_var_not_seen_before_label,
	WR_ERR_unexpected_export_keyword,
	WR_ERR_new_assign_by_label_or_offset_not_both,
	WR_ERR_struct_not_exported,
	WR_ERR_empty_parens,

	WR_ERR_run_must_be_called_by_itself_first,
	WR_ERR_hash_table_size_exceeded,
	WR_ERR_hash_table_invalid_key,
	WR_ERR_wrench_function_not_found,
	WR_ERR_array_must_be_indexed,
	WR_ERR_scontext_not_found,
	WR_ERR_context_not_yielded,
	WR_ERR_cannot_call_function_context_yielded,

	WR_ERR_hash_declaration_in_array,
	WR_ERR_array_declaration_in_hash,

	WR_ERR_stack_overflow,

	WR_ERR_bad_goto_label,
	WR_ERR_bad_goto_location,
	WR_ERR_goto_target_not_found,

	WR_ERR_switch_with_no_cases,
	WR_ERR_switch_case_or_default_expected,
	WR_ERR_switch_construction_error,
	WR_ERR_switch_bad_case_hash,
	WR_ERR_switch_duplicate_case,

	WR_ERR_bad_bytecode_CRC,

	WR_ERR_execute_function_zero_called_more_than_once,

	WR_ERR_malloc_failed,

	WR_ERR_USER_err_out_of_range,

	WR_ERR_division_by_zero,
	
	WR_warning_enums_follow,

	WR_USER = 100, // user-defined below here

	
	WR_ERR_LAST = 255,
};

/***************************************************************/
/***************************************************************/
//                       State Management

// create/destroy a WRState object that can run multiple contexts/threads
WRState* wr_newState( int stackSize =WRENCH_DEFAULT_STACK_SIZE );
void wr_destroyState( WRState* w );

// allocate/free memory with the same allocator wrench is using, this
// is particularly important for the "takeOwnership" flag below
void* wr_malloc( size_t size );
void wr_free( void* ptr );

// hashing function used inside wrench, it's a stripped down murmer,
// not technically the "best" but very good, very fast and very compact
uint32_t wr_hash( const void* dat, const int len, uint32_t serial=0 );
uint32_t wr_hashStr( const char* dat, uint32_t serial=0 );

// set a context void* that all function callbacks will have access
// to through context->w->ctx
//         or context->ctxLocal;
inline void wr_setStateContext( WRState* w, void* ctx );
inline void wr_setLocalContext( WRContext* c, void* ctx );
inline void* wr_getStateContext( WRState* w );
inline void* wr_getLocalContext( WRContext* c );

/***************************************************************/
/**************************************************************/
//                        Running Code

// compile source code and return a wr_malloc'ed block of bytecode ready
// for wr_run(), this memory must be manually wr_free'ed
// return value is a WRError
// optionally an "errMsg" buffer can be passed which will output a
//     human-readable string of what went wrong and where.
// compilerOptionFlags : OR'ed mask of of WrenchCompilerOptionFlags:
enum WrenchCompilerFlags
{
	WR_INCLUDE_GLOBALS   = 1<<0, // include all globals NOTE: wr_getGlobalRef(...)
								 // will not function without this
	
	WR_EMBED_DEBUG_CODE  = 1<<1, // include per-instruction NOTE: WrenchDebugInterface
								 // will not function without this
	
	WR_EMBED_SOURCE_CODE = 1<<2, // include a copy of the source code
	
	WR_NON_STRICT_VAR    = 1<<3, // require 'var' to declare a variable (disabled by default)
};

WRError wr_compile( const char* source,
					const int size,
					unsigned char** out,
					int* outLen,
					WRstr* errMsg =0,
					const uint8_t compilerOptionFlags = WR_INCLUDE_GLOBALS );

// disassemble the bytecode and output human readable
void wr_disassemble( const uint8_t* bytecode, const unsigned int len, WRstr& out, const bool includeComments =true );
void wr_disassemble( const uint8_t* bytecode, const unsigned int len, char** out, unsigned int* outLen =0 );

// w:          state (see wr_newState)
// block:      location of bytecode
// blockSize:  number of bytes in the block
// destroyContext: automatically destroy the resulting context, by
//                 default wr_run will return the allocated context
//                 this context can be used for callbacks or destroyed
//                 with wr_destroyContext(), but will be garbage
//                 collected automatically when wr_destroyState()
//                 is called on w
// takeOwnership: assume 'block' was allocated with wr_malloc(), it
//                will be freed with  wr_free()
WRContext* wr_run( WRState* w,
				   const unsigned char* block,
				   const int blockSize,
				   const bool takeOwnershipOfBlock =false,
				   const bool destroyContext =false );

// macro for automatically freeing the WRContext
#define wr_runOnce( w, block, blockSize ) wr_destroyContext( wr_run((w),(block),(blockSize)) )

// Run the source code as an atomic operation
bool wr_runCommand( WRState* w, const char* sourceCode, const int size =-1 );

// Add more code to the context, the new code cannot access the
// existing block, but all of its functions will become available to
// the parent.
// takeOwnership: assume 'block' was allocated with wr_malloc(), it
//                will be freed with  wr_free()
WRContext* wr_import( WRContext* context, const unsigned char* block, const int blockSize, bool takeOwnership =false );

// After wr_run(...) or wr_callFunction(...) has run, there is always a
// return value (default 0) this function fetches it
WRValue* wr_returnValueFromLastCall( WRContext* context );

// wr_run actually calls these two functions back-to back. If you want
// to separate the process you can call them on your own.
// takeOwnership: assume 'block' was allocated with wr_malloc(), it
//                will be freed with  wr_free()
WRContext* wr_newContext( WRState* w, const unsigned char* block, const int blockSize, bool takeOwnership =false );
WRValue* wr_executeContext( WRContext* context );

// after wr_run() this allows any function in the script to be
// called with the given arguments, returning a single value
//
// context:      the context in which the function was loaded
// functionName: plaintext name of the function to be called
// argv:         array of WRValues to call the function with (optional)
// argn:         how many arguments argv contains (optional, but if
//               zero then argv will be ignored)
// RETURNS:      WRValue returned by the function/program, or NULL for
//               error (see w->err for the code)

// If the context is yielded, callFunction(...) will always continue
// where the yielded function left off and ignore any parameters

// exactly the same as the above but the hash is supplied directly to
// save compute time, use wr_hashStr(...) to obtain the hash of the
// functionName
// NOTE: You CAN call library functions through this interface as well!
       WRValue* wr_callFunction( WRContext* context, const int32_t hash, const WRValue* argv =0, const int argn =0 );
inline WRValue* wr_callFunction( WRContext* context, const char* functionName, const WRValue* argv =0, const int argn =0 ) { return wr_callFunction( context, wr_hashStr(functionName), argv, argn ); }

// The raw function pointer can be pre-loaded with wr_getFunction() and
// and then called with an absolute minimum of overhead
WRValue* wr_callFunction( WRContext* context, WRFunction* function, const WRValue* argv =0, const int argn =0 );

// once wr_run() is called the returned context object can be used to
// pre-fetch a function pointer. This reduces the overhead of calling
// that function to almost nothing.
// THIS DOES NOT WORK FOR LIBRARY FUNCTIONS.
       WRFunction* wr_getFunction( WRContext* context, const uint32_t functionHash );
inline WRFunction* wr_getFunction( WRContext* context, const char* functionName ) { return wr_getFunction(context, wr_hashStr(functionName)); }

// Continue a yielded context.
// Return value semantics:
//   non-null : script reached return/stop; pointer is the script return value
//   null     : script yielded again (check wr_getYieldInfo()), or error
// If context is NOT yielded this sets:
//   "WR_ERR_context_not_yielded"
WRValue* wr_continue( WRContext* context );

// If this context in in a yielded state, return true, otherwise false
// args : will be pointed to the first arg yield(...) pushed
// argn : number of args yield(...) pushed
// returnValue : pointer to the return value yield(...) will have (default 0)
// if the function was forcibly yielded there will be no args and the
// return value will be ignored
bool wr_getYieldInfo( WRContext* context, WRValue** args =0, int* argnum =0, WRValue** returnValue =0);

// after wrench executes it may have set an error code, this is how to
// retrieve it. This system is coarse at the moment. Re-entering the
// interpreter clears the last error
WRError wr_getLastError( WRState* w );

// want direct access to a global? okay then, if the code was compiled
// with symbols (see compile(...) suite) then this will give you a
// reference to it
WRValue* wr_getGlobalRef( WRContext* context, const char* label );

// Destroy a context you no longer need and free up all the memory it
// was using
// NOTE: all contexts ARE AUTOMATICALLY FREED when wr_destroyState(...)
//       is called, it is NOT necessary to call this on each context
void wr_destroyContext( WRContext* context );

// how many bytes of memory must be allocated before the gc will run, default
// set here, can be adjusted at runtime with the
// wr_setAllocatedMemoryGCHint()
#define WRENCH_DEFAULT_ALLOCATED_MEMORY_GC_HINT 4000
void wr_setAllocatedMemoryGCHint( WRState* w, const uint32_t bytes );

/***************************************************************/
/***************************************************************/
//                 Callbacks from wrench                         

// register a function inside a state that can be called (by ALL
// contexts)
// callback will contain:
// c:             context called from
// argv:          pointer to arguments function was called
//                with (may be null!)
// argn:          how many arguments it was called with
// retVal:        this value will be passed back, default: integer zero
// usr:           opaque pointer function was registered with
typedef void (*WR_C_CALLBACK)(WRContext* c, const WRValue* argv, const int argn, WRValue& retVal, void* usr );

// IMPORTANT: The values passed may be references (keepin' it real) so
// always use the getters inside the WRValue class:

// int:      .asInt();
// float:    .asFloat();
// uint32_t: .asHash();
// binary:   .array( unsigned int* size, char* type );
// string:   .c_str( unsigned int* len );

// tests:
// .isFloat() fast check if this is a float
// .isInt() fast check if this is an int
// .isString() fast check if this is a string


// this will do its best to represent the value as a string to the
// supplied buffer, len is maximum size allowed
// string: .asString(char* string, size_t len )

// w:        state to register with (will be available to all contexts)
// name:     name of the function
// function: callback (see typedef above)
// usr:      opaque pointer that will be passed to the callback (optional)
void wr_registerFunction( WRState* w, const char* name, WR_C_CALLBACK function, void* usr =0 );

// serialize WRValues to and from binary
// NOTE: on success, wr_serialize returns a malloc()'d buffer that must be freed!
bool wr_serialize( char** buf, int* len, const WRValue& value ); 
bool wr_deserialize( WRContext* context, WRValue& value, const char* buf, const int len );

/***************************************************************/
/***************************************************************/
//                   wrench Library calls

// Wrench allows libraries to be loaded in the form of <name>::<func>
// these calls impose no overhead if not used

// library calls do assume a deep knowledge of the internal workings of
// wrench so the callback is an absolute minimum of info:
// stackTop: current top of the stack, arguments have been pushed here,
//           and any return value must be placed here, not pushed! see
//           one of the many library examples
// argn:     how many arguments this function was called with. no
//           guarantees are made about matching a function signature,
//           library calls validate if they care.
// stackTop         : [return value]
// stackTop - 1     : [argN ]
// stackTop - 2     : [argN-1]
// ...
// stackTop - (N-1) : [arg2]
// stackTop - N     : [arg1]
typedef void (*WR_LIB_CALLBACK)( WRValue* stackTop, const int argn, WRContext* context );

// w:         state to register with (will be available to all contexts)
// signature: library signature must be in the form of <lib>::<name>
//            ex: "math::cos"
// function:  callback (see typedef above)
void wr_registerLibraryFunction( WRState* w, const char* signature, WR_LIB_CALLBACK function );
void wr_registerLibraryConstant( WRState* w, const char* signature, const int32_t i );
void wr_registerLibraryConstant( WRState* w, const char* signature, const float f );

// you can write your own libraries (look at std.cpp for all the
// standard ones) but several have been provided:

void wr_loadAllLibs( WRState* w ); // load all libraries in one call

void wr_loadSysLib( WRState* w ); // system/internal functions
void wr_loadMathLib( WRState* w ); // provides most of the calls in math.h
void wr_loadStdLib( WRState* w ); // standard functions like sprintf/rand/
void wr_loadIOLib( WRState* w ); // IO funcs (time/file/io)
void wr_loadStringLib( WRState* w ); // string functions
void wr_loadMessageLib( WRState* w ); // messaging between contexts
void wr_loadSerializeLib( WRState* w ); // serialize WRValues to and from binary
void wr_loadDebugLib( WRState* w ); // debugger interaction functions
void wr_loadTCPLib( WRState* w ); // TCP/IP functions
void wr_loadContainerLib( WRState* w ); // array/hash/queue/stack/list

// arduino-specific functions, be sure to add arduino_lib.cpp to your
// sketch. much thanks to Koepel for contributing
void wr_loadAllArduinoLibs( WRState* w );

// embedded-specific libs
#define WR_FASTLED_DATA_PIN 6  // define your data pin here
void wr_loadFastLEDLib( WRState* w );

void wr_loadArduinoSTDLib( WRState* w ); 
void wr_loadArduinoIOLib( WRState* w ); 
void wr_loadArduinoLCDLib( WRState* w ); 

/***************************************************************/
/***************************************************************/
//             wrench types and how to make them!

// The WRValue is the basic data unit inside wrench, it can be a
// float/int/array whatever... think of it as a thneed. It's 8 bytes on
// a 32 bit system and you can create one on the stack no problemo, no
// heap will be harmed

// load a value up and make it ready for calling a function
// a string has to exist in a context so it can be worked with
// ALSO can use the WRValue methods 'set...' directly
WRValue& wr_makeString( WRContext* context, WRValue* val, const char* data, const int len =0 );
inline WRValue& wr_makeInt( WRValue* val, int i );
inline WRValue& wr_makeFloat( WRValue* val, float f );

// turning a value into a container,
// NOTE!! Allocates a hash table which must be released with destroy!!
WRValue& wr_makeContainer( WRValue* val, const uint16_t sizeHint =0 );
void wr_destroyContainer( WRValue* val );

// create an instance of the named struct (as if it were new'ed inside
// wrench) if the name is not a valid struct in the given context the
// return value is null.
// This value is automatically cleaned up when the context is destroyed
WRValue* wr_instanceStruct( WRValue* val, WRContext* context, const char* name, const WRValue* argv =0, const int argn =0 );

// NOTE: value memory is managed by called and must remain valid for
// duration of the container!!
void wr_addValueToContainer( WRValue* container, const char* name, WRValue* value );
void wr_addArrayToContainer( WRValue* container, const char* name, char* array, const uint32_t size );

// memory managed by the container, fire and forget:
void wr_addFloatToContainer( WRValue* container, const char* name, const float f );
void wr_addIntToContainer( WRValue* container, const char* name, const int i );

// read back the value (if found) from a user-created container
WRValue* wr_getValueFromContainer( WRValue const& container, const char* name );

/******************************************************************/
//                    "standard" functions

extern int32_t wr_Seed;
extern const int32_t wr_randEx( const int32_t from, const int32_t to );


// inside-baseball time. This has to be here so stuff that includes
// "wrench.h" can create a WRValue.

/*

 wrench values have a 32-bit type vector
 
type bits: XXXX  XXXX  ____  ____  ____  ____  __TT
                                        [8-bit type]
           [             32-bit xtype              ]

the bottom two bits 'T' represent the main type:
int       00 -- integer, ui / i are the value
float     01 -- float, f is the value
reference 10 -- reference, *r is a pointer to the real value
extended  11 -- extended type, see upper bits.

This is so operation tables can be constructed 4x4 for very quick
resolution of how to add/subtract/AND/etc... two values without a
time-consuming if-else-tree. single bit shift + OR and we're there:

            (typeA << 2) | typeB

Always results in a value from 0-15 which can index into a function
table:

    FunctionTypedef wr_func[16] = 
    {
      do_I_I,  do_I_F,  do_I_R,  do_I_E,
      do_F_I,  do_F_F,  do_F_R,  do_F_E,
      do_R_I,  do_R_F,  do_R_R,  do_R_E,
      do_E_I,  do_E_F,  do_E_R,  do_E_E,
    };

such that
wr_func[ (typeA << 2) | typeB ]( look ma, no ifs! );

In the case of "extended" types these are the ones that take extra
logic: arrays and hashes.

The "extended" types are:

(x is "don't care")

0x20xxxxxx  raw array: r->c points to a flat array, and
                       r->p2 holds the size of the array in the
                              middle 21 bits:

                              0x1FFFFF00
                              
                              such that shifting them >>8 yields
                              the actual array size
                       This is done so the value is never garbage
                       collected, or matched to the wrong type, but
                       does limit the size of an array to 2megabytes

0x40xxxxxx  debug break: stop execution and do debugging work

0x60xxxxxx  iterator: p2 holds the element we are on
                        va holds the array to be iterated

0x80xxxxxx  reference array: refers to a particular element of an
                             array.

                             r-> points to the array we are holding an
                             element of

                             p2 holds the element in the middle 21
                             bits:

							 0x1FFFFF00
                             
							 such that shifting them >>8 yields
                             the actual element
                             
						     This is done so the value is never garbage
							 collected, or matched to the wrong type, but
							 does limit the size of an array to 2megabytes
                              
0xA0xxxxxx  array/utility hash:
                        va-> holds a pointer to the actual array object (gc_object)
                        which has been allocated and is subject to
                        garbage collection

                        there are 4 kinds of "array" objects:

						SV_VALUE            0x01 array of WRValues
						SV_CHAR             0x02 array of chars (this
                         						 is how strings are represented)
						SV_VOID_HASH_TABLE  0x04 hash table of void*
						                         (do not gc)
						SV_HASH_TABLE       0x03 hash table of WRValues
						                         (DO descend for gc)

0xC0xxxxxx  struct: This value is a constructed "struct" object with a
                   hash table of values
                   
                   va-> refers to the container of initialized objects
                  

0xE0xxxxxx  hash table: hash table of WRValues

                   va-> refers to the hash table object

*/


//------------------------------------------------------------------------------
#if __cplusplus <= 199711L
enum WRValueType
#else
enum WRValueType : uint8_t
#endif
{
	WR_INT =   0x00,
	WR_FLOAT = 0x01,
	WR_REF =   0x02,
	WR_EX =    0x03,
};

//------------------------------------------------------------------------------
#if __cplusplus <= 199711L
enum WRExType
#else
enum WRExType : uint8_t
#endif
{
	WR_EX_INVALID    = 0x00,  // 0000

	WR_EX_RAW_ARRAY  = 0x20,  // 0010

//	0x40 unused

	WR_EX_ITERATOR	       = 0x60,  // 0110
	WR_EX_CONTAINER_MEMBER = 0x80,  // 1000
	WR_EX_ARRAY            = 0xA0,  // 1010

	// see EXPECTS_HASH_INDEX!!
	WR_EX_STRUCT     = 0xC0,  // 1100
	WR_EX_HASH_TABLE = 0xE0,  // 1110
};

//------------------------------------------------------------------------------
enum WRGCFlags
{
	GCFlag_NoContext = 1<<0,
	GCFlag_Marked = 1<<1,
	GCFlag_Perm = 1<<2,
};

//------------------------------------------------------------------------------
enum WRGCObjectType
{
	SV_VOID_HASH_TABLE = 0x0, // must be zero so memset(0) defaults to it
	SV_HASH_TABLE = 0x01,
	SV_HASH_ENTRY = 0x02,
	SV_HASH_INTERNAL = 0x03,
	
	SV_VALUE = 0x04, // !!must ALWAYS be last two so >= works
	SV_CHAR = 0x05,  // !!
};

#ifdef ARDUINO
#include <Arduino.h>
#endif

#if (defined(BYTE_ORDER) && BYTE_ORDER == BIG_ENDIAN) || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) || (defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN) || defined(__BIG_ENDIAN__) || defined(__ARMEB__) || defined(__THUMBEB__) || defined(__AARCH64EB__) || defined(_MIBSEB) || defined(__MIBSEB) || defined(__MIBSEB__) || defined(ARDUINO_BIG_ENDIAN)
  #define WRENCH_BIG_ENDIAN
#elif (defined(BYTE_ORDER) && BYTE_ORDER == LITTLE_ENDIAN) || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) || (defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN) || defined(__LITTLE_ENDIAN__) || defined(__ARMEL__) || defined(_M_PPC) || defined(__THUMBEL__) || defined(__AARCH64EL__) || defined(_MSC_VER) || defined(_MIPSEL) || defined(__MIPSEL) || defined(__MIPSEL__) || defined(ARDUINO_LITTLE_ENDIAN)
  #define WRENCH_LITTLE_ENDIAN
#else
  #error "Endian-ness not detected! Please contact curt.hartung@gmail.com so it can be added <01>"
#endif

#ifdef WRENCH_HANDLE_MALLOC_FAIL
extern bool g_mallocFailed; // used as an internal global flag for when a malloc came back null
#endif

extern WR_ALLOC g_malloc;
extern WR_FREE g_free;

class WRGCObject;
class WRGCBase;

//------------------------------------------------------------------------------
struct WRIteratorEntry
{
	int type; // SV_VALUE, SV_CHAR or SV_HASH_TABLE

	const WRValue* key; // if this is a hash table the key value
	const WRValue* value; // for SV_HASH_TABLE and SV_VALUE (character will be null)

	int index; // array entry for non-hash tables
	char character; // for SV_CHAR (value will be null)
};

//------------------------------------------------------------------------------
struct WRValue
{
	WRValue( int val ) { init(val); }
	WRValue( float val ) { init(val); }

	WRValue* init() { p = 0; p2 = WR_INT; return this; }
	WRValue* init( int val ) { i = val; p2 = WR_INT; return this; }
	WRValue* init( float val ) { f = val; p2 = WR_FLOAT; return this; }

	// never reference the data members directly, they are unions and
	// bad things will happen. Always access them with one of these
	// methods
	int asInt() const;
	float asFloat() const;

	// setting this value, do any extra work required. Note that
	// setString() requires a context, since the string is actually a
	// created array that needs to be referenced
	void setString( WRContext* context, const char* data, const int len =0 ) { wr_makeString(context, this, data, len); }
	void setFloat( const float F ) { wr_makeFloat(this, F); }
	void setInt( const int I ) { wr_makeInt(this, I); }

	bool isFloat() const {return type == WR_FLOAT || (type == WR_REF && r->type == WR_FLOAT); }
	bool isInt() const { return type == WR_INT || (type == WR_REF && r->type == WR_INT); }
	bool isString( int* len =0 ) const;
	bool isWrenchArray( int* len =0 ) const;
	bool isRawArray( int* len =0 ) const;
	bool isHashTable( int* members=0 ) const;
	bool isStruct() const;

	// if this value is an array, return 'index'-th element
	// if create is true and this value is NOT an array, it will be converted into one
	WRValue* indexArray( WRContext* context, const uint32_t index, const bool create ) const;
	
	// if this value is a hash table, return [or create] the 'index' hash item
	// if create is true and this value is NOT a hash, it will be converted into one
	WRValue* indexHash( WRContext* context, const uint32_t hash, const bool create ) const;

	// if this value is a struct return the element, otherwise null
	WRValue* indexStruct( const char* label ) const;

	// string: must point to a buffer long enough to contain at least maxLen bytes.
	// the "string" pointer will be passed back, if maxLen is 0 (not
	// reccomended) the string is assumed to be unlimited size
	char* asString( char* string, unsigned int maxLen =0, unsigned int* strLen =0 ) const;

	// malloc a string of sufficient size and copy/format the contents
	// of this value into it, the string muyst be g_free'ed
	char* asMallocString( unsigned int* strLen =0 ) const;
	class MallocStrScoped // helper class for asMallocString()
	{
	public:
		operator bool()        const { return m_str != 0; }
		operator const char*() const { return m_str; }
		unsigned int size()    const { return m_size; }
		MallocStrScoped( WRValue const& V ) : m_size(0), m_str(V.asMallocString(&m_size)) {}
		~MallocStrScoped() { g_free((char*)m_str); }
	private:
		unsigned int m_size;
		const char* m_str;
	};

	// same as "asString" but will print it in a more debug-symbol-y
	char* technicalAsString( char* string, unsigned int maxLen, bool valuesInHex =false, unsigned int* strLen =0 ) const;

	// return a raw pointer to the data array if this is one, otherwise null
	void* array( unsigned int* len =0, char arrayType =SV_CHAR ) const;
	int arraySize() const; // returns length of the array or -1 if this value is not an array

//private: // is what this SHOULD be.. but that's impractical since the
	// VM is not an object that can be friended.

	WRValue& singleValue() const; // return a single value for comparison
	WRValue& deref() const; // if this value is a CONTAINER_MEMBER, copy the referred value in

	uint32_t getHash() const { return (type <= WR_FLOAT) ? ui : getHashEx(); } // easy cases
	uint32_t getHashEx() const; // harder

	union // first 4 bytes 
	{
		int32_t i;
		uint32_t ui;
		float f;
		WRValue* frame;
		const void* p;
		void* r1;
		char* c;
		WRValue* r;
		WRGCObject* va;
		WRGCBase* vb;
		WR_C_CALLBACK ccb;
		WR_LIB_CALLBACK lcb;
		WRFunction* wrf;
	};

	union
	{
#ifdef WRENCH_LITTLE_ENDIAN
		struct
		{
#if (__cplusplus <= 199711L)
			uint8_t type; // carries the type
			uint8_t padL; // pad carries an array pointer (up to 2megs using bottom 4 bits of xtype)
			uint8_t padH;
			uint8_t xtype; // carries the extra type (if it exists)
#else
			WRValueType type; // carries the type
			uint8_t padL; // pad carries an array pointer (up to 2megs using bottom 4 bits of xtype)
			uint8_t padH;
			WRExType xtype; // carries the extra type (if it exists)
#endif
		};
#else // WRENCH_BIG_ENDIAN
		struct
		{
#if (__cplusplus <= 199711L)
			uint8_t xtype; // carries the extra type (if it exists)
			uint8_t padH;
			uint8_t padL; // pad carries an array pointer (up to 2megs using bottom 4 bits of xtype)
			uint8_t type; // carries the type
#else
			WRExType xtype; // carries the extra type (if it exists)
			uint8_t padH;
			uint8_t padL; // pad carries an array pointer (up to 2megs using bottom 4 bits of xtype)
			WRValueType type; // carries the type
#endif
		};
#endif
		uintptr_t p2;
		
		union
		{
			intptr_t returnOffset;
			WRValue* r2;
			void* usr;
		};
	};

	WRValue() {}
	inline WRValue& operator= (const WRValue& V) { r1 = V.r1; r2 = V.r2; return *this; }
	inline WRValue(const WRValue& V) { r1 = V.r1; r2 = V.r2; }

public:

	//------------------------------------------------------------------------------
	class Iterator
	{
	public:
		Iterator( WRValue const& V );
		
		Iterator() : m_va(0) {}

		bool operator!=( const Iterator& other )
		{
			return m_va != other.m_va;
		}

		WRIteratorEntry const& operator* () const { return m_current; }

		const Iterator operator++();
			
	private:

		WRIteratorEntry m_current;
		WRGCObject const* m_va;
		unsigned int m_element;
	};
	const Iterator begin() const { return Iterator(deref()); }
	const Iterator end() const { return Iterator(); }
};

//------------------------------------------------------------------------------
inline WRValue& wr_makeInt( WRValue* val, int i ) { return *(val->init( i )); }
inline WRValue& wr_makeFloat( WRValue* val, float f ) { return *(val->init( f )); }

//------------------------------------------------------------------------------
class WRGCBase
{
public:

	// the order here matters for data alignment

#if (__cplusplus <= 199711L)
	int8_t m_type;
#else
	WRGCObjectType m_type;
#endif

	int8_t m_flags;

	union
	{
		uint16_t m_mod;
		uint16_t m_hashItem;
	};

	union
	{
		void* m_data;
		char* m_SCdata;
		unsigned char* m_Cdata;
		WRValue* m_Vdata;
		WRGCBase* m_referencedTable;
	};

	WRGCBase* m_nextGC;

	void clear()
	{
		if ( m_type >= SV_VALUE )
		{
			g_free( m_Cdata );
		}
	}
};

//------------------------------------------------------------------------------
// Helper class to represent a wrench value, in all cases it does NOT
// manage the memory, but relies on a WRContext to do that
class WrenchValue
{
public:
		
	WrenchValue( WRContext* context, const char* label ) : m_context(context), m_value( wr_getGlobalRef(context, label) ) {}
	WrenchValue( WRContext* context, WRValue* value ) : m_context(context), m_value(value ? &(value->deref()) : 0) {}

	bool isValid() const { return m_value ? true : false; }

	// if the value is not the correct type it will be converted to
	// that type, preserving the value as best it can
	operator int32_t* () { return Int(); }
	int32_t* Int();
	
	operator float* () { return (float*)Float(); }
	float* Float();

	// this will convert it to an array if it isn't one
	WRValue& operator[] ( const int index ) { return *asArrayMember( index ); }
	WRValue* asArrayMember( const int index );
	int arraySize() const { return m_value ? m_value->arraySize() : -1; } // returns -1 if this is not an array

	// IMPORTANT: the returned pointers refer to internal storage in the hash
	// table and can become stale if the hash table grows/re-hashes; reacquire
	// with getHashTableValue() after modifications.
	// convert this value to a hash table (if needed) and create key entry
	WRValue* addHashTableValue( const char* key );
	WRValue* getHashTableValue( const char* key ); // return null if not found or this is not a hash table

private:
	WRContext* m_context;
	WRValue* m_value;
};


#ifdef WRENCH_TIME_SLICES
//------------------------------------------------------------------------------
// Helper class to schedule and run tasks
struct WrenchScheduledTask;

class WrenchScheduler
{
public:
	WrenchScheduler( const int stackSizePerThread = WRENCH_DEFAULT_STACK_SIZE );
	~WrenchScheduler();

	WRState* state() const { return m_w; }

	void tick( const int instructionsPerSlice =1000 );

	// returns a task ID
	int addThread( const uint8_t* byteCode, const int size, const int instructionsThisSlice =1000, const bool takeOwnership =false );
	bool removeTask( const int taskId );

	int lastErr() const { return m_lastErr; }       // WRError of last faulted task, 0 if none
	int lastErrTaskId() const { return m_lastErrId; } // id of the task that faulted
	void clearErr() { m_lastErr = 0; m_lastErrId = 0; }

private:
	WRState* m_w;
	WrenchScheduledTask* m_tasks;
	int m_lastErr;
	int m_lastErrId;
};
#endif

//------------------------------------------------------------------------------
class WRGCObject : public WRGCBase
{
public:

	uint32_t m_size;
	union
	{
		uint32_t* m_hashTable;
		const uint8_t* m_ROMHashTable;
		WRContext* m_creatorContext;
	};

	int init( const unsigned int size, const WRGCObjectType type, bool clear );
	
	WRValue* getAsRawValueHashTable( const uint32_t hash, int* index =0 );
	
	WRValue* exists( const uint32_t hash, bool removeIfPresent );
	
	void* get( const uint32_t l, int* index =0 );
	
	uint32_t growHash( const uint32_t hash, const uint16_t sizeHint =0, int* sizeAllocated =0 );
	uint32_t getIndexOfHit( const uint32_t hash, const bool inserting );

private:

	WRGCObject& operator= ( WRGCObject& A );
	WRGCObject(WRGCObject& A);
};

//------------------------------------------------------------------------------
struct WRContext
{
	uint16_t globals;

	uint32_t allocatedMemoryHint; // _approximately_ how much memory has been allocated since last gc

	const unsigned char* bottom;
	const unsigned char* codeStart;
	int32_t bottomSize;

	void* ctxLocal; // user value assigned with this context, opaque to wrench

	WRValue* stack;

	const unsigned char* stopLocation;

	WRGCBase* svAllocated;

#ifdef WRENCH_INCLUDE_DEBUG_CODE
	WRDebugServerInterface* debugInterface;
#endif

	WRState* w;

	WRGCObject registry; // the 'next' pointer in this registry is used as the context LL next

	const unsigned char* yield_pc;
	WRValue* yield_stackTop;
	WRValue* yield_frameBase;
	const WRValue* yield_argv;
	uint8_t yield_argn;
	uint8_t yieldArgs;
	uint16_t stackOffset;

	WRFunction* localFunctions;
	uint8_t numLocalFunctions;

	uint8_t flags;

	WRContext* imported; // linked list of contexts this one imported

	WRContext* nextStateContextLink;

	void markBase( WRGCBase* svb );
	void mark( WRValue* s );
	void gc( WRValue* stackTop );

	WRGCObject* getSVA( int size, WRGCObjectType type, bool init );
};

struct WRLibraryCleanup;

//------------------------------------------------------------------------------
struct WRState
{
#ifdef WRENCH_TIME_SLICES
	int instructionsPerSlice;
	int sliceInstructionCount;
	int yieldEnabled;
	int lastSlicesUsed;
#endif

	WRContext* contextList;

	WRLibraryCleanup* libCleanupFunctions;

	WRGCObject globalRegistry;

	void* ctx; // state-wide context pointer, opaque to wrench

	uint32_t allocatedMemoryLimit; // WRENCH_DEFAULT_ALLOCATED_MEMORY_GC_HINT by default
	uint16_t stackSize; // how much stack to give each context
	int8_t err;

};

#define WRENCH_NULL_HASH 0xABABABAB  // -1414812757 / -1.2197928214371934e-12, can't be zero since we use int/floats as their own hash

#ifdef WRENCH_INCLUDE_DEBUG_CODE
#ifndef WRENCH_COMBINED
#include "utils/simple_ll.h"
#endif

struct WrenchPacket;
class WRDebugServerInterface;
class WRDebugServerInterfacePrivate;
class WRDebugClientInterfacePrivate;
class WrenchDebugCommInterface;
template<class> class SimpleLL;

//------------------------------------------------------------------------------
struct WrenchDebugValue
{
	char name[64];
	WRValue value;
};

//------------------------------------------------------------------------------
struct WrenchSymbol
{
	char label[64];
};

//------------------------------------------------------------------------------
struct WrenchFunction
{
	char name[64];
	int arguments;
	SimpleLL<WrenchSymbol>* vars;
};

//------------------------------------------------------------------------------
struct WrenchCallStackEntry
{
	int32_t onLine;
	uint16_t stackOffset;

	uint8_t fromUnitIndex;
	uint8_t thisUnitIndex;
	uint16_t locals;

	uint8_t arguments;
};

//------------------------------------------------------------------------------
enum WRP_ProcState
{
	WRP_Unloaded =0,
	WRP_Loaded,
	WRP_Running,
	WRP_Complete
};

//------------------------------------------------------------------------------
// This object runs as the main interface to a running debug image, it
// can drive it remotely over a serial (or whatever) link or directly
// through a pointer.
class WRDebugClientInterface
{
public:
	// server is remote, define these methods to talk to it
	WRDebugClientInterface( WrenchDebugCommInterface* CommInterface );

	// load and prepare a program for running
	// if byteCode/size is null then previoous code is re-loaded
	void load( const uint8_t* byteCode =0, const int size =0 ); 

	bool getSourceCode( const char** data, int* len ); // get a new'ed pointer to the source code
	uint32_t getSourceCodeHash(); // get the crc of the code that was compiled

	SimpleLL<WrenchFunction>& getFunctions(); // global is function[0] 

	// 0 is global, 1,2,3... etc are frames
	SimpleLL<WrenchCallStackEntry>* getCallstack();
	bool getStackDump( char* out, const unsigned int maxOut );
	
	const char* getFunctionLabel( const int index );
	const char* getValueLabel( const int index, const int depth );
	WRValue* getValue( WRValue& value, const int index, const int depth );

	void run( const int toLine =0 );

	void stepInto( const int steps =1 );
	void stepOver( const int steps =1 );

	void setBreakpoint( const int lineNumber );
	void clearBreakpoint( const int lineNumber );

	WRP_ProcState getProcState();

	void setDebugEmitCallback( void (*print)( const char* debugText ) );

	// "hidden" internals to keep header clean
	WRDebugClientInterfacePrivate* I;
	~WRDebugClientInterface();

	void init();
};

//------------------------------------------------------------------------------
// This object is run on the target machine to manage the running
// image. It has a minimal interface and is meant to be driven entirely
// by WRDebugClientInterface
class WRDebugServerInterface
{
public:

	WRDebugServerInterface( WRState* w, WrenchDebugCommInterface* CommInterface );

	void tick();

	// NOTE: loadBytes() does NOT make a local copy! bytes must remain valid for the life of this object!
	WRContext* loadBytes( const uint8_t* bytes, const int len );

	// "hidden" internals to keep header clean
	WRDebugServerInterfacePrivate *I;
	~WRDebugServerInterface();
};

#endif

#ifdef WRENCH_REALLY_COMPACT
#ifndef WRENCH_COMPACT
#define WRENCH_COMPACT
#endif
#endif

#define WRENCH_CLI_DEBUG
#ifdef WRENCH_CLI_DEBUG
extern WRContext* g_context;
#endif
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

//------------------------------------------------------------------------------
inline void wr_setStateContext( WRState* w, void* ctx ) { w->ctx = ctx; }
inline void wr_setLocalContext( WRContext* c, void* ctx ) { c->ctxLocal = ctx; }
inline void* wr_getStateContext( WRState* w ) { return w->ctx; }
inline void* wr_getLocalContext( WRContext* c ) { return c->ctxLocal; }

#ifdef STR_FILE_OPERATIONS
#include <sys/stat.h>
#endif

#if __cplusplus > 199711L
#define STR_COPY_ARG
#endif

// same as str.h but for char only so no template overhead, also no
// new/delete just malloc/free

const unsigned int c_sizeofBaseString = 15; // this lib tries not to use dynamic RAM unless it has to
const int c_formatBaseTrySize = 80;

//-----------------------------------------------------------------------------
class WRstr
{
public:
	WRstr() { m_smallbuf[m_len = 0] = 0 ; m_str = m_smallbuf; m_buflen = c_sizeofBaseString; }
	WRstr( const WRstr& str) { m_len = 0; m_str = m_smallbuf; m_buflen = c_sizeofBaseString; set(str, str.size()); } 
	WRstr( const WRstr* str ) { m_len = 0; m_str = m_smallbuf; m_buflen = c_sizeofBaseString; if ( str ) { set(*str, str->size()); } } 
	WRstr( const char* s, const unsigned int len ) { m_len = 0; m_str = m_smallbuf; m_buflen = c_sizeofBaseString; set(s, len); }
	WRstr( const char* s ) { m_len = 0; m_str = m_smallbuf; m_buflen = c_sizeofBaseString; set(s, (unsigned int)strlen(s)); }
	WRstr( const char c ) { m_len = 1; m_str = m_smallbuf; m_smallbuf[0] = c; m_smallbuf[1] = 0; m_buflen = c_sizeofBaseString; } 

#ifdef STR_FILE_OPERATIONS
	inline bool fileToBuffer( const char* fileName, const bool appendToBuffer =false );
	inline bool bufferToFile( const char* fileName, const bool append =false ) const;
#else
	bool fileToBuffer( const char* fileName, const bool appendToBuffer =false ) { return false; }
	bool bufferToFile( const char* fileName, const bool append =false ) const { return false; }
#endif

	WRstr& clear() { m_str[m_len = 0] = 0; return *this; }

#ifdef STR_COPY_ARG
	WRstr& format( const char* format, ... ) { va_list arg; va_start( arg, format ); clear(); appendFormatVA( format, arg ); va_end( arg ); return *this; }
	WRstr& formatVA( const char* format, va_list arg ) { clear(); return appendFormatVA(format, arg); }
	WRstr& appendFormat( const char* format, ... ) { va_list arg; va_start( arg, format ); appendFormatVA( format, arg ); va_end( arg ); return *this; }
	inline WRstr& appendFormatVA( const char* format, va_list arg );
#else
	inline WRstr& format( const char* format, ... );
	inline WRstr& appendFormat( const char* format, ... );
#endif

	inline void release( char** toBuf, unsigned int* len =0 ); // always suceeds and returns dynamic memory
	inline WRstr& giveOwnership( char* str, const unsigned int len );

	static const unsigned int npos = (unsigned int)-1;
	unsigned int find( const char c, const unsigned int from =0 ) const { char buf[2] = { c, 0 }; return find(buf, from); }
	unsigned int rfind( const char c, const unsigned int from =npos ) const { char buf[2] = { c, 0 }; return rfind(buf, from); }
	unsigned int findCase( const char c, const unsigned int from =0 ) const { char buf[2] = { c, 0 }; return findCase(buf, from); }
	inline unsigned int find( const char* str, const unsigned int from =0 ) const;
	inline unsigned int rfind( const char* str, const unsigned int from =npos ) const;
	inline unsigned int findCase( const char* str, const unsigned int from =0 ) const;

	WRstr& setSize( const unsigned int size, const bool preserveContents =true ) { alloc(size, preserveContents); m_len = size; m_str[size] = 0; return *this; }

	inline WRstr& alloc( const unsigned int characters, const bool preserveContents =true );

	inline WRstr& trim();
	inline WRstr& truncate( const unsigned int newLen ); // reduce size to 'newlen'
	WRstr& shave( const unsigned int e ) { return (e > m_len) ? clear() : truncate(m_len - e); } // remove 'x' trailing characters
	inline WRstr& shift( const unsigned int from );
	inline WRstr substr( const unsigned int begin, const unsigned int len ) const;

	unsigned int size() const { return m_len; } // see length

	const char* c_str( const unsigned int offset =0 ) const { return m_str + offset; }
	char* p_str( const unsigned int offset =0 ) const { return m_str + offset; }

	operator const void*() const { return m_str; }
	operator const char*() const { return m_str; }

	WRstr& set( const char* buf, const unsigned int len ) { m_len = 0; m_str[0] = 0; return insert( buf, len ); }
	WRstr& set( const WRstr& str ) { return set( str.m_str, str.m_len ); }
	WRstr& set( const char c ) { clear(); m_str[0]=c; m_str[1]=0; m_len = 1; return *this; }

	bool isMatch( const char* buf ) const { return strcmp(buf, m_str) == 0; }
#ifdef _WIN32
	bool isMatchCase( const char* buf ) const { return _strnicmp(buf, m_str, m_len) == 0; }
#else
	bool isMatchCase( const char* buf ) const { return strncasecmp(buf, m_str, m_len) == 0; }
#endif
	static inline bool isWildMatch( const char* pattern, const char* haystack );
	inline bool isWildMatch( const char* pattern ) const { return isWildMatch( pattern, m_str ); }
				  
	static inline bool isWildMatchCase( const char* pattern, const char* haystack );
	inline bool isWildMatchCase( const char* pattern ) const { return isWildMatchCase( pattern, m_str ); }

	inline WRstr& insert( const char* buf, const unsigned int len, const unsigned int startPos =0 );
	inline WRstr& insert( const WRstr& s, const unsigned int startPos =0 ) { return insert(s.m_str, s.m_len, startPos); }

	inline WRstr& append( const char* buf, const unsigned int len ) { return insert(buf, len, m_len); } 
	inline WRstr& append( const char c );
	inline WRstr& append( const WRstr& s ) { return insert(s.m_str, s.m_len, m_len); }

	// define the usual suspects:

	const char& operator[]( const int l ) const { return get((unsigned int)l); }
	const char& operator[]( const unsigned int l ) const  { return get(l); }
	char& operator[]( const int l )  { return get((unsigned int)l); }
	char& operator[]( const unsigned int l ) { return get(l); }

	char& get( const unsigned int l ) { return m_str[l]; }
	const char& get( const unsigned int l ) const { return m_str[l]; }

	WRstr& operator += ( const WRstr& str ) { return append(str.m_str, str.m_len); }
	WRstr& operator += ( const char* s ) { return append(s, (unsigned int)strlen(s)); }
	WRstr& operator += ( const char c ) { return append(c); }

	WRstr& operator = ( const WRstr& str ) { if ( &str != this ) set(str, str.size()); return *this; }
	WRstr& operator = ( const WRstr* str ) { if ( !str ) { clear(); } else if ( this != str ) { set(*str, str->size()); } return *this; }
	WRstr& operator = ( const char* c ) { set(c, (unsigned int)strlen(c)); return *this; }
	WRstr& operator = ( const char c ) { set(&c, 1); return *this; }

	friend bool operator == ( const WRstr& s1, const WRstr& s2 ) { return s1.m_len == s2.m_len && (strncmp(s1.m_str, s2.m_str, s1.m_len) == 0); }
	friend bool operator == ( const char* z, const WRstr& s ) { return s.isMatch( z ); }
	friend bool operator == ( const WRstr& s, const char* z ) { return s.isMatch( z ); }
	friend bool operator != ( const WRstr& s1, const WRstr& s2 ) { return s1.m_len != s2.m_len || (strncmp(s1.m_str, s2.m_str, s1.m_len) != 0); }
	friend bool operator != ( const WRstr& s, const char* z ) { return !s.isMatch( z ); }
	friend bool operator != ( const char* z, const WRstr& s ) { return !s.isMatch( z ); }
	friend bool operator != ( const WRstr& s, char* z ) { return !s.isMatch( z ); }
	friend bool operator != ( char* z, const WRstr& s ) { return !s.isMatch( z ); }

	friend WRstr operator + ( const WRstr& str, const char* s) { WRstr T(str); T += s; return T; }
	friend WRstr operator + ( const WRstr& str, const char c) { WRstr T(str); T += c; return T; }
	friend WRstr operator + ( const char* s, const WRstr& str ) { WRstr T(s, (unsigned int)strlen(s)); T += str; return T; }
	friend WRstr operator + ( const char c, const WRstr& str ) { WRstr T(c); T += str; return T; }
	friend WRstr operator + ( const WRstr& str1, const WRstr& str2 ) { WRstr T(str1); T += str2; return T; }

	~WRstr() { if ( m_str != m_smallbuf ) g_free(m_str); }

protected:

	operator char*() const { return m_str; } // prevent accidental use

	char *m_str; // first element so if the class is cast as a C and de-referenced it always works

	unsigned int m_buflen; // how long the buffer itself is
	unsigned int m_len; // how long the string is in the buffer
	char m_smallbuf[ c_sizeofBaseString + 1 ]; // small temporary buffer so a malloc/free is not imposed for small strings
};

//------------------------------------------------------------------------------
unsigned int WRstr::rfind( const char* str, const unsigned int from ) const
{
	int f = (int)(from > m_len ? m_len : from);
	if ( !str || !str[0] )
	{
		return 0;
	}

	for( ; f >= 0; --f )
	{
		for( int i=0;;++i )
		{
			if ( !str[i] )
			{
				return f;
			}
			if ( str[i] != m_str[f + i] )
			{
				break;
			}
		}
	}
	return npos;
}

//------------------------------------------------------------------------------
unsigned int WRstr::find( const char* str, const unsigned int from ) const
{
	unsigned int f = from > m_len ? 0 : from;
	if ( !str || !str[0] )
	{
		return 0;
	}

	for( ; f < m_len; ++f )
	{
		for( int i=0;;++i )
		{
			if ( !str[i] )
			{
				return f;
			}

			char c = m_str[f + i];
			if ( !c )
			{
				return npos;
			}

			if ( str[i] != c )
			{
				break;
			}
		}
	}
	return npos;
}

//------------------------------------------------------------------------------
unsigned int WRstr::findCase( const char* str, const unsigned int from ) const
{
	unsigned int f = from > m_len ? 0 : from;
	if ( !str || !str[0] )
	{
		return 0;
	}

	for( ; f < m_len; ++f )
	{
		for( int i=0;;++i )
		{
			if ( !str[i] )
			{
				return f;
			}

			char c = m_str[f + i];
			if ( !c )
			{
				return npos;
			}

			if ( tolower(str[i]) != tolower(c) )
			{
				break;
			}
		}
	}
	return npos;
}

#ifdef STR_FILE_OPERATIONS
//-----------------------------------------------------------------------------
bool WRstr::fileToBuffer( const char* fileName, const bool appendToBuffer )
{
	if ( !fileName )
	{
		return false;
	}

#ifdef _WIN32
	struct _stat sbuf;
	int ret = _stat( fileName, &sbuf );
#else
	struct stat sbuf;
	int ret = stat( fileName, &sbuf );
#endif

	if ( ret != 0 )
	{
		return false;
	}

	FILE *infil = fopen( fileName, "rb" );
	if ( !infil )
	{
		return false;
	}

	if ( appendToBuffer )
	{
		alloc( sbuf.st_size + m_len, true );
		m_str[ sbuf.st_size + m_len ] = 0;
		ret = (int)fread( m_str + m_len, sbuf.st_size, 1, infil );
		m_len += sbuf.st_size;
	}
	else
	{
		alloc( sbuf.st_size, false );
		m_len = sbuf.st_size;
		m_str[ m_len ] = 0;
		ret = (int)fread( m_str, m_len, 1, infil );
	}

	fclose( infil );
	return ret == 1;
}

//-----------------------------------------------------------------------------
bool WRstr::bufferToFile( const char* fileName, const bool append) const
{
	if ( !fileName )
	{
		return false;
	}

	FILE *outfil = append ? fopen( fileName, "a+b" ) : fopen( fileName, "wb" );
	if ( !outfil )
	{
		return false;
	}

	int ret = (int)fwrite( m_str, m_len, 1, outfil );
	fclose( outfil );

	return (m_len == 0) || (ret == 1);
}
#endif

//-----------------------------------------------------------------------------
void WRstr::release( char** toBuf, unsigned int* len )
{
	if ( len )
	{
		*len = m_len;
	}
	
	if ( !m_len )
	{
		*toBuf = 0;
	}
	else if ( m_str == m_smallbuf )
	{
		*toBuf = (char*)g_malloc( m_len + 1 );
		memcpy( *toBuf, m_str, m_len + 1 );
	}
	else
	{
		*toBuf = m_str;
		m_str = m_smallbuf;
		m_buflen = c_sizeofBaseString;
	}

	m_len = 0;
	m_str[0] = 0;
}

//-----------------------------------------------------------------------------
WRstr& WRstr::giveOwnership( char* buf, const unsigned int len )
{
	if ( !buf || !len )
	{
		clear();
		return *this;
	}

	if ( m_str != m_smallbuf )
	{
		g_free( m_str );
	}

	if ( len < c_sizeofBaseString )
	{
		m_str = m_smallbuf;
		memcpy( m_str, buf, len );
		g_free( buf );
		m_len = len;
	}
	else
	{
		m_str = buf;
	}

	m_len = len;
	m_buflen = len;
	m_str[m_len] = 0;
	
	return *this;
}

//-----------------------------------------------------------------------------
WRstr& WRstr::trim()
{
	unsigned int start = 0;

	// find start
	for( ; start<m_len && isspace( (char)*(m_str + start) ) ; start++ );

	// is the whole thing whitespace?
	if ( start == m_len )
	{
		clear();
		return *this;
	}

	// copy down the characters one at a time, noting the last
	// non-whitespace character position, which will become the length
	unsigned int pos = 0;
	unsigned int marker = start;
	for( ; start<m_len; start++,pos++ )
	{
		if ( !isspace((char)(m_str[pos] = m_str[start])) )
		{
			marker = pos;
		}
	}

	m_len = marker + 1;
	m_str[m_len] = 0;

	return *this;
}

//-----------------------------------------------------------------------------
WRstr& WRstr::alloc( const unsigned int characters, const bool preserveContents )
{
	if ( characters >= m_buflen ) // only need to alloc if more space is requested than we have
	{
		char* newStr = (char*)g_malloc( characters + 1 ); // create the space

		if ( preserveContents ) 
		{
			memcpy( newStr, m_str, m_buflen ); // preserve whatever we had
		}

		if ( m_str != m_smallbuf )
		{
			g_free( m_str );
		}

		m_str = newStr;
		m_buflen = characters;		
	}

	return *this;
}

//-----------------------------------------------------------------------------
WRstr& WRstr::truncate( const unsigned int newLen )
{
	if ( newLen >= m_len )
	{
		return *this;
	}

	if ( newLen < c_sizeofBaseString )
	{
		if ( m_str != m_smallbuf )
		{
			m_buflen = c_sizeofBaseString;
			memcpy( m_smallbuf, m_str, newLen );
			g_free( m_str );
			m_str = m_smallbuf;
		}
	}

	m_str[ newLen ] = 0;
	m_len = newLen;

	return *this;
}

//-----------------------------------------------------------------------------
WRstr& WRstr::shift( const unsigned int from )
{
	if ( from >= m_len )
	{
		return clear();
	}

	m_len -= from;
	memmove( m_str, m_str + from, m_len + 1 );

	return *this;
}

//------------------------------------------------------------------------------
WRstr WRstr::substr( const unsigned int begin, const unsigned int len ) const
{
	WRstr ret;
	if ( begin < m_len )
	{
		unsigned int amount = (begin + len) > m_len ? m_len - begin : len;
		ret.set( m_str + begin, amount );
	}
	return ret;
}

//-----------------------------------------------------------------------------
bool WRstr::isWildMatch( const char* pattern, const char* haystack )
{
	if ( !pattern )
	{
		return false;
	}

	if ( pattern[0] == 0 )
	{
		return haystack[0] == 0;
	}

	const char* after = 0;
	const char* str = haystack;
	char t;
	char w;

	for(;;)
	{
		t = *str;
		w = *pattern;
		if ( !t )
		{
			if ( !w )
			{
				return true; // "x" matches "x"
			}
			else if (w == '*')
			{
				++pattern;
				continue; // "x*" matches "x" or "xy"
			}

			return false; // "x" doesn't match "xy"
		}
		else if ( t != w )
		{
			if (w == '*')
			{
				after = ++pattern;
				continue; // "*y" matches "xy"
			}
			else if (after)
			{
				pattern = after;
				w = *pattern;
				if ( !w )
				{
					return true; // "*" matches "x"
				}
				else if (t == w)
				{
					++pattern;
				}
				++str;
				continue; // "*sip*" matches "mississippi"
			}
			else
			{
				return false; // "x" doesn't match "y"
			}
		}

		++str;
		++pattern;
	}
}

//-----------------------------------------------------------------------------
bool WRstr::isWildMatchCase( const char* pattern, const char* haystack )
{
	if ( !pattern )
	{
		return false;
	}

	if ( pattern[0] == 0 )
	{
		return haystack[0] == 0;
	}

	const char* after = 0;
	const char* str = haystack;
	char t;
	char w;

	for(;;)
	{
		t = *str;
		w = *pattern;
		if ( !t )
		{
			if ( !w )
			{
				return true; // "x" matches "x"
			}
			else if (w == '*')
			{
				++pattern;
				continue; // "x*" matches "x" or "xy"
			}

			return false; // "x" doesn't match "xy"
		}
		else if ( tolower(t) != tolower(w) )
		{
			if (w == '*')
			{
				after = ++pattern;
				continue; // "*y" matches "xy"
			}
			else if (after)
			{
				pattern = after;
				w = *pattern;
				if ( !w )
				{
					return true; // "*" matches "x"
				}
				else if (t == w)
				{
					++pattern;
				}
				++str;
				continue; // "*sip*" matches "mississippi"
			}
			else
			{
				return false; // "x" doesn't match "y"
			}
		}

		++str;
		++pattern;
	}
}

//-----------------------------------------------------------------------------
WRstr& WRstr::insert( const char* buf, const unsigned int len, const unsigned int startPos /*=0*/ )
{
	if ( len != 0 ) // insert 0? done
	{
		alloc( m_len + len + startPos, true ); // make sure there is enough room for the new string

		if ( startPos < m_len ) // text after the insert, move everything up
		{
			memmove( m_str + len + startPos, m_str + startPos, m_len );
		}

		memcpy( m_str + startPos, buf, len );

		m_len += len;
		m_str[m_len] = 0;
	}

	return *this;
}

//-----------------------------------------------------------------------------
WRstr& WRstr::append( const char c )
{
	if ( (m_len+1) >= m_buflen )
	{
		alloc( ((m_len * 3) / 2) + 1, true ); // single-character, expect a lot more are coming so alloc some buffer space
	}
	m_str[ m_len++ ] = c;
	m_str[ m_len ] = 0;

	return *this;
}

#ifdef STR_COPY_ARG

//------------------------------------------------------------------------------
WRstr& WRstr::appendFormatVA( const char* format, va_list arg )
{
	char buf[ c_formatBaseTrySize + 1 ]; // SOME space, malloc if we need a ton more

	va_list vacopy;
	va_copy( vacopy, arg ); // must be done BEFORE the vsnprintf() on some systems

	int len = vsnprintf( buf, c_formatBaseTrySize, format, arg );

	if ( len < c_formatBaseTrySize )
	{
		insert( buf, len, m_len );
	}
	else
	{
		char* alloc = (char*)g_malloc( ++len );

		len = vsnprintf(alloc, len, format, arg);

		if ( m_len )
		{
			insert( alloc, len, m_len );
			g_free( alloc );
		}
		else
		{
			giveOwnership( alloc, len );
		}
	}

	va_end( vacopy );

	return *this;
}

#else

//------------------------------------------------------------------------------
WRstr& WRstr::format( const char* format, ... )
{
	va_list arg;
	char buf[ c_formatBaseTrySize + 1 ]; // SOME space, malloc if we need a ton more

	va_start( arg, format );
	int len = vsnprintf( buf, c_formatBaseTrySize, format, arg );
	va_end( arg );

	if ( len < c_formatBaseTrySize+1 )
	{
		set( buf, len );
	}
	else
	{
		char* alloc = (char*)g_malloc(++len);

		va_start( arg, format );
		len = vsnprintf( alloc, len, format, arg );
		va_end( arg );

		giveOwnership( alloc, len );
	}

	return *this;
}

//-----------------------------------------------------------------------------
WRstr& WRstr::appendFormat( const char* format, ... )
{
	va_list arg;
	char buf[ c_formatBaseTrySize + 1 ]; // SOME space, malloc if we need a ton more

	va_start( arg, format );
	int len = vsnprintf( buf, c_formatBaseTrySize, format, arg );
	va_end( arg );

	if ( len < c_formatBaseTrySize+1 )
	{
		insert( buf, len, m_len );
	}
	else
	{
		char* alloc = (char*)g_malloc(++len);

		va_start( arg, format );
		len = vsnprintf( alloc, len, format, arg );
		va_end( arg );

		if ( m_len )
		{
			insert( alloc, len, m_len );
			g_free( alloc );
		}
		else
		{
			giveOwnership( alloc, len );
		}
	}

	return *this;
}
#endif


#ifndef WRENCH_COMBINED
#include "utils/utils.h"
#include "utils/serializer.h"
#include "utils/simple_args.h"
#include "vm/vm.h"
#include "utils/opcode.h"
#include "cc/opcode_stream.h"
#include "debug/packet.h"
#include "debug/debug.h"
#include "debug/debugger.h"
#include "cc/cc.h"
#include "lib/std_tcp.h"
#include "lib/std_serial.h"
#include "debug/debug_comm.h"
#include "lib/std_io_defs.h"
#endif

#endif
