load("//aos:config.bzl", "aos_config")
load("//aos/flatbuffers:generate.bzl", "static_flatbuffer")
load("//aos/util:config_validator_macro.bzl", "config_validator_test")
load("//frc971:downloader.bzl", "robot_downloader")
load("//tools/build_rules:template.bzl", "jinja2_template")

config_validator_test(
    name = "config_validator_test",
    config = "//y2022:aos_config",
)

robot_downloader(
    binaries = [
        ":setpoint_setter",
        "//aos/network:web_proxy_main",
        "//aos/events/logging:log_cat",
        "//aos/events/logging:logger_main",
        ":joystick_reader",
        ":wpilib_interface",
        "//aos/network:message_bridge_client",
        "//aos/network:message_bridge_server",
        "//y2022/actors:binaries",
        "//y2022/control_loops/drivetrain:drivetrain",
        "//y2022/control_loops/drivetrain:trajectory_generator",
        "//y2022/control_loops/superstructure:superstructure",
    ],
    data = [
        ":aos_config",
        ":message_bridge_client.sh",
        "@ctre_phoenix6_api_cpp_athena//:shared_libraries",
        "@ctre_phoenix6_tools_athena//:shared_libraries",
        "@ctre_phoenix_api_cpp_athena//:shared_libraries",
        "@ctre_phoenix_cci_athena//:shared_libraries",
    ],
    dirs = [
        "//y2022/actors:splines",
        "//y2022/www:www_files",
    ],
    target_compatible_with = ["@platforms//os:linux"],
)

robot_downloader(
    name = "pi_download",
    binaries = [
        "//frc971/vision:intrinsics_calibration",
        "//y2022/vision:viewer",
        "//y2022/localizer:imu_main",
        "//y2022/localizer:localizer_main",
        "//y2022/vision:image_decimator",
        "//frc971/image_streamer:image_streamer",
        "//aos/events/logging:log_cat",
        "//aos/events/logging:logger_main",
        "//aos/network:message_bridge_client",
        "//aos/network:message_bridge_server",
        "//aos/network:web_proxy_main",
        "//y2022/vision:camera_reader",
        "//y2022/vision:ball_color_detector",
    ],
    data = [
        ":aos_config",
        ":message_bridge_client.sh",
        "//y2022/vision:image_streamer_start",
    ],
    dirs = [
        "//y2022/www:www_files",
        "//frc971/image_streamer/www:www_files",
    ],
    target_compatible_with = ["//tools/platforms/hardware:raspberry_pi"],
    target_type = "pi",
)

aos_config(
    name = "aos_config",
    src = "y2022.json",
    flatbuffers = [
        "//aos/network:message_bridge_client_fbs",
        "//aos/network:message_bridge_server_fbs",
        "//aos/network:timestamp_fbs",
        "//frc971/input:robot_state_fbs",
        "//frc971/vision:vision_fbs",
        "//frc971/vision:calibration_fbs",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
    deps = [
        ":config_imu",
        ":config_logger",
        ":config_pi1",
        ":config_pi2",
        ":config_pi3",
        ":config_pi4",
        ":config_roborio",
    ],
)

[
    aos_config(
        name = "config_" + pi,
        src = "y2022_" + pi + ".json",
        flatbuffers = [
            "//aos/network:message_bridge_client_fbs",
            "//aos/network:message_bridge_server_fbs",
            "//aos/network:timestamp_fbs",
            "//aos/network:remote_message_fbs",
            "//frc971/vision:vision_fbs",
            "//frc971/control_loops/drivetrain/localization:localizer_output_fbs",
            "//frc971/vision:calibration_fbs",
            "//y2022/vision:target_estimate_fbs",
            "//y2022/vision:ball_color_fbs",
        ],
        target_compatible_with = ["@platforms//os:linux"],
        visibility = ["//visibility:public"],
        deps = [
            "//aos/events:aos_config",
            "//frc971/control_loops/drivetrain:aos_config",
            "//frc971/input:aos_config",
        ],
    )
    for pi in [
        "pi1",
        "pi2",
        "pi3",
        "pi4",
    ]
]

aos_config(
    name = "config_imu",
    src = "y2022_imu.json",
    flatbuffers = [
        "//aos/network:message_bridge_client_fbs",
        "//aos/network:message_bridge_server_fbs",
        "//aos/network:timestamp_fbs",
        "//aos/network:remote_message_fbs",
        "//y2022/localizer:localizer_status_fbs",
        "//frc971/control_loops/drivetrain/localization:localizer_output_fbs",
        "//y2022/localizer:localizer_visualization_fbs",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
    deps = [
        "//aos/events:aos_config",
        "//frc971/control_loops/drivetrain:aos_config",
    ],
)

aos_config(
    name = "config_logger",
    src = "y2022_logger.json",
    flatbuffers = [
        "//aos/network:message_bridge_client_fbs",
        "//aos/network:message_bridge_server_fbs",
        "//aos/network:timestamp_fbs",
        "//aos/network:remote_message_fbs",
        "//frc971/vision:vision_fbs",
        "//frc971/vision:calibration_fbs",
        "//y2022/vision:ball_color_fbs",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
    deps = [
        "//aos/events:aos_config",
        "//frc971/control_loops/drivetrain:aos_config",
        "//frc971/input:aos_config",
    ],
)

aos_config(
    name = "config_roborio",
    src = "y2022_roborio.json",
    flatbuffers = [
        ":setpoint_fbs",
        "//aos/network:remote_message_fbs",
        "//aos/network:message_bridge_client_fbs",
        "//aos/network:message_bridge_server_fbs",
        "//aos/network:timestamp_fbs",
        "//y2022/vision:ball_color_fbs",
        "//y2019/control_loops/drivetrain:target_selector_fbs",
        "//y2022/control_loops/superstructure:superstructure_goal_fbs",
        "//y2022/control_loops/superstructure:superstructure_output_fbs",
        "//y2022/control_loops/superstructure:superstructure_position_fbs",
        "//y2022/control_loops/superstructure:superstructure_can_position_fbs",
        "//y2022/control_loops/superstructure:superstructure_status_fbs",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        "//aos/events:aos_config",
        "//frc971/autonomous:aos_config",
        "//frc971/control_loops/drivetrain:aos_config",
        "//frc971/input:aos_config",
        "//frc971/wpilib:aos_config",
    ],
)

[
    jinja2_template(
        name = "y2022_pi" + str(num) + ".json",
        src = "y2022_pi_template.json",
        parameters = {"NUM": str(num)},
        target_compatible_with = ["@platforms//os:linux"],
    )
    for num in range(1, 6)
]

cc_library(
    name = "constants",
    srcs = [
        "constants.cc",
    ],
    hdrs = [
        "constants.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//aos/mutex",
        "//aos/network:team_number",
        "//frc971:constants",
        "//frc971/control_loops:pose",
        "//frc971/control_loops:static_zeroing_single_dof_profiled_subsystem",
        "//frc971/shooter_interpolation:interpolation",
        "//frc971/wpilib:wpilib_utils",
        "//frc971/zeroing:pot_and_absolute_encoder",
        "//y2022/control_loops/drivetrain:polydrivetrain_plants",
        "//y2022/control_loops/superstructure/catapult:catapult_plants",
        "//y2022/control_loops/superstructure/climber:climber_plants",
        "//y2022/control_loops/superstructure/intake:intake_plants",
        "//y2022/control_loops/superstructure/turret:turret_plants",
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
    target_compatible_with = ["//tools/platforms/hardware:roborio"],
    deps = [
        ":constants",
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
        "//frc971/queues:gyro_fbs",
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
        "//third_party:phoenix6",
        "//third_party:wpilib",
        "//y2022/control_loops/superstructure:led_indicator_lib",
        "//y2022/control_loops/superstructure:superstructure_can_position_fbs",
        "//y2022/control_loops/superstructure:superstructure_output_fbs",
        "//y2022/control_loops/superstructure:superstructure_position_fbs",
    ],
)

cc_binary(
    name = "joystick_reader",
    srcs = [
        ":joystick_reader.cc",
    ],
    deps = [
        ":constants",
        ":setpoint_fbs",
        "//aos:init",
        "//aos/actions:action_lib",
        "//aos/logging",
        "//frc971/autonomous:auto_fbs",
        "//frc971/autonomous:base_autonomous_actor",
        "//frc971/control_loops:profiled_subsystem_fbs",
        "//frc971/input:action_joystick_input",
        "//frc971/input:drivetrain_input",
        "//frc971/input:joystick_input",
        "//y2022/control_loops/drivetrain:drivetrain_base",
        "//y2022/control_loops/superstructure:superstructure_goal_fbs",
        "//y2022/control_loops/superstructure:superstructure_status_fbs",
    ],
)

static_flatbuffer(
    name = "setpoint_fbs",
    srcs = [
        "setpoint.fbs",
    ],
    target_compatible_with = ["@platforms//os:linux"],
)

cc_binary(
    name = "setpoint_setter",
    srcs = ["setpoint_setter.cc"],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        ":constants",
        ":setpoint_fbs",
        "//aos:init",
        "//aos/events:shm_event_loop",
    ],
)

py_library(
    name = "python_init",
    srcs = ["__init__.py"],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
)

sh_binary(
    name = "log_web_proxy",
    srcs = ["log_web_proxy.sh"],
    data = [
        ":aos_config",
        "//aos/network:log_web_proxy_main",
        "//y2022/www:field_main_bundle.min.js",
        "//y2022/www:files",
    ],
    target_compatible_with = ["@platforms//os:linux"],
)
