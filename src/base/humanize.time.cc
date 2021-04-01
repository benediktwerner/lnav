/**
 * Copyright (c) 2021, Timothy Stack
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * * Neither the name of Timothy Stack nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#include <chrono>

#include "fmt/format.h"
#include "time_util.hh"
#include "humanize.time.hh"

namespace humanize {
namespace time {

using namespace std::chrono_literals;

point point::from_tv(const timeval &tv)
{
    return point(tv);
}

std::string point::as_time_ago()
{
    struct timeval current_time = this->p_recent_point
        .value_or(current_timeval());
    const char *fmt;
    char buffer[64];
    int amount;

    if (this->p_convert_to_local) {
        current_time.tv_sec = convert_log_time_to_local(current_time.tv_sec);
    }

    auto delta = std::chrono::seconds(current_time.tv_sec - this->p_past_point.tv_sec);
    if (delta < 0s) {
        return "in the future";
    } else if (delta < 1min) {
        return "just now";
    } else if (delta < 2min) {
        return "one minute ago";
    } else if (delta < 1h) {
        fmt = "%d minutes ago";
        amount = std::chrono::duration_cast<std::chrono::minutes>(delta).count();
    } else if (delta < 2h) {
        return "one hour ago";
    } else if (delta < 24h) {
        fmt = "%d hours ago";
        amount = std::chrono::duration_cast<std::chrono::hours>(delta).count();
    } else if (delta < 48h) {
        return "one day ago";
    } else if (delta < 365 * 24h) {
        fmt = "%d days ago";
        amount = delta / 24h;
    } else if (delta < (2 * 365 * 24h)) {
        return "over a year ago";
    } else {
        fmt = "over %d years ago";
        amount = delta / (365 * 24h);
    }

    snprintf(buffer, sizeof(buffer), fmt, amount);

    return std::string(buffer);
}

std::string point::as_precise_time_ago()
{
    struct timeval now, diff;

    now = this->p_recent_point.value_or(current_timeval());
    if (this->p_convert_to_local) {
        now.tv_sec = convert_log_time_to_local(now.tv_sec);
    }

    timersub(&now, &this->p_past_point, &diff);
    if (diff.tv_sec < 0) {
        return this->as_time_ago();
    } else if (diff.tv_sec <= 1) {
        return "a second ago";
    } else if (diff.tv_sec < (10 * 60)) {
        if (diff.tv_sec < 60) {
            return fmt::format("{:2} seconds ago", diff.tv_sec);
        }

        time_t seconds = diff.tv_sec % 60;
        time_t minutes = diff.tv_sec / 60;

        return fmt::format("{:2} minute{} and {:2} second{} ago",
                           minutes,
                           minutes > 1 ? "s" : "",
                           seconds,
                           seconds == 1 ? "" : "s");
    } else {
        return this->as_time_ago();
    }
}

}
}
