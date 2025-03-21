#include "absl/flags/flag.h"

#include "aos/configuration.h"
#include "aos/events/logging/log_reader.h"
#include "aos/events/logging/log_writer.h"
#include "aos/events/simulated_event_loop.h"
#include "aos/init.h"
#include "aos/json_to_flatbuffer.h"
#include "aos/network/team_number.h"
#include "aos/util/simulation_logger.h"
#include "y2023/control_loops/drivetrain/drivetrain_base.h"
#include "y2023/localizer/localizer.h"

ABSL_FLAG(std::string, config, "y2023/aos_config.json",
          "Name of the config file to replay using.");
ABSL_FLAG(int32_t, team, 9971, "Team number to use for logfile replay.");
ABSL_FLAG(std::string, output_folder, "/tmp/replayed",
          "Name of the folder to write replayed logs to.");

int main(int argc, char **argv) {
  aos::InitGoogle(&argc, &argv);

  aos::network::OverrideTeamNumber(absl::GetFlag(FLAGS_team));

  const aos::FlatbufferDetachedBuffer<aos::Configuration> config =
      aos::configuration::ReadConfig(absl::GetFlag(FLAGS_config));

  // sort logfiles
  const std::vector<aos::logger::LogFile> logfiles =
      aos::logger::SortParts(aos::logger::FindLogs(argc, argv));

  // open logfiles
  aos::logger::LogReader reader(logfiles, &config.message());

  reader.RemapLoggedChannel("/localizer", "y2023.localizer.Status");
  for (const auto pi : {"pi1", "pi2", "pi3", "pi4"}) {
    reader.RemapLoggedChannel(absl::StrCat("/", pi, "/camera"),
                              "y2023.localizer.Visualization");
  }
  reader.RemapLoggedChannel("/localizer", "frc971.controls.LocalizerOutput");

  auto factory =
      std::make_unique<aos::SimulatedEventLoopFactory>(reader.configuration());

  reader.Register(factory.get());

  // List of nodes to create loggers for (note: currently just roborio; this
  // code was refactored to allow easily adding new loggers to accommodate
  // debugging and potential future changes).
  const std::vector<std::string> nodes_to_log = {"imu"};
  std::vector<std::unique_ptr<aos::util::LoggerState>> loggers =
      aos::util::MakeLoggersForNodes(reader.event_loop_factory(), nodes_to_log,
                                     absl::GetFlag(FLAGS_output_folder));

  const aos::Node *node = nullptr;
  if (aos::configuration::MultiNode(reader.configuration())) {
    node = aos::configuration::GetNode(reader.configuration(), "imu");
  }

  std::unique_ptr<aos::EventLoop> localizer_event_loop =
      reader.event_loop_factory()->MakeEventLoop("localizer", node);
  localizer_event_loop->SkipTimingReport();

  y2023::localizer::Localizer localizer(
      localizer_event_loop.get(),
      y2023::control_loops::drivetrain::GetDrivetrainConfig());

  reader.event_loop_factory()->Run();

  reader.Deregister();

  return 0;
}
