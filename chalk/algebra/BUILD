package(default_visibility = ["//visibility:public"])

cc_library(
    name = "property",
    hdrs = ["property.h"],
    deps = [
        "//chalk/algebra/internal:property"
    ],
)

cc_library(
    name = "dense_polynomial",
    hdrs = ["dense_polynomial.h"],
    deps = [
        ":monomial",
        ":property",
        "//chalk/base:type_traits",
        "//chalk/base:lazy",
        "@com_google_absl//absl/container:flat_hash_map",
    ],
)

cc_test(
    name = "dense_polynomial_test",
    srcs = ["dense_polynomial_test.cc"],
    deps = [
        ":dense_polynomial",
        "@com_google_googletest//:gtest_main",
    ]
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
    name = "polynomial",
    hdrs = ["polynomial.h"],
    deps = [
        ":monomial",
        ":property",
        "//chalk/base:type_traits",
        "//chalk/base:lazy",
        "@com_google_absl//absl/container:flat_hash_map",
    ],
)

cc_test(
    name = "polynomial_test",
    srcs = ["polynomial_test.cc"],
    deps = [
        ":polynomial",
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
