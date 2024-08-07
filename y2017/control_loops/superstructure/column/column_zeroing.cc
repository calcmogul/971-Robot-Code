#include "y2017/control_loops/superstructure/column/column_zeroing.h"

#include "frc971/zeroing/wrap.h"

namespace y2017::control_loops::superstructure::column {

ColumnZeroingEstimator::ColumnZeroingEstimator(
    const ZeroingConstants &column_constants)
    : indexer_(column_constants.indexer_zeroing),
      turret_(column_constants.turret_zeroing),
      turret_zeroed_distance_(column_constants.turret_zeroed_distance) {
  Reset();
}

void ColumnZeroingEstimator::Reset() {
  zeroed_ = false;
  error_ = false;
  offset_ready_ = false;
  indexer_.Reset();
  turret_.Reset();
}

void ColumnZeroingEstimator::TriggerError() {
  if (!error_) {
    AOS_LOG(ERROR, "Manually triggered zeroing error.\n");
    error_ = true;
  }
}

void ColumnZeroingEstimator::UpdateEstimate(const ColumnPosition &position) {
  indexer_.UpdateEstimate(*position.indexer());
  turret_.UpdateEstimate(*position.turret());

  if (indexer_.zeroed() && turret_.zeroed()) {
    indexer_offset_ = indexer_.offset();

    // Compute the current turret position.
    const double current_turret =
        indexer_offset_ + position.indexer()->encoder() + turret_.offset() +
        position.turret()->encoder();

    // Now, we can compute the turret position which is closest to 0 radians
    // (within +- M_PI).
    const double adjusted_turret =
        ::frc971::zeroing::Wrap(0.0, current_turret, M_PI * 2.0);

    // Now, compute the actual turret offset.
    turret_offset_ = adjusted_turret - position.turret()->encoder() -
                     (indexer_offset_ + position.indexer()->encoder());
    offset_ready_ = true;

    // If we are close enough to 0, we are zeroed.  Otherwise, we don't know
    // which revolution we are on and need more info.  We will always report the
    // turret position as within +- M_PI from 0 with the provided offset.
    if (::std::abs(position.indexer()->encoder() +
                   position.turret()->encoder() + indexer_offset_ +
                   turret_offset_) < turret_zeroed_distance_) {
      zeroed_ = true;
    }

    // TODO(austin): Latch the offset when we get zeroed for the first time and
    // see if we get conflicting information further on.
  }
}

flatbuffers::Offset<ColumnZeroingEstimator::State>
ColumnZeroingEstimator::GetEstimatorState(
    flatbuffers::FlatBufferBuilder *fbb) const {
  flatbuffers::Offset<frc971::HallEffectAndPositionEstimatorState>
      indexer_offset = indexer_.GetEstimatorState(fbb);
  flatbuffers::Offset<frc971::HallEffectAndPositionEstimatorState>
      turret_offset = turret_.GetEstimatorState(fbb);

  State::Builder state_builder(*fbb);
  state_builder.add_indexer(indexer_offset);
  state_builder.add_turret(turret_offset);
  state_builder.add_error(error());
  state_builder.add_zeroed(zeroed());
  return state_builder.Finish();
}

}  // namespace y2017::control_loops::superstructure::column
