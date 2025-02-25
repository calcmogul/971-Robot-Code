#include "frc971/input/drivetrain_input.h"

#include <cmath>
#include <cstdio>
#include <cstring>

#include "aos/commonmath.h"
#include "aos/logging/logging.h"
#include "drivetrain_input.h"
#include "frc971/control_loops/control_loops_generated.h"
#include "frc971/control_loops/drivetrain/drivetrain_goal_generated.h"
#include "frc971/control_loops/drivetrain/drivetrain_status_generated.h"
#include "frc971/input/driver_station_data.h"

using ::frc971::input::driver_station::ButtonLocation;
using ::frc971::input::driver_station::ControlBit;
using ::frc971::input::driver_station::JoystickAxis;
using ::frc971::input::driver_station::POVLocation;

namespace drivetrain = frc971::control_loops::drivetrain;

namespace frc971::input {

const ButtonLocation kShiftHigh(2, 3), kShiftHigh2(2, 2), kShiftLow(2, 1);

void DrivetrainInputReader::HandleDrivetrain(
    const ::frc971::input::driver_station::Data &data) {
  const auto wheel_and_throttle = GetWheelAndThrottle(data);
  const double wheel = wheel_and_throttle.wheel;
  const double wheel_velocity = wheel_and_throttle.wheel_velocity;
  const double wheel_torque = wheel_and_throttle.wheel_torque;
  const double throttle = wheel_and_throttle.throttle;
  const double throttle_velocity = wheel_and_throttle.throttle_velocity;
  const double throttle_torque = wheel_and_throttle.throttle_torque;
  const bool high_gear = wheel_and_throttle.high_gear;

  drivetrain_status_fetcher_.Fetch();
  if (drivetrain_status_fetcher_.get()) {
    robot_velocity_ = drivetrain_status_fetcher_->robot_speed();
  }

  // If we have a vision align function, and it is in control, don't run the
  // normal driving code.
  if (vision_align_fn_) {
    if (vision_align_fn_(data)) {
      return;
    }
  }

  bool is_control_loop_driving = false;
  bool is_line_following = false;

  if (data.IsPressed(turn1_)) {
    switch (turn1_use_) {
      case TurnButtonUse::kControlLoopDriving:
        is_control_loop_driving = true;
        break;
      case TurnButtonUse::kLineFollow:
        is_line_following = true;
        break;
    }
  }

  if (data.IsPressed(turn2_)) {
    switch (turn2_use_) {
      case TurnButtonUse::kControlLoopDriving:
        is_control_loop_driving = true;
        break;
      case TurnButtonUse::kLineFollow:
        is_line_following = true;
        break;
    }
  }

  if (drivetrain_status_fetcher_.get()) {
    if (is_control_loop_driving && !last_is_control_loop_driving_) {
      left_goal_ = drivetrain_status_fetcher_->estimated_left_position() +
                   wheel * wheel_multiplier_;
      right_goal_ = drivetrain_status_fetcher_->estimated_right_position() -
                    wheel * wheel_multiplier_;
    }
  }

  const double current_left_goal =
      left_goal_ - wheel * wheel_multiplier_ + throttle * 0.3;
  const double current_right_goal =
      right_goal_ + wheel * wheel_multiplier_ + throttle * 0.3;
  auto builder = drivetrain_goal_sender_.MakeBuilder();

  frc971::ProfileParameters::Builder linear_builder =
      builder.MakeBuilder<frc971::ProfileParameters>();

  linear_builder.add_max_velocity(3.0);
  linear_builder.add_max_acceleration(20.0);

  flatbuffers::Offset<frc971::ProfileParameters> linear_offset =
      linear_builder.Finish();

  auto goal_builder = builder.MakeBuilder<drivetrain::Goal>();
  goal_builder.add_wheel(wheel);
  goal_builder.add_wheel_velocity(wheel_velocity);
  goal_builder.add_wheel_torque(wheel_torque);
  goal_builder.add_throttle(throttle);
  goal_builder.add_throttle_velocity(throttle_velocity);
  goal_builder.add_throttle_torque(throttle_torque);
  goal_builder.add_highgear(high_gear);
  goal_builder.add_quickturn(data.IsPressed(quick_turn_));
  goal_builder.add_controller_type(
      is_line_following ? drivetrain::ControllerType::LINE_FOLLOWER
                        : (is_control_loop_driving
                               ? drivetrain::ControllerType::MOTION_PROFILE
                               : drivetrain::ControllerType::POLYDRIVE));
  goal_builder.add_left_goal(current_left_goal);
  goal_builder.add_right_goal(current_right_goal);
  goal_builder.add_linear(linear_offset);

  if (builder.Send(goal_builder.Finish()) != aos::RawSender::Error::kOk) {
    AOS_LOG(WARNING, "sending stick values failed\n");
  }

  last_is_control_loop_driving_ = is_control_loop_driving;
}

void SwerveDrivetrainInputReader::HandleDrivetrain(
    const ::frc971::input::driver_station::Data &data) {
  const auto swerve_goals = GetSwerveGoals(data);
  const double vx = swerve_goals.vx;
  const double vy = swerve_goals.vy;
  const double omega = swerve_goals.omega;

  auto builder = goal_sender_.MakeStaticBuilder();

  auto joystick_goal = builder->add_joystick_goal();

  joystick_goal->set_vx(vx);
  joystick_goal->set_vy(vy);
  joystick_goal->set_omega(omega);
  joystick_goal->set_auto_align(auto_align_);

  builder.CheckOk(builder.Send());
}

std::unique_ptr<SwerveDrivetrainInputReader> SwerveDrivetrainInputReader::Make(
    ::aos::EventLoop *event_loop, const SwerveConfig swerve_config) {
  // Swerve Controller
  // axis (2, 2) will give you alternative omega axis (controls with vertical
  // movement)
  const JoystickAxis kVxAxis(3, 1), kVyAxis(1, 1), kOmegaAxis(1, 2);
  const ButtonLocation kAutoAlignButton(2, 6);

  std::unique_ptr<SwerveDrivetrainInputReader> result(
      new SwerveDrivetrainInputReader(event_loop, swerve_config, kVxAxis,
                                      kVyAxis, kOmegaAxis, kAutoAlignButton));
  return result;
}

double JoystickCurve(double x) {
  return std::copysign(std::abs(std::pow(x, 2)), x);
}

SwerveDrivetrainInputReader::SwerveGoals
SwerveDrivetrainInputReader::GetSwerveGoals(
    const ::frc971::input::driver_station::Data &data) {
  // xbox
  constexpr double kMovementDeadband = 0.0;
  constexpr double kRotationDeadband = 0.0;
  constexpr double kVelScale = 12.0;
  constexpr double kOmegaScale = 5.0;
  constexpr double kRoundtoOneThreshold = 0.95;

  double raw_omega = data.GetAxis(omega_axis_);

  double raw_vx = data.GetAxis(vx_axis_) + (swerve_config_.vx_offset / 8.0);

  double raw_vy = -data.GetAxis(vy_axis_) + (swerve_config_.vy_offset / 8.0);

  if (std::abs(raw_vx) > kRoundtoOneThreshold) {
    raw_vx = std::copysign(1.0, raw_vx);
  }
  if (std::abs(raw_vy) > kRoundtoOneThreshold) {
    raw_vy = std::copysign(1.0, raw_vy);
  }
  if (std::abs(raw_omega) > kRoundtoOneThreshold) {
    raw_omega = std::copysign(1.0, raw_omega);
  }

  const double omega =
      kOmegaScale * aos::Deadband(pow(raw_omega, 1), kRotationDeadband, 1.0) +
      swerve_config_.omega_offset;

  // Link to the cubicish function: https://www.desmos.com/3d/shvumybi1g
  // TODO add a deadband (currently there is none)
  const double speed =
      kVelScale *
      aos::Deadband(std::hypot(JoystickCurve(raw_vx), JoystickCurve(raw_vy)),
                    kMovementDeadband, 1.0);

  const double theta = std::atan2(raw_vy, raw_vx);

  const double vx = -speed * std::cos(theta);
  const double vy = speed * std::sin(theta);

  if (data.PosEdge(auto_align_button_)) {
    auto_align_ = true;
  } else if (data.NegEdge(auto_align_button_)) {
    auto_align_ = false;
  }

  return SwerveDrivetrainInputReader::SwerveGoals{vx, vy, omega};
}

DrivetrainInputReader::WheelAndThrottle
SteeringWheelDrivetrainInputReader::GetWheelAndThrottle(
    const ::frc971::input::driver_station::Data &data) {
  const double wheel = -data.GetAxis(wheel_);
  const double throttle = -data.GetAxis(throttle_);

  if (!data.GetControlBit(ControlBit::kEnabled)) {
    high_gear_ = default_high_gear_;
  }

  if (data.PosEdge(kShiftLow)) {
    high_gear_ = false;
  }

  if (data.PosEdge(kShiftHigh) || data.PosEdge(kShiftHigh2)) {
    high_gear_ = true;
  }

  return DrivetrainInputReader::WheelAndThrottle{
      wheel, 0.0, 0.0, throttle, 0.0, 0.0, high_gear_};
}

double UnwrappedAxis(const ::frc971::input::driver_station::Data &data,
                     const JoystickAxis &high_bits,
                     const JoystickAxis &low_bits) {
  const float high_bits_data = data.GetAxis(high_bits);
  const float low_bits_data = data.GetAxis(low_bits);
  const int16_t high_bits_data_int =
      (high_bits_data < 0.0f ? high_bits_data * 128.0f
                             : high_bits_data * 127.0f);
  const int16_t low_bits_data_int =
      (low_bits_data < 0.0f ? low_bits_data * 128.0f : low_bits_data * 127.0f);

  const uint16_t high_bits_data_uint =
      ((static_cast<uint16_t>(high_bits_data_int) & 0xff) + 0x80) & 0xff;
  const uint16_t low_bits_data_uint =
      ((static_cast<uint16_t>(low_bits_data_int) & 0xff) + 0x80) & 0xff;

  const uint16_t data_uint = (high_bits_data_uint << 8) | low_bits_data_uint;

  const int32_t data_int = static_cast<int32_t>(data_uint) - 0x8000;

  if (data_int < 0) {
    return static_cast<double>(data_int) / static_cast<double>(0x8000);
  } else {
    return static_cast<double>(data_int) / static_cast<double>(0x7fff);
  }
}

DrivetrainInputReader::WheelAndThrottle
PistolDrivetrainInputReader::GetWheelAndThrottle(
    const ::frc971::input::driver_station::Data &data) {
  const double wheel = -UnwrappedAxis(data, wheel_, wheel_low_);
  const double wheel_velocity =
      -UnwrappedAxis(data, wheel_velocity_high_, wheel_velocity_low_) * 50.0;
  const double wheel_torque =
      -UnwrappedAxis(data, wheel_torque_high_, wheel_torque_low_) / 2.0;

  double throttle = UnwrappedAxis(data, throttle_, throttle_low_);
  const double throttle_velocity =
      UnwrappedAxis(data, throttle_velocity_high_, throttle_velocity_low_) *
      50.0;
  const double throttle_torque =
      UnwrappedAxis(data, throttle_torque_high_, throttle_torque_low_) / 2.0;

  // TODO(austin): Deal with haptics here.
  if (throttle < 0) {
    throttle = ::std::max(-1.0, throttle / 0.7);
  }

  if (data.IsPressed(slow_down_)) {
    throttle *= 0.5;
  }

  if (!data.GetControlBit(ControlBit::kEnabled)) {
    high_gear_ = default_high_gear_;
  }

  if (data.PosEdge(shift_low_)) {
    high_gear_ = false;
  }

  if (data.PosEdge(shift_high_)) {
    high_gear_ = true;
  }

  // Emprically, the current pistol grip tends towards steady-state errors at
  // ~0.01-0.02 on both the wheel/throttle. Having the throttle correctly snap
  // to zero is more important than the wheel for our internal logic, so force a
  // deadband there.
  constexpr double kThrottleDeadband = 0.05;
  throttle = aos::Deadband(throttle, kThrottleDeadband, 1.0);

  return DrivetrainInputReader::WheelAndThrottle{
      wheel,     wheel_velocity,    wheel_torque,
      throttle,  throttle_velocity, throttle_torque,
      high_gear_};
}

DrivetrainInputReader::WheelAndThrottle
XboxDrivetrainInputReader::GetWheelAndThrottle(
    const ::frc971::input::driver_station::Data &data) {
  // xbox
  constexpr double kWheelDeadband = 0.05;
  constexpr double kThrottleDeadband = 0.05;
  const double wheel =
      aos::Deadband(-data.GetAxis(wheel_), kWheelDeadband, 1.0);

  const double unmodified_throttle =
      aos::Deadband(-data.GetAxis(throttle_), kThrottleDeadband, 1.0);

  // Apply a sin function that's scaled to make it feel better.
  constexpr double throttle_range = M_PI_2 * 0.9;

  double throttle = ::std::sin(throttle_range * unmodified_throttle) /
                    ::std::sin(throttle_range);
  throttle = ::std::sin(throttle_range * throttle) / ::std::sin(throttle_range);
  throttle = 2.0 * unmodified_throttle - throttle;
  return DrivetrainInputReader::WheelAndThrottle{wheel, 0.0, 0.0, throttle,
                                                 0.0,   0.0, true};
}

std::unique_ptr<SteeringWheelDrivetrainInputReader>
SteeringWheelDrivetrainInputReader::Make(::aos::EventLoop *event_loop,
                                         bool default_high_gear) {
  const JoystickAxis kSteeringWheel(1, 1), kDriveThrottle(2, 2);
  const ButtonLocation kQuickTurn(1, 5);
  const ButtonLocation kTurn1(1, 7);
  const ButtonLocation kTurn2(1, 11);
  std::unique_ptr<SteeringWheelDrivetrainInputReader> result(
      new SteeringWheelDrivetrainInputReader(
          event_loop, kSteeringWheel, kDriveThrottle, kQuickTurn, kTurn1,
          TurnButtonUse::kControlLoopDriving, kTurn2,
          TurnButtonUse::kControlLoopDriving));
  result.get()->set_default_high_gear(default_high_gear);

  return result;
}

std::unique_ptr<PistolDrivetrainInputReader> PistolDrivetrainInputReader::Make(
    ::aos::EventLoop *event_loop, bool default_high_gear,
    PistolTopButtonUse top_button_use, PistolSecondButtonUse second_button_use,
    PistolBottomButtonUse bottom_button_use) {
  // Pistol Grip controller
  const JoystickAxis kTriggerHigh(1, 1), kTriggerLow(1, 4),
      kTriggerVelocityHigh(1, 2), kTriggerVelocityLow(1, 5),
      kTriggerTorqueHigh(1, 3), kTriggerTorqueLow(1, 6);

  const JoystickAxis kWheelHigh(2, 1), kWheelLow(2, 4),
      kWheelVelocityHigh(2, 2), kWheelVelocityLow(2, 5), kWheelTorqueHigh(2, 3),
      kWheelTorqueLow(2, 6);

  const ButtonLocation kQuickTurn(1, 3);

  const ButtonLocation kTopButton(1, 1);

  const ButtonLocation kSecondButton(1, 2);
  const ButtonLocation kBottomButton(1, 4);
  // Non-existant button for nops.
  const ButtonLocation kDummyButton(1, 15);

  // TODO(james): Make a copy assignment operator for ButtonLocation so we don't
  // have to shoehorn in these ternary operators.
  const ButtonLocation kTurn1 =
      (second_button_use == PistolSecondButtonUse::kTurn1) ? kSecondButton
                                                           : kDummyButton;
  // Turn2 does closed loop driving.
  const ButtonLocation kTurn2 =
      (top_button_use == PistolTopButtonUse::kLineFollow) ? kTopButton
                                                          : kDummyButton;

  const ButtonLocation kShiftHigh =
      (top_button_use == PistolTopButtonUse::kShift) ? kTopButton
                                                     : kDummyButton;
  const ButtonLocation kShiftLow =
      (second_button_use == PistolSecondButtonUse::kShiftLow) ? kSecondButton
                                                              : kDummyButton;

  std::unique_ptr<PistolDrivetrainInputReader> result(
      new PistolDrivetrainInputReader(
          event_loop, kWheelHigh, kWheelLow, kTriggerVelocityHigh,
          kTriggerVelocityLow, kTriggerTorqueHigh, kTriggerTorqueLow,
          kTriggerHigh, kTriggerLow, kWheelVelocityHigh, kWheelVelocityLow,
          kWheelTorqueHigh, kWheelTorqueLow, kQuickTurn, kShiftHigh, kShiftLow,
          kTurn1,
          (bottom_button_use == PistolBottomButtonUse::kControlLoopDriving)
              ? kBottomButton
              : kTurn2,
          (top_button_use == PistolTopButtonUse::kNone) ? kTopButton
                                                        : kBottomButton));

  result->set_default_high_gear(default_high_gear);
  return result;
}

std::unique_ptr<XboxDrivetrainInputReader> XboxDrivetrainInputReader::Make(
    ::aos::EventLoop *event_loop) {
  // xbox
  const JoystickAxis kSteeringWheel(1, 5), kDriveThrottle(1, 2);
  const ButtonLocation kQuickTurn(1, 5);

  // Nop
  const ButtonLocation kTurn1(1, 1);
  const ButtonLocation kTurn2(1, 2);

  std::unique_ptr<XboxDrivetrainInputReader> result(
      new XboxDrivetrainInputReader(event_loop, kSteeringWheel, kDriveThrottle,
                                    kQuickTurn, kTurn1,
                                    TurnButtonUse::kControlLoopDriving, kTurn2,
                                    TurnButtonUse::kControlLoopDriving));
  return result;
}
::std::unique_ptr<DrivetrainInputReader> DrivetrainInputReader::Make(
    ::aos::EventLoop *event_loop, InputType type,
    const drivetrain::DrivetrainConfig<double> &dt_config) {
  std::unique_ptr<DrivetrainInputReader> drivetrain_input_reader;

  using InputType = DrivetrainInputReader::InputType;

  switch (type) {
    case InputType::kSteeringWheel:
      drivetrain_input_reader = SteeringWheelDrivetrainInputReader::Make(
          event_loop, dt_config.default_high_gear);
      break;
    case InputType::kPistol: {
      // For backwards compatibility
      PistolTopButtonUse top_button_use =
          dt_config.pistol_grip_shift_enables_line_follow
              ? PistolTopButtonUse::kLineFollow
              : dt_config.top_button_use;

      PistolSecondButtonUse second_button_use = dt_config.second_button_use;
      PistolBottomButtonUse bottom_button_use = dt_config.bottom_button_use;

      if (top_button_use == PistolTopButtonUse::kLineFollow) {
        second_button_use = PistolSecondButtonUse::kTurn1;
      }

      drivetrain_input_reader = PistolDrivetrainInputReader::Make(
          event_loop, dt_config.default_high_gear, top_button_use,
          second_button_use, bottom_button_use);
      break;
    }
    case InputType::kXbox:
      drivetrain_input_reader = XboxDrivetrainInputReader::Make(event_loop);
      break;
  }
  return drivetrain_input_reader;
}

}  // namespace frc971::input
