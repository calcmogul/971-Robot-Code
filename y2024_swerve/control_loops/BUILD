load("//tools/build_rules:js.bzl", "ts_project")

cc_binary(
    name = "swerve_control_loops",
    srcs = [
        "swerve_control_loops_main.cc",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":parameters",
        "//aos:init",
        "//aos/events:shm_event_loop",
        "//aos/time",
        "//frc971/constants:constants_sender_lib",
        "//frc971/control_loops/swerve:swerve_control_loops",
        "//y2024_swerve/constants:constants_fbs",
    ],
)

ts_project(
    name = "swerve_plotter",
    srcs = ["swerve_plotter.ts"],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
    deps = [
        "//aos/network/www:aos_plotter",
        "//aos/network/www:colors",
        "//aos/network/www:proxy",
    ],
)

cc_library(
    name = "parameters",
    hdrs = ["parameters.h"],
    deps = [
        "//frc971/control_loops/swerve:simplified_dynamics",
        "//y2024_swerve:constants",
    ],
)
