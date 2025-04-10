load("@com_github_google_flatbuffers//:typescript.bzl", "flatbuffer_ts_library")
load("//aos/flatbuffers:generate.bzl", "static_flatbuffer")
load("//tools/build_rules:js.bzl", "ts_project")

ts_project(
    name = "localizer_plotter",
    srcs = ["localizer_plotter.ts"],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
    deps = [
        "//aos/network/www:aos_plotter",
        "//aos/network/www:colors",
        "//aos/network/www:proxy",
        "//frc971/wpilib:imu_plot_utils",
    ],
)

static_flatbuffer(
    name = "status_fbs",
    srcs = [
        "status.fbs",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
    deps = [
        "//frc971/control_loops/drivetrain:drivetrain_status_fbs",
        "//frc971/imu_reader:imu_failures_fbs",
    ],
)

flatbuffer_ts_library(
    name = "status_ts_fbs",
    srcs = ["status.fbs"],
    visibility = ["//visibility:public"],
    deps = [
        "//frc971/control_loops/drivetrain:drivetrain_status_ts_fbs",
        "//frc971/imu_reader:imu_failures_ts_fbs",
    ],
)

static_flatbuffer(
    name = "visualization_fbs",
    srcs = [
        "visualization.fbs",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":status_fbs",
    ],
)

flatbuffer_ts_library(
    name = "visualization_ts_fbs",
    srcs = ["visualization.fbs"],
    visibility = ["//visibility:public"],
    deps = [
        ":status_ts_fbs",
    ],
)

static_flatbuffer(
    name = "scoring_map_fbs",
    srcs = [
        "scoring_map.fbs",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//frc971/vision:target_map_fbs",
    ],
)

static_flatbuffer(
    name = "relative_scoring_map_fbs",
    srcs = [
        "relative_scoring_map.fbs",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":scoring_map_fbs",
    ],
)

cc_library(
    name = "map_expander_lib",
    srcs = ["map_expander_lib.cc"],
    hdrs = ["map_expander_lib.h"],
    deps = [
        ":relative_scoring_map_fbs",
        ":scoring_map_fbs",
        "//aos:flatbuffers",
        "//aos:json_to_flatbuffer",
        "//frc971/vision:target_map_utils",
    ],
)

cc_binary(
    name = "map_expander",
    srcs = ["map_expander.cc"],
    data = [
        "//y2023/constants:scoring_map",
        "//y2023/vision/maps",
    ],
    deps = [
        ":map_expander_lib",
        "//aos:init",
    ],
)

cc_test(
    name = "map_expander_lib_test",
    srcs = ["map_expander_lib_test.cc"],
    data = [
        "//y2023/constants:scoring_map",
        "//y2023/vision/maps",
    ],
    deps = [
        ":map_expander_lib",
        "//aos/testing:flatbuffer_eq",
        "//aos/testing:googletest",
    ],
)

cc_library(
    name = "localizer",
    srcs = ["localizer.cc"],
    hdrs = ["localizer.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":status_fbs",
        ":visualization_fbs",
        "//aos/containers:sized_array",
        "//aos/events:event_loop",
        "//aos/network:message_bridge_client_fbs",
        "//frc971/constants:constants_sender_lib",
        "//frc971/control_loops:pose",
        "//frc971/control_loops/drivetrain:hybrid_ekf",
        "//frc971/control_loops/drivetrain:improved_down_estimator",
        "//frc971/control_loops/drivetrain:localizer_fbs",
        "//frc971/control_loops/drivetrain/localization:localizer_output_fbs",
        "//frc971/control_loops/drivetrain/localization:utils",
        "//frc971/imu_reader:imu_watcher",
        "//frc971/vision:target_map_fbs",
        "//frc971/vision:target_map_utils",
        "//y2023:constants",
        "//y2023/constants:constants_fbs",
    ],
)

cc_test(
    name = "localizer_test",
    srcs = ["localizer_test.cc"],
    args = ["--allow_duplicate_samples=true"],
    data = ["//y2023:aos_config"],
    deps = [
        ":localizer",
        ":status_fbs",
        "//aos/events:simulated_event_loop",
        "//aos/events/logging:log_writer",
        "//aos/testing:googletest",
        "//frc971/control_loops/drivetrain:drivetrain_test_lib",
        "//frc971/control_loops/drivetrain:localizer_fbs",
        "//frc971/vision:target_map_utils",
        "//y2023/constants:simulated_constants_sender",
        "//y2023/control_loops/drivetrain:drivetrain_base",
    ],
)

cc_binary(
    name = "localizer_main",
    srcs = ["localizer_main.cc"],
    visibility = ["//visibility:public"],
    deps = [
        ":localizer",
        "//aos:init",
        "//aos/events:shm_event_loop",
        "//frc971/constants:constants_sender_lib",
        "//y2023/control_loops/drivetrain:drivetrain_base",
    ],
)

ts_project(
    name = "corrections_plotter",
    srcs = ["corrections_plotter.ts"],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
    deps = [
        ":visualization_ts_fbs",
        "//aos/network/www:aos_plotter",
        "//aos/network/www:colors",
        "//aos/network/www:proxy",
    ],
)

cc_binary(
    name = "localizer_replay",
    srcs = ["localizer_replay.cc"],
    data = [
        "//y2023:aos_config",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        ":localizer",
        "//aos:configuration",
        "//aos:init",
        "//aos:json_to_flatbuffer",
        "//aos/events:simulated_event_loop",
        "//aos/events/logging:log_reader",
        "//aos/events/logging:log_writer",
        "//aos/util:simulation_logger",
        "//y2023/control_loops/drivetrain:drivetrain_base",
    ],
)
