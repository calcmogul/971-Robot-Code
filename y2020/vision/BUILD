load("//aos/flatbuffers:generate.bzl", "static_flatbuffer")

static_flatbuffer(
    name = "galactic_search_path_fbs",
    srcs = ["galactic_search_path.fbs"],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//y2020:__subpackages__"],
)

cc_binary(
    name = "camera_reader",
    srcs = [
        "camera_reader_main.cc",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//y2020:__subpackages__"],
    deps = [
        ":camera_reader_lib",
        "//aos:init",
        "//aos/events:shm_event_loop",
    ],
)

cc_library(
    name = "camera_reader_lib",
    srcs = [
        "camera_reader.cc",
    ],
    hdrs = [
        "camera_reader.h",
    ],
    data = [
        "//y2020:aos_config",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//y2020:__subpackages__"] + ["//y2022:__subpackages__"],
    deps = [
        "//aos:flatbuffer_merge",
        "//aos/events:event_loop",
        "//aos/network:team_number",
        "//frc971/vision:v4l2_reader",
        "//frc971/vision:vision_fbs",
        "//third_party:opencv",
        "//y2020/vision/sift:sift971",
        "//y2020/vision/sift:sift_fbs",
        "//y2020/vision/sift:sift_training_fbs",
        "//y2020/vision/tools/python_code:sift_training_data",
    ],
)

cc_binary(
    name = "viewer",
    srcs = [
        "viewer.cc",
    ],
    data = [
        "//y2020:aos_config",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//y2020:__subpackages__"],
    deps = [
        "//aos:init",
        "//aos/events:shm_event_loop",
        "//frc971/vision:v4l2_reader",
        "//frc971/vision:vision_fbs",
        "//third_party:opencv",
        "//y2020/vision/sift:sift_fbs",
    ],
)

cc_binary(
    name = "viewer_replay",
    srcs = [
        "viewer_replay.cc",
    ],
    data = [
        "//y2020:aos_config",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//y2020:__subpackages__"],
    deps = [
        "//aos:init",
        "//aos/events:simulated_event_loop",
        "//aos/events/logging:log_reader",
        "//frc971/vision:vision_fbs",
        "//third_party:opencv",
    ],
)

cc_binary(
    name = "extrinsics_calibration",
    srcs = [
        "extrinsics_calibration.cc",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//y2020:__subpackages__"],
    deps = [
        "//aos:init",
        "//aos/events/logging:log_reader",
        "//frc971/control_loops:profiled_subsystem_fbs",
        "//frc971/vision:extrinsics_calibration",
        "//y2020/control_loops/superstructure:superstructure_status_fbs",
    ],
)
