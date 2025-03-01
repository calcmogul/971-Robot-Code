#ifndef AOS_INPUT_DRIVETRAIN_INPUT_H_
#define AOS_INPUT_DRIVETRAIN_INPUT_H_

#include <cmath>
#include <cstdio>
#include <cstring>
#include <memory>

#include "aos/logging/logging.h"
#include "frc971/control_loops/control_loops_generated.h"
#include "frc971/control_loops/drivetrain/drivetrain_config.h"
#include "frc971/control_loops/drivetrain/drivetrain_goal_generated.h"
#include "frc971/control_loops/drivetrain/drivetrain_status_generated.h"
#include "frc971/control_loops/swerve/swerve_drivetrain_goal_static.h"
#include "frc971/control_loops/swerve/swerve_drivetrain_joystick_goal_static.h"
#include "frc971/input/driver_station_data.h"

namespace frc971::input {

using control_loops::drivetrain::PistolBottomButtonUse;
using control_loops::drivetrain::PistolSecondButtonUse;
using control_loops::drivetrain::PistolTopButtonUse;

// We have a couple different joystick configurations used to drive our skid
// steer robots.  These configurations don't change very often, and there are a
// small, discrete, set of them.  The interface to the drivetrain is the same
// across all of our drivetrains, so we can abstract that away from our users.
//
// We want to be able to re-use that code across years, and switch joystick
// types quickly and efficiently.
//
// DrivetrainInputReader is the abstract base class which provides a consistent
// API to control drivetrains.
//
// To construct the appropriate DrivetrainInputReader, call
// DrivetrainInputReader::Make with the input type enum.
// To use it, call HandleDrivetrain(data) every time a joystick packet is
// received.
//
// Base class for the interface to the drivetrain implemented by multiple
// joystick types.
class DrivetrainInputReader {
 public:
  // What to use the turn1/2 buttons for.
  enum class TurnButtonUse {
    // Use the button to enable control loop driving.
    kControlLoopDriving,
    // Use the button to set line following mode.
    kLineFollow,
  };
  // Inputs driver station button and joystick locations
  DrivetrainInputReader(::aos::EventLoop *event_loop,
                        driver_station::JoystickAxis wheel,
                        driver_station::JoystickAxis throttle,
                        driver_station::ButtonLocation quick_turn,
                        driver_station::ButtonLocation turn1,
                        TurnButtonUse turn1_use,
                        driver_station::ButtonLocation turn2,
                        TurnButtonUse turn2_use)
      : wheel_(wheel),
        throttle_(throttle),
        quick_turn_(quick_turn),
        turn1_(turn1),
        turn1_use_(turn1_use),
        turn2_(turn2),
        turn2_use_(turn2_use),
        drivetrain_status_fetcher_(
            event_loop
                ->MakeFetcher<::frc971::control_loops::drivetrain::Status>(
                    "/drivetrain")),
        drivetrain_goal_sender_(
            event_loop->MakeSender<::frc971::control_loops::drivetrain::Goal>(
                "/drivetrain")) {}

  virtual ~DrivetrainInputReader() = default;

  // List of driver joystick types.
  enum class InputType {
    kSteeringWheel,
    kPistol,
    kXbox,
  };

  // Constructs the appropriate DrivetrainInputReader.
  static std::unique_ptr<DrivetrainInputReader> Make(
      ::aos::EventLoop *event_loop, InputType type,
      const ::frc971::control_loops::drivetrain::DrivetrainConfig<double>
          &dt_config);

  // Processes new joystick data and publishes drivetrain goal messages.
  void HandleDrivetrain(const ::frc971::input::driver_station::Data &data);

  // Sets the scalar for the steering wheel for closed loop mode converting
  // steering ratio to meters displacement on the two wheels.
  void set_wheel_multiplier(double wheel_multiplier) {
    wheel_multiplier_ = wheel_multiplier;
  }

  // Returns the current robot velocity in m/s.
  double robot_velocity() const { return robot_velocity_; }

  // Returns the current drivetrain status.
  const control_loops::drivetrain::Status *drivetrain_status() const {
    return drivetrain_status_fetcher_.get();
  }

  void set_vision_align_fn(
      ::std::function<bool(const ::frc971::input::driver_station::Data &data)>
          vision_align_fn) {
    vision_align_fn_ = vision_align_fn;
  }

 protected:
  const driver_station::JoystickAxis wheel_;
  const driver_station::JoystickAxis throttle_;
  const driver_station::ButtonLocation quick_turn_;
  // Button for enabling control loop driving.
  const driver_station::ButtonLocation turn1_;
  const TurnButtonUse turn1_use_;
  // But for enabling line following.
  const driver_station::ButtonLocation turn2_;
  const TurnButtonUse turn2_use_;

  bool last_is_control_loop_driving_ = false;

  // Structure containing the (potentially adjusted) steering and throttle
  // values from the joysticks.
  struct WheelAndThrottle {
    double wheel;
    double wheel_velocity;
    double wheel_torque;
    double throttle;
    double throttle_velocity;
    double throttle_torque;
    bool high_gear;
  };

 private:
  // Computes the steering and throttle from the provided driverstation data.
  virtual WheelAndThrottle GetWheelAndThrottle(
      const ::frc971::input::driver_station::Data &data) = 0;

  ::aos::Fetcher<::frc971::control_loops::drivetrain::Status>
      drivetrain_status_fetcher_;
  ::aos::Sender<::frc971::control_loops::drivetrain::Goal>
      drivetrain_goal_sender_;

  double robot_velocity_ = 0.0;
  // Goals to send to the drivetrain in closed loop mode.
  double left_goal_ = 0.0;
  double right_goal_ = 0.0;
  // The scale for the joysticks for closed loop mode converting
  // joysticks to meters displacement on the two wheels.
  double wheel_multiplier_ = 0.5;

  ::std::function<bool(const ::frc971::input::driver_station::Data &data)>
      vision_align_fn_;
};

class SwerveDrivetrainInputReader {
 public:
  virtual ~SwerveDrivetrainInputReader() = default;

  struct SwerveConfig {
    // These values are used for zeroing the x and y part of the controller.
    double vx_offset;
    double vy_offset;
    double omega_offset;
  };

  // Constructs the appropriate DrivetrainInputReader.
  static std::unique_ptr<SwerveDrivetrainInputReader> Make(
      ::aos::EventLoop *event_loop, const SwerveConfig swerve_config);

  // Processes new joystick data and publishes drivetrain goal messages.
  void HandleDrivetrain(const ::frc971::input::driver_station::Data &data);

  // Sets the scalar for the steering wheel for closed loop mode converting
  // steering ratio to meters displacement on the two wheels.
  void set_wheel_multiplier(double wheel_multiplier) {
    wheel_multiplier_ = wheel_multiplier;
  }

 protected:
  const driver_station::JoystickAxis vx_axis_;
  const driver_station::JoystickAxis vy_axis_;
  const driver_station::JoystickAxis omega_axis_;
  const driver_station::ButtonLocation auto_align_button_;
  const driver_station::ButtonLocation foc_override_button_;

  // Structure containing the (potentially adjusted) steering and throttle
  // values from the joysticks.
  struct SwerveGoals {
    double vx;
    double vy;
    double omega;
  };

 private:
  SwerveDrivetrainInputReader(
      ::aos::EventLoop *event_loop, const SwerveConfig swerve_config,
      driver_station::JoystickAxis vx_axis,
      driver_station::JoystickAxis vy_axis,
      driver_station::JoystickAxis omega_axis,
      driver_station::ButtonLocation auto_align_button,
      driver_station::ButtonLocation foc_override_button)
      : vx_axis_(vx_axis),
        vy_axis_(vy_axis),
        omega_axis_(omega_axis),
        auto_align_button_(auto_align_button),
        foc_override_button_(foc_override_button),
        goal_sender_(event_loop->MakeSender<control_loops::swerve::GoalStatic>(
            "/drivetrain")),
        joystick_state_fetcher_(
            event_loop->MakeFetcher<::aos::JoystickState>("/aos")),
        swerve_config_(swerve_config) {}
  // Computes the steering and throttle from the provided driverstation data.
  SwerveGoals GetSwerveGoals(const ::frc971::input::driver_station::Data &data);

  ::aos::Sender<control_loops::swerve::GoalStatic> goal_sender_;

  ::aos::Fetcher<aos::JoystickState> joystick_state_fetcher_;

  // The scale for the joysticks for closed loop mode converting
  // joysticks to meters displacement on the two wheels.
  double wheel_multiplier_ = 0.5;

  const SwerveConfig swerve_config_;

  bool auto_align_ = false;
  bool foc_override_ = false;
};

// Implements DrivetrainInputReader for the original steering wheel.
class SteeringWheelDrivetrainInputReader : public DrivetrainInputReader {
 public:
  using DrivetrainInputReader::DrivetrainInputReader;

  // Creates a DrivetrainInputReader with the corresponding joystick ports and
  // axis for the big steering wheel and throttle stick.
  static std::unique_ptr<SteeringWheelDrivetrainInputReader> Make(
      ::aos::EventLoop *event_loop, bool default_high_gear);

  // Sets the default shifter position
  void set_default_high_gear(bool default_high_gear) {
    high_gear_ = default_high_gear;
    default_high_gear_ = default_high_gear;
  }

 private:
  WheelAndThrottle GetWheelAndThrottle(
      const ::frc971::input::driver_station::Data &data) override;
  bool high_gear_;
  bool default_high_gear_;
};

class PistolDrivetrainInputReader : public DrivetrainInputReader {
 public:
  using DrivetrainInputReader::DrivetrainInputReader;

  // Creates a DrivetrainInputReader with the corresponding joystick ports and
  // axis for the (cheap) pistol grip controller.
  static std::unique_ptr<PistolDrivetrainInputReader> Make(
      ::aos::EventLoop *event_loop, bool default_high_gear,
      PistolTopButtonUse top_button_use,
      PistolSecondButtonUse second_button_use,
      PistolBottomButtonUse bottom_button_use);

 private:
  PistolDrivetrainInputReader(
      ::aos::EventLoop *event_loop, driver_station::JoystickAxis wheel_high,
      driver_station::JoystickAxis wheel_low,
      driver_station::JoystickAxis wheel_velocity_high,
      driver_station::JoystickAxis wheel_velocity_low,
      driver_station::JoystickAxis wheel_torque_high,
      driver_station::JoystickAxis wheel_torque_low,
      driver_station::JoystickAxis throttle_high,
      driver_station::JoystickAxis throttle_low,
      driver_station::JoystickAxis throttle_velocity_high,
      driver_station::JoystickAxis throttle_velocity_low,
      driver_station::JoystickAxis throttle_torque_high,
      driver_station::JoystickAxis throttle_torque_low,
      driver_station::ButtonLocation quick_turn,
      driver_station::ButtonLocation shift_high,
      driver_station::ButtonLocation shift_low,
      driver_station::ButtonLocation turn1,
      driver_station::ButtonLocation turn2,
      driver_station::ButtonLocation slow_down)
      : DrivetrainInputReader(event_loop, wheel_high, throttle_high, quick_turn,
                              turn1, TurnButtonUse::kLineFollow, turn2,
                              TurnButtonUse::kControlLoopDriving),
        wheel_low_(wheel_low),
        wheel_velocity_high_(wheel_velocity_high),
        wheel_velocity_low_(wheel_velocity_low),
        wheel_torque_high_(wheel_torque_high),
        wheel_torque_low_(wheel_torque_low),
        throttle_low_(throttle_low),
        throttle_velocity_high_(throttle_velocity_high),
        throttle_velocity_low_(throttle_velocity_low),
        throttle_torque_high_(throttle_torque_high),
        throttle_torque_low_(throttle_torque_low),
        shift_high_(shift_high),
        shift_low_(shift_low),
        slow_down_(slow_down) {}

  WheelAndThrottle GetWheelAndThrottle(
      const ::frc971::input::driver_station::Data &data) override;

  // Sets the default shifter position
  void set_default_high_gear(bool default_high_gear) {
    high_gear_ = default_high_gear;
    default_high_gear_ = default_high_gear;
  }

  const driver_station::JoystickAxis wheel_low_;
  const driver_station::JoystickAxis wheel_velocity_high_;
  const driver_station::JoystickAxis wheel_velocity_low_;
  const driver_station::JoystickAxis wheel_torque_high_;
  const driver_station::JoystickAxis wheel_torque_low_;

  const driver_station::JoystickAxis throttle_low_;
  const driver_station::JoystickAxis throttle_velocity_high_;
  const driver_station::JoystickAxis throttle_velocity_low_;
  const driver_station::JoystickAxis throttle_torque_high_;
  const driver_station::JoystickAxis throttle_torque_low_;

  const driver_station::ButtonLocation shift_high_;
  const driver_station::ButtonLocation shift_low_;
  const driver_station::ButtonLocation slow_down_;

  bool high_gear_;
  bool default_high_gear_;
};

class XboxDrivetrainInputReader : public DrivetrainInputReader {
 public:
  using DrivetrainInputReader::DrivetrainInputReader;

  // Creates a DrivetrainInputReader with the corresponding joystick ports and
  // axis for the Xbox controller.
  static std::unique_ptr<XboxDrivetrainInputReader> Make(
      ::aos::EventLoop *event_loop);

 private:
  WheelAndThrottle GetWheelAndThrottle(
      const ::frc971::input::driver_station::Data &data) override;
};

}  // namespace frc971::input

#endif  // AOS_INPUT_DRIVETRAIN_INPUT_H_
