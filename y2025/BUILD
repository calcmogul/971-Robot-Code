load("//aos:config.bzl", "aos_config")
load("//aos/util:config_validator_macro.bzl", "config_validator_test")
load("//frc971:downloader.bzl", "robot_downloader")

config_validator_test(
    name = "config_validator_test",
    config = "//y2025:aos_config",
)

robot_downloader(
    binaries = [
        "//aos/network:web_proxy_main",
        "//aos/events/logging:log_cat",
        "//y2025/constants:constants_sender",
        "//aos/events:aos_timing_report_streamer",
        "//aos/events/logging:logger_main",
        "//aos/starter:irq_affinity",
        ":joystick_reader",
        ":wpilib_interface",
        "//frc971/can_logger",
        "//aos/network:message_bridge_client",
        "//aos/network:message_bridge_server",
        "//y2025/control_loops/superstructure:superstructure",
    ],
    data = [
        ":aos_config",
        "//aos/starter:roborio_irq_config.json",
        "//y2025/constants:constants.json",
        "@ctre_phoenix6_api_cpp_athena//:shared_libraries",
        "@ctre_phoenix6_tools_athena//:shared_libraries",
        "@ctre_phoenix_api_cpp_athena//:shared_libraries",
        "@ctre_phoenix_cci_athena//:shared_libraries",
    ],
    dirs = [
        "//y2025/www:www_files",
    ],
    target_compatible_with = ["@platforms//os:linux"],
)

robot_downloader(
    name = "orin_download",
    binaries = [
        "//aos/network:message_bridge_client",
        "//aos/network:message_bridge_server",
        "//frc971/wpilib:joystick_republish",
        "//aos/events:aos_timing_report_streamer",
        "//aos/events/logging:log_cat",
        "//y2025:auto_align_updater",
        "//aos:aos_jitter",
        "//aos/network:web_proxy_main",
        "//aos/starter:irq_affinity",
        "//aos/util:foxglove_websocket",
        "//frc971/orin:hardware_monitor",
        "//frc971/orin:argus_camera",
        "//frc971/vision:intrinsics_calibration",
        "//aos/util:filesystem_monitor",
        "//frc971/orin:argus_monitor",
        "//y2025/vision:viewer",
        "//y2025/vision:apriltag_detector",
        "//y2025/constants:constants_sender",
        "//y2025/orin:ctre_interface",
        "//y2025/control_loops/swerve:swerve_control_loops",
        "//frc971/orin:localizer_logger",
        "//frc971/vision:foxglove_image_converter",
        "//frc971/imu_fdcan:dual_imu_blender",
        "//frc971/imu_fdcan:can_translator",
        "//y2025/localizer:localizer_main",
        "//y2025/orin:can_logger",
        "//y2025/autonomous:autonomous_main",
        "//y2025/autonomous:autonomous_superstructure",
        "//frc971/vision:image_logger",
        "//frc971/vision:camera_focus",
        "//y2025/vision:frame_logger",
    ],
    data = [
        ":aos_config",
        "//frc971/orin:orin_irq_config.json",
        "//y2025/autonomous:trajectory_bfbs",
        "//y2025/constants:constants.json",
        "//y2025/www:www_files",
        "@ctre_phoenix6_arm64//:shared_libraries",
    ],
    dirs = [
        "//y2025/www:www_files",
    ],
    target_compatible_with = ["//tools/platforms/hardware:raspberry_pi"],
    target_type = "pi",
)

aos_config(
    name = "aos_config",
    src = "y2025.json",
    flatbuffers = [
        "//aos/network:message_bridge_client_fbs",
        "//aos/network:message_bridge_server_fbs",
        "//aos/network:timestamp_fbs",
        "//frc971/input:robot_state_fbs",
        "//frc971/vision:vision_fbs",
        "//frc971/vision:target_map_fbs",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
    deps = [
        ":config_orin1",
        ":config_roborio",
    ],
)

aos_config(
    name = "config_orin1",
    src = "y2025_orin1.json",
    flatbuffers = [
        "//aos/network:message_bridge_client_fbs",
        "//aos/network:message_bridge_server_fbs",
        "//y2025/autonomous:autonomous_superstructure_goal_fbs",
        "//frc971/imu_fdcan:dual_imu_fbs",
        "//frc971/control_loops/swerve:autonomous_init_fbs",
        "//frc971/control_loops/swerve:swerve_localizer_state_fbs",
        "//frc971/imu_fdcan:can_translator_status_fbs",
        "//frc971/imu_fdcan:dual_imu_blender_status_fbs",
        "//y2025/localizer:status_fbs",
        "//frc971/control_loops/swerve:position_goal_fbs",
        "//frc971/control_loops/swerve:swerve_drivetrain_can_position_fbs",
        "//frc971/control_loops/swerve:swerve_drivetrain_output_fbs",
        "//frc971/control_loops/swerve:swerve_drivetrain_goal_fbs",
        "//frc971/control_loops/swerve:swerve_drivetrain_position_fbs",
        "//frc971/control_loops/swerve:swerve_drivetrain_status_fbs",
        "//y2025/constants:constants_fbs",
        "//frc971/control_loops/drivetrain/localization:localizer_output_fbs",
        "//frc971/can_logger:can_logging_fbs",
        "//frc971/orin:hardware_stats_fbs",
        "//aos/network:timestamp_fbs",
        "//aos/util:filesystem_fbs",
        "//aos/network:remote_message_fbs",
        "//frc971/vision:calibration_fbs",
        "//frc971/vision:target_map_fbs",
        "//frc971/vision:vision_fbs",
        "//frc971/control_loops/swerve:autonomous_controller_debug_fbs",
        "//y2025/control_loops/superstructure:superstructure_goal_fbs",
        "@com_github_foxglove_schemas//:schemas",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
    deps = [
        "//aos/events:aos_config",
        "//frc971/control_loops/drivetrain:aos_config",
    ],
)

aos_config(
    name = "config_roborio",
    src = "y2025_roborio.json",
    flatbuffers = [
        "//frc971:can_configuration_fbs",
        "//aos/network:remote_message_fbs",
        "//aos/network:message_bridge_client_fbs",
        "//aos/network:message_bridge_server_fbs",
        "//frc971/wpilib:pdp_values_fbs",
        "//y2025/constants:constants_fbs",
        "//aos/network:timestamp_fbs",
        "//frc971/control_loops/swerve:swerve_drivetrain_can_position_fbs",
        "//frc971/control_loops/swerve:swerve_drivetrain_goal_fbs",
        "//frc971/control_loops/swerve:swerve_drivetrain_position_fbs",
        "//y2025/control_loops/superstructure:superstructure_goal_fbs",
        "//y2025/control_loops/superstructure:superstructure_can_position_fbs",
        "//y2025/control_loops/superstructure:superstructure_can_range_status_fbs",
        "//y2025/control_loops/superstructure:superstructure_output_fbs",
        "//y2025/control_loops/superstructure:superstructure_position_fbs",
        "//y2025/control_loops/superstructure:superstructure_status_fbs",
        "//frc971/can_logger:can_logging_fbs",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        "//aos/events:aos_config",
        "//frc971/control_loops/drivetrain:aos_config",
        "//frc971/input:aos_config",
    ],
)

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
        "//frc971/zeroing:absolute_encoder",
        "//frc971/zeroing:pot_and_absolute_encoder",
        "//y2025/constants:constants_fbs",
        "//y2025/control_loops/drivetrain:rotation_plants",
        "//y2025/control_loops/superstructure/elevator:elevator_plants",
        "//y2025/control_loops/superstructure/ground_intake_pivot:ground_intake_pivot_plants",
        "//y2025/control_loops/superstructure/pivot:pivot_plants",
        "//y2025/control_loops/superstructure/wrist:wrist_plants",
        "@com_google_absl//absl/base",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
    ],
)

cc_binary(
    name = "auto_align_updater",
    srcs = ["auto_align_updater.cc"],
    visibility = ["//visibility:public"],
    deps = [
        "//aos:init",
        "//aos/events:shm_event_loop",
        "//frc971/constants:constants_sender_lib",
        "//frc971/control_loops:pose",
        "//frc971/control_loops/swerve:position_goal_fbs",
        "//frc971/control_loops/swerve:swerve_localizer_state_fbs",
        "//frc971/vision:target_map_utils",
        "//y2025/constants:constants_fbs",
        "//y2025/control_loops/superstructure:superstructure_goal_fbs",
        "//y2025/control_loops/swerve:parameters",
        "//y2025/localizer",
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
        "//frc971:can_configuration_fbs",
        "//frc971/constants:constants_sender_lib",
        "//frc971/control_loops:control_loop",
        "//frc971/control_loops:control_loops_fbs",
        "//frc971/control_loops/drivetrain:drivetrain_can_position_fbs",
        "//frc971/control_loops/drivetrain:drivetrain_position_fbs",
        "//frc971/control_loops/swerve:swerve_drivetrain_position_fbs",
        "//frc971/input:robot_state_fbs",
        "//frc971/queues:gyro_fbs",
        "//frc971/wpilib:ADIS16448",
        "//frc971/wpilib:buffered_pcm",
        "//frc971/wpilib:can_drivetrain_writer",
        "//frc971/wpilib:can_range",
        "//frc971/wpilib:can_sensor_reader",
        "//frc971/wpilib:dma",
        "//frc971/wpilib:drivetrain_writer",
        "//frc971/wpilib:encoder_and_potentiometer",
        "//frc971/wpilib:generic_can_writer",
        "//frc971/wpilib:joystick_sender",
        "//frc971/wpilib:logging_fbs",
        "//frc971/wpilib:pdp_fetcher",
        "//frc971/wpilib:sensor_reader",
        "//frc971/wpilib:talonfx",
        "//frc971/wpilib:wpilib_robot_base",
        "//frc971/wpilib/swerve:swerve_drivetrain_writer",
        "//frc971/wpilib/swerve:swerve_util",
        "//third_party:phoenix",
        "//third_party:phoenix6",
        "//third_party:wpilib",
        "//y2025/constants:constants_fbs",
        "//y2025/control_loops/superstructure:led_indicator",
        "//y2025/control_loops/superstructure:superstructure_can_position_fbs",
        "//y2025/control_loops/superstructure:superstructure_can_range_status_fbs",
        "//y2025/control_loops/superstructure:superstructure_output_fbs",
        "//y2025/control_loops/superstructure:superstructure_position_fbs",
        "//y2025/control_loops/superstructure/elevator:elevator_plants",
        "//y2025/control_loops/superstructure/pivot:pivot_plants",
    ],
)

cc_binary(
    name = "joystick_reader",
    srcs = [
        ":joystick_reader.cc",
    ],
    deps = [
        ":constants",
        "//aos:init",
        "//aos/actions:action_lib",
        "//aos/logging",
        "//frc971/constants:constants_sender_lib",
        "//frc971/control_loops:profiled_subsystem_fbs",
        "//frc971/control_loops:static_zeroing_single_dof_profiled_subsystem",
        "//frc971/control_loops/swerve:swerve_localizer_state_fbs",
        "//frc971/input:action_joystick_input",
        "//frc971/input:drivetrain_input",
        "//frc971/input:joystick_input",
        "//frc971/input:redundant_joystick_data",
        "//frc971/input:swerve_joystick_input",
        "//y2025/control_loops/superstructure:superstructure_goal_fbs",
        "//y2025/control_loops/superstructure:superstructure_status_fbs",
        "@org_tuxfamily_eigen//:eigen",
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
        "//frc971/www:starter_main_bundle.min.js",
        "//y2025/www:field_main_bundle.min.js",
        "//y2025/www:files",
    ],
    target_compatible_with = ["@platforms//os:linux"],
)
