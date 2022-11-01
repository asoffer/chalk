package(default_visibility = ["//visibility:public"])

cc_library(
    name = "character",
    hdrs = ["character.h"],
    srcs = ["character.cc"],
    deps = [
        "//chalk/combinatorics:partition",
        "@com_google_absl//absl/types:span",
        "@com_google_absl//absl/container:flat_hash_map",
    ],
)

cc_test(
    name = "character_test",
    srcs = ["character_test.cc"],
    deps = [
        ":character",
        "@com_google_googletest//:gtest_main",
    ]
)
