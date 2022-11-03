package(default_visibility = ["//visibility:public"])

cc_library(
    name = "composition",
    hdrs = ["composition.h"],
    deps = [],
)

cc_test(
    name = "composition_test",
    srcs = ["composition_test.cc"],
    deps = [
        ":composition",
        "@com_google_googletest//:gtest_main",
    ]
)

cc_library(
    name = "dyck_path",
    hdrs = ["dyck_path.h"],
    srcs = ["dyck_path.cc"],
    deps = [
        ":composition",
        ":image",
        "//chalk:integer",
        "@com_google_absl//absl/functional:function_ref",
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
    name = "image",
    hdrs = ["image.h"],
    srcs = ["image.cc"],
    deps = [],
)

cc_library(
    name = "partition",
    hdrs = ["partition.h"],
    srcs = ["partition.cc"],
    deps = [
        ":composition",
        "//chalk:integer",
        "//chalk/base:iterator"
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

cc_library(
    name = "sequence",
    hdrs = ["sequence.h"],
    deps = [],
)

cc_test(
    name = "sequence_test",
    srcs = ["sequence_test.cc"],
    deps = [
        ":sequence",
        "@com_google_googletest//:gtest_main",
    ]
)
