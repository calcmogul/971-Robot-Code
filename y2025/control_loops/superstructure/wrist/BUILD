package(default_visibility = ["//y2025:__subpackages__"])

genrule(
    name = "genrule_wrist",
    outs = [
        "wrist_plant.h",
        "wrist_plant.cc",
        "wrist_plant.json",
        "integral_wrist_plant.h",
        "integral_wrist_plant.cc",
        "integral_wrist_plant.json",
    ],
    cmd = "$(location //y2025/control_loops/python:wrist) $(OUTS)",
    target_compatible_with = ["@platforms//os:linux"],
    tools = [
        "//y2025/control_loops/python:wrist",
    ],
)

cc_library(
    name = "wrist_plants",
    srcs = [
        "integral_wrist_plant.cc",
        "wrist_plant.cc",
    ],
    hdrs = [
        "integral_wrist_plant.h",
        "wrist_plant.h",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
    deps = [
        "//frc971/control_loops:hybrid_state_feedback_loop",
        "//frc971/control_loops:state_feedback_loop",
    ],
)

filegroup(
    name = "wrist_json",
    srcs = ["integral_wrist_plant.json"],
    visibility = ["//visibility:public"],
)
