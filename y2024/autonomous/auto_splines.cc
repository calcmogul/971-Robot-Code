#include "y2024/autonomous/auto_splines.h"

#include "aos/flatbuffer_merge.h"
#include "frc971/control_loops/control_loops_generated.h"

namespace y2024::autonomous {

namespace {
flatbuffers::Offset<frc971::MultiSpline> FixSpline(
    aos::Sender<frc971::control_loops::drivetrain::SplineGoal>::Builder
        *builder,
    flatbuffers::Offset<frc971::MultiSpline> spline_offset,
    aos::Alliance alliance) {
  frc971::MultiSpline *spline =
      GetMutableTemporaryPointer(*builder->fbb(), spline_offset);
  flatbuffers::Vector<float> *spline_x = spline->mutable_spline_x();

  // For 2024: The field is mirrored across the center line, and is not
  // rotationally symmetric. As such, we only flip the X coordinates when
  // changing side of the field.
  if (alliance == aos::Alliance::kRed) {
    for (size_t ii = 0; ii < spline_x->size(); ++ii) {
      spline_x->Mutate(ii, -spline_x->Get(ii));
    }
  }
  return spline_offset;
}
}  // namespace

flatbuffers::Offset<frc971::MultiSpline> AutonomousSplines::BasicSSpline(
    aos::Sender<frc971::control_loops::drivetrain::SplineGoal>::Builder
        *builder,
    aos::Alliance alliance) {
  flatbuffers::Offset<frc971::Constraint> longitudinal_constraint_offset;
  flatbuffers::Offset<frc971::Constraint> lateral_constraint_offset;
  flatbuffers::Offset<frc971::Constraint> voltage_constraint_offset;

  {
    frc971::Constraint::Builder longitudinal_constraint_builder =
        builder->MakeBuilder<frc971::Constraint>();
    longitudinal_constraint_builder.add_constraint_type(
        frc971::ConstraintType::LONGITUDINAL_ACCELERATION);
    longitudinal_constraint_builder.add_value(1.0);
    longitudinal_constraint_offset = longitudinal_constraint_builder.Finish();
  }

  {
    frc971::Constraint::Builder lateral_constraint_builder =
        builder->MakeBuilder<frc971::Constraint>();
    lateral_constraint_builder.add_constraint_type(
        frc971::ConstraintType::LATERAL_ACCELERATION);
    lateral_constraint_builder.add_value(1.0);
    lateral_constraint_offset = lateral_constraint_builder.Finish();
  }

  {
    frc971::Constraint::Builder voltage_constraint_builder =
        builder->MakeBuilder<frc971::Constraint>();
    voltage_constraint_builder.add_constraint_type(
        frc971::ConstraintType::VOLTAGE);
    voltage_constraint_builder.add_value(6.0);
    voltage_constraint_offset = voltage_constraint_builder.Finish();
  }

  flatbuffers::Offset<
      flatbuffers::Vector<flatbuffers::Offset<frc971::Constraint>>>
      constraints_offset =
          builder->fbb()->CreateVector<flatbuffers::Offset<frc971::Constraint>>(
              {longitudinal_constraint_offset, lateral_constraint_offset,
               voltage_constraint_offset});

  const float startx = 0.4;
  const float starty = 3.4;
  flatbuffers::Offset<flatbuffers::Vector<float>> spline_x_offset =
      builder->fbb()->CreateVector<float>({0.0f + startx, 0.6f + startx,
                                           0.6f + startx, 0.4f + startx,
                                           0.4f + startx, 1.0f + startx});
  flatbuffers::Offset<flatbuffers::Vector<float>> spline_y_offset =
      builder->fbb()->CreateVector<float>({starty - 0.0f, starty - 0.0f,
                                           starty - 0.3f, starty - 0.7f,
                                           starty - 1.0f, starty - 1.0f});

  frc971::MultiSpline::Builder multispline_builder =
      builder->MakeBuilder<frc971::MultiSpline>();

  multispline_builder.add_spline_count(1);
  multispline_builder.add_constraints(constraints_offset);
  multispline_builder.add_spline_x(spline_x_offset);
  multispline_builder.add_spline_y(spline_y_offset);

  return FixSpline(builder, multispline_builder.Finish(), alliance);
}

flatbuffers::Offset<frc971::MultiSpline> AutonomousSplines::TestSpline(
    aos::Sender<frc971::control_loops::drivetrain::SplineGoal>::Builder
        *builder,
    aos::Alliance alliance) {
  return FixSpline(
      builder,
      aos::CopyFlatBuffer<frc971::MultiSpline>(test_spline_, builder->fbb()),
      alliance);
}

flatbuffers::Offset<frc971::MultiSpline> AutonomousSplines::StraightLine(
    aos::Sender<frc971::control_loops::drivetrain::SplineGoal>::Builder
        *builder,
    aos::Alliance alliance) {
  flatbuffers::Offset<flatbuffers::Vector<float>> spline_x_offset =
      builder->fbb()->CreateVector<float>(
          {-12.3, -11.9, -11.5, -11.1, -10.6, -10.0});
  flatbuffers::Offset<flatbuffers::Vector<float>> spline_y_offset =
      builder->fbb()->CreateVector<float>({1.25, 1.25, 1.25, 1.25, 1.25, 1.25});

  frc971::MultiSpline::Builder multispline_builder =
      builder->MakeBuilder<frc971::MultiSpline>();

  multispline_builder.add_spline_count(1);
  multispline_builder.add_spline_x(spline_x_offset);
  multispline_builder.add_spline_y(spline_y_offset);

  return FixSpline(builder, multispline_builder.Finish(), alliance);
}

flatbuffers::Offset<frc971::MultiSpline>
AutonomousSplines::MobilityAndShootSpline(
    aos::Sender<frc971::control_loops::drivetrain::SplineGoal>::Builder
        *builder,
    aos::Alliance alliance) {
  return FixSpline(builder,
                   aos::CopyFlatBuffer<frc971::MultiSpline>(
                       mobility_and_shoot_spline_, builder->fbb()),
                   alliance);
}

flatbuffers::Offset<frc971::MultiSpline> AutonomousSplines::FourPieceSpline1(
    aos::Sender<frc971::control_loops::drivetrain::SplineGoal>::Builder
        *builder,
    aos::Alliance alliance) {
  return FixSpline(builder,
                   aos::CopyFlatBuffer<frc971::MultiSpline>(
                       four_piece_spline_1_, builder->fbb()),
                   alliance);
}
flatbuffers::Offset<frc971::MultiSpline> AutonomousSplines::FourPieceSpline2(
    aos::Sender<frc971::control_loops::drivetrain::SplineGoal>::Builder
        *builder,
    aos::Alliance alliance) {
  return FixSpline(builder,
                   aos::CopyFlatBuffer<frc971::MultiSpline>(
                       four_piece_spline_2_, builder->fbb()),
                   alliance);
}
flatbuffers::Offset<frc971::MultiSpline> AutonomousSplines::FourPieceSpline3(
    aos::Sender<frc971::control_loops::drivetrain::SplineGoal>::Builder
        *builder,
    aos::Alliance alliance) {
  return FixSpline(builder,
                   aos::CopyFlatBuffer<frc971::MultiSpline>(
                       four_piece_spline_3_, builder->fbb()),
                   alliance);
}

flatbuffers::Offset<frc971::MultiSpline> AutonomousSplines::FourPieceSpline4(
    aos::Sender<frc971::control_loops::drivetrain::SplineGoal>::Builder
        *builder,
    aos::Alliance alliance) {
  return FixSpline(builder,
                   aos::CopyFlatBuffer<frc971::MultiSpline>(
                       four_piece_spline_4_, builder->fbb()),
                   alliance);
}

flatbuffers::Offset<frc971::MultiSpline> AutonomousSplines::FourPieceSpline5(
    aos::Sender<frc971::control_loops::drivetrain::SplineGoal>::Builder
        *builder,
    aos::Alliance alliance) {
  return FixSpline(builder,
                   aos::CopyFlatBuffer<frc971::MultiSpline>(
                       four_piece_spline_5_, builder->fbb()),
                   alliance);
}

flatbuffers::Offset<frc971::MultiSpline>
AutonomousSplines::TwoPieceStealSpline1(
    aos::Sender<frc971::control_loops::drivetrain::SplineGoal>::Builder
        *builder,
    aos::Alliance alliance) {
  return FixSpline(builder,
                   aos::CopyFlatBuffer<frc971::MultiSpline>(
                       two_piece_steal_spline_1_, builder->fbb()),
                   alliance);
}

flatbuffers::Offset<frc971::MultiSpline>
AutonomousSplines::TwoPieceStealSpline2(
    aos::Sender<frc971::control_loops::drivetrain::SplineGoal>::Builder
        *builder,
    aos::Alliance alliance) {
  return FixSpline(builder,
                   aos::CopyFlatBuffer<frc971::MultiSpline>(
                       two_piece_steal_spline_2_, builder->fbb()),
                   alliance);
}
flatbuffers::Offset<frc971::MultiSpline>
AutonomousSplines::TwoPieceStealSpline3(
    aos::Sender<frc971::control_loops::drivetrain::SplineGoal>::Builder
        *builder,
    aos::Alliance alliance) {
  return FixSpline(builder,
                   aos::CopyFlatBuffer<frc971::MultiSpline>(
                       two_piece_steal_spline_3_, builder->fbb()),
                   alliance);
}

flatbuffers::Offset<frc971::MultiSpline>
AutonomousSplines::TwoPieceStealSpline4(
    aos::Sender<frc971::control_loops::drivetrain::SplineGoal>::Builder
        *builder,
    aos::Alliance alliance) {
  return FixSpline(builder,
                   aos::CopyFlatBuffer<frc971::MultiSpline>(
                       two_piece_steal_spline_4_, builder->fbb()),
                   alliance);
}

flatbuffers::Offset<frc971::MultiSpline>
AutonomousSplines::TwoPieceViaStageSpline1(
    aos::Sender<frc971::control_loops::drivetrain::SplineGoal>::Builder
        *builder,
    aos::Alliance alliance) {
  return FixSpline(builder,
                   aos::CopyFlatBuffer<frc971::MultiSpline>(
                       two_piece_via_stage_spline_1_, builder->fbb()),
                   alliance);
}

flatbuffers::Offset<frc971::MultiSpline>
AutonomousSplines::TwoPieceViaStageSpline2(
    aos::Sender<frc971::control_loops::drivetrain::SplineGoal>::Builder
        *builder,
    aos::Alliance alliance) {
  return FixSpline(builder,
                   aos::CopyFlatBuffer<frc971::MultiSpline>(
                       two_piece_via_stage_spline_2_, builder->fbb()),
                   alliance);
}
flatbuffers::Offset<frc971::MultiSpline>
AutonomousSplines::TwoPieceViaStageSpline3(
    aos::Sender<frc971::control_loops::drivetrain::SplineGoal>::Builder
        *builder,
    aos::Alliance alliance) {
  return FixSpline(builder,
                   aos::CopyFlatBuffer<frc971::MultiSpline>(
                       two_piece_via_stage_spline_3_, builder->fbb()),
                   alliance);
}

flatbuffers::Offset<frc971::MultiSpline>
AutonomousSplines::TwoPieceViaStageSpline4(
    aos::Sender<frc971::control_loops::drivetrain::SplineGoal>::Builder
        *builder,
    aos::Alliance alliance) {
  return FixSpline(builder,
                   aos::CopyFlatBuffer<frc971::MultiSpline>(
                       two_piece_via_stage_spline_4_, builder->fbb()),
                   alliance);
}

}  // namespace y2024::autonomous
