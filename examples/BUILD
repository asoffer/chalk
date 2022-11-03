package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "bounce_area_involution",
    srcs = ["bounce_area_involution.cc"],
    deps = [
        "//chalk/combinatorics:dyck_path",
        "//chalk/combinatorics:image",
        "//chalk/combinatorics:partition",
        "//chalk/combinatorics:sequence",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/functional:function_ref",
        "@com_google_absl//absl/types:span",
    ],
)

cc_binary(
    name = "tensor_square_conjecture",
    srcs = ["tensor_square_conjecture.cc"],
    deps = [
        "//chalk/combinatorics:partition",
        "//chalk/symmetric_group:character",
        "@com_google_absl//absl/types:span",
    ],
)
