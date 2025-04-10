load("//aos/flatbuffers:generate.bzl", "static_flatbuffer")

package(default_visibility = ["//visibility:public"])

static_flatbuffer(
    name = "constants_fbs",
    srcs = ["constants.fbs"],
)

cc_library(
    name = "averager",
    hdrs = [
        "averager.h",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@org_tuxfamily_eigen//:eigen",
    ],
)

cc_test(
    name = "averager_test",
    srcs = [
        "averager_test.cc",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        ":averager",
        "//aos/testing:googletest",
        "@org_tuxfamily_eigen//:eigen",
    ],
)

cc_library(
    name = "imu_zeroer",
    srcs = [
        "imu_zeroer.cc",
    ],
    hdrs = [
        "imu_zeroer.h",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        ":averager",
        "//frc971/control_loops:control_loops_fbs",
        "//frc971/control_loops/drivetrain:drivetrain_status_fbs",
        "//frc971/wpilib:imu_fbs",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@org_tuxfamily_eigen//:eigen",
    ],
)

cc_test(
    name = "imu_zeroer_test",
    srcs = [
        "imu_zeroer_test.cc",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        ":imu_zeroer",
        "//aos:flatbuffers",
        "//aos/testing:googletest",
        "@org_tuxfamily_eigen//:eigen",
    ],
)

cc_library(
    name = "zeroing",
    srcs = [
    ],
    hdrs = [
        "zeroing.h",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        ":wrap",
        "//aos/containers:error_list",
        "//aos/logging",
        "//frc971:constants",
        "//frc971/control_loops:control_loops_fbs",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
    ],
)

cc_library(
    name = "zeroing_test_lib",
    testonly = True,
    hdrs = ["zeroing_test.h"],
    deps = [
        ":zeroing",
        "//aos/testing:googletest",
        "//frc971/control_loops:control_loops_fbs",
        "//frc971/control_loops:position_sensor_sim",
    ],
)

[
    (
        cc_library(
            name = lib,
            srcs = [lib + ".cc"],
            hdrs = [lib + ".h"],
            deps = [
                ":wrap",
                ":zeroing",
                "//aos/containers:error_list",
                "//aos/logging",
                "//frc971:constants",
                "//frc971/control_loops:control_loops_fbs",
                "@com_google_absl//absl/log",
                "@com_google_absl//absl/log:check",
            ],
        ),
        cc_test(
            name = lib + "_test",
            srcs = [lib + "_test.cc"],
            deps = [
                lib,
                ":zeroing",
                ":zeroing_test_lib",
                "//aos/testing:googletest",
                "//frc971/control_loops:control_loops_fbs",
                "//frc971/control_loops:position_sensor_sim",
            ],
        ),
    )
    for lib in [
        "absolute_and_absolute_encoder",
        "absolute_encoder",
        "continuous_absolute_encoder",
        "hall_effect_and_position",
        "pot_and_absolute_encoder",
        "pot_and_index",
        "pulse_index",
    ]
]

cc_library(
    name = "wrap",
    srcs = [
        "wrap.cc",
    ],
    hdrs = [
        "wrap.h",
    ],
)

cc_test(
    name = "wrap_test",
    srcs = [
        "wrap_test.cc",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        ":wrap",
        "//aos/testing:googletest",
    ],
)

cc_test(
    name = "unwrap_test",
    srcs = [
        "unwrap_test.cc",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        ":wrap",
        "//aos/testing:googletest",
    ],
)
