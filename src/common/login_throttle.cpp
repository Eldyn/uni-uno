#include "common/login_throttle.hpp"

LoginThrottle::LoginThrottle(int max_fails, std::chrono::seconds lockout)
    : max_fails_(max_fails), lockout_(lockout) {}

bool LoginThrottle::IsLockedAt(const std::string& key, Clock::time_point now) {
    auto it = entries_.find(key);
    return it != entries_.end() && now < it->second.locked_until;
}

void LoginThrottle::RecordFailureAt(const std::string& key, Clock::time_point now) {
    auto [it, inserted] = entries_.try_emplace(key, Entry{0, now, Clock::time_point{}});
    Entry& e = it->second;

    // Failures older than one window no longer count toward the threshold.
    if (!inserted && now - e.window_start > lockout_) {
        e.fails = 0;
        e.window_start = now;
    }

    if (++e.fails >= max_fails_) {
        e.locked_until = now + lockout_;
        e.fails = 0;            // the lock now gates the key; restart counting
        e.window_start = now;
    }
}

void LoginThrottle::EvictBefore(Clock::time_point now, std::chrono::seconds max_idle) {
    for (auto it = entries_.begin(); it != entries_.end();) {
        const Entry& e = it->second;
        const bool unlocked = now >= e.locked_until;
        const bool idle     = now - e.window_start > max_idle;
        if (unlocked && idle) {
            it = entries_.erase(it);
        } else {
            ++it;
        }
    }
}
