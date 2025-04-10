#include "auto_align.h"

using frc971::control_loops::swerve::AutoAlign;

// TODO Move these constants to a json or something
ABSL_FLAG(double, kDVx, 3.8, "Gain for x derivative error");
ABSL_FLAG(double, kDVy, 3.8, "Gain for y derivative error");
ABSL_FLAG(double, kDVtheta, 0.4, "Gain for theta derivative error");
ABSL_FLAG(double, kPVx, 4.0, "Gain for x position error");
ABSL_FLAG(double, kPVy, 4.0, "Gain for y position error");
ABSL_FLAG(double, kPVtheta, 6.0, "Gain for theta position error");

ABSL_FLAG(double, kVelLimit, 1.8, "Velocity Limit");
ABSL_FLAG(double, kOmegaLimit, 13.0, "Omega Limit");

ABSL_FLAG(double, kXOffset, 0.0, "X Offset");
ABSL_FLAG(double, kYOffset, 0.0, "Y Offset");
ABSL_FLAG(double, kThetaOffset, 0.0, "Theta Offset");

AutoAlign::AutoAlign(aos::EventLoop *event_loop)
    : event_loop_(event_loop),
      swerve_goal_sender_(
          event_loop->MakeSender<frc971::control_loops::swerve::GoalStatic>(
              "/autonomous_auto_align")),
      position_goal_fetcher_(
          event_loop->MakeFetcher<frc971::control_loops::swerve::PositionGoal>(
              "/autonomous_auto_align")),
      localizer_state_fetcher_(
          event_loop
              ->MakeFetcher<frc971::control_loops::swerve::LocalizerState>(
                  "/localizer")) {}

void AutoAlign::Iterate() {
  auto builder = swerve_goal_sender_.MakeStaticBuilder();
  localizer_state_fetcher_.Fetch();
  if (localizer_state_fetcher_.get() == nullptr) {
    return;
  }

  double x = localizer_state_fetcher_->x();
  double y = localizer_state_fetcher_->y();
  double theta = localizer_state_fetcher_->theta();

  // check if there's a new goal
  position_goal_fetcher_.Fetch();
  if (position_goal_fetcher_.get() != nullptr) {
    theta_only_ =
        (!position_goal_fetcher_->has_x() && !position_goal_fetcher_->has_y());

    if (!theta_only_) {
      goal_x_ = position_goal_fetcher_.get()->x();
      goal_y_ = position_goal_fetcher_.get()->y();
    }

    goal_theta_ = position_goal_fetcher_.get()->theta();
  }

  const double x_goal = goal_x_;
  const double y_goal = goal_y_;
  const double theta_goal = goal_theta_;

  const double kVelocityLimit = absl::GetFlag(FLAGS_kVelLimit);
  const double kOmegaLimit = absl::GetFlag(FLAGS_kOmegaLimit);

  double x_error = x_goal - x + absl::GetFlag(FLAGS_kXOffset);
  double y_error = y_goal - y + absl::GetFlag(FLAGS_kYOffset);
  // Shortest distance (theta_error should be between -pi and pi)
  double theta_error = aos::math::NormalizeAngle(
      theta_goal - theta + absl::GetFlag(FLAGS_kThetaOffset));

  double dt =
      aos::time::DurationInSeconds(event_loop_->monotonic_now() - last_time_);

  double dx_error = (x_error - prev_x_error_) / dt;
  double dy_error = (y_error - prev_y_error_) / dt;
  double dtheta_error = (theta_error - prev_theta_error_) / dt;

  double goal_vx = absl::GetFlag(FLAGS_kPVx) * x_error +
                   absl::GetFlag(FLAGS_kDVx) * dx_error;

  double goal_vy = absl::GetFlag(FLAGS_kPVy) * y_error +
                   absl::GetFlag(FLAGS_kDVy) * dy_error;

  double goal_omega = absl::GetFlag(FLAGS_kPVtheta) * theta_error +
                      absl::GetFlag(FLAGS_kDVtheta) * dtheta_error;

  prev_x_error_ = x_error;
  prev_y_error_ = y_error;
  prev_theta_error_ = theta_error;

  // limit the velocity
  // goal_vx^2 + goal_vy^2 <= magnitude^2
  double magnitude = goal_vx * goal_vx + goal_vy * goal_vy;
  if (magnitude > kVelocityLimit * kVelocityLimit) {
    magnitude = std::sqrt(magnitude);
    goal_vx /= magnitude;
    goal_vy /= magnitude;
    goal_vx *= kVelocityLimit;
    goal_vy *= kVelocityLimit;
  }

  // limit the omega (angular velocity)
  goal_omega = std::clamp(goal_omega, -kOmegaLimit, kOmegaLimit);

  // send joystick goal
  auto joystick_goal = builder->add_joystick_goal();
  if (!theta_only_) {
    joystick_goal->set_vx(goal_vx);
    joystick_goal->set_vy(goal_vy);
  }
  joystick_goal->set_omega(goal_omega);

  builder.CheckOk(builder.Send());
}
