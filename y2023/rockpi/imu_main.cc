#include "absl/flags/flag.h"

#include "aos/events/shm_event_loop.h"
#include "aos/init.h"
#include "aos/realtime.h"
#include "frc971/imu_reader/imu.h"
#include "y2023/constants.h"

ABSL_FLAG(std::string, config, "aos_config.json",
          "Path to the config file to use.");

int main(int argc, char *argv[]) {
  aos::InitGoogle(&argc, &argv);

  aos::FlatbufferDetachedBuffer<aos::Configuration> config =
      aos::configuration::ReadConfig(absl::GetFlag(FLAGS_config));

  aos::ShmEventLoop event_loop(&config.message());
  frc971::imu::Imu imu(&event_loop,
                       y2023::constants::Values::DrivetrainEncoderToMeters(1));

  event_loop.SetRuntimeAffinity(aos::MakeCpusetFromCpus({0}));
  event_loop.SetRuntimeRealtimePriority(55);

  event_loop.Run();

  return 0;
}
