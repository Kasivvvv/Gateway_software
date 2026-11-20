/**@file
 * This file is part of the CANopen Library Unit Test Suite.
 *
 * @copyright 2026 N7 Space Sp. z o.o.
 *
 * Unit Test Suite was developed under a programme of,
 * and funded by, the European Space Agency.
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cerrno>

#include <CppUTest/TestHarness.h>

#include <lely/compat/time.h>

namespace {
int clock_gettime_calls = 0;
clockid_t clock_gettime_clock_id = -1;
bool clock_gettime_fail = false;

void
ResetClockGettimeWrap() {
  clock_gettime_calls = 0;
  clock_gettime_clock_id = -1;
  clock_gettime_fail = false;
}

bool
TimespecLessOrEqual(const timespec& lhs, const timespec& rhs) {
  return lhs.tv_sec < rhs.tv_sec ||
         (lhs.tv_sec == rhs.tv_sec && lhs.tv_nsec <= rhs.tv_nsec);
}
}  // namespace

#if LELY_UNIT_TEST_COMPAT_TIME_WRAP

// The compatibility timespec_get() is built into this test (see compat-time.c)
// and linked with -Wl,--wrap=clock_gettime, so that its calls to
// clock_gettime() can be observed and made to fail.

extern "C" {

int __real_clock_gettime(clockid_t clock_id, struct timespec* tp);
int __wrap_clock_gettime(clockid_t clock_id, struct timespec* tp);

int
__wrap_clock_gettime(const clockid_t clock_id, struct timespec* const tp) {
  ++clock_gettime_calls;
  clock_gettime_clock_id = clock_id;
  if (clock_gettime_fail) {
    errno = EINVAL;
    return -1;
  }
  return __real_clock_gettime(clock_id, tp);
}

}  // extern "C"

#endif  // LELY_UNIT_TEST_COMPAT_TIME_WRAP

TEST_GROUP(Compat_Time) {
  timespec ts = {0, 0};

  TEST_SETUP() { ResetClockGettimeWrap(); }
  TEST_TEARDOWN() { ResetClockGettimeWrap(); }
};

/// @name timespec_get()
///@{

/// \Given N/A
///
/// \When timespec_get() is called with the TIME_UTC base
///
/// \Then TIME_UTC is returned, the time interval is set to a valid value
///       between two CLOCK_REALTIME readings taken before and after the call
///       \Calls clock_gettime()
TEST(Compat_Time, TimespecGet_TimeUtc) {
  timespec before = {0, 0};
  timespec after = {0, 0};

  CHECK_EQUAL(0, clock_gettime(CLOCK_REALTIME, &before));
#if LELY_UNIT_TEST_COMPAT_TIME_WRAP
  const int calls = clock_gettime_calls;
#endif
  const int ret = timespec_get(&ts, TIME_UTC);
#if LELY_UNIT_TEST_COMPAT_TIME_WRAP
  CHECK_EQUAL(calls + 1, clock_gettime_calls);
  CHECK_EQUAL(CLOCK_REALTIME, clock_gettime_clock_id);
#endif
  CHECK_EQUAL(0, clock_gettime(CLOCK_REALTIME, &after));

  CHECK_EQUAL(TIME_UTC, ret);
  CHECK_COMPARE(ts.tv_nsec, >=, 0);
  CHECK_COMPARE(ts.tv_nsec, <, 1000000000L);
  CHECK(TimespecLessOrEqual(before, ts));
  CHECK(TimespecLessOrEqual(ts, after));
}

/// \Given N/A
///
/// \When timespec_get() is called with a base other than TIME_UTC
///
/// \Then 0 is returned, the time interval is not modified
TEST(Compat_Time, TimespecGet_InvalidBase) {
  ts = {1, 2};

#if LELY_UNIT_TEST_COMPAT_TIME_WRAP
  const int calls = clock_gettime_calls;
#endif
  const int ret = timespec_get(&ts, TIME_UTC + 1);
#if LELY_UNIT_TEST_COMPAT_TIME_WRAP
  CHECK_EQUAL(calls, clock_gettime_calls);
#endif

  CHECK_EQUAL(0, ret);
  CHECK_EQUAL(1, ts.tv_sec);
  CHECK_EQUAL(2, ts.tv_nsec);
}

#if LELY_UNIT_TEST_COMPAT_TIME_WRAP
/// \Given clock_gettime() failing
///
/// \When timespec_get() is called with the TIME_UTC base
///
/// \Then 0 is returned
///       \Calls clock_gettime()
TEST(Compat_Time, TimespecGet_ClockGettimeError) {
  clock_gettime_fail = true;

  const int calls = clock_gettime_calls;
  const int ret = timespec_get(&ts, TIME_UTC);

  CHECK_EQUAL(0, ret);
  CHECK_EQUAL(calls + 1, clock_gettime_calls);
  CHECK_EQUAL(CLOCK_REALTIME, clock_gettime_clock_id);
}
#endif  // LELY_UNIT_TEST_COMPAT_TIME_WRAP

///@}
