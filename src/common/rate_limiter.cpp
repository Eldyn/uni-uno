#include "common/rate_limiter.hpp"

#include <algorithm>

RateLimiter::RateLimiter(double capacity, double refill_per_sec)
    : capacity_(capacity), refill_per_sec_(refill_per_sec) {}

bool RateLimiter::AllowAt(const std::string& key, Clock::time_point now) {
    // A fresh key starts with a full bucket so a first request is always allowed.
    auto [it, inserted] = buckets_.try_emplace(key, Bucket{capacity_, now});
    Bucket& bucket = it->second;

    if (!inserted) {
        const double elapsed = std::chrono::duration<double>(now - bucket.last).count();
        bucket.tokens = std::min(capacity_, bucket.tokens + elapsed * refill_per_sec_);
    }
    bucket.last = now;

    if (bucket.tokens >= 1.0) {
        bucket.tokens -= 1.0;
        return true;
    }
    return false;
}

void RateLimiter::EvictBefore(Clock::time_point now, std::chrono::seconds max_idle) {
    for (auto it = buckets_.begin(); it != buckets_.end();) {
        if (now - it->second.last > max_idle) {
            it = buckets_.erase(it);
        } else {
            ++it;
        }
    }
}
