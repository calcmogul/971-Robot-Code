#include "absl/flags/flag.h"
#include "absl/log/check.h"
#include "absl/log/log.h"

#include "aos/events/shm_event_loop.h"
#include "aos/init.h"
#include "aos/logging/dynamic_logging.h"
#include "aos/network/message_bridge_server_lib.h"
#include "aos/network/sctp_lib.h"
#include "aos/sha256.h"

ABSL_FLAG(std::string, config, "aos_config.json", "Path to the config.");
ABSL_FLAG(int32_t, rt_priority, -1, "If > 0, run as this RT priority");
ABSL_FLAG(bool, wants_sctp_authentication, false,
          "When set, try to use SCTP authentication if provided by the kernel");

namespace aos::message_bridge {

using ::aos::util::ReadFileToVecOrDie;

int Main() {
  aos::FlatbufferDetachedBuffer<aos::Configuration> config =
      aos::configuration::ReadConfig(absl::GetFlag(FLAGS_config));

  aos::ShmEventLoop event_loop(&config.message());
  if (absl::GetFlag(FLAGS_rt_priority) > 0) {
    event_loop.SetRuntimeRealtimePriority(absl::GetFlag(FLAGS_rt_priority));
  }

  MessageBridgeServer app(&event_loop, Sha256(config.span()),
                          absl::GetFlag(FLAGS_wants_sctp_authentication)
                              ? SctpAuthMethod::kAuth
                              : SctpAuthMethod::kNoAuth);

  logging::DynamicLogging dynamic_logging(&event_loop);

  // TODO(austin): Track which messages didn't make it in time and need to be
  // logged locally and forwarded.

  event_loop.Run();

  return EXIT_SUCCESS;
}

}  // namespace aos::message_bridge

int main(int argc, char **argv) {
  aos::InitGoogle(&argc, &argv);

  return aos::message_bridge::Main();
}
