#ifndef Y2020_CONTROL_LOOPS_DRIVETRAIN_LOCALIZER_H_
#define Y2020_CONTROL_LOOPS_DRIVETRAIN_LOCALIZER_H_

#include "aos/containers/ring_buffer.h"
#include "aos/events/event_loop.h"
#include "frc971/control_loops/drivetrain/hybrid_ekf.h"
#include "frc971/control_loops/drivetrain/localizer.h"
#include "y2020/control_loops/superstructure/superstructure_status_generated.h"
#include "y2020/vision/sift/sift_generated.h"

namespace y2020 {
namespace control_loops {
namespace drivetrain {

// This class handles the localization for the 2020 robot. In order to handle
// camera updates, we get the ImageMatchResult message from the cameras and then
// project the result onto the 2-D X/Y plane and use the implied robot
// position/heading from that as the measurement. This is distinct from 2019,
// when we used a heading/distance/skew measurement update. This is because
// updating with x/y/theta directly seems to be better conditioned (even if it
// may not reflect the measurement noise quite as accurately). The poor
// conditioning seemed to work in 2019, but due to the addition of a couple of
// velocity offset states that allow us to use the accelerometer more
// effectively, things started to become unstable.
class Localizer : public frc971::control_loops::drivetrain::LocalizerInterface {
 public:
  typedef frc971::control_loops::TypedPose<double> Pose;
  typedef frc971::control_loops::drivetrain::HybridEkf<double> HybridEkf;
  typedef typename HybridEkf::State State;
  typedef typename HybridEkf::StateIdx StateIdx;
  typedef typename HybridEkf::StateSquare StateSquare;
  typedef typename HybridEkf::Input Input;
  typedef typename HybridEkf::Output Output;
  Localizer(aos::EventLoop *event_loop,
            const frc971::control_loops::drivetrain::DrivetrainConfig<double>
                &dt_config);
  State Xhat() const override { return ekf_.X_hat(); }
  frc971::control_loops::drivetrain::TrivialTargetSelector *target_selector()
      override {
    return &target_selector_;
  }

  void Update(const ::Eigen::Matrix<double, 2, 1> &U,
              aos::monotonic_clock::time_point now, double left_encoder,
              double right_encoder, double gyro_rate,
              const Eigen::Vector3d &accel) override;

  void Reset(aos::monotonic_clock::time_point t, const State &state) {
    ekf_.ResetInitialState(t, state, ekf_.P());
  }

  void ResetPosition(aos::monotonic_clock::time_point t, double x, double y,
                     double theta, double /*theta_override*/,
                     bool /*reset_theta*/) override {
    const double left_encoder = ekf_.X_hat(StateIdx::kLeftEncoder);
    const double right_encoder = ekf_.X_hat(StateIdx::kRightEncoder);
    ekf_.ResetInitialState(t, (Ekf::State() << x, y, theta, left_encoder, 0,
                               right_encoder, 0, 0, 0, 0, 0, 0).finished(),
                           ekf_.P());
  };

 private:
  // Storage for a single turret position data point.
  struct TurretData {
    aos::monotonic_clock::time_point receive_time =
        aos::monotonic_clock::min_time;
    double position = 0.0;  // rad
    double velocity = 0.0;  // rad/sec
  };

  // Processes new image data and updates the EKF.
  void HandleImageMatch(const frc971::vision::sift::ImageMatchResult &result);

  // Processes the most recent turret position and stores it in the turret_data_
  // buffer.
  void HandleSuperstructureStatus(
      const y2020::control_loops::superstructure::Status &status);

  // Retrieves the turret data closest to the provided time.
  TurretData GetTurretDataForTime(aos::monotonic_clock::time_point time);

  aos::EventLoop *const event_loop_;
  const frc971::control_loops::drivetrain::DrivetrainConfig<double> dt_config_;
  HybridEkf ekf_;

  std::vector<aos::Fetcher<frc971::vision::sift::ImageMatchResult>>
      image_fetchers_;

  // Buffer of recent turret data--this is used so that when we receive a camera
  // frame from the turret, we can back out what the turret angle was at that
  // time.
  aos::RingBuffer<TurretData, 200> turret_data_;

  // Target selector to allow us to satisfy the LocalizerInterface requirements.
  frc971::control_loops::drivetrain::TrivialTargetSelector target_selector_;
};

}  // namespace drivetrain
}  // namespace control_loops
}  // namespace y2020

#endif  // Y2020_CONTROL_LOOPS_DRIVETRAIN_LOCALIZER_H_