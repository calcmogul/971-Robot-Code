load("//frc971/downloader:downloader.bzl", "aos_downloader_dir")
load("//tools/build_rules:js.bzl", "rollup_bundle", "ts_project")

filegroup(
    name = "files",
    srcs = glob([
        "**/*.html",
        "**/*.css",
        "**/*.png",
    ]) + ["2025.png"],
    visibility = ["//visibility:public"],
)

genrule(
    name = "2025_field_png",
    srcs = ["//third_party/y2025/field:pictures"],
    outs = ["2025.png"],
    cmd = "cp third_party/y2025/field/2025.png $@",
)

ts_project(
    name = "field_main",
    srcs = [
        "constants.ts",
        "field_handler.ts",
        "field_main.ts",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        "//aos/network:connect_ts_fbs",
        "//aos/network:message_bridge_client_ts_fbs",
        "//aos/network:message_bridge_server_ts_fbs",
        "//aos/network:web_proxy_ts_fbs",
        "//aos/network/www:proxy",
        "//frc971/control_loops:control_loops_ts_fbs",
        "//frc971/control_loops/drivetrain:drivetrain_can_position_ts_fbs",
        "//frc971/control_loops/drivetrain:drivetrain_position_ts_fbs",
        "//frc971/control_loops/drivetrain:drivetrain_status_ts_fbs",
        "//frc971/control_loops/drivetrain/localization:localizer_output_ts_fbs",
        "//frc971/control_loops/swerve:autonomous_init_ts_fbs",
        "//frc971/control_loops/swerve:swerve_drivetrain_status_ts_fbs",
        "//frc971/control_loops/swerve:swerve_localizer_state_ts_fbs",
        "//frc971/vision:target_map_ts_fbs",
        "//y2025/control_loops/superstructure:superstructure_goal_ts_fbs",
        "//y2025/control_loops/superstructure:superstructure_position_ts_fbs",
        "//y2025/control_loops/superstructure:superstructure_status_ts_fbs",
        "//y2025/localizer:status_ts_fbs",
        "@com_github_google_flatbuffers//ts:flatbuffers_ts",
    ],
)

rollup_bundle(
    name = "field_main_bundle",
    entry_point = "field_main.ts",
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//y2025:__subpackages__"],
    deps = [
        ":field_main",
    ],
)

aos_downloader_dir(
    name = "www_files",
    srcs = [
        ":field_main_bundle.min.js",
        ":files",
        "//frc971/analysis:plot_index_bundle.min.js",
        "//frc971/www:starter_files",
    ],
    dir = "www",
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
)
