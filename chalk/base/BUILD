package(default_visibility = ["//visibility:public"])

cc_library(
    name = "iterator",
    hdrs = ["iterator.h"],
)

cc_library(
    name = "lazy",
    hdrs = ["lazy.h"],
)

cc_test(
    name = "lazy_test",
    srcs = ["lazy_test.cc"],
    deps = [
        ":lazy",
        "@com_google_googletest//:gtest_main",
    ]
)

cc_library(
    name = "type_traits",
    hdrs = ["type_traits.h"],
)

cc_test(
    name = "type_traits_test",
    srcs = ["type_traits_test.cc"],
    deps = [
        ":type_traits",
    ]
)
