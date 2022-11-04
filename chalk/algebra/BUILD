package(default_visibility = ["//visibility:public"])

cc_library(
    name = "property",
    hdrs = ["property.h"],
    deps = [
        "//chalk/algebra/internal:property"
    ],
)

cc_library(
    name = "monomial",
    hdrs = ["monomial.h"],
    deps = [
        ":property",
        ":variable",
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

cc_library(
    name = "variable",
    hdrs = ["variable.h"],
    deps = [],
)

cc_test(
    name = "variable_test",
    srcs = ["variable_test.cc"],
    deps = [
        ":variable",
        "@com_google_googletest//:gtest_main",
    ]
)
