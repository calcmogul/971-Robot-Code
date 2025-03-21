#include <unistd.h>

#include <cmath>
#include <cstdio>
#include <cstring>

#include "absl/flags/flag.h"

#include "aos/actions/actions.h"
#include "aos/init.h"
#include "aos/logging/logging.h"
#include "aos/network/team_number.h"
#include "aos/util/log_interval.h"
#include "frc971/autonomous/base_autonomous_actor.h"
#include "frc971/constants/constants_sender_lib.h"
#include "frc971/control_loops/drivetrain/localizer_generated.h"
#include "frc971/control_loops/profiled_subsystem_generated.h"
#include "frc971/control_loops/static_zeroing_single_dof_profiled_subsystem.h"
#include "frc971/input/action_joystick_input.h"
#include "frc971/input/driver_station_data.h"
#include "frc971/input/drivetrain_input.h"
#include "frc971/input/joystick_input.h"
#include "frc971/input/redundant_joystick_data.h"
#include "frc971/zeroing/wrap.h"
#include "y2024/constants/constants_generated.h"
#include "y2024/control_loops/drivetrain/drivetrain_base.h"
#include "y2024/control_loops/superstructure/superstructure_goal_static.h"
#include "y2024/control_loops/superstructure/superstructure_status_static.h"

using frc971::CreateProfileParameters;
using frc971::input::driver_station::ButtonLocation;
using frc971::input::driver_station::ControlBit;
using frc971::input::driver_station::JoystickAxis;
using frc971::input::driver_station::POVLocation;
using Side = frc971::control_loops::drivetrain::RobotSide;

ABSL_FLAG(double, speaker_altitude_position_override, -1,
          "If set, use this as the altitude angle for the fixed shot.");
ABSL_FLAG(bool, allow_force_preload, false,
          "If set, enable the kForcePreload button.");

namespace y2024::input::joysticks {

namespace superstructure = y2024::control_loops::superstructure;

// TODO(Xander): add button location from physical wiring
// Note: Due to use_redundant_joysticks, the AOS_LOG statements
// for the internal joystick code will give offset joystick numbering.
const ButtonLocation kIntake(2, 2);

const ButtonLocation kSpitRollers(1, 13);
const ButtonLocation kSpitExtend(1, 12);
const ButtonLocation kIntakeRollers(2, 5);

const ButtonLocation kCatapultLoad(2, 1);
const ButtonLocation kAmp(2, 4);
const ButtonLocation kFire(2, 8);
const ButtonLocation kForceLoad(2, 9);
const ButtonLocation kDriverFire(1, 1);
const ButtonLocation kTrap(2, 6);
const ButtonLocation kAutoAim(1, 8);
const ButtonLocation kAimSpeaker(2, 11);
const ButtonLocation kAimPodium(0, 0);
const ButtonLocation kShoot(0, 0);
const ButtonLocation kRaiseClimber(3, 2);
const ButtonLocation kRaiseFastClimber(3, 1);
const ButtonLocation kAimShuttle(2, 10);
const ButtonLocation kTurretShuttle(1, 10);
const ButtonLocation kExtraButtonTwo(0, 0);
const ButtonLocation kExtraButtonThree(0, 0);
const ButtonLocation kExtraButtonFour(0, 0);

class Reader : public ::frc971::input::ActionJoystickInput {
 public:
  Reader(::aos::EventLoop *event_loop, const y2024::Constants *robot_constants)
      : ::frc971::input::ActionJoystickInput(
            event_loop,
            ::y2024::control_loops::drivetrain::GetDrivetrainConfig(event_loop),
            ::frc971::input::DrivetrainInputReader::InputType::kPistol,
            {.use_redundant_joysticks = true}),
        superstructure_goal_sender_(
            event_loop->MakeSender<control_loops::superstructure::GoalStatic>(
                "/superstructure")),
        superstructure_status_fetcher_(
            event_loop->MakeFetcher<control_loops::superstructure::Status>(
                "/superstructure")),
        robot_constants_(robot_constants) {
    CHECK(robot_constants_ != nullptr);
  }

  void AutoEnded() override { AOS_LOG(INFO, "Auto ended.\n"); }

  void HandleTeleop(
      const ::frc971::input::driver_station::Data &data) override {
    superstructure_status_fetcher_.Fetch();
    if (!superstructure_status_fetcher_.get()) {
      AOS_LOG(ERROR, "Got no superstructure status message.\n");
      return;
    }

    aos::Sender<superstructure::GoalStatic>::StaticBuilder
        superstructure_goal_builder =
            superstructure_goal_sender_.MakeStaticBuilder();

    if (data.IsPressed(kIntake) || climbed_count_ > 25) {
      // Intake is pressed
      superstructure_goal_builder->set_intake_pivot(
          superstructure::IntakePivotGoal::DOWN);
    } else {
      superstructure_goal_builder->set_intake_pivot(
          superstructure::IntakePivotGoal::UP);
    }

    if (data.IsPressed(kIntakeRollers)) {
      // Intake is pressed
      superstructure_goal_builder->set_intake_goal(
          superstructure::IntakeGoal::INTAKE);
    } else if (data.IsPressed(kSpitRollers)) {
      superstructure_goal_builder->set_intake_goal(
          superstructure::IntakeGoal::SPIT);
    } else {
      superstructure_goal_builder->set_intake_goal(
          superstructure::IntakeGoal::NONE);
    }

    if (data.IsPressed(kSpitExtend)) {
      superstructure_goal_builder->set_spit_extend(true);
    } else {
      superstructure_goal_builder->set_spit_extend(false);
    }

    if (data.IsPressed(kAmp)) {
      superstructure_goal_builder->set_note_goal(superstructure::NoteGoal::AMP);
    } else if (data.IsPressed(kTrap)) {
      superstructure_goal_builder->set_note_goal(
          superstructure::NoteGoal::TRAP);
    } else if (data.IsPressed(kCatapultLoad)) {
      superstructure_goal_builder->set_note_goal(
          superstructure::NoteGoal::CATAPULT);
    } else {
      superstructure_goal_builder->set_note_goal(
          superstructure::NoteGoal::NONE);
    }
    auto shooter_goal = superstructure_goal_builder->add_shooter_goal();
    shooter_goal->set_preloaded(absl::GetFlag(FLAGS_allow_force_preload) &&
                                data.IsPressed(kForceLoad));
    if (data.IsPressed(kAutoAim)) {
      shooter_goal->set_auto_aim(
          control_loops::superstructure::AutoAimMode::SPEAKER);
    } else if (data.IsPressed(kAimShuttle)) {
      shooter_goal->set_auto_aim(
          control_loops::superstructure::AutoAimMode::SHUTTLE);
    } else if (data.IsPressed(kTurretShuttle)) {
      shooter_goal->set_auto_aim(
          control_loops::superstructure::AutoAimMode::TURRET_SHUTTLE);
    }

    // Updating aiming for shooter goal, only one type of aim should be possible
    // at a time, auto-aiming is preferred over the setpoints.
    if (data.IsPressed(kAimSpeaker)) {
      auto catapult_goal = shooter_goal->add_catapult_goal();
      catapult_goal->set_shot_velocity(robot_constants_->common()
                                           ->shooter_speaker_set_point()
                                           ->shot_velocity());
      if (absl::GetFlag(FLAGS_speaker_altitude_position_override) > 0) {
        PopulateStaticZeroingSingleDOFProfiledSubsystemGoal(
            shooter_goal->add_altitude_position(),
            absl::GetFlag(FLAGS_speaker_altitude_position_override));
      } else {
        PopulateStaticZeroingSingleDOFProfiledSubsystemGoal(
            shooter_goal->add_altitude_position(),
            robot_constants_->common()
                ->shooter_speaker_set_point()
                ->altitude_position());
      }
      PopulateStaticZeroingSingleDOFProfiledSubsystemGoal(
          shooter_goal->add_turret_position(), robot_constants_->common()
                                                   ->shooter_speaker_set_point()
                                                   ->turret_position());
    }
    superstructure_goal_builder->set_fire(data.IsPressed(kFire) ||
                                          data.IsPressed(kDriverFire));

    if (data.IsPressed(kRaiseClimber)) {
      ++climbed_count_;
      superstructure_goal_builder->set_climber_goal_voltage(4.0);
    } else if (data.IsPressed(kRaiseFastClimber)) {
      ++climbed_count_;
      superstructure_goal_builder->set_climber_goal_voltage(12.0);
    }

    if (climbed_count_ > 25) {
      PopulateStaticZeroingSingleDOFProfiledSubsystemGoal(
          shooter_goal->add_turret_position(), 0.0);
    }

    superstructure_goal_builder.CheckOk(superstructure_goal_builder.Send());
  }

 private:
  ::aos::Sender<control_loops::superstructure::GoalStatic>
      superstructure_goal_sender_;
  ::aos::Fetcher<control_loops::superstructure::Status>
      superstructure_status_fetcher_;
  const y2024::Constants *robot_constants_;

  int climbed_count_ = 0;
};

}  // namespace y2024::input::joysticks

int main(int argc, char **argv) {
  ::aos::InitGoogle(&argc, &argv);

  aos::FlatbufferDetachedBuffer<aos::Configuration> config =
      aos::configuration::ReadConfig("aos_config.json");
  frc971::constants::WaitForConstants<y2024::Constants>(&config.message());

  ::aos::ShmEventLoop constant_fetcher_event_loop(&config.message());
  frc971::constants::ConstantsFetcher<y2024::Constants> constants_fetcher(
      &constant_fetcher_event_loop);
  const y2024::Constants *robot_constants = &constants_fetcher.constants();

  ::aos::ShmEventLoop event_loop(&config.message());
  ::y2024::input::joysticks::Reader reader(&event_loop, robot_constants);

  event_loop.Run();

  return 0;
}
