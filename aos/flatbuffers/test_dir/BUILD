load("//aos/flatbuffers:generate.bzl", "static_flatbuffer")

static_flatbuffer(
    name = "include_reflection_fbs",
    srcs = ["include_reflection.fbs"],
    visibility = ["//visibility:public"],
    deps = ["//aos/flatbuffers/reflection:reflection_fbs"],
)

static_flatbuffer(
    name = "include_fbs",
    srcs = ["include.fbs"],
    visibility = ["//visibility:public"],
)

static_flatbuffer(
    name = "type_coverage_fbs",
    srcs = ["type_coverage.fbs"],
    visibility = ["//visibility:public"],
)

filegroup(
    name = "test_data",
    srcs = ["type_coverage.json"],
    visibility = ["//visibility:public"],
)

sh_binary(
    name = "copy_test_static_header",
    srcs = ["copy_test_static_header.sh"],
    args = ["$(location //aos/flatbuffers:test_static_file)"],
    data = ["//aos/flatbuffers:test_static_file"],
)

sh_test(
    name = "compare_generated_files",
    srcs = ["compare_generated_files.sh"],
    args = [
        "$(location //aos/flatbuffers:test_static_file)",
        "$(location sample_test_static.h)",
    ],
    data = [
        "//aos/flatbuffers:test_static_file",
        "@org_frc971//aos/flatbuffers/test_dir:sample_test_static.h",
    ],
)
