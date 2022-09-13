package(default_visibility = ["//visibility:public"])

cc_library(
    name = "dyck_path",
    hdrs = ["dyck_path.h"],
    srcs = ["dyck_path.cc"],
    deps = [
        "//:integer",
        "//base:iterator"
    ],
)

cc_test(
    name = "dyck_path_test",
    srcs = ["dyck_path_test.cc"],
    deps = [
        ":dyck_path",
        "@com_google_googletest//:gtest_main",
    ]
)

cc_library(
    name = "partition",
    hdrs = ["partition.h"],
    srcs = ["partition.cc"],
    deps = [
        "//:integer",
        "//base:iterator"
    ],
)

cc_test(
    name = "partition_test",
    srcs = ["partition_test.cc"],
    deps = [
        ":partition",
        "@com_google_googletest//:gtest_main",
    ]
)
