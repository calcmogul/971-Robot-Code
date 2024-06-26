#include "aos/ipc_lib/robust_ownership_tracker.h"

namespace aos::ipc_lib {

::std::string RobustOwnershipTracker::DebugString() const {
  ::std::stringstream s;
  s << "{.tid=aos_mutex(" << ::std::hex << mutex_.futex;

  if (mutex_.futex != 0) {
    s << ":";
    if (mutex_.futex & FUTEX_OWNER_DIED) {
      s << "FUTEX_OWNER_DIED|";
    }
    s << "tid=" << (mutex_.futex & FUTEX_TID_MASK);
  }

  s << "),}";
  return s.str();
}

}  // namespace aos::ipc_lib
