#include <doctest/doctest.h>
#include "common/login_throttle.hpp"

#include <chrono>

using Clock = LoginThrottle::Clock;
using namespace std::chrono_literals;

TEST_CASE("locks after reaching the failure threshold") {
    LoginThrottle t(3, 60s);
    const auto t0 = Clock::now();

    CHECK_FALSE(t.IsLockedAt("k", t0));
    t.RecordFailureAt("k", t0);
    t.RecordFailureAt("k", t0);
    CHECK_FALSE(t.IsLockedAt("k", t0));  // 2 failures, still allowed
    t.RecordFailureAt("k", t0);
    CHECK(t.IsLockedAt("k", t0));         // 3rd failure → locked
}

TEST_CASE("lock expires after the lockout window") {
    LoginThrottle t(2, 60s);
    const auto t0 = Clock::now();

    t.RecordFailureAt("k", t0);
    t.RecordFailureAt("k", t0);
    CHECK(t.IsLockedAt("k", t0 + 59s));
    CHECK_FALSE(t.IsLockedAt("k", t0 + 61s));
}

TEST_CASE("a successful login (Reset) clears failures") {
    LoginThrottle t(2, 60s);
    const auto t0 = Clock::now();

    t.RecordFailureAt("k", t0);
    t.Reset("k");
    t.RecordFailureAt("k", t0);
    CHECK_FALSE(t.IsLockedAt("k", t0));  // only one failure since reset
}

TEST_CASE("failures decay outside the window") {
    LoginThrottle t(2, 60s);
    const auto t0 = Clock::now();

    t.RecordFailureAt("k", t0);
    t.RecordFailureAt("k", t0 + 120s);   // first failure decayed → counts as 1
    CHECK_FALSE(t.IsLockedAt("k", t0 + 120s));
}

TEST_CASE("keys are tracked independently") {
    LoginThrottle t(1, 60s);
    const auto t0 = Clock::now();

    t.RecordFailureAt("a", t0);
    CHECK(t.IsLockedAt("a", t0));
    CHECK_FALSE(t.IsLockedAt("b", t0));
}

TEST_CASE("eviction removes stale unlocked entries") {
    LoginThrottle t(5, 60s);
    const auto t0 = Clock::now();

    t.RecordFailureAt("k", t0);
    CHECK(t.Size() == 1);
    t.EvictBefore(t0 + 1000s, 900s);
    CHECK(t.Size() == 0);
}
