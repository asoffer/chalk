package(default_visibility = ["//visibility:public"])

cc_library(
    name = "integer",
    hdrs = ["integer.h"],
    srcs = ["integer.cc"],
    deps = [
        "@com_google_absl//absl/numeric:int128",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:span",
    ],
)

cc_test(
    name = "integer_test",
    srcs = ["integer_test.cc"],
    deps = [
        ":integer",
        "@com_google_googletest//:gtest_main",
    ]
)
