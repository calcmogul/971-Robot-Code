cc_binary(
    name = "libNiFpgaLv.so.13",
    srcs = [
        "src/shims/fpgalv/main.c",
    ],
    linkshared = True,
    linkstatic = False,
)

cc_binary(
    name = "libnirio_emb_can.so.23",
    srcs = [
        "src/shims/embcan/main.c",
    ],
    linkshared = True,
    linkstatic = False,
)

cc_library(
    name = "ni-libraries",
    srcs = [
        "libNiFpgaLv.so.13",
        "libnirio_emb_can.so.23",
        "src/lib/chipobject/libRoboRIO_FRC_ChipObject.so.25.0.0",
        "src/lib/netcomm/libFRC_NetworkCommunication.so.25.0.0",
        "src/lib/visa/libvisa.so.23.3.0",
    ],
    hdrs = glob(["src/include/**"]),
    includes = [
        "src/include",
    ],
    linkopts = ["-ldl"],
    linkstatic = True,
    target_compatible_with = ["@//tools/platforms/hardware:roborio"],
    visibility = ["//visibility:public"],
)
