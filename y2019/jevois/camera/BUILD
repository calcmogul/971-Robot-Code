package(default_visibility = ["//visibility:public"])

cc_library(
    name = "reader",
    srcs = ["reader.cc"],
    hdrs = ["reader.h"],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        "//aos/time",
        "//aos/vision/image:camera_params",
        "//aos/vision/image:image_types",
        "//aos/vision/image:reader",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
    ],
)

cc_library(
    name = "image_stream",
    srcs = ["image_stream.cc"],
    hdrs = ["image_stream.h"],
    target_compatible_with = ["@platforms//os:linux"],
    deps = [
        ":reader",
        "//aos/logging",
        "//aos/vision/events:epoll_events",
    ],
)
