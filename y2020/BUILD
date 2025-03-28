load("//aos:config.bzl", "aos_config")
load("//aos/flatbuffers:generate.bzl", "static_flatbuffer")
load("//aos/util:config_validator_macro.bzl", "config_validator_test")
load("//frc971:downloader.bzl", "robot_downloader")
load("//tools/build_rules:template.bzl", "jinja2_template")

config_validator_test(
    name = "config_validator_test",
    config = "//y2020:aos_config",
)

robot_downloader(
    binaries = [
        ":setpoint_setter",
        "//aos/network:web_proxy_main",
        "//aos/events/logging:logger_main",
        ":joystick_reader",
        ":wpilib_interface",
        "//aos/network:message_bridge_client",
        "//aos/network:message_bridge_server",
        "//y2020/actors:binaries",
        "//y2020/control_loops/drivetrain:drivetrain",
        "//y2020/control_loops/drivetrain:trajectory_generator",
        "//y2020/control_loops/superstructure:superstructure",
    ],
    data = [
        ":aos_config",
        "@ctre_phoenix6_api_cpp_athena//:shared_libraries",
        "@ctre_phoenix6_tools_athena//:shared_libraries",
        "@ctre_phoenix_api_cpp_athena//:shared_libraries",
        "@ctre_phoenix_cci_athena//:shared_libraries",
    ],
    dirs = [
        "//y2020/actors:splines",
        "//y2020/www:www_files",
    ],
    target_compatible_with = ["@platforms//os:linux"],
)

robot_downloader(
    name = "pi_download",
    binaries = [
        "//frc971/vision:intrinsics_calibration",
        "//y2020/vision:viewer",
        "//aos/events/logging:logger_main",
        "//aos/network:message_bridge_client",
        "//aos/network:message_bridge_server",
        "//aos/network:web_proxy_main",
        "//y2020/vision:camera_reader",
    ],
    data = [
        ":aos_config",
    ],
    dirs = [
        "//y2020/www:www_files",
    ],
    target_compatible_with = ["//tools/platforms/hardware:raspberry_pi"],
    target_type = "pi",
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
        "//frc971/control_loops:static_zeroing_single_dof_profiled_subsystem",
        "//frc971/shooter_interpolation:interpolation",
        "//frc971/zeroing:absolute_and_absolute_encoder",
        "//frc971/zeroing:absolute_encoder",
        "//frc971/zeroing:pot_and_absolute_encoder",
        "//y2020/control_loops/drivetrain:polydrivetrain_plants",
        "//y2020/control_loops/superstructure/accelerator:accelerator_plants",
        "//y2020/control_loops/superstructure/control_panel:control_panel_plants",
        "//y2020/control_loops/superstructure/finisher:finisher_plants",
        "//y2020/control_loops/superstructure/hood:hood_plants",
        "//y2020/control_loops/superstructure/intake:intake_plants",
        "//y2020/control_loops/superstructure/turret:turret_plants",
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
        "//frc971/wpilib:ADIS16448",
        "//frc971/wpilib:ADIS16470",
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
        "//y2020/control_loops/superstructure:superstructure_output_fbs",
        "//y2020/control_loops/superstructure:superstructure_position_fbs",
        "//y2020/control_loops/superstructure/shooter:shooter_tuning_readings_fbs",
    ],
)

cc_binary(
    name = "joystick_reader",
    srcs = [
        ":joystick_reader.cc",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
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
        "//frc971/zeroing:wrap",
        "//y2020:constants",
        "//y2020/control_loops/drivetrain:drivetrain_base",
        "//y2020/control_loops/superstructure:superstructure_goal_fbs",
        "//y2020/control_loops/superstructure:superstructure_status_fbs",
    ],
)

aos_config(
    name = "aos_config",
    src = "y2020.json",
    flatbuffers = [
        "//aos/network:message_bridge_client_fbs",
        "//aos/network:message_bridge_server_fbs",
        "//aos/network:timestamp_fbs",
        "//frc971/vision:vision_fbs",
        "//y2020/vision/sift:sift_fbs",
        "//y2020/vision/sift:sift_training_fbs",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
    deps = [
        ":config_logger",
        ":config_pi1",
        ":config_pi2",
        ":config_pi3",
        ":config_pi4",
        ":config_pi5",
        ":config_roborio",
    ],
)

[
    aos_config(
        name = "config_" + pi,
        src = "y2020_" + pi + ".json",
        flatbuffers = [
            "//aos/network:message_bridge_client_fbs",
            "//aos/network:message_bridge_server_fbs",
            "//aos/network:remote_message_fbs",
            "//aos/network:timestamp_fbs",
            "//frc971/vision:vision_fbs",
            "//y2020/vision:galactic_search_path_fbs",
            "//y2020/vision/sift:sift_fbs",
            "//y2020/vision/sift:sift_training_fbs",
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
        "pi5",
    ]
]

aos_config(
    name = "config_logger",
    src = "y2020_logger.json",
    flatbuffers = [
        "//aos/network:message_bridge_client_fbs",
        "//aos/network:message_bridge_server_fbs",
        "//aos/network:remote_message_fbs",
        "//aos/network:timestamp_fbs",
        "//frc971/vision:vision_fbs",
        "//y2020/vision/sift:sift_fbs",
        "//y2020/vision/sift:sift_training_fbs",
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
    src = "y2020_roborio.json",
    flatbuffers = [
        ":setpoint_fbs",
        "//aos/network:remote_message_fbs",
        "//aos/network:message_bridge_client_fbs",
        "//aos/network:message_bridge_server_fbs",
        "//aos/network:timestamp_fbs",
        "//y2020/control_loops/superstructure/shooter:shooter_tuning_params_fbs",
        "//y2020/control_loops/superstructure/shooter:shooter_tuning_readings_fbs",
        "//y2020/control_loops/superstructure:superstructure_goal_fbs",
        "//y2019/control_loops/drivetrain:target_selector_fbs",
        "//y2020/control_loops/drivetrain:localizer_debug_fbs",
        "//y2020/control_loops/superstructure:superstructure_output_fbs",
        "//y2020/control_loops/superstructure:superstructure_position_fbs",
        "//y2020/control_loops/superstructure:superstructure_status_fbs",
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
        "//y2020/www:camera_main_bundle.min.js",
        "//y2020/www:field_main_bundle.min.js",
        "//y2020/www:files",
    ],
    target_compatible_with = ["@platforms//os:linux"],
)

sh_binary(
    name = "web_proxy",
    srcs = ["web_proxy.sh"],
    data = [
        ":aos_config",
        "//aos/network:web_proxy_main",
        "//y2020/www:camera_main_bundle.min.js",
        "//y2020/www:field_main_bundle.min.js",
        "//y2020/www:files",
    ],
    target_compatible_with = ["@platforms//os:linux"],
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
        ":setpoint_fbs",
        "//aos:init",
        "//aos/events:shm_event_loop",
    ],
)

[
    jinja2_template(
        name = "y2020_pi" + str(num) + ".json",
        src = "y2020_pi_template.json",
        parameters = {"NUM": str(num)},
        target_compatible_with = ["@platforms//os:linux"],
    )
    for num in range(1, 6)
]
