genrule(
    name = "genrule_intake",
    outs = [
        "intake_plant.h",
        "intake_plant.cc",
        "intake_integral_plant.h",
        "intake_integral_plant.cc",
    ],
    cmd = "$(location //y2017/control_loops/python:intake) $(OUTS)",
    target_compatible_with = ["@platforms//os:linux"],
    tools = [
        "//y2017/control_loops/python:intake",
    ],
)

cc_library(
    name = "intake_plants",
    srcs = [
        "intake_integral_plant.cc",
        "intake_plant.cc",
    ],
    hdrs = [
        "intake_integral_plant.h",
        "intake_plant.h",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
    deps = [
        "//frc971/control_loops:state_feedback_loop",
    ],
)

cc_library(
    name = "intake",
    srcs = [
        "intake.cc",
    ],
    hdrs = [
        "intake.h",
    ],
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
    deps = [
        ":intake_plants",
        "//frc971/control_loops:profiled_subsystem",
        "//frc971/zeroing:pot_and_absolute_encoder",
        "//y2017:constants",
        "//y2017/control_loops/superstructure:superstructure_goal_fbs",
    ],
)
