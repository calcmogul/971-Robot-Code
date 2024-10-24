load("@aspect_bazel_lib//lib:run_binary.bzl", "run_binary")
load("//aos:config.bzl", "aos_config")
load("//aos/flatbuffers:generate.bzl", "static_flatbuffer")

package(default_visibility = ["//visibility:public"])

static_flatbuffer(
    name = "swerve_zeroing_fbs",
    srcs = ["swerve_zeroing.fbs"],
    visibility = ["//visibility:public"],
    deps = [
        "//frc971/zeroing:constants_fbs",
    ],
)

static_flatbuffer(
    name = "swerve_drivetrain_goal_fbs",
    srcs = ["swerve_drivetrain_goal.fbs"],
)

static_flatbuffer(
    name = "swerve_drivetrain_status_fbs",
    srcs = ["swerve_drivetrain_status.fbs"],
    deps = ["//frc971/control_loops:profiled_subsystem_fbs"],
)

static_flatbuffer(
    name = "swerve_drivetrain_output_fbs",
    srcs = ["swerve_drivetrain_output.fbs"],
    deps = ["//frc971/control_loops:can_talonfx_fbs"],
)

static_flatbuffer(
    name = "swerve_drivetrain_can_position_fbs",
    srcs = ["swerve_drivetrain_can_position.fbs"],
    deps = ["//frc971/control_loops:can_talonfx_fbs"],
)

static_flatbuffer(
    name = "swerve_drivetrain_position_fbs",
    srcs = ["swerve_drivetrain_position.fbs"],
    deps = ["//frc971/control_loops:control_loops_fbs"],
)

py_binary(
    name = "simulation",
    srcs = [
        "simulation.py",
    ],
    deps = [
        "//frc971/control_loops/python:controls",
        "@pip//matplotlib",
        "@pip//numpy",
        "@pip//pygobject",
        "@pip//sympy",
    ],
)

cc_library(
    name = "swerve_control_loops",
    srcs = ["swerve_control_loops.cc"],
    hdrs = ["swerve_control_loops.h"],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        ":swerve_drivetrain_can_position_fbs",
        ":swerve_drivetrain_goal_fbs",
        ":swerve_drivetrain_output_fbs",
        ":swerve_drivetrain_position_fbs",
        ":swerve_drivetrain_status_fbs",
        ":swerve_zeroing_fbs",
        "//frc971/control_loops:control_loop",
        "//frc971/control_loops:static_zeroing_single_dof_profiled_subsystem",
        "//frc971/zeroing:continuous_absolute_encoder",
    ],
)

cc_test(
    name = "swerve_control_test",
    srcs = ["swerve_control_test.cc"],
    data = [
        ":aos_config",
        "//frc971/control_loops/swerve/test_module:rotation_json",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        ":swerve_control_loops",
        ":swerve_drivetrain_can_position_fbs",
        ":swerve_drivetrain_goal_fbs",
        ":swerve_drivetrain_output_fbs",
        ":swerve_drivetrain_position_fbs",
        ":swerve_drivetrain_status_fbs",
        "//aos/events:shm_event_loop",
        "//aos/testing:googletest",
        "//frc971/control_loops:control_loop_test",
        "//frc971/control_loops:state_feedback_loop",
        "//frc971/control_loops:team_number_test_environment",
    ],
)

aos_config(
    name = "aos_config",
    src = "swerve.json",
    flatbuffers = [
        ":swerve_drivetrain_goal_fbs",
        ":swerve_drivetrain_output_fbs",
        ":swerve_drivetrain_position_fbs",
        ":swerve_drivetrain_can_position_fbs",
        ":swerve_drivetrain_status_fbs",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    deps = ["//frc971/input:aos_config"],
)

cc_library(
    name = "motors",
    hdrs = [
        "motors.h",
    ],
    deps = [
        "@symengine",
    ],
)

cc_binary(
    name = "generate_physics",
    srcs = [
        "generate_physics.cc",
    ],
    deps = [
        ":motors",
        "//aos:init",
        "//aos/util:file",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@symengine",
    ],
)

run_binary(
    name = "dynamics_codegen_cc",
    outs = [
        "dynamics.cc",
        "dynamics.h",
        "dynamics.py",
    ],
    args = [
        "--output_base=$(BINDIR)/",
        "--cc_output_path=$(location :dynamics.cc)",
        "--h_output_path=$(location :dynamics.h)",
        "--casadi_py_output_path=$(location :dynamics.py)",
    ],
    tool = ":generate_physics",
)

run_binary(
    name = "dynamics_codegen_nocaster",
    outs = [
        "nocaster_dynamics.py",
    ],
    args = [
        "--caster=0.0",
        "--output_base=$(BINDIR)/",
        "--casadi_py_output_path=$(location :nocaster_dynamics.py)",
    ],
    tool = ":generate_physics",
)

run_binary(
    name = "dynamics_codegen_bigcaster",
    outs = [
        "bigcaster_dynamics.py",
    ],
    args = [
        "--caster=0.05",
        "--output_base=$(BINDIR)/",
        "--casadi_py_output_path=$(location :bigcaster_dynamics.py)",
    ],
    tool = ":generate_physics",
)

cc_library(
    name = "eigen_dynamics",
    srcs = ["dynamics.cc"],
    hdrs = ["dynamics.h"],
    deps = [
        "@org_tuxfamily_eigen//:eigen",
    ],
)

py_library(
    name = "dynamics",
    srcs = [
        "bigcaster_dynamics.py",
        "dynamics.py",
        "nocaster_dynamics.py",
    ],
    deps = [
        "@pip//casadi",
    ],
)

py_library(
    name = "jax_dynamics",
    srcs = [
        "jax_dynamics.py",
    ],
    deps = [
        ":dynamics",
        "//frc971/control_loops/python:controls",
        "@pip//jax",
    ],
)

py_test(
    name = "physics_test_cpu",
    srcs = [
        "physics_test.py",
    ],
    data = [":cpp_dynamics.so"],
    env = {
        "JAX_PLATFORMS": "cpu",
    },
    main = "physics_test.py",
    target_compatible_with = ["@platforms//cpu:x86_64"],
    deps = [
        ":dynamics",
        ":jax_dynamics",
        ":physics_test_utils",
        "@pip//casadi",
        "@pip//numpy",
        "@pip//scipy",
    ],
)

py_test(
    name = "physics_test_gpu",
    srcs = [
        "physics_test.py",
    ],
    data = [":cpp_dynamics.so"],
    env = {
        "JAX_PLATFORMS": "cuda",
    },
    main = "physics_test.py",
    target_compatible_with = ["@platforms//cpu:x86_64"],
    deps = [
        ":dynamics",
        ":jax_dynamics",
        ":physics_test_utils",
        "@pip//casadi",
        "@pip//numpy",
        "@pip//scipy",
    ],
)

py_binary(
    name = "casadi_velocity_mpc",
    srcs = [
        "casadi_velocity_mpc.py",
    ],
    deps = [
        ":dynamics",
        "@pip//absl_py",
        "@pip//casadi",
        "@pip//matplotlib",
        "@pip//numpy",
        "@pip//pygobject",
        "@pip//scipy",
    ],
)

py_library(
    name = "physics_test_utils",
    srcs = [
        "physics_test_utils.py",
    ],
    deps = [
        ":dynamics",
        "@pip//casadi",
        "@pip//numpy",
    ],
)

py_binary(
    name = "physics_debug",
    srcs = [
        "physics_debug.py",
    ],
    deps = [
        ":physics_test_utils",
        "@pip//matplotlib",
        "@pip//pygobject",
        "@pip//scipy",
    ],
)

py_binary(
    name = "debug_fatrop",
    srcs = [
        "debug_fatrop.py",
    ],
    deps = [
        "@pip//casadi",
        "@pip//matplotlib",
        "@pip//numpy",
        "@pip//pygobject",
        "@pip//scipy",
    ],
)

py_binary(
    name = "smooth_function_graph",
    srcs = [
        "smooth_function_graph.py",
    ],
    deps = [
        "@pip//matplotlib",
        "@pip//numpy",
        "@pip//pygobject",
        "@pip//scipy",
    ],
)

py_binary(
    name = "multi_casadi_velocity_mpc",
    srcs = ["multi_casadi_velocity_mpc.py"],
    data = [":casadi_velocity_mpc"],
    deps = [
        ":dynamics",
        "@pip//absl_py",
        "@pip//matplotlib",
        "@pip//numpy",
        "@pip//pygobject",
    ],
)

cc_binary(
    name = "cpp_dynamics.so",
    # Just use the python dynamics directly if you want them; this is just for testing.
    testonly = True,
    srcs = ["dynamics_python_bindings.cc"],
    linkshared = True,
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        ":eigen_dynamics",
        "//third_party/python",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
    ],
)
