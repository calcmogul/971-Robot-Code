package(default_visibility = ["//y2025:__subpackages__"])

genrule(
    name = "genrule_elevator",
    outs = [
        "elevator_plant.h",
        "elevator_plant.cc",
        "elevator_plant.json",
        "integral_elevator_plant.h",
        "integral_elevator_plant.cc",
        "integral_elevator_plant.json",
    ],
    cmd = "$(location //y2025/control_loops/python:elevator) $(OUTS)",
    target_compatible_with = ["@platforms//os:linux"],
    tools = [
        "//y2025/control_loops/python:elevator",
    ],
)

cc_library(
    name = "elevator_plants",
    srcs = [
        "elevator_plant.cc",
        "integral_elevator_plant.cc",
    ],
    hdrs = [
        "elevator_plant.h",
        "integral_elevator_plant.h",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
    deps = [
        "//frc971/control_loops:hybrid_state_feedback_loop",
        "//frc971/control_loops:state_feedback_loop",
    ],
)

filegroup(
    name = "elevator_json",
    srcs = ["integral_elevator_plant.json"],
    visibility = ["//visibility:public"],
)
