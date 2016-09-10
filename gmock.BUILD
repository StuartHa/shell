cc_library(
    name = "main",
    srcs = glob(
        ["src/*.cc"],
        exclude = ["src/gmock-all.cc"]
    ),
    hdrs = glob([
        "include/**/*.h"
    ]),
    copts = ["-Iexternal/gmock/include"],
    linkopts = ["-pthread"],
    visibility = ["//visibility:public"],
)
