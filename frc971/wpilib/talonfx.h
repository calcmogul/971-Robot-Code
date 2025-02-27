#ifndef FRC971_WPILIB_TALONFX_MOTOR_H_
#define FRC971_WPILIB_TALONFX_MOTOR_H_

#include <chrono>
#include <cinttypes>
#include <vector>

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "ctre/phoenix6/TalonFX.hpp"

#include "aos/commonmath.h"
#include "aos/init.h"
#include "aos/logging/logging.h"
#include "frc971/control_loops/can_talonfx_static.h"

namespace control_loops = ::frc971::control_loops;

namespace frc971::wpilib {

struct TalonFXParams {
  int device_id;
  bool inverted;
};

static constexpr units::frequency::hertz_t kCANUpdateFreqHz = 200_Hz;
static constexpr double kMaxBringupPower = 12.0;

// Class which represents a motor controlled by a TalonFX motor controller over
// CAN.
class TalonFX {
 public:
  TalonFX(int device_id, bool inverted, std::string canbus,
          std::vector<ctre::phoenix6::BaseStatusSignal *> *signals,
          double stator_current_limit, double supply_current_limit);

  TalonFX(TalonFXParams params, std::string canbus,
          std::vector<ctre::phoenix6::BaseStatusSignal *> *signals,
          double stator_current_limit, double supply_current_limit);

  void PrintConfigs();
  void WriteConfigs();

  // Currently this only uses FOC from what I understand, which we should avoid
  // using if we dont have an FOC profile for the motor itself.
  ctre::phoenix::StatusCode WriteCurrent(double current, double max_voltage);

  // The only case where we don't want to use FOC is when using Trapezoidal
  // control.
  ctre::phoenix::StatusCode WriteVoltage(double voltage,
                                         bool enable_foc = true);

  ctre::phoenix6::hardware::TalonFX *talon() { return &talon_; }

  // The position of the TalonFX output shaft is multiplied by gear_ratio
  void SerializePosition(control_loops::CANTalonFXStatic *can_falcon,
                         double gear_ratio);

  int device_id() const { return device_id_; }
  float device_temp() const { return device_temp_.GetValue().value(); }
  float supply_voltage() const { return supply_voltage_.GetValue().value(); }
  float supply_current() const { return supply_current_.GetValue().value(); }
  float torque_current() const { return torque_current_.GetValue().value(); }
  float duty_cycle() const { return duty_cycle_.GetValue().value(); }
  float position() const {
    return static_cast<units::angle::radian_t>(position_.GetValue()).value();
  }

  // returns the monotonic timestamp of the latest timesynced reading in the
  // timebase of the the syncronized CAN bus clock.
  int64_t GetTimestamp() {
    std::chrono::nanoseconds latest_timestamp =
        torque_current_.GetTimestamp().GetTime();

    return latest_timestamp.count();
  }

  void RefreshNontimesyncedSignals() { device_temp_.Refresh(); };

  void set_stator_current_limit(double stator_current_limit) {
    stator_current_limit_ = stator_current_limit;
  }

  void set_supply_current_limit(double supply_current_limit) {
    supply_current_limit_ = supply_current_limit;
  }

  void set_neutral_mode(ctre::phoenix6::signals::NeutralModeValue value) {
    neutral_mode_ = value;
  }

  static double SafeSpeed(double voltage) {
    return (::aos::Clip(voltage, -kMaxBringupPower, kMaxBringupPower) / 12.0);
  }

 private:
  ctre::phoenix6::hardware::TalonFX talon_;
  int device_id_;

  ctre::phoenix6::signals::NeutralModeValue neutral_mode_;
  ctre::phoenix6::signals::InvertedValue inverted_;

  ctre::phoenix6::StatusSignal<units::temperature::celsius_t> device_temp_;
  ctre::phoenix6::StatusSignal<units::voltage::volt_t> supply_voltage_;
  ctre::phoenix6::StatusSignal<units::current::ampere_t> supply_current_,
      torque_current_;
  ctre::phoenix6::StatusSignal<units::angle::turn_t> position_;
  ctre::phoenix6::StatusSignal<units::dimensionless::scalar_t> duty_cycle_;

  double stator_current_limit_;
  double supply_current_limit_;
};
}  // namespace frc971::wpilib
#endif  // FRC971_WPILIB_TALONFX_MOTOR_H_
