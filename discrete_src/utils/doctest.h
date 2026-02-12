#ifndef DOCTEST_H
#define DOCTEST_H

#include <cstdio>
#include <sstream>
#include <string>
#include <vector>

namespace doctest
{
	struct TestFailure {};

	struct TestCase
	{
		const char* name;
		void (*func)();
	};

	inline std::vector<TestCase>& registry()
	{
		static std::vector<TestCase> tests;
		return tests;
	}

	inline int& failureCount()
	{
		static int count = 0;
		return count;
	}

	inline void addTest(const char* name, void (*func)())
	{
		TestCase t;
		t.name = name;
		t.func = func;
		registry().push_back(t);
	}

	inline void reportFailure(const char* expr, const char* file, int line)
	{
		std::printf("  FAIL: %s (%s:%d)\n", expr, file, line);
		++failureCount();
	}

	inline void message(const std::string& text)
	{
		std::printf("  NOTE: %s\n", text.c_str());
	}

	inline int runAllTests()
	{
		int total = (int)registry().size();
		int failedBefore = failureCount();

		for (int i = 0; i < total; ++i)
		{
			TestCase& t = registry()[i];
			std::printf("cross-module test [%d/%d]: %s\n", i + 1, total, t.name);
			try
			{
				t.func();
			}
			catch (const TestFailure&)
			{
				// REQUIRE() failure already counted
			}
		}

		int failed = failureCount() - failedBefore;
		if (failed == 0)
		{
			std::printf("cross-module summary: PASS (%d cases)\n", total);
			return 0;
		}

		std::printf("cross-module summary: FAIL (%d failures across %d cases)\n", failed, total);
		return 1;
	}
}

#define DOCTEST_CONCAT_IMPL(x, y) x##y
#define DOCTEST_CONCAT(x, y) DOCTEST_CONCAT_IMPL(x, y)

#define TEST_SUITE(name) namespace DOCTEST_CONCAT(doctest_suite_, __LINE__)

#define TEST_CASE(name) \
	static void DOCTEST_CONCAT(doctest_case_fn_, __LINE__)(); \
	namespace { \
		struct DOCTEST_CONCAT(doctest_case_reg_, __LINE__) { \
			DOCTEST_CONCAT(doctest_case_reg_, __LINE__)() { \
				doctest::addTest(name, &DOCTEST_CONCAT(doctest_case_fn_, __LINE__)); \
			} \
		} DOCTEST_CONCAT(doctest_case_reg_instance_, __LINE__); \
	} \
	static void DOCTEST_CONCAT(doctest_case_fn_, __LINE__)()

#define CHECK(expr) \
	do { \
		if (!(expr)) { \
			doctest::reportFailure(#expr, __FILE__, __LINE__); \
		} \
	} while (0)

#define REQUIRE(expr) \
	do { \
		if (!(expr)) { \
			doctest::reportFailure(#expr, __FILE__, __LINE__); \
			throw doctest::TestFailure(); \
		} \
	} while (0)

#define MESSAGE(expr) \
	do { \
		std::ostringstream DOCTEST_CONCAT(doctest_msg_, __LINE__); \
		DOCTEST_CONCAT(doctest_msg_, __LINE__) << expr; \
		doctest::message(DOCTEST_CONCAT(doctest_msg_, __LINE__).str()); \
	} while (0)

#ifdef DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
int main()
{
	return doctest::runAllTests();
}
#endif

#endif
