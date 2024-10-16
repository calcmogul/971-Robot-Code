#include "y2023/constants.h"

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
#include "y2023/control_loops/superstructure/roll/integral_roll_plant.h"
#include "y2023/control_loops/superstructure/wrist/integral_wrist_plant.h"

namespace y2023::constants {

Values MakeValues(uint16_t team) {
  LOG(INFO) << "creating a Constants for team: " << team;

  Values r;
  auto *const arm_proximal = &r.arm_proximal;
  auto *const arm_distal = &r.arm_distal;
  auto *const roll_joint = &r.roll_joint;
  r.wrist_flipped = true;

  arm_proximal->zeroing.average_filter_size = Values::kZeroingSampleSize;
  arm_proximal->zeroing.one_revolution_distance =
      M_PI * 2.0 * constants::Values::kProximalEncoderRatio();
  arm_proximal->zeroing.zeroing_threshold = 0.0005;
  arm_proximal->zeroing.moving_buffer_size = 20;
  arm_proximal->zeroing.allowable_encoder_error = 0.9;

  arm_distal->zeroing.average_filter_size = Values::kZeroingSampleSize;
  arm_distal->zeroing.zeroing_threshold = 0.0005;
  arm_distal->zeroing.moving_buffer_size = 20;
  arm_distal->zeroing.allowable_encoder_error = 0.9;

  roll_joint->zeroing.average_filter_size = Values::kZeroingSampleSize;
  roll_joint->zeroing.one_revolution_distance =
      M_PI * 2.0 * constants::Values::kRollJointEncoderRatio();
  roll_joint->zeroing.zeroing_threshold = 0.0005;
  roll_joint->zeroing.moving_buffer_size = 20;
  roll_joint->zeroing.allowable_encoder_error = 0.9;

  switch (team) {
    // A set of constants for tests.
    case 1:
      arm_proximal->zeroing.measured_absolute_position = 0.0;
      arm_proximal->potentiometer_offset = 0.0;

      arm_distal->zeroing.measured_absolute_position = 0.0;
      arm_distal->potentiometer_offset = 0.0;
      arm_distal->zeroing.one_revolution_distance =
          M_PI * 2.0 * constants::Values::kDistalEncoderRatio();

      roll_joint->zeroing.measured_absolute_position = 0.0;
      roll_joint->potentiometer_offset = 0.0;

      break;

    case kCompTeamNumber:
      arm_proximal->zeroing.measured_absolute_position = 0.911747959388894;
      arm_proximal->potentiometer_offset =
          10.5178592988554 + 0.0944609125285876 - 0.00826532984625095 +
          0.167359305216504 + 0.135144500925909 - 0.214909475332252 +
          0.0377032255050543;

      arm_distal->zeroing.measured_absolute_position = 0.294291930885304;
      arm_distal->potentiometer_offset =
          7.673132586937 - 0.0799284644472573 - 0.0323574039310657 +
          0.0143810684138064 + 0.00945555248207735 + 0.452446388633863 +
          0.0194863477007102 + 0.235993332670562 + 0.00138417783482921 -
          1.29562640607084 - 0.390356125757262 - 0.267002511437832 -
          0.611626839639182 + 2.55745730136924 + 0.503121678457021 +
          0.0440779746883177;

      arm_distal->zeroing.one_revolution_distance =
          M_PI * 2.0 * constants::Values::kDistalEncoderRatio() *
          (3.12725165289659 + 0.002) / 3.1485739705977704;

      roll_joint->zeroing.measured_absolute_position = 1.82824749141201;
      roll_joint->potentiometer_offset =
          0.624713611895747 + 3.10458504917251 - 0.0966407797407789 +
          0.0257708772364788 - 0.0395076737853459 - 6.87914956118006 -
          0.097581301615046 + 3.3424421683095 - 3.97605190912604 +
          0.709274294168941 - 0.0817908884966825 + 0.0420732537514303;

      break;

    case kPracticeTeamNumber:
      arm_proximal->zeroing.measured_absolute_position = 0.585858300637312;
      arm_proximal->potentiometer_offset =
          0.931355973012855 + 8.6743197253382 - 0.101200335326309 -
          0.0820901660993467 - 0.0703733798337964 - 0.0294645384848748 -
          0.577156175549626 - 0.000944609125286267 + 2.725142829769231 -
          6.38385012428248;

      arm_distal->zeroing.measured_absolute_position = 0.650190739752296;
      arm_distal->potentiometer_offset =
          0.436664933370656 + 0.49457213779426 + 6.78213223139724 -
          0.0220711555235029 - 0.0162945074111813 + 0.00630344935527365 -
          0.0164398318919943 - 0.145833494945215 + 0.234878799868491 +
          0.125924230298394 + 0.147136306208754 - 0.69167546169753 -
          0.308761538844425 + 0.610386472488493 + 0.08384162885249 +
          0.0262274735196811 + 0.5153995156153 - 0.4485275474911 -
          0.417143262506383 + 0.0808060249784878;

      arm_distal->zeroing.one_revolution_distance =
          M_PI * 2.0 * constants::Values::kDistalEncoderRatio();

      roll_joint->zeroing.measured_absolute_position = 0.79919822250646;
      roll_joint->potentiometer_offset =
          -(3.87038557084874 - 0.0241774522172967 + 0.0711345168020632 -
            0.866186131631967 - 0.0256788357596952 + 0.18101759154572017 -
            0.0208958996127179 - 0.186395903925026 + 0.45801689548395 -
            0.5935210745062 + 0.166256655718334 - 0.12591438680483 +
            0.11972765117321 - 0.318724743041507) +
          0.0201047336425017 - 1.0173426655158 - 0.186085272847293 -
          0.0317706563397807 - 2.6357823523782 + 0.871932806570122 +
          1.09682107821155 - 0.193945964842277 + 0.811834321668829 -
          0.913134567575683;

      break;

    case kCodingRobotTeamNumber:
      arm_proximal->zeroing.measured_absolute_position = 0.0;
      arm_proximal->potentiometer_offset = 0.0;

      arm_distal->zeroing.measured_absolute_position = 0.0;
      arm_distal->potentiometer_offset = 0.0;

      roll_joint->zeroing.measured_absolute_position = 0.0;
      roll_joint->potentiometer_offset = 0.0;

      break;

    default:
      LOG(FATAL) << "unknown team: " << team;

      // TODO(milind): add pot range checks once we add ranges
  }

  return r;
}

Values MakeValues() { return MakeValues(aos::network::GetTeamNumber()); }

}  // namespace y2023::constants
