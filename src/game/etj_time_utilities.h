/*
 * MIT License
 *
 * Copyright (c) 2023 ETJump team <zero@etjump.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#ifdef min
  #undef min
#endif
#ifdef max
  #undef max
#endif
#include <iomanip>
#include <string>
#include <sstream>

#include "etj_string_utilities.h"

namespace ETJump {
struct Clock {
  int hours;
  int min;
  int sec;
  int ms;
};

struct Date {
  int year; // year
  int mon;  // month
  int day;  // day of the month
  int days; // days passed since January 1
};

struct Time {
  Clock clock;
  Date date;

  bool operator==(const Time &other) const {
    return (clock.hours == other.clock.hours && clock.min == other.clock.min &&
            clock.sec == other.clock.sec && clock.ms == other.clock.ms &&
            date.year == other.date.year && date.mon == other.date.mon &&
            date.day == other.date.day && date.days == other.date.days);
  }

  bool operator!=(const Time &other) const { return !(*this == other); }

  bool operator<(const Time &other) const {
    if (date.year != other.date.year) {
      return date.year < other.date.year;
    }
    if (date.mon != other.date.mon) {
      return date.mon < other.date.mon;
    }
    if (date.day != other.date.day) {
      return date.day < other.date.day;
    }
    if (clock.hours != other.clock.hours) {
      return clock.hours < other.clock.hours;
    }
    if (clock.min != other.clock.min) {
      return clock.min < other.clock.min;
    }
    if (clock.sec != other.clock.sec) {
      return clock.sec < other.clock.sec;
    }
    return clock.ms < other.clock.ms;
  }

  bool operator<=(const Time &other) const {
    return (*this < other) || (*this == other);
  }

  bool operator>(const Time &other) const { return !(*this <= other); }

  bool operator>=(const Time &other) const { return !(*this < other); }

  std::string toDateTimeString() const {
    return stringFormat("%04d-%02d-%02d %02d:%02d:%02d", this->date.year,
                        this->date.mon, this->date.day, this->clock.hours,
                        this->clock.min, this->clock.sec);
  }

  // https://en.cppreference.com/w/cpp/io/manip/get_time
  static Time fromString(const std::string &input,
                         const std::string &format = "%Y-%m-%d %H:%M:%S") {
    std::istringstream ss(input);
    std::tm t = {};
    ss >> std::get_time(&t, format.c_str());

    Time time;
    time.date.year = t.tm_year + 1900; 
    time.date.mon =
        t.tm_mon + 1; 
    time.date.day = t.tm_mday;
    time.date.days = t.tm_yday;

    time.clock.hours = t.tm_hour;
    time.clock.min = t.tm_min;
    time.clock.sec = t.tm_sec;

    return time;
  }
};

long long getCurrentTimestamp();
Clock getCurrentClock();
Clock toClock(long long timestamp, bool useHours);
Date getCurrentDate();
Time getCurrentTime();
} // namespace ETJump
