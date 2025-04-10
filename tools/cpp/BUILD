load(":toolchain_config.bzl", "cc_toolchain_config")
load(":toolchain_make_variables.bzl", "cc_toolchain_make_variables")

package(default_visibility = ["//visibility:public"])

# Use this instead of @bazel_tools//tools/cpp:current_cc_toolchain.
# This one supports platforms.
cc_toolchain_make_variables(name = "cc_toolchain_make_variables")

[
    cc_toolchain_config(
        name = "{}_toolchain_config".format(cpu),
        cpu = cpu,
    )
    for cpu in [
        "cortex-m4f",
        "cortex-m4f-k22",
        "rp2040",
        "cortex-m4f-imu",
        "roborio",
    ]
]

cc_library(
    name = "empty_main",
    srcs = ["empty_main.c"],
    target_compatible_with = ["@platforms//os:linux"],
)

cc_library(
    name = "malloc",
    deps = select({
        "//tools:has_asan": [],
        "//tools:has_tsan": [],
        "//tools:cpu_cortex_m4f": [],
        "//tools:cpu_cortex-m4f-imu": [],
        "//tools:cpu_cortex_m0plus": [],
        # TODO(phil): Support this properly.
        #"//tools:cpu_cortex_m4f_k22": [],
        "//conditions:default": [],
        # TODO(austin): Figure out how to make malloc hooks work with the new tcmalloc...
        #"//conditions:default": ["@com_google_tcmalloc//tcmalloc"],
    }),
)

cc_library(
    name = "stl",
    target_compatible_with = ["@platforms//os:linux"],
)

filegroup(
    name = "empty",
    srcs = [],
)

# Compiler inputs given by --copt etc in //tools:bazel.rc.
filegroup(
    name = "flags_compiler_inputs",
    srcs = select({
        "//tools:has_asan": [
            "asan-blacklist",
        ],
        "//tools:has_ubsan": [
            "ubsan-blacklist",
        ],
        "//conditions:default": [],
    }),
)

filegroup(
    name = "roborio-compiler-files",
    srcs = [
        ":flags_compiler_inputs",
        "//tools/cpp/arm-frc-linux-gnueabi:as",
        "//tools/cpp/arm-frc-linux-gnueabi:libs",
        "//tools/cpp/arm-frc-linux-gnueabi:tool-wrappers",
        "@arm_frc_linux_gnueabi_repo//:compiler_pieces",
    ],
)

filegroup(
    name = "roborio_linker_files",
    srcs = [
        "//tools/cpp/arm-frc-linux-gnueabi:ar",
        "//tools/cpp/arm-frc-linux-gnueabi:gcc",
        "//tools/cpp/arm-frc-linux-gnueabi:ld",
        "//tools/cpp/arm-frc-linux-gnueabi:libs",
        "@arm_frc_linux_gnueabi_repo//:compiler_pieces",
    ],
)

filegroup(
    name = "roborio_ar_files",
    srcs = [
        "//tools/cpp/arm-frc-linux-gnueabi:ar",
        "@arm_frc_linux_gnueabi_repo//:compiler_pieces",
    ],
)

filegroup(
    name = "roborio_compiler_files",
    srcs = [
        "//tools/cpp/arm-frc-linux-gnueabi:gcc",
        "//tools/cpp/arm-frc-linux-gnueabi:ld",
        "@arm_frc_linux_gnueabi_repo//:compiler_components",
        "@arm_frc_linux_gnueabi_repo//:compiler_pieces",
    ],
)

filegroup(
    name = "roborio_strip_files",
    srcs = [
        "//tools/cpp/arm-frc-linux-gnueabi:strip",
        "@arm_frc_linux_gnueabi_repo//:compiler_pieces",
    ],
)

cc_toolchain(
    name = "cc-compiler-roborio",
    all_files = ":roborio-compiler-files",
    ar_files = ":roborio_ar_files",
    as_files = ":roborio_compiler_files",
    compiler_files = ":roborio_compiler_files",
    dwp_files = ":empty",
    linker_files = ":roborio_linker_files",
    objcopy_files = "//tools/cpp/arm-frc-linux-gnueabi:objcopy",
    strip_files = ":roborio_strip_files",
    supports_param_files = 1,
    toolchain_config = ":roborio_toolchain_config",
    toolchain_identifier = "roborio_linux",
)

toolchain(
    name = "cc-toolchain-roborio",
    exec_compatible_with = [
        "@platforms//os:linux",
        "@platforms//cpu:x86_64",
    ],
    target_compatible_with = [
        "@platforms//os:linux",
        "//tools/platforms/hardware:roborio",
    ],
    toolchain = ":cc-compiler-roborio",
    toolchain_type = "@bazel_tools//tools/cpp:toolchain_type",
)

filegroup(
    name = "gcc_arm_none_eabi_none_files",
    srcs = [
        "//tools/cpp/gcc_arm_none_eabi:tool-wrappers",
        "@gcc_arm_none_eabi//:compiler_pieces",
    ],
)

filegroup(
    name = "gcc_arm_none_eabi_compiler_files",
    srcs = [
        "//tools/cpp/gcc_arm_none_eabi:as",
        "//tools/cpp/gcc_arm_none_eabi:gcc",
        "//tools/cpp/gcc_arm_none_eabi:ld",
        "@gcc_arm_none_eabi//:compiler_pieces",
    ],
)

filegroup(
    name = "gcc_arm_none_eabi_linker_files",
    srcs = [
        "//frc971/imu_fdcan/Dual_IMU:linker_scripts",
        "//motors/core:linkerscripts",
        "//tools/cpp/gcc_arm_none_eabi:ar",
        "//tools/cpp/gcc_arm_none_eabi:gcc",
        "//tools/cpp/gcc_arm_none_eabi:ld",
        "@gcc_arm_none_eabi//:compiler_pieces",
    ],
)

filegroup(
    name = "gcc_arm_none_eabi_ar_files",
    srcs = [
        "//tools/cpp/gcc_arm_none_eabi:ar",
        "@gcc_arm_none_eabi//:compiler_pieces",
    ],
)

cc_toolchain(
    name = "cc-compiler-rp2040",
    all_files = ":gcc_arm_none_eabi_none_files",
    ar_files = ":gcc_arm_none_eabi_ar_files",
    compiler_files = ":gcc_arm_none_eabi_compiler_files",
    dwp_files = ":empty",
    linker_files = ":gcc_arm_none_eabi_linker_files",
    objcopy_files = "//tools/cpp/gcc_arm_none_eabi:objcopy",
    strip_files = "//tools/cpp/gcc_arm_none_eabi:strip",
    supports_param_files = 1,
    toolchain_config = ":rp2040_toolchain_config",
    toolchain_identifier = "rp2040",
)

toolchain(
    name = "cc-toolchain-rp2040",
    exec_compatible_with = [
        "@platforms//os:linux",
        "@platforms//cpu:x86_64",
    ],
    target_compatible_with = [
        "@platforms//os:none",
        "//tools/platforms/hardware:cortex_m0plus",
    ],
    toolchain = ":cc-compiler-rp2040",
    toolchain_type = "@bazel_tools//tools/cpp:toolchain_type",
)

cc_toolchain(
    name = "cc-compiler-cortex-m4f-imu",
    all_files = ":gcc_arm_none_eabi_none_files",
    ar_files = ":gcc_arm_none_eabi_ar_files",
    compiler_files = ":gcc_arm_none_eabi_compiler_files",
    dwp_files = ":empty",
    linker_files = ":gcc_arm_none_eabi_linker_files",
    objcopy_files = "//tools/cpp/gcc_arm_none_eabi:objcopy",
    strip_files = "//tools/cpp/gcc_arm_none_eabi:strip",
    supports_param_files = 1,
    toolchain_config = ":cortex-m4f-imu_toolchain_config",
    toolchain_identifier = "cortex-m4f-imu",
)

toolchain(
    name = "cc-toolchain-cortex-m4f-imu",
    exec_compatible_with = [
        "@platforms//os:linux",
        "@platforms//cpu:x86_64",
    ],
    target_compatible_with = [
        "@platforms//os:none",
        "//tools/platforms/hardware:cortex-m4f-imu",
    ],
    toolchain = ":cc-compiler-cortex-m4f-imu",
    toolchain_type = "@bazel_tools//tools/cpp:toolchain_type",
)

cc_toolchain(
    name = "cc-compiler-cortex-m4f",
    all_files = ":gcc_arm_none_eabi_none_files",
    ar_files = ":gcc_arm_none_eabi_ar_files",
    compiler_files = ":gcc_arm_none_eabi_compiler_files",
    dwp_files = ":empty",
    linker_files = ":gcc_arm_none_eabi_linker_files",
    objcopy_files = "//tools/cpp/gcc_arm_none_eabi:objcopy",
    strip_files = "//tools/cpp/gcc_arm_none_eabi:strip",
    supports_param_files = 1,
    toolchain_config = ":cortex-m4f_toolchain_config",
    toolchain_identifier = "cortex-m4f",
)

toolchain(
    name = "cc-toolchain-cortex-m4f",
    exec_compatible_with = [
        "@platforms//os:linux",
        "@platforms//cpu:x86_64",
    ],
    target_compatible_with = [
        "@platforms//os:none",
        "//tools/platforms/hardware:cortex_m4f",
    ],
    toolchain = ":cc-compiler-cortex-m4f",
    toolchain_type = "@bazel_tools//tools/cpp:toolchain_type",
)

cc_toolchain(
    name = "cc-compiler-cortex-m4f-k22",
    all_files = ":gcc_arm_none_eabi_none_files",
    compiler_files = ":gcc_arm_none_eabi_compiler_files",
    dwp_files = ":empty",
    linker_files = ":gcc_arm_none_eabi_linker_files",
    objcopy_files = ":empty",
    strip_files = ":empty",
    supports_param_files = 1,
    toolchain_config = ":cortex-m4f-k22_toolchain_config",
    toolchain_identifier = "cortex-m4f-k22",
)

toolchain(
    name = "cc-toolchain-cortex-m4f-k22",
    exec_compatible_with = [
        "@platforms//os:linux",
        "@platforms//cpu:x86_64",
    ],
    target_compatible_with = [
        "@platforms//os:none",
        "//tools/platforms/hardware:cortex_m4f",
    ],
    toolchain = ":cc-compiler-cortex-m4f-k22",
    toolchain_type = "@bazel_tools//tools/cpp:toolchain_type",
)
