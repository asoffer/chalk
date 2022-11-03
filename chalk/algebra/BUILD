package(default_visibility = ["//visibility:public"])

cc_library(
    name = "property",
    hdrs = ["property.h"],
    deps = [],
)

cc_library(
    name = "monomial",
    hdrs = ["monomial.h"],
    deps = [
        ":property",
    ],
)

cc_test(
    name = "monomial_test",
    srcs = ["monomial_test.cc"],
    deps = [
        ":monomial",
        "@com_google_googletest//:gtest_main",
    ]
)
