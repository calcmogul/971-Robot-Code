load("//aos/flatbuffers:generate.bzl", "static_flatbuffer")

package(default_visibility = ["//visibility:public"])

static_flatbuffer(
    name = "superstructure_goal_fbs",
    srcs = [
        "superstructure_goal.fbs",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        "//frc971/control_loops:control_loops_fbs",
    ],
)

static_flatbuffer(
    name = "superstructure_position_fbs",
    srcs = [
        "superstructure_position.fbs",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        "//frc971/control_loops:control_loops_fbs",
    ],
)

static_flatbuffer(
    name = "superstructure_status_fbs",
    srcs = [
        "superstructure_status.fbs",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        "//frc971/control_loops:control_loops_fbs",
        "//frc971/control_loops:profiled_subsystem_fbs",
    ],
)

static_flatbuffer(
    name = "superstructure_output_fbs",
    srcs = [
        "superstructure_output.fbs",
    ],
    target_compatible_with = ["@platforms//os:linux"],
)

cc_library(
    name = "superstructure_lib",
    srcs = [
        "superstructure.cc",
    ],
    hdrs = [
        "superstructure.h",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        ":superstructure_goal_fbs",
        ":superstructure_output_fbs",
        ":superstructure_position_fbs",
        ":superstructure_status_fbs",
        ":vision_distance_average",
        "//aos/events:event_loop",
        "//frc971/control_loops:control_loop",
        "//y2017:constants",
        "//y2017/control_loops/superstructure/column",
        "//y2017/control_loops/superstructure/hood",
        "//y2017/control_loops/superstructure/intake",
        "//y2017/control_loops/superstructure/shooter",
    ],
)

cc_test(
    name = "superstructure_lib_test",
    srcs = [
        "superstructure_lib_test.cc",
    ],
    data = ["//y2017:aos_config"],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        ":superstructure_goal_fbs",
        ":superstructure_lib",
        ":superstructure_output_fbs",
        ":superstructure_position_fbs",
        ":superstructure_status_fbs",
        "//aos:math",
        "//aos/testing:googletest",
        "//aos/time",
        "//frc971/control_loops:control_loop_test",
        "//frc971/control_loops:position_sensor_sim",
        "//frc971/control_loops:team_number_test_environment",
        "//y2017/control_loops/superstructure/column:column_plants",
        "//y2017/control_loops/superstructure/hood:hood_plants",
        "//y2017/control_loops/superstructure/intake:intake_plants",
        "//y2017/control_loops/superstructure/shooter:shooter_plants",
    ],
)

cc_binary(
    name = "superstructure",
    srcs = [
        "superstructure_main.cc",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        ":superstructure_lib",
        "//aos:init",
        "//aos/events:shm_event_loop",
    ],
)

cc_library(
    name = "vision_time_adjuster",
    srcs = [
        "vision_time_adjuster.cc",
    ],
    hdrs = [
        "vision_time_adjuster.h",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        "//aos/containers:ring_buffer",
        "//frc971/control_loops:control_loops_fbs",
        "//frc971/control_loops:profiled_subsystem_fbs",
        "//frc971/control_loops/drivetrain:drivetrain_status_fbs",
        "//y2017/control_loops/drivetrain:polydrivetrain_plants",
        "//y2017/vision:vision_fbs",
    ],
)

cc_test(
    name = "vision_time_adjuster_test",
    srcs = [
        "vision_time_adjuster_test.cc",
    ],
    data = ["//y2017:aos_config"],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        ":vision_time_adjuster",
        "//aos/events:simulated_event_loop",
        "//aos/testing:googletest",
        "//aos/testing:test_logging",
        "//aos/time",
    ],
)

cc_library(
    name = "vision_distance_average",
    hdrs = [
        "vision_distance_average.h",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        "//aos/containers:ring_buffer",
        "//aos/time",
        "//y2017/vision:vision_fbs",
    ],
)

cc_test(
    name = "vision_distance_average_test",
    srcs = [
        "vision_distance_average_test.cc",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        ":vision_distance_average",
        "//aos:flatbuffers",
        "//aos/testing:googletest",
        "//aos/time",
    ],
)
