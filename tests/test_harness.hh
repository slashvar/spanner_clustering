#ifndef TEST_HARNESS_HH_
#define TEST_HARNESS_HH_

#include <cmath>
#include <cstdio>
#include <exception>
#include <string>

struct test_failure : std::exception {
  std::string msg;
  test_failure(const char* file, int line, const char* expr)
      : msg(std::string(file) + ":" + std::to_string(line) + ": " + expr) {}
  const char* what() const noexcept override { return msg.c_str(); }
};

static int _tests_run = 0, _tests_failed = 0;

#define RUN_TEST(fn)                                  \
  do {                                                \
    _tests_run++;                                     \
    printf("[ RUN  ] %s\n", #fn);                     \
    try {                                             \
      fn();                                           \
      printf("[ PASS ] %s\n", #fn);                   \
    } catch (const test_failure& e) {                 \
      printf("[ FAIL ] %s\n", e.what());              \
      _tests_failed++;                                \
    } catch (const std::exception& e) {               \
      printf("[ FAIL ] %s: exception: %s\n", #fn, e.what()); \
      _tests_failed++;                                \
    }                                                 \
  } while (0)

#define ASSERT_TRUE(cond)                                   \
  do {                                                      \
    if (!(cond)) throw test_failure(__FILE__, __LINE__, #cond); \
  } while (0)

#define ASSERT_EQ(a, b) ASSERT_TRUE((a) == (b))

#define APPROX_EQ(a, b, eps)                                     \
  do {                                                           \
    double _a = (a), _b = (b);                                   \
    if (std::fabs(_a - _b) >= (eps))                             \
      throw test_failure(__FILE__, __LINE__,                      \
        (std::to_string(_a) + " != " + std::to_string(_b)).c_str()); \
  } while (0)

#define SUMMARY()                                                \
  do {                                                           \
    printf("\n%d test(s), %d failed\n", _tests_run, _tests_failed); \
    return _tests_failed ? 1 : 0;                                \
  } while (0)

#endif /* TEST_HARNESS_HH_ */
