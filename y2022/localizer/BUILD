load("@com_github_google_flatbuffers//:typescript.bzl", "flatbuffer_ts_library")
load("//aos:flatbuffers.bzl", "cc_static_flatbuffer")
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
    name = "localizer_status_fbs",
    srcs = [
        "localizer_status.fbs",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
    deps = [
        "//frc971/control_loops:control_loops_fbs",
        "//frc971/control_loops/drivetrain:drivetrain_status_fbs",
        "//frc971/imu_reader:imu_failures_fbs",
    ],
)

flatbuffer_ts_library(
    name = "localizer_status_ts_fbs",
    srcs = ["localizer_status.fbs"],
    visibility = ["//visibility:public"],
    deps = [
        "//frc971/control_loops:control_loops_ts_fbs",
        "//frc971/control_loops/drivetrain:drivetrain_status_ts_fbs",
        "//frc971/imu_reader:imu_failures_ts_fbs",
    ],
)

static_flatbuffer(
    name = "localizer_visualization_fbs",
    srcs = ["localizer_visualization.fbs"],
    visibility = ["//visibility:public"],
    deps = [
        ":localizer_status_fbs",
        "//frc971/control_loops:control_loops_fbs",
        "//frc971/control_loops/drivetrain:drivetrain_status_fbs",
    ],
)

flatbuffer_ts_library(
    name = "localizer_visualization_ts_fbs",
    srcs = ["localizer_visualization.fbs"],
    visibility = ["//visibility:public"],
    deps = [
        ":localizer_status_ts_fbs",
        "//frc971/control_loops:control_loops_ts_fbs",
        "//frc971/control_loops/drivetrain:drivetrain_status_ts_fbs",
    ],
)

cc_static_flatbuffer(
    name = "localizer_schema",
    function = "frc971::controls::LocalizerStatusSchema",
    target = ":localizer_status_fbs_reflection_out",
    visibility = ["//visibility:public"],
)

cc_library(
    name = "localizer",
    srcs = ["localizer.cc"],
    hdrs = ["localizer.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":localizer_status_fbs",
        ":localizer_visualization_fbs",
        "//aos/containers:ring_buffer",
        "//aos/containers:sized_array",
        "//aos/events:event_loop",
        "//aos/network:message_bridge_server_fbs",
        "//aos/time",
        "//frc971/control_loops:c2d",
        "//frc971/control_loops:control_loops_fbs",
        "//frc971/control_loops/drivetrain:drivetrain_output_fbs",
        "//frc971/control_loops/drivetrain:drivetrain_status_fbs",
        "//frc971/control_loops/drivetrain:improved_down_estimator",
        "//frc971/control_loops/drivetrain:localizer_fbs",
        "//frc971/control_loops/drivetrain/localization:localizer_output_fbs",
        "//frc971/control_loops/drivetrain/localization:utils",
        "//frc971/imu_reader:imu_watcher",
        "//frc971/input:joystick_state_fbs",
        "//frc971/vision:calibration_fbs",
        "//frc971/wpilib:imu_batch_fbs",
        "//frc971/wpilib:imu_fbs",
        "//y2022:constants",
        "//y2022/control_loops/superstructure:superstructure_status_fbs",
        "//y2022/vision:target_estimate_fbs",
        "@org_tuxfamily_eigen//:eigen",
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
        "//y2022/control_loops/drivetrain:drivetrain_base",
    ],
)

cc_test(
    name = "localizer_test",
    srcs = ["localizer_test.cc"],
    args = ["--allow_duplicate_samples=true"],
    data = [
        "//y2022:aos_config",
    ],
    shard_count = 13,
    deps = [
        ":localizer",
        "//aos/events:simulated_event_loop",
        "//aos/events/logging:log_writer",
        "//aos/testing:googletest",
        "//frc971/control_loops/drivetrain:drivetrain_test_lib",
        "//y2022/control_loops/drivetrain:drivetrain_base",
    ],
)

cc_binary(
    name = "localizer_replay",
    srcs = ["localizer_replay.cc"],
    data = [
        "//y2022:aos_config",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        ":localizer",
        ":localizer_schema",
        "//aos:configuration",
        "//aos:init",
        "//aos:json_to_flatbuffer",
        "//aos/events:simulated_event_loop",
        "//aos/events/logging:log_reader",
        "//aos/events/logging:log_writer",
        "//aos/util:simulation_logger",
        "//y2022/control_loops/drivetrain:drivetrain_base",
    ],
)

cc_binary(
    name = "imu_main",
    srcs = ["imu_main.cc"],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
    deps = [
        "//aos:init",
        "//aos/events:shm_event_loop",
        "//frc971/imu_reader:imu",
        "//y2022:constants",
    ],
)
