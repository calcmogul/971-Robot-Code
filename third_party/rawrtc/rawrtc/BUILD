load("@org_frc971//tools/build_rules:select.bzl", "compiler_select")

cc_library(
    name = "rawrtc",
    srcs = glob([
        "src/**/*.c",
        "src/**/*.h",
    ]),
    hdrs = glob(["include/**/*.h"]),
    copts = compiler_select({
        "clang": [
            "-Wno-incompatible-pointer-types-discards-qualifiers",
        ],
        "gcc": [
            "-Wno-discarded-qualifiers",
            "-Wno-incompatible-pointer-types",
            "-Wno-implicit-fallthrough",
        ],
    }) + [
        "-Wno-implicit-function-declaration",
        "-Wno-int-conversion",
        "-Wno-cast-qual",
        "-Wno-missing-braces",
        "-Iexternal/com_github_rawrtc_rawrtc/",
        "-DUSE_OPENSSL",
        #"-DUSE_ZLIB",
        "-DHAVE_INET6",
        "-DHAVE_STDBOOL_H",
        "-DHAVE_INTTYPES_H",
    ],
    defines = [
        "HAVE_STDBOOL_H",
    ],
    includes = ["include/"],
    local_defines = [
        "RAWRTC_VERSION=\\\"0.5.1\\\"",
        "RAWRTC_SKIP_DH_CHECK",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@com_github_rawrtc_rawrtc_common//:rawrtcc",
        "@com_github_rawrtc_rawrtc_data_channel//:rawrtcdc",
    ],
)

cc_library(
    name = "tools_helper",
    srcs = [
        "tools/helper/common.c",
        "tools/helper/handler.c",
        "tools/helper/parameters.c",
        "tools/helper/utils.c",
    ],
    hdrs = [
        "tools/helper/common.h",
        "tools/helper/handler.h",
        "tools/helper/parameters.h",
        "tools/helper/utils.h",
    ],
    copts = [
        "-Wno-missing-braces",
        "-Wno-incompatible-pointer-types-discards-qualifiers",
    ] + compiler_select({
        "clang": [],
        "gcc": [
            "-Wno-discarded-qualifiers",
        ],
    }),
    visibility = ["//visibility:public"],
    deps = [":rawrtc"],
)

cc_binary(
    name = "peer-connection",
    srcs = [
        "tools/peer-connection.c",
    ],
    copts = [
        "-Wno-missing-braces",
    ] + compiler_select({
        "clang": [
            "-Wno-incompatible-pointer-types-discards-qualifiers",
        ],
        "gcc": [
            "-Wno-incompatible-pointer-types",
            "-Wno-discarded-qualifiers",
        ],
    }),
    includes = ["tools"],
    deps = [
        ":rawrtc",
        ":tools_helper",
    ],
)
