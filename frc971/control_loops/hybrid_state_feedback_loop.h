#ifndef FRC971_CONTROL_LOOPS_HYBRID_STATE_FEEDBACK_LOOP_H_
#define FRC971_CONTROL_LOOPS_HYBRID_STATE_FEEDBACK_LOOP_H_

#include <cassert>
#include <chrono>
#include <memory>
#include <utility>
#include <vector>

#include "Eigen/Dense"
#include "absl/log/check.h"
#include "absl/log/log.h"

#include "aos/logging/logging.h"
#include "aos/macros.h"
#include "aos/time/time.h"
#include "frc971/control_loops/c2d.h"
#include "frc971/control_loops/control_loop.h"
#include "frc971/control_loops/state_feedback_loop.h"
#include "unsupported/Eigen/MatrixFunctions"

template <int number_of_states, int number_of_inputs, int number_of_outputs,
          typename Scalar = double>
struct StateFeedbackHybridPlantCoefficients final {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

  StateFeedbackHybridPlantCoefficients(
      const StateFeedbackHybridPlantCoefficients &other)
      : A_continuous(other.A_continuous),
        B_continuous(other.B_continuous),
        C(other.C),
        D(other.D),
        U_min(other.U_min),
        U_max(other.U_max),
        U_limit_coefficient(other.U_limit_coefficient),
        U_limit_constant(other.U_limit_constant),
        delayed_u(other.delayed_u) {}

  StateFeedbackHybridPlantCoefficients(
      const Eigen::Matrix<Scalar, number_of_states, number_of_states>
          &A_continuous,
      const Eigen::Matrix<Scalar, number_of_states, number_of_inputs>
          &B_continuous,
      const Eigen::Matrix<Scalar, number_of_outputs, number_of_states> &C,
      const Eigen::Matrix<Scalar, number_of_outputs, number_of_inputs> &D,
      const Eigen::Matrix<Scalar, number_of_inputs, 1> &U_max,
      const Eigen::Matrix<Scalar, number_of_inputs, 1> &U_min,
      const Eigen::Matrix<Scalar, number_of_inputs, number_of_states>
          &U_limit_coefficient,
      const Eigen::Matrix<Scalar, number_of_inputs, 1> &U_limit_constant,
      size_t delayed_u)
      : A_continuous(A_continuous),
        B_continuous(B_continuous),
        C(C),
        D(D),
        U_min(U_min),
        U_max(U_max),
        U_limit_coefficient(U_limit_coefficient),
        U_limit_constant(U_limit_constant),
        delayed_u(delayed_u) {}

  const Eigen::Matrix<Scalar, number_of_states, number_of_states> A_continuous;
  const Eigen::Matrix<Scalar, number_of_states, number_of_inputs> B_continuous;
  const Eigen::Matrix<Scalar, number_of_outputs, number_of_states> C;
  const Eigen::Matrix<Scalar, number_of_outputs, number_of_inputs> D;
  const Eigen::Matrix<Scalar, number_of_inputs, 1> U_min;
  const Eigen::Matrix<Scalar, number_of_inputs, 1> U_max;
  const Eigen::Matrix<Scalar, number_of_inputs, number_of_states>
      U_limit_coefficient;
  const Eigen::Matrix<Scalar, number_of_inputs, 1> U_limit_constant;

  const size_t delayed_u;
};

template <int number_of_states, int number_of_inputs, int number_of_outputs,
          typename Scalar = double>
class StateFeedbackHybridPlant {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

  StateFeedbackHybridPlant(
      ::std::vector<::std::unique_ptr<StateFeedbackHybridPlantCoefficients<
          number_of_states, number_of_inputs, number_of_outputs>>>
          &&coefficients)
      : coefficients_(::std::move(coefficients)), index_(0) {
    Reset();
  }

  StateFeedbackHybridPlant(StateFeedbackHybridPlant &&other)
      : index_(other.index_) {
    coefficients_ = ::std::move(other.coefficients_);
    X_ = ::std::move(other.X_);
    Y_ = ::std::move(other.Y_);
    A_ = ::std::move(other.A_);
    B_ = ::std::move(other.B_);
    DelayedU_ = ::std::move(other.DelayedU_);
  }

  virtual ~StateFeedbackHybridPlant() {}

  const Eigen::Matrix<Scalar, number_of_states, number_of_states> &A() const {
    return A_;
  }
  Scalar A(int i, int j) const { return A()(i, j); }
  const Eigen::Matrix<Scalar, number_of_states, number_of_inputs> &B() const {
    return B_;
  }
  Scalar B(int i, int j) const { return B()(i, j); }
  const Eigen::Matrix<Scalar, number_of_outputs, number_of_states> &C() const {
    return coefficients().C;
  }
  Scalar C(int i, int j) const { return C()(i, j); }
  const Eigen::Matrix<Scalar, number_of_outputs, number_of_inputs> &D() const {
    return coefficients().D;
  }
  Scalar D(int i, int j) const { return D()(i, j); }
  const Eigen::Matrix<Scalar, number_of_inputs, 1> &U_min() const {
    return coefficients().U_min;
  }
  Scalar U_min(int i, int j) const { return U_min()(i, j); }
  const Eigen::Matrix<Scalar, number_of_inputs, 1> &U_max() const {
    return coefficients().U_max;
  }
  Scalar U_max(int i, int j) const { return U_max()(i, j); }
  const Eigen::Matrix<Scalar, number_of_inputs, number_of_states> &
  U_limit_coefficient() const {
    return coefficients().U_limit_coefficient;
  }
  Scalar U_limit_coefficient(int i, int j) const {
    return U_limit_coefficient()(i, j);
  }
  const Eigen::Matrix<Scalar, number_of_inputs, 1> &U_limit_constant() const {
    return coefficients().U_limit_constant;
  }
  Scalar U_limit_constant(int i, int j = 0) const {
    return U_limit_constant()(i, j);
  }

  const Eigen::Matrix<Scalar, number_of_states, 1> &X() const { return X_; }
  Scalar X(int i) const { return X()(i); }
  const Eigen::Matrix<Scalar, number_of_outputs, 1> &Y() const { return Y_; }
  Scalar Y(int i) const { return Y()(i); }

  Eigen::Matrix<Scalar, number_of_states, 1> &mutable_X() { return X_; }
  Scalar &mutable_X(int i) { return mutable_X()(i); }
  Eigen::Matrix<Scalar, number_of_outputs, 1> &mutable_Y() { return Y_; }
  Scalar &mutable_Y(int i) { return mutable_Y()(i); }

  const StateFeedbackHybridPlantCoefficients<number_of_states, number_of_inputs,
                                             number_of_outputs, Scalar> &
  coefficients(int index) const {
    return *coefficients_[index];
  }

  const StateFeedbackHybridPlantCoefficients<number_of_states, number_of_inputs,
                                             number_of_outputs, Scalar> &
  coefficients() const {
    return *coefficients_[index_];
  }

  int index() const { return index_; }
  void set_index(int index) {
    assert(index >= 0);
    assert(index < static_cast<int>(coefficients_.size()));
    index_ = index;
  }

  void Reset() {
    X_.setZero();
    Y_.setZero();
    A_.setZero();
    B_.setZero();
    DelayedU_.setZero();
    UpdateAB(::std::chrono::microseconds(5050));
  }

  // Assert that U is within the hardware range.
  virtual void CheckU(const Eigen::Matrix<Scalar, number_of_inputs, 1> &U) {
    for (int i = 0; i < kNumInputs; ++i) {
      if (U(i, 0) > U_max(i, 0) + static_cast<Scalar>(0.00001) ||
          U(i, 0) < U_min(i, 0) - static_cast<Scalar>(0.00001)) {
        AOS_LOG(FATAL, "U out of range\n");
      }
    }
  }

  // Computes the new X and Y given the control input.
  void Update(const Eigen::Matrix<Scalar, number_of_inputs, 1> &U,
              ::std::chrono::nanoseconds dt, Scalar voltage_battery) {
    CHECK_NE(0, dt.count());

    // Powers outside of the range are more likely controller bugs than things
    // that the plant should deal with.
    CheckU(U);

    Eigen::Matrix<Scalar, number_of_inputs, 1> current_U =
        DelayedU_ * voltage_battery / static_cast<Scalar>(12.0);

    X_ = Update(X(), current_U, dt);
    Y_ = C() * X() + D() * current_U;
    DelayedU_ = U;
  }

  Eigen::Matrix<Scalar, number_of_states, 1> Update(
      const Eigen::Matrix<Scalar, number_of_states, 1> X,
      const Eigen::Matrix<Scalar, number_of_inputs, 1> &U,
      ::std::chrono::nanoseconds dt) {
    // TODO(austin): Hmmm, looks like we aren't compensating for battery voltage
    // or the unit delay...  Might want to do that if we care about performance
    // again.
    UpdateAB(dt);
    const Eigen::Matrix<Scalar, number_of_states, 1> new_X =
        A() * X + B() * DelayedU_;
    DelayedU_ = U;

    return new_X;
  }

 protected:
  // these are accessible from non-templated subclasses
  static const int kNumStates = number_of_states;
  static const int kNumOutputs = number_of_outputs;
  static const int kNumInputs = number_of_inputs;

 private:
  void UpdateAB(::std::chrono::nanoseconds dt) {
    ::frc971::controls::C2D(coefficients().A_continuous,
                            coefficients().B_continuous, dt, &A_, &B_);
  }

  Eigen::Matrix<Scalar, number_of_states, 1> X_;
  Eigen::Matrix<Scalar, number_of_outputs, 1> Y_;

  Eigen::Matrix<Scalar, number_of_states, number_of_states> A_;
  Eigen::Matrix<Scalar, number_of_states, number_of_inputs> B_;

  ::std::vector<::std::unique_ptr<StateFeedbackHybridPlantCoefficients<
      number_of_states, number_of_inputs, number_of_outputs>>>
      coefficients_;

  Eigen::Matrix<Scalar, number_of_inputs, 1> DelayedU_;

  int index_;

  DISALLOW_COPY_AND_ASSIGN(StateFeedbackHybridPlant);
};

// A container for all the observer coefficients.
template <int number_of_states, int number_of_inputs, int number_of_outputs,
          typename Scalar = double>
struct HybridKalmanCoefficients final {
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

  const Eigen::Matrix<Scalar, number_of_states, number_of_states> Q_continuous;
  const Eigen::Matrix<Scalar, number_of_outputs, number_of_outputs>
      R_continuous;
  const Eigen::Matrix<Scalar, number_of_states, number_of_states>
      P_steady_state;

  const size_t delayed_u;

  HybridKalmanCoefficients(
      const Eigen::Matrix<Scalar, number_of_states, number_of_states>
          &Q_continuous,
      const Eigen::Matrix<Scalar, number_of_outputs, number_of_outputs>
          &R_continuous,
      const Eigen::Matrix<Scalar, number_of_states, number_of_states>
          &P_steady_state,
      size_t delayed_u)
      : Q_continuous(Q_continuous),
        R_continuous(R_continuous),
        P_steady_state(P_steady_state),
        delayed_u(delayed_u) {
    CHECK(!delayed_u) << ": Delayed hybrid filters aren't supported yet.";
  }
};

template <int number_of_states, int number_of_inputs, int number_of_outputs,
          typename Scalar = double>
class HybridKalman {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

  explicit HybridKalman(
      ::std::vector<::std::unique_ptr<HybridKalmanCoefficients<
          number_of_states, number_of_inputs, number_of_outputs, Scalar>>>
          &&observers)
      : coefficients_(::std::move(observers)) {
    R_.setZero();
    X_hat_.setZero();
    P_ = coefficients().P_steady_state;
  }

  HybridKalman(HybridKalman &&other) : index_(other.index_) {
    coefficients_ = ::std::move(other.coefficients_);
    X_hat_ = ::std::move(other.X_hat_);
    P_ = ::std::move(other.P_);
    Q_ = ::std::move(other.Q_);
    R_ = ::std::move(other.R_);
  }

  // Getters for Q
  const Eigen::Matrix<Scalar, number_of_states, number_of_states> &Q() const {
    return Q_;
  }
  Scalar Q(int i, int j) const { return Q()(i, j); }
  // Getters for R
  const Eigen::Matrix<Scalar, number_of_outputs, number_of_outputs> &R() const {
    return R_;
  }
  Scalar R(int i, int j) const { return R()(i, j); }

  // Getters for P
  const Eigen::Matrix<Scalar, number_of_states, number_of_states> &P() const {
    return P_;
  }
  Scalar P(int i, int j) const { return P()(i, j); }

  // Getters for X_hat
  const Eigen::Matrix<Scalar, number_of_states, 1> &X_hat() const {
    return X_hat_;
  }
  Eigen::Matrix<Scalar, number_of_states, 1> &mutable_X_hat() { return X_hat_; }
  Scalar &mutable_X_hat(int i) { return mutable_X_hat()(i); }
  Scalar X_hat(int i) const { return X_hat_(i); }

  void Reset(StateFeedbackHybridPlant<number_of_states, number_of_inputs,
                                      number_of_outputs> *plant) {
    X_hat_.setZero();
    P_ = coefficients().P_steady_state;
    UpdateQR(plant, coefficients().Q_continuous, coefficients().R_continuous,
             frc971::controls::kLoopFrequency);
  }

  void Predict(StateFeedbackHybridPlant<number_of_states, number_of_inputs,
                                        number_of_outputs, Scalar> *plant,
               const Eigen::Matrix<Scalar, number_of_inputs, 1> &new_u,
               ::std::chrono::nanoseconds dt) {
    Predict(plant, new_u, dt, coefficients().Q_continuous,
            coefficients().R_continuous);
  }

  void Predict(
      StateFeedbackHybridPlant<number_of_states, number_of_inputs,
                               number_of_outputs, Scalar> *plant,
      const Eigen::Matrix<Scalar, number_of_inputs, 1> &new_u,
      ::std::chrono::nanoseconds dt,
      Eigen::Matrix<Scalar, number_of_states, number_of_states> Q_continuous,
      Eigen::Matrix<Scalar, number_of_outputs, number_of_outputs>
          R_continuous) {
    // Trigger the predict step.  This will update A() and B() in the plant.
    mutable_X_hat() = plant->Update(X_hat(), new_u, dt);

    UpdateQR(plant, Q_continuous, R_continuous, dt);
    P_ = plant->A() * P_ * plant->A().transpose() + Q_;
  }

  void Correct(
      const StateFeedbackHybridPlant<number_of_states, number_of_inputs,
                                     number_of_outputs, Scalar> &plant,
      const Eigen::Matrix<Scalar, number_of_inputs, 1> &U,
      const Eigen::Matrix<Scalar, number_of_outputs, 1> &Y) {
    DynamicCorrect(plant.C(), plant.D(), U, Y, R_);
  }

  // Corrects based on the sensor information available.
  template <int number_of_measurements>
  void DynamicCorrect(
      const Eigen::Matrix<Scalar, number_of_measurements, number_of_states> &C,
      const Eigen::Matrix<Scalar, number_of_measurements, number_of_inputs> &D,
      const Eigen::Matrix<Scalar, number_of_inputs, 1> &U,
      const Eigen::Matrix<Scalar, number_of_outputs, 1> &Y,
      const Eigen::Matrix<Scalar, number_of_outputs, number_of_outputs> &R) {
    Eigen::Matrix<Scalar, number_of_outputs, 1> Y_bar =
        Y - (C * X_hat_ + D * U);
    Eigen::Matrix<Scalar, number_of_outputs, number_of_outputs> S =
        C * P_ * C.transpose() + R;
    Eigen::Matrix<Scalar, number_of_states, number_of_outputs> KalmanGain;
    KalmanGain = (S.transpose().ldlt().solve((P() * C.transpose()).transpose()))
                     .transpose();
    X_hat_ = X_hat_ + KalmanGain * Y_bar;
    P_ =
        (Eigen::Matrix<Scalar, number_of_states, number_of_states>::Identity() -
         KalmanGain * C) *
        P();
  }

  // Sets the current controller to be index, clamped to be within range.
  void set_index(int index) {
    if (index < 0) {
      index_ = 0;
    } else if (index >= static_cast<int>(coefficients_.size())) {
      index_ = static_cast<int>(coefficients_.size()) - 1;
    } else {
      index_ = index;
    }
  }

  int index() const { return index_; }

  const HybridKalmanCoefficients<number_of_states, number_of_inputs,
                                 number_of_outputs> &
  coefficients(int index) const {
    return *coefficients_[index];
  }

  const HybridKalmanCoefficients<number_of_states, number_of_inputs,
                                 number_of_outputs> &
  coefficients() const {
    return *coefficients_[index_];
  }

 private:
  void UpdateQR(
      StateFeedbackHybridPlant<number_of_states, number_of_inputs,
                               number_of_outputs> *plant,
      Eigen::Matrix<Scalar, number_of_states, number_of_states> Q_continuous,
      Eigen::Matrix<Scalar, number_of_outputs, number_of_outputs> R_continuous,
      ::std::chrono::nanoseconds dt) {
    frc971::controls::DiscretizeQ(Q_continuous,
                                  plant->coefficients().A_continuous, dt, &Q_);

    Eigen::Matrix<Scalar, number_of_outputs, number_of_outputs> Rtemp =
        (R_continuous + R_continuous.transpose()) / static_cast<Scalar>(2.0);

    R_ = Rtemp / ::aos::time::TypedDurationInSeconds<Scalar>(dt);
  }

  // Internal state estimate.
  Eigen::Matrix<Scalar, number_of_states, 1> X_hat_;
  // Internal covariance estimate.
  Eigen::Matrix<Scalar, number_of_states, number_of_states> P_;

  // Discretized Q and R for the kalman filter.
  Eigen::Matrix<Scalar, number_of_states, number_of_states> Q_;
  Eigen::Matrix<Scalar, number_of_outputs, number_of_outputs> R_;

  int index_ = 0;
  ::std::vector<::std::unique_ptr<HybridKalmanCoefficients<
      number_of_states, number_of_inputs, number_of_outputs>>>
      coefficients_;
};

#endif  // FRC971_CONTROL_LOOPS_HYBRID_STATE_FEEDBACK_LOOP_H_
