load("@com_github_google_flatbuffers//:typescript.bzl", "flatbuffer_ts_library")
load("//aos/flatbuffers:generate.bzl", "static_flatbuffer")
load("//tools/build_rules:js.bzl", "ts_project")

package(default_visibility = ["//visibility:public"])

static_flatbuffer(
    name = "superstructure_goal_fbs",
    srcs = [
        "superstructure_goal.fbs",
    ],
    deps = [
        "//frc971/control_loops:control_loops_fbs",
        "//frc971/control_loops:profiled_subsystem_fbs",
    ],
)

flatbuffer_ts_library(
    name = "superstructure_goal_ts_fbs",
    srcs = [
        "superstructure_goal.fbs",
    ],
    deps = [
        "//frc971/control_loops:control_loops_ts_fbs",
        "//frc971/control_loops:profiled_subsystem_ts_fbs",
    ],
)

static_flatbuffer(
    name = "superstructure_output_fbs",
    srcs = [
        "superstructure_output.fbs",
    ],
)

static_flatbuffer(
    name = "superstructure_status_fbs",
    srcs = [
        "superstructure_status.fbs",
    ],
    deps = [
        "//frc971/control_loops:control_loops_fbs",
        "//frc971/control_loops:profiled_subsystem_fbs",
    ],
)

flatbuffer_ts_library(
    name = "superstructure_status_ts_fbs",
    srcs = [
        "superstructure_status.fbs",
    ],
    deps = [
        "//frc971/control_loops:control_loops_ts_fbs",
        "//frc971/control_loops:profiled_subsystem_ts_fbs",
    ],
)

static_flatbuffer(
    name = "superstructure_position_fbs",
    srcs = [
        "superstructure_position.fbs",
    ],
    deps = [
        "//frc971/control_loops:control_loops_fbs",
        "//frc971/control_loops:profiled_subsystem_fbs",
    ],
)

flatbuffer_ts_library(
    name = "superstructure_position_ts_fbs",
    srcs = [
        "superstructure_position.fbs",
    ],
    deps = [
        "//frc971/control_loops:control_loops_ts_fbs",
        "//frc971/control_loops:profiled_subsystem_ts_fbs",
    ],
)

static_flatbuffer(
    name = "superstructure_can_position_fbs",
    srcs = ["superstructure_can_position.fbs"],
    deps = ["//frc971/control_loops:can_talonfx_fbs"],
)

static_flatbuffer(
    name = "superstructure_can_range_status_fbs",
    srcs = ["superstructure_can_range_status.fbs"],
    deps = ["//frc971/wpilib:can_range_fbs"],
)

flatbuffer_ts_library(
    name = "superstructure_can_position_ts_fbs",
    srcs = ["superstructure_can_position.fbs"],
    target_compatible_with = ["@platforms//os:linux"],
    deps = ["//frc971/control_loops:can_talonfx_ts_fbs"],
)

cc_library(
    name = "superstructure_lib",
    srcs = [
        "superstructure.cc",
    ],
    hdrs = [
        "superstructure.h",
    ],
    data = [],
    deps = [
        ":superstructure_can_position_fbs",
        ":superstructure_goal_fbs",
        ":superstructure_output_fbs",
        ":superstructure_position_fbs",
        ":superstructure_status_fbs",
        "//aos:flatbuffer_merge",
        "//aos/events:event_loop",
        "//frc971/constants:constants_sender_lib",
        "//frc971/control_loops:control_loop",
        "//frc971/control_loops:static_zeroing_single_dof_profiled_subsystem",
        "//frc971/zeroing:absolute_encoder",
        "//frc971/zeroing:pot_and_absolute_encoder",
        "//y2025:constants",
        "//y2025/constants:constants_fbs",
        "//y2025/constants:simulated_constants_sender",
    ],
)

cc_binary(
    name = "superstructure",
    srcs = [
        "superstructure_main.cc",
    ],
    deps = [
        ":superstructure_lib",
        "//aos:init",
        "//aos/events:shm_event_loop",
    ],
)

cc_test(
    name = "superstructure_lib_test",
    srcs = [
        "superstructure_lib_test.cc",
    ],
    data = [
        "//y2025:aos_config",
    ],
    deps = [
        ":superstructure_can_position_fbs",
        ":superstructure_goal_fbs",
        ":superstructure_lib",
        ":superstructure_output_fbs",
        ":superstructure_position_fbs",
        ":superstructure_status_fbs",
        "//aos:json_to_flatbuffer",
        "//aos:math",
        "//aos/events/logging:log_writer",
        "//aos/testing:googletest",
        "//aos/time",
        "//frc971/control_loops:capped_test_plant",
        "//frc971/control_loops:control_loop_test",
        "//frc971/control_loops:position_sensor_sim",
        "//frc971/control_loops:subsystem_simulator",
        "//frc971/control_loops:team_number_test_environment",
        "//y2025/control_loops/superstructure/elevator:elevator_plants",
        "//y2025/control_loops/superstructure/ground_intake_pivot:ground_intake_pivot_plants",
        "//y2025/control_loops/superstructure/pivot:pivot_plants",
        "//y2025/control_loops/superstructure/wrist:wrist_plants",
    ],
)

cc_binary(
    name = "superstructure_replay",
    srcs = ["superstructure_replay.cc"],
    deps = [
        ":superstructure_lib",
        "//aos:configuration",
        "//aos:init",
        "//aos/events:simulated_event_loop",
        "//aos/events/logging:log_reader",
        "//aos/network:team_number",
    ],
)

cc_library(
    name = "led_indicator",
    srcs = ["led_indicator.cc"],
    hdrs = ["led_indicator.h"],
    data = [
        "@ctre_phoenix_api_cpp_athena//:shared_libraries",
        "@ctre_phoenix_cci_athena//:shared_libraries",
    ],
    target_compatible_with = ["//tools/platforms/hardware:roborio"],
    deps = [
        ":superstructure_goal_fbs",
        ":superstructure_output_fbs",
        ":superstructure_position_fbs",
        ":superstructure_status_fbs",
        "//aos/events:event_loop",
        "//aos/network:message_bridge_client_fbs",
        "//aos/network:message_bridge_server_fbs",
        "//frc971/control_loops:control_loop",
        "//frc971/control_loops:control_loops_fbs",
        "//frc971/control_loops:profiled_subsystem_fbs",
        "//frc971/control_loops/swerve:swerve_drivetrain_status_fbs",
        "//frc971/queues:gyro_fbs",
        "//third_party:phoenix",
        "//third_party:wpilib",
        "@com_google_absl//absl/flags:flag",
    ],
)

ts_project(
    name = "superstructure_plotter",
    srcs = ["superstructure_plotter.ts"],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        "//aos/network/www:aos_plotter",
        "//aos/network/www:colors",
        "//aos/network/www:proxy",
    ],
)
