load("//aos/flatbuffers:generate.bzl", "static_flatbuffer")

cc_library(
    name = "arm",
    srcs = [
        "arm.cc",
    ],
    hdrs = [
        "arm.h",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
    deps = [
        ":generated_graph",
        "//frc971/control_loops/double_jointed_arm:ekf",
        "//frc971/control_loops/double_jointed_arm:graph",
        "//frc971/control_loops/drivetrain:drivetrain_can_position_fbs",
        "//frc971/zeroing",
        "//frc971/zeroing:pot_and_absolute_encoder",
        "//y2023:constants",
        "//y2023/control_loops/superstructure:superstructure_position_fbs",
        "//y2023/control_loops/superstructure:superstructure_status_fbs",
        "//y2023/control_loops/superstructure/arm:arm_constants",
        "//y2023/control_loops/superstructure/arm:trajectory",
        "//y2023/control_loops/superstructure/roll:roll_plants",
        "//y2023/vision:game_pieces_fbs",
    ],
)

genrule(
    name = "generated_arm_trajectory_genrule",
    outs = [
        "arm_trajectories_generated.bfbs",
    ],
    cmd = "$(location //y2023/control_loops/superstructure/arm:arm_trajectory_generator) --output $(OUTS)",
    target_compatible_with = ["@platforms//os:linux"],
    tools = [
        "//y2023/control_loops/superstructure/arm:arm_trajectory_generator",
    ],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "arm_trajectory_generator",
    srcs = [
        "arm_trajectory_gen.cc",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
    deps = [
        ":arm_constants",
        ":arm_trajectories_fbs",
        "//aos:flatbuffers",
        "//aos:json_to_flatbuffer",
        "//frc971/control_loops/double_jointed_arm:graph",
        "//y2023:constants",
        "//y2023/control_loops/superstructure/arm:generated_graph",
        "//y2023/control_loops/superstructure/arm:trajectory",
        "//y2023/control_loops/superstructure/roll:roll_plants",
    ],
)

genrule(
    name = "generated_graph_genrule",
    outs = [
        "generated_graph.h",
        "generated_graph.cc",
    ],
    cmd = "$(location //y2023/control_loops/python:graph_codegen) $(OUTS)",
    target_compatible_with = ["@platforms//os:linux"],
    tools = [
        "//y2023/control_loops/python:graph_codegen",
    ],
)

static_flatbuffer(
    name = "arm_trajectories_fbs",
    srcs = [
        "arm_trajectories.fbs",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "generated_graph",
    srcs = [
        "generated_graph.cc",
    ],
    hdrs = ["generated_graph.h"],
    copts = [
        "-O1",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
    deps = [
        ":arm_constants",
        "//frc971/control_loops/double_jointed_arm:graph",
        "//y2023/control_loops/superstructure/arm:trajectory",
        "//y2023/control_loops/superstructure/roll:roll_plants",
    ],
)

cc_library(
    name = "arm_constants",
    hdrs = ["arm_constants.h"],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
    deps = [
        "//frc971/control_loops/double_jointed_arm:dynamics",
    ],
)

cc_binary(
    name = "arm_design",
    srcs = [
        "arm_design.cc",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        ":arm_constants",
        "//aos:init",
        "//aos/analysis:in_process_plotter",
        "//frc971/control_loops:dlqr",
        "//frc971/control_loops:jacobian",
        "//frc971/control_loops/double_jointed_arm:dynamics",
    ],
)

cc_library(
    name = "trajectory",
    srcs = ["trajectory.cc"],
    hdrs = ["trajectory.h"],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
    deps = [
        "//frc971/control_loops:binomial",
        "//frc971/control_loops:dlqr",
        "//frc971/control_loops:fixed_quadrature",
        "//frc971/control_loops:hybrid_state_feedback_loop",
        "//frc971/control_loops/double_jointed_arm:dynamics",
        "//frc971/control_loops/double_jointed_arm:ekf",
        "//y2023/control_loops/superstructure/arm:arm_trajectories_fbs",
        "@org_tuxfamily_eigen//:eigen",
    ],
)

cc_test(
    name = "trajectory_test",
    srcs = ["trajectory_test.cc"],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        ":arm_constants",
        ":trajectory",
        "//aos/testing:googletest",
        "//y2023/control_loops/superstructure/roll:roll_plants",
    ],
)

cc_binary(
    name = "trajectory_plot",
    srcs = [
        "trajectory_plot.cc",
    ],
    target_compatible_with = ["@platforms//cpu:x86_64"],
    deps = [
        ":arm_constants",
        ":generated_graph",
        ":trajectory",
        "//aos/analysis:in_process_plotter",
        "//frc971/control_loops:binomial",
        "//frc971/control_loops:fixed_quadrature",
        "//frc971/control_loops/double_jointed_arm:ekf",
        "//y2023/control_loops/superstructure/roll:roll_plants",
        "@com_google_absl//absl/flags:flag",
        "@org_tuxfamily_eigen//:eigen",
    ],
)
