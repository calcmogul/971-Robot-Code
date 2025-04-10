#include <unistd.h>

#include <cmath>
#include <cstdio>
#include <cstring>

#include "Eigen/Dense"
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
#include "frc971/control_loops/swerve/swerve_localizer_state_generated.h"
#include "frc971/input/action_joystick_input.h"
#include "frc971/input/driver_station_data.h"
#include "frc971/input/drivetrain_input.h"
#include "frc971/input/joystick_input.h"
#include "frc971/input/redundant_joystick_data.h"
#include "frc971/input/swerve_joystick_input.h"
#include "frc971/zeroing/wrap.h"
#include "y2025/constants/constants_generated.h"
#include "y2025/control_loops/superstructure/superstructure_goal_static.h"
#include "y2025/control_loops/superstructure/superstructure_status_generated.h"

using frc971::CreateProfileParameters;
using frc971::input::driver_station::ButtonLocation;
using frc971::input::driver_station::ControlBit;
using frc971::input::driver_station::JoystickAxis;
using frc971::input::driver_station::POVLocation;
using y2025::control_loops::superstructure::GoalStatic;
using y2025::control_loops::superstructure::Status;

namespace y2025::input::joysticks {

namespace superstructure = y2025::control_loops::superstructure;

namespace swerve = frc971::control_loops::swerve;

const ButtonLocation kLeftL4(3, 4);
const ButtonLocation kRightL4(3, 5);
const ButtonLocation kLeftL3(3, 3);
const ButtonLocation kRightL3(3, 6);
const ButtonLocation kLeftL2(3, 2);
const ButtonLocation kRightL2(3, 7);
const ButtonLocation kL1(3, 1);

const ButtonLocation kHumanPlayer(3, 10);
// TODO: update this with a real button location
const ButtonLocation kHumanPlayerBackup(2, 2);

const ButtonLocation kFront(2, 8);
const ButtonLocation kBack(2, 4);
const ButtonLocation kAlgaeGround(2, 5);
const ButtonLocation kAlgaeProcessor(2, 1);

const ButtonLocation kEndEffectorSpit(3, 11);

const ButtonLocation kClimb(2, 7);
const ButtonLocation kRetract(1, 4);
const ButtonLocation kGroundIntake(3, 12);

const ButtonLocation kDontMove(2, 6);

const ButtonLocation kAlgaeL2(3, 8);
const ButtonLocation kAlgaeL3(3, 9);
const ButtonLocation kBarge(2, 3);

const ButtonLocation kThetaLock(0, 0);

using y2025::control_loops::superstructure::AutoAlignDirection;
using y2025::control_loops::superstructure::ClimberGoal;
using y2025::control_loops::superstructure::ElevatorGoal;
using y2025::control_loops::superstructure::EndEffectorGoal;
using y2025::control_loops::superstructure::GroundIntakeGoal;
using y2025::control_loops::superstructure::GroundIntakeRollerGoal;
using y2025::control_loops::superstructure::PivotGoal;
using y2025::control_loops::superstructure::RobotSide;
using y2025::control_loops::superstructure::WristGoal;

class Reader : public ::frc971::input::SwerveJoystickInput {
 public:
  Reader(::aos::EventLoop *event_loop,
         const y2025::RobotConstants *robot_constants,
         const y2025::Common *common)
      : ::frc971::input::SwerveJoystickInput(
            event_loop,
            {.vx_offset = robot_constants->input_config()->vx_offset(),
             .vy_offset = robot_constants->input_config()->vy_offset(),
             .omega_offset = robot_constants->input_config()->omega_offset(),
             .use_redundant_joysticks =
                 robot_constants->input_config()->use_redundant_joysticks()}),
        superstructure_goal_sender_(
            event_loop->MakeSender<GoalStatic>("/superstructure")),
        superstructure_status_fetcher_(
            event_loop->MakeFetcher<Status>("/superstructure")),
        localizer_state_fetcher_(
            event_loop
                ->MakeFetcher<frc971::control_loops::swerve::LocalizerState>(
                    "/orin1/localizer")),
        robot_constants_(robot_constants),
        common_(common) {
    CHECK(robot_constants_ != nullptr);
  }
  void AutoEnded() { AOS_LOG(INFO, "Auto ended.\n"); }

  void HandleTeleop(
      const ::frc971::input::driver_station::Data &data) override {
    const int team_number = aos::network::GetTeamNumber();
    aos::Sender<superstructure::GoalStatic>::StaticBuilder
        superstructure_goal_builder =
            superstructure_goal_sender_.MakeStaticBuilder();

    superstructure_goal_builder->set_theta_lock(data.IsPressed(kThetaLock));

    if (team_number == 9971) {
      superstructure_goal_builder.CheckOk(superstructure_goal_builder.Send());
      return;
    }

    superstructure_status_fetcher_.Fetch();

    if (!superstructure_status_fetcher_.get()) {
      AOS_LOG(ERROR, "Got no superstructure status message.\n");
      return;
    }

    if (climber_l1_latched_) {
      superstructure_goal_builder->set_elevator_goal(ElevatorGoal::CLIMB);
      superstructure_goal_builder->set_pivot_goal(PivotGoal::CLIMB);
      superstructure_goal_builder->set_wrist_goal(WristGoal::CLIMB);
    } else {
      superstructure_goal_builder->set_elevator_goal(ElevatorGoal::NEUTRAL);
      superstructure_goal_builder->set_pivot_goal(PivotGoal::NEUTRAL);
      superstructure_goal_builder->set_wrist_goal(WristGoal::NEUTRAL);
    }

    if (data.IsPressed(kDontMove)) {
      // Skipping setting any set points.
    } else if (data.IsPressed(kLeftL4) || data.IsPressed(kRightL4)) {
      superstructure_goal_builder->set_elevator_goal(ElevatorGoal::SCORE_L4);
      superstructure_goal_builder->set_pivot_goal(PivotGoal::SCORE_L4);
      superstructure_goal_builder->set_wrist_goal(WristGoal::SCORE_L4);
      climber_l1_latched_ = false;
    } else if (data.IsPressed(kLeftL3) || data.IsPressed(kRightL3)) {
      superstructure_goal_builder->set_elevator_goal(ElevatorGoal::SCORE_L3);
      superstructure_goal_builder->set_pivot_goal(PivotGoal::SCORE_L3);
      superstructure_goal_builder->set_wrist_goal(WristGoal::SCORE_L3);
      climber_l1_latched_ = false;
    } else if (data.IsPressed(kLeftL2) || data.IsPressed(kRightL2)) {
      superstructure_goal_builder->set_elevator_goal(ElevatorGoal::SCORE_L2);
      superstructure_goal_builder->set_pivot_goal(PivotGoal::SCORE_L2);
      superstructure_goal_builder->set_wrist_goal(WristGoal::SCORE_L2);
      climber_l1_latched_ = false;
    } else if (data.IsPressed(kL1)) {
      superstructure_goal_builder->set_elevator_goal(ElevatorGoal::SCORE_L1);
      superstructure_goal_builder->set_pivot_goal(PivotGoal::SCORE_L1);
      superstructure_goal_builder->set_wrist_goal(WristGoal::SCORE_L1);
      climber_l1_latched_ = false;
    } else if (data.IsPressed(kHumanPlayer)) {
      superstructure_goal_builder->set_elevator_goal(ElevatorGoal::INTAKE_HP);
      superstructure_goal_builder->set_pivot_goal(PivotGoal::INTAKE_HP);
      superstructure_goal_builder->set_wrist_goal(WristGoal::INTAKE_HP);
    } else if (data.IsPressed(kHumanPlayerBackup)) {
      superstructure_goal_builder->set_elevator_goal(
          ElevatorGoal::INTAKE_HP_BACKUP);
      superstructure_goal_builder->set_pivot_goal(PivotGoal::INTAKE_HP_BACKUP);
      superstructure_goal_builder->set_wrist_goal(WristGoal::INTAKE_HP_BACKUP);
      climber_l1_latched_ = false;
    } else if (data.IsPressed(kAlgaeL2)) {
      superstructure_goal_builder->set_elevator_goal(ElevatorGoal::ALGAE_L2);
      superstructure_goal_builder->set_pivot_goal(PivotGoal::ALGAE_L2);
      superstructure_goal_builder->set_wrist_goal(WristGoal::ALGAE_L2);
      climber_l1_latched_ = false;
    } else if (data.IsPressed(kAlgaeL3)) {
      superstructure_goal_builder->set_elevator_goal(ElevatorGoal::ALGAE_L3);
      superstructure_goal_builder->set_pivot_goal(PivotGoal::ALGAE_L3);
      superstructure_goal_builder->set_wrist_goal(WristGoal::ALGAE_L3);
      climber_l1_latched_ = false;
    } else if (data.IsPressed(kBarge)) {
      superstructure_goal_builder->set_elevator_goal(ElevatorGoal::BARGE);
      superstructure_goal_builder->set_pivot_goal(PivotGoal::BARGE);
      superstructure_goal_builder->set_wrist_goal(WristGoal::BARGE);
      climber_l1_latched_ = false;
    } else if (data.IsPressed(kAlgaeProcessor)) {
      superstructure_goal_builder->set_elevator_goal(
          ElevatorGoal::ALGAE_PROCESSOR);
      superstructure_goal_builder->set_wrist_goal(WristGoal::ALGAE_PROCESSOR);
      superstructure_goal_builder->set_pivot_goal(PivotGoal::ALGAE_PROCESSOR);
    } else if (data.IsPressed(kAlgaeGround)) {
      superstructure_goal_builder->set_elevator_goal(
          ElevatorGoal::ALGAE_GROUND);
      superstructure_goal_builder->set_wrist_goal(WristGoal::ALGAE_GROUND);
      superstructure_goal_builder->set_pivot_goal(PivotGoal::ALGAE_GROUND);
    }

    if (data.IsPressed(kGroundIntake)) {
      superstructure_goal_builder->set_ground_intake_goal(
          GroundIntakeGoal::CORAL_INTAKE);
      superstructure_goal_builder->set_ground_intake_roller_goal(
          GroundIntakeRollerGoal::INTAKE);
      superstructure_goal_builder->set_elevator_goal(
          ElevatorGoal::INTAKE_GROUND);
      superstructure_goal_builder->set_pivot_goal(PivotGoal::INTAKE_GROUND);
      superstructure_goal_builder->set_wrist_goal(WristGoal::INTAKE_GROUND);
      superstructure_goal_builder->set_end_effector_goal(
          EndEffectorGoal::INTAKE);
      climber_l1_latched_ = false;
    } else {
      superstructure_goal_builder->set_ground_intake_goal(
          GroundIntakeGoal::NEUTRAL);
      superstructure_goal_builder->set_ground_intake_roller_goal(
          GroundIntakeRollerGoal::NEUTRAL);
    }

    localizer_state_fetcher_.Fetch();

    bool between_reefs = std::abs(localizer_state_fetcher_->x()) <
                         std::abs(common_->reef_locations()->Get(0)->x());

    if (data.IsPressed(kLeftL2) || data.IsPressed(kLeftL3) ||
        data.IsPressed(kLeftL4)) {
      superstructure_goal_builder->set_auto_align_direction(
          between_reefs ? AutoAlignDirection::RIGHT : AutoAlignDirection::LEFT);
    } else if (data.IsPressed(kRightL4) || data.IsPressed(kRightL3) ||
               data.IsPressed(kRightL2)) {
      superstructure_goal_builder->set_auto_align_direction(
          between_reefs ? AutoAlignDirection::LEFT : AutoAlignDirection::RIGHT);
    } else {
      superstructure_goal_builder->set_auto_align_direction(
          AutoAlignDirection::CENTER);
    }

    if (data.IsPressed(kEndEffectorSpit)) {
      superstructure_goal_builder->set_end_effector_goal(EndEffectorGoal::SPIT);
    } else if (data.IsPressed(kHumanPlayer) ||
               data.IsPressed(kHumanPlayerBackup) || data.IsPressed(kL1)) {
      superstructure_goal_builder->set_end_effector_goal(
          EndEffectorGoal::INTAKE);
    } else {
      superstructure_goal_builder->set_end_effector_goal(
          EndEffectorGoal::NEUTRAL);
    }

    if (data.IsPressed(kBack)) {
      superstructure_goal_builder->set_robot_side(RobotSide::BACK);
    } else if (data.IsPressed(kFront)) {
      superstructure_goal_builder->set_robot_side(RobotSide::FRONT);
    } else if (data.IsPressed(kHumanPlayer) ||
               data.IsPressed(kHumanPlayerBackup)) {
      superstructure_goal_builder->set_robot_side(
          frontFacing(common_->hp_locations()) ? RobotSide::FRONT
                                               : RobotSide::BACK);
    } else if (data.IsPressed(kL1) || data.IsPressed(kLeftL2) ||
               data.IsPressed(kLeftL3) || data.IsPressed(kLeftL4) ||
               data.IsPressed(kRightL2) || data.IsPressed(kRightL3) ||
               data.IsPressed(kRightL4) || data.IsPressed(kAlgaeL2) ||
               data.IsPressed(kAlgaeL3)) {
      superstructure_goal_builder->set_robot_side(
          frontFacing(common_->reef_locations()) ? RobotSide::FRONT
                                                 : RobotSide::BACK);

    } else if (data.IsPressed(kBarge)) {
      localizer_state_fetcher_.Fetch();

      double theta = localizer_state_fetcher_->theta();
      double x = localizer_state_fetcher_->x();

      bool front_facing = x * cos(theta) > 0.0;

      superstructure_goal_builder->set_robot_side(
          front_facing ? RobotSide::BACK : RobotSide::FRONT);
    } else if (data.IsPressed(kAlgaeProcessor)) {
      superstructure_goal_builder->set_robot_side(
          frontFacing(common_->processor_locations()) ? RobotSide::FRONT
                                                      : RobotSide::BACK);
    } else {
      superstructure_goal_builder->set_robot_side(RobotSide::FRONT);
    }

    if (data.IsPressed(kClimb)) {
      superstructure_goal_builder->set_climber_goal(ClimberGoal::CLIMB);
      climber_l1_latched_ = true;
    } else if (data.IsPressed(kRetract)) {
      superstructure_goal_builder->set_climber_goal(ClimberGoal::RETRACT);
    } else {
      superstructure_goal_builder->set_climber_goal(ClimberGoal::NEUTRAL);
    }

    superstructure_goal_builder.CheckOk(superstructure_goal_builder.Send());
  }

 private:
  ::aos::Sender<GoalStatic> superstructure_goal_sender_;
  ::aos::Fetcher<Status> superstructure_status_fetcher_;
  ::aos::Fetcher<frc971::control_loops::swerve::LocalizerState>
      localizer_state_fetcher_;
  const y2025::RobotConstants *robot_constants_;
  const y2025::Common *common_;
  bool climber_l1_latched_ = false;

  bool frontFacing(
      const flatbuffers::Vector<flatbuffers::Offset<y2025::Location>>
          *locations) {
    localizer_state_fetcher_.Fetch();
    if (!localizer_state_fetcher_.get()) {
      return true;
    }

    Eigen::Matrix<double, 2, 1> robot_pos;
    robot_pos << localizer_state_fetcher_->x(), localizer_state_fetcher_->y();

    std::vector<Eigen::Matrix<double, 2, 1>> points;

    for (size_t i = 0; i < locations->size(); i++) {
      Eigen::Matrix<double, 2, 1> location;
      location << locations->Get(i)->x(), locations->Get(i)->y();
      points.push_back(location - robot_pos);
    }

    Eigen::Matrix<double, 2, 1> closest_point = points.at(0);
    for (size_t i = 1; i < points.size(); i++) {
      closest_point = closest_point.squaredNorm() > points.at(i).squaredNorm()
                          ? points.at(i)
                          : closest_point;
    }

    Eigen::Matrix<double, 2, 1> robot_pointing;
    robot_pointing << cos(localizer_state_fetcher_->theta()),
        sin(localizer_state_fetcher_->theta());

    return robot_pointing.dot(closest_point) > 0;
  }
};

}  // namespace y2025::input::joysticks

int main(int argc, char **argv) {
  ::aos::InitGoogle(&argc, &argv);

  aos::FlatbufferDetachedBuffer<aos::Configuration> config =
      aos::configuration::ReadConfig("aos_config.json");
  frc971::constants::WaitForConstants<y2025::Constants>(&config.message());

  ::aos::ShmEventLoop constant_fetcher_event_loop(&config.message());
  frc971::constants::ConstantsFetcher<y2025::Constants> constants_fetcher(
      &constant_fetcher_event_loop);
  const y2025::RobotConstants *robot_constants =
      constants_fetcher.constants().robot();
  const y2025::Common *common = constants_fetcher.constants().common();

  ::aos::ShmEventLoop event_loop(&config.message());
  ::y2025::input::joysticks::Reader reader(&event_loop, robot_constants,
                                           common);

  event_loop.Run();

  return 0;
}
