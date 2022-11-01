package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "bounce_area_involution",
    srcs = ["bounce_area_involution.cc"],
    deps = [
        "//combinatorics:dyck_path",
        "//combinatorics:partition",
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
        "//combinatorics:partition",
        "//symmetric_group:character",
        "@com_google_absl//absl/types:span",
    ],
)
