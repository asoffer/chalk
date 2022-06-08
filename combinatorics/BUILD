package(default_visibility = ["//visibility:public"])

cc_library(
    name = "partition",
    hdrs = ["partition.h"],
    srcs = ["partition.cc"],
)

cc_test(
    name = "partition_test",
    srcs = ["partition_test.cc"],
    deps = [
        ":partition",
        "@com_google_googletest//:gtest_main",
    ]
)
