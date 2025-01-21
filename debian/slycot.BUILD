load("@//tools/build_rules:fortran.bzl", "f2c_library")
load("@//tools/build_rules:select.bzl", "compiler_select")

# TODO(austin): I bet this is wrong.
licenses(["restricted"])

f2c_library(
    name = "slicot",
    srcs = glob(["slycot/src/*.f"]),
    copts = [
        # This gets triggered because it doesn't realize xerbla doesn't return.
        # TODO(Brian): Try and get __attribute__((noreturn)) on xerbla somehow.
        "-Wno-uninitialized",
        "-Wno-unused-but-set-variable",
    ] + compiler_select({
        "clang": [
            "-Wno-unused-but-set-parameter",
            "-Wno-deprecated-non-prototype",
        ],
        "gcc": [
            "-Wno-discarded-qualifiers",
        ],
    }),
    visibility = ["//visibility:public"],
    deps = [
        "@clapack",
    ],
)
