#ifndef Y2024_SWERVE_CONSTANTS_H
#define Y2024_SWERVE_CONSTANTS_H

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <cstdint>

#include "frc971/constants.h"

namespace y2024_swerve::constants {
struct Values {
  static const int kZeroingSampleSize = 200;

  static const int kDrivetrainWriterPriority = 35;
  static const int kDrivetrainTxPriority = 36;
  static const int kDrivetrainRxPriority = 36;

  // TODO (maxwell): Make this the real value;
  static constexpr double kDrivetrainCyclesPerRevolution() { return 512.0; }
  static constexpr double kDrivetrainEncoderRatio() { return 1.0; }

  static constexpr double kDrivetrainStatorCurrentLimit() { return 35.0; }
  static constexpr double kDrivetrainSupplyCurrentLimit() { return 60.0; }

  static constexpr double kRotationModuleRatio() {
    return 9.0 / 24.0 * 14.0 / 72.0;
  }

  static constexpr double kWheelRadius = 2.0 * 0.0254;

  static constexpr double kTranslationModuleRatio() {
    return 1.0 / 6.75 * kWheelRadius;
  }

  static constexpr double kMaxDrivetrainEncoderPulsesPerSecond() {
    return 1200000;
  }

  frc971::constants::ContinuousAbsoluteEncoderZeroingConstants
      front_left_zeroing_constants,
      front_right_zeroing_constants, back_left_zeroing_constants,
      back_right_zeroing_constants;
};
// Creates and returns a Values instance for the constants.
// Should be called before realtime because this allocates memory.
// Only the first call to either of these will be used.
Values MakeValues(uint16_t team);

// Calls MakeValues with aos::network::GetTeamNumber()
Values MakeValues();
}  // namespace y2024_swerve::constants

#endif  // Y2024_SWERVE_CONSTANTS_H
