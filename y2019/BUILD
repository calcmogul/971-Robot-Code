load("@rules_cc//cc:defs.bzl", "cc_proto_library")
load("@rules_proto//proto:defs.bzl", "proto_library")
load("//aos:config.bzl", "aos_config")
load("//aos/flatbuffers:generate.bzl", "static_flatbuffer")
load("//frc971:downloader.bzl", "robot_downloader")

robot_downloader(
    binaries = [
        ":joystick_reader",
        ":wpilib_interface",
        "//y2019/control_loops/drivetrain:drivetrain",
        "//y2019/control_loops/drivetrain:trajectory_generator",
        "//y2019/control_loops/superstructure:superstructure",
        "//y2019/actors:binaries",
        "//y2019/vision/server",
    ],
    data = [
        ":aos_config",
        "@ctre_phoenix_api_cpp_athena//:shared_libraries",
        "@ctre_phoenix_cci_athena//:shared_libraries",
    ],
    dirs = [
        "//y2019/vision/server:www_files",
    ],
    target_compatible_with = ["@platforms//os:linux"],
)

cc_library(
    name = "constants",
    srcs = [
        "constants.cc",
    ],
    hdrs = [
        "constants.h",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
    deps = [
        "//aos/network:team_number",
        "//aos/stl_mutex",
        "//frc971:constants",
        "//frc971/control_loops:pose",
        "//frc971/control_loops:static_zeroing_single_dof_profiled_subsystem",
        "//frc971/control_loops/drivetrain:camera",
        "//frc971/zeroing:absolute_encoder",
        "//frc971/zeroing:pot_and_absolute_encoder",
        "//y2019/control_loops/drivetrain:polydrivetrain_plants",
        "//y2019/control_loops/superstructure/elevator:elevator_plants",
        "//y2019/control_loops/superstructure/intake:intake_plants",
        "//y2019/control_loops/superstructure/stilts:stilts_plants",
        "//y2019/control_loops/superstructure/wrist:wrist_plants",
        "//y2019/vision:constants",
        "@com_google_absl//absl/base",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
    ],
)

cc_binary(
    name = "wpilib_interface",
    srcs = [
        "wpilib_interface.cc",
    ],
    # This library uses some deprecated parts of the SPI API.
    copts = ["-Wno-deprecated-declarations"],
    target_compatible_with = ["//tools/platforms/hardware:roborio"],
    deps = [
        ":camera_log_fbs",
        ":constants",
        ":status_light_fbs",
        "//aos:init",
        "//aos:math",
        "//aos/events:shm_event_loop",
        "//aos/logging",
        "//aos/stl_mutex",
        "//aos/time",
        "//aos/util:log_interval",
        "//aos/util:phased_loop",
        "//aos/util:wrapping_counter",
        "//frc971/autonomous:auto_mode_fbs",
        "//frc971/control_loops:control_loop",
        "//frc971/control_loops:control_loops_fbs",
        "//frc971/control_loops/drivetrain:drivetrain_position_fbs",
        "//frc971/input:robot_state_fbs",
        "//frc971/wpilib:ADIS16448",
        "//frc971/wpilib:buffered_pcm",
        "//frc971/wpilib:drivetrain_writer",
        "//frc971/wpilib:encoder_and_potentiometer",
        "//frc971/wpilib:interrupt_edge_counting",
        "//frc971/wpilib:joystick_sender",
        "//frc971/wpilib:logging_fbs",
        "//frc971/wpilib:loop_output_handler",
        "//frc971/wpilib:pdp_fetcher",
        "//frc971/wpilib:sensor_reader",
        "//frc971/wpilib:wpilib_interface",
        "//frc971/wpilib:wpilib_robot_base",
        "//third_party:phoenix",
        "//third_party:wpilib",
        "//y2019/control_loops/drivetrain:camera_fbs",
        "//y2019/control_loops/superstructure:superstructure_output_fbs",
        "//y2019/control_loops/superstructure:superstructure_position_fbs",
        "//y2019/jevois:spi",
    ],
)

cc_library(
    name = "joystick_angle",
    srcs = [
        "joystick_angle.cc",
    ],
    hdrs = [
        "joystick_angle.h",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        "//frc971/input:drivetrain_input",
        "//frc971/zeroing:wrap",
    ],
)

cc_test(
    name = "joystick_angle_test",
    srcs = [
        "joystick_angle_test.cc",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        ":joystick_angle",
        "//aos/testing:googletest",
    ],
)

cc_binary(
    name = "joystick_reader",
    srcs = [
        ":joystick_reader.cc",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        ":camera_log_fbs",
        ":vision_proto",
        "//aos:init",
        "//aos/actions:action_lib",
        "//aos/logging",
        "//aos/network:team_number",
        "//aos/stl_mutex",
        "//aos/time",
        "//aos/util:log_interval",
        "//aos/vision/events:udp",
        "//frc971/autonomous:auto_fbs",
        "//frc971/autonomous:base_autonomous_actor",
        "//frc971/control_loops:profiled_subsystem_fbs",
        "//frc971/control_loops/drivetrain:localizer_fbs",
        "//frc971/input:action_joystick_input",
        "//frc971/input:drivetrain_input",
        "//frc971/input:joystick_input",
        "//y2019/control_loops/drivetrain:drivetrain_base",
        "//y2019/control_loops/drivetrain:target_selector_fbs",
        "//y2019/control_loops/superstructure:superstructure_goal_fbs",
        "//y2019/control_loops/superstructure:superstructure_position_fbs",
        "//y2019/control_loops/superstructure:superstructure_status_fbs",
        "@com_google_protobuf//:protobuf",
    ],
)

static_flatbuffer(
    name = "camera_log_fbs",
    srcs = [
        "camera_log.fbs",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
)

static_flatbuffer(
    name = "status_light_fbs",
    srcs = [
        "status_light.fbs",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
)

aos_config(
    name = "aos_config",
    src = "y2019.json",
    flatbuffers = [
        ":status_light_fbs",
        "//y2019/control_loops/drivetrain:camera_fbs",
        "//y2019/control_loops/drivetrain:target_selector_fbs",
        "//y2019/control_loops/superstructure:superstructure_goal_fbs",
        "//y2019/control_loops/superstructure:superstructure_output_fbs",
        "//y2019/control_loops/superstructure:superstructure_position_fbs",
        "//y2019/control_loops/superstructure:superstructure_status_fbs",
        ":camera_log_fbs",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
    deps = [
        "//frc971/autonomous:aos_config",
        "//frc971/control_loops/drivetrain:aos_config",
        "//frc971/input:aos_config",
        "//frc971/wpilib:aos_config",
    ],
)

proto_library(
    name = "vision_proto_proto",
    srcs = ["vision.proto"],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
)

cc_proto_library(
    name = "vision_proto",
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
    deps = ["vision_proto_proto"],
)

py_library(
    name = "python_init",
    srcs = ["__init__.py"],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
)
