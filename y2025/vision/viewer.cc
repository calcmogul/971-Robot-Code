#include "absl/strings/match.h"
#include <opencv2/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "aos/events/shm_event_loop.h"
#include "aos/init.h"
#include "aos/json_to_flatbuffer.h"
#include "aos/time/time.h"
#include "frc971/constants/constants_sender_lib.h"
#include "frc971/vision/vision_generated.h"
#include "frc971/vision/vision_util_lib.h"
#include "y2025/vision/vision_util.h"

ABSL_FLAG(std::string, capture, "",
          "If set, capture a single image and save it to this filename.");
ABSL_FLAG(std::string, channel, "/camera", "Channel name for the image.");
ABSL_FLAG(std::string, config, "aos_config.json",
          "Path to the config file to use.");
ABSL_FLAG(int32_t, rate, 100, "Time in milliseconds to wait between images");
ABSL_FLAG(double, scale, 1.0, "Scale factor for images being displayed");

namespace y2025::vision {
namespace {

using frc971::vision::CameraImage;

bool DisplayLoop(const cv::Mat intrinsics, const cv::Mat dist_coeffs,
                 aos::Fetcher<CameraImage> *image_fetcher) {
  const CameraImage *image;

  // Read next image
  if (!image_fetcher->Fetch()) {
    VLOG(2) << "Couldn't fetch next image";
    return true;
  }
  image = image_fetcher->get();
  CHECK(image != nullptr) << "Couldn't read image";

  // Create color image:
  cv::Mat image_color_mat(cv::Size(image->cols(), image->rows()), CV_8UC2,
                          (void *)image->data()->data());
  cv::Mat bgr_image(cv::Size(image->cols(), image->rows()), CV_8UC3);
  cv::cvtColor(image_color_mat, bgr_image, cv::COLOR_YUV2BGR_YUYV);

  if (!absl::GetFlag(FLAGS_capture).empty()) {
    if (absl::EndsWith(absl::GetFlag(FLAGS_capture), ".bfbs")) {
      aos::WriteFlatbufferToFile(absl::GetFlag(FLAGS_capture),
                                 image_fetcher->CopyFlatBuffer());
    } else {
      cv::imwrite(absl::GetFlag(FLAGS_capture), bgr_image);
    }

    return false;
  }

  cv::Mat undistorted_image;
  cv::undistort(bgr_image, undistorted_image, intrinsics, dist_coeffs);
  if (absl::GetFlag(FLAGS_scale) != 1.0) {
    cv::resize(undistorted_image, undistorted_image, cv::Size(),
               absl::GetFlag(FLAGS_scale), absl::GetFlag(FLAGS_scale));
  }
  cv::imshow("Display", undistorted_image);

  int keystroke = cv::waitKey(1);
  if ((keystroke & 0xFF) == static_cast<int>('c')) {
    // Convert again, to get clean image
    cv::cvtColor(image_color_mat, bgr_image, cv::COLOR_YUV2BGR_YUYV);
    std::stringstream name;
    name << "capture-" << aos::realtime_clock::now() << ".png";
    cv::imwrite(name.str(), bgr_image);
    LOG(INFO) << "Saved image file: " << name.str();
  } else if ((keystroke & 0xFF) == static_cast<int>('q')) {
    return false;
  }
  return true;
}

void ViewerMain() {
  aos::FlatbufferDetachedBuffer<aos::Configuration> config =
      aos::configuration::ReadConfig(absl::GetFlag(FLAGS_config));

  frc971::constants::WaitForConstants<y2025::Constants>(&config.message());

  aos::ShmEventLoop event_loop(&config.message());

  frc971::constants::ConstantsFetcher<y2025::Constants> constants_fetcher(
      &event_loop);
  CHECK(absl::GetFlag(FLAGS_channel).length() == 8);
  int camera_id = std::stoi(absl::GetFlag(FLAGS_channel).substr(7, 1));
  const auto *calibration_data = FindCameraCalibration(
      constants_fetcher.constants(), event_loop.node()->name()->string_view(),
      camera_id);
  const cv::Mat intrinsics = frc971::vision::CameraIntrinsics(calibration_data);
  const cv::Mat dist_coeffs =
      frc971::vision::CameraDistCoeffs(calibration_data);

  aos::Fetcher<CameraImage> image_fetcher =
      event_loop.MakeFetcher<CameraImage>(absl::GetFlag(FLAGS_channel));

  // Run the display loop
  event_loop.AddPhasedLoop(
      [&](int) {
        if (!DisplayLoop(intrinsics, dist_coeffs, &image_fetcher)) {
          LOG(INFO) << "Calling event_loop Exit";
          event_loop.Exit();
        };
      },
      ::std::chrono::milliseconds(absl::GetFlag(FLAGS_rate)));

  event_loop.Run();

  image_fetcher = aos::Fetcher<CameraImage>();
}

}  // namespace
}  // namespace y2025::vision

int main(int argc, char **argv) {
  aos::InitGoogle(&argc, &argv);
  y2025::vision::ViewerMain();
}
