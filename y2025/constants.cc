#include "y2025/constants.h"

#include <cinttypes>
#include <map>

#if __has_feature(address_sanitizer)
#include "sanitizer/lsan_interface.h"
#endif

#include "absl/base/call_once.h"
#include "absl/log/check.h"
#include "absl/log/log.h"

#include "aos/mutex/mutex.h"
#include "aos/network/team_number.h"

namespace y2025::constants {

Values MakeValues(uint16_t team) {
  LOG(INFO) << "creating a Constants for team: " << team;

  Values r;

  switch (team) {
    // A set of constants for tests.
    case 1:
      break;

    case kRobotTeamNumber:
      break;

    default:
      LOG(FATAL) << "unknown team: " << team;
  }

  return r;
}

Values MakeValues() { return MakeValues(aos::network::GetTeamNumber()); }

}  // namespace y2025::constants
