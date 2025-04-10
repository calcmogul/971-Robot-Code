load("//frc971:downloader.bzl", "robot_downloader")

cc_binary(
    name = "joystick_reader",
    srcs = [
        "joystick_reader.cc",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        "//aos:init",
        "//aos/actions:action_lib",
        "//aos/logging",
        "//aos/time",
        "//aos/util:log_interval",
        "//frc971/autonomous:base_autonomous_actor",
        "//frc971/input:drivetrain_input",
        "//frc971/input:joystick_input",
        "//y2014_bot3/control_loops/drivetrain:drivetrain_base",
        "//y2014_bot3/control_loops/rollers:rollers_goal_fbs",
    ],
)

robot_downloader(
    binaries = [
        ":joystick_reader",
        ":wpilib_interface",
        "//y2014_bot3/actors:autonomous_action",
        "//y2014_bot3/control_loops/drivetrain:drivetrain",
        "//y2014_bot3/control_loops/rollers:rollers",
    ],
    target_compatible_with = ["@platforms//os:linux"],
)

cc_binary(
    name = "wpilib_interface",
    srcs = [
        "wpilib_interface.cc",
    ],
    target_compatible_with = ["//tools/platforms/hardware:roborio"],
    deps = [
        "//aos:init",
        "//aos/logging",
        "//aos/stl_mutex",
        "//aos/time",
        "//aos/util:log_interval",
        "//aos/util:phased_loop",
        "//aos/util:wrapping_counter",
        "//frc971/control_loops:control_loop",
        "//frc971/control_loops:control_loops_fbs",
        "//frc971/control_loops/drivetrain:drivetrain_output_fbs",
        "//frc971/control_loops/drivetrain:drivetrain_position_fbs",
        "//frc971/input:robot_state_fbs",
        "//frc971/wpilib:buffered_pcm",
        "//frc971/wpilib:dma",
        "//frc971/wpilib:drivetrain_writer",
        "//frc971/wpilib:gyro_sender",
        "//frc971/wpilib:joystick_sender",
        "//frc971/wpilib:logging_fbs",
        "//frc971/wpilib:loop_output_handler",
        "//frc971/wpilib:pdp_fetcher",
        "//frc971/wpilib:sensor_reader",
        "//frc971/wpilib:wpilib_robot_base",
        "//third_party:wpilib",
        "//y2014_bot3/control_loops/drivetrain:drivetrain_base",
        "//y2014_bot3/control_loops/rollers:rollers_lib",
    ],
)

py_library(
    name = "python_init",
    srcs = ["__init__.py"],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
)
