#include "aos/network/testing_time_converter.h"

#include <chrono>
#include <deque>
#include <optional>
#include <tuple>

#include "aos/events/event_scheduler.h"
#include "aos/network/multinode_timestamp_filter.h"
#include "aos/time/time.h"

namespace aos {
namespace message_bridge {

namespace chrono = std::chrono;

TestingTimeConverter ::TestingTimeConverter(size_t node_count)
    : InterpolatedTimeConverter(node_count),
      last_monotonic_(node_count, monotonic_clock::epoch()) {
  CHECK_GE(node_count, 1u);
}

TestingTimeConverter::~TestingTimeConverter() {
  if (at_end_) {
    CHECK(!NextTimestamp()) << ": At the end but there is more data.";
  }
}

void TestingTimeConverter::StartEqual() {
  CHECK(first_);
  first_ = false;
  ts_.emplace_back(std::make_tuple(last_distributed_, last_monotonic_));
}

chrono::nanoseconds TestingTimeConverter::AddMonotonic(
    std::vector<monotonic_clock::duration> times) {
  CHECK_EQ(times.size(), last_monotonic_.size());
  for (size_t i = 0; i < times.size(); ++i) {
    CHECK_GT(times[i].count(), 0);
    last_monotonic_[i] += times[i];
  }
  chrono::nanoseconds dt(0);
  if (!first_) {
    dt = *std::max_element(times.begin(), times.end());
    last_distributed_ += dt;
  } else {
    first_ = false;
  }
  ts_.emplace_back(std::make_tuple(last_distributed_, last_monotonic_));
  return dt;
}

chrono::nanoseconds TestingTimeConverter::AddMonotonic(
    std::vector<monotonic_clock::time_point> times) {
  CHECK_EQ(times.size(), last_monotonic_.size());
  chrono::nanoseconds dt(0);
  if (!first_) {
    dt = times[0] - last_monotonic_[0];
    for (size_t i = 0; i < times.size(); ++i) {
      CHECK_GT(times[i], last_monotonic_[i]);
      dt = std::max(dt, times[i] - times[0]);
    }
    last_distributed_ += dt;
    last_monotonic_ = times;
  } else {
    first_ = false;
    last_monotonic_ = times;
  }
  ts_.emplace_back(std::make_tuple(last_distributed_, std::move(times)));
  return dt;
}

void TestingTimeConverter::AddNextTimestamp(
    distributed_clock::time_point time,
    std::vector<monotonic_clock::time_point> times) {
  CHECK_EQ(times.size(), last_monotonic_.size());
  if (!first_) {
    CHECK_GT(time, last_distributed_);
    for (size_t i = 0; i < times.size(); ++i) {
      CHECK_GT(times[i], last_monotonic_[i]);
    }
  } else {
    first_ = false;
  }
  last_distributed_ = time;
  last_monotonic_ = times;

  ts_.emplace_back(std::make_tuple(time, std::move(times)));
}

std::optional<std::tuple<distributed_clock::time_point,
                         std::vector<monotonic_clock::time_point>>>
TestingTimeConverter::NextTimestamp() {
  CHECK(!first_) << ": Tried to pull a timestamp before one was added.  This "
                    "is unlikely to be what you want.";
  if (ts_.empty()) {
    return std::nullopt;
  }
  auto result = ts_.front();
  ts_.pop_front();
  return result;
}

}  // namespace message_bridge
}  // namespace aos