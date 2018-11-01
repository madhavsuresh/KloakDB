http_archive(
    name = "com_github_gflags_gflags",
    sha256 = "6e16c8bc91b1310a44f3965e616383dbda48f83e8c1eaa2370a215057b00cabe",
    strip_prefix = "gflags-77592648e3f3be87d6c7123eb81cbad75f9aef5a",
    urls = [
        "https://mirror.bazel.build/github.com/gflags/gflags/archive/77592648e3f3be87d6c7123eb81cbad75f9aef5a.tar.gz",
        "https://github.com/gflags/gflags/archive/77592648e3f3be87d6c7123eb81cbad75f9aef5a.tar.gz",
    ],
)

http_archive(
    name = "com_github_grpc_grpc",
    strip_prefix = "grpc-01313976e1a44b5c9625d3a349fffa55471beff4",
    urls = [
        "https://github.com/grpc/grpc/archive/01313976e1a44b5c9625d3a349fffa55471beff4.tar.gz",
    ],
)

load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")

grpc_deps()

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "build_stack_rules_proto",
    strip_prefix = "rules_proto-1d6550fc2e625d47dc4faadac92d7cb20e3ba5c5",
    urls = ["https://github.com/stackb/rules_proto/archive/1d6550fc2e625d47dc4faadac92d7cb20e3ba5c5.tar.gz"],
)

load("@build_stack_rules_proto//cpp:deps.bzl", "cpp_proto_library")

cpp_proto_library()

load("@build_stack_rules_proto//cpp:deps.bzl", "cpp_grpc_library")

cpp_grpc_library()

git_repository(
    name = "com_github_glog_glog",
    commit = "e364e754a60af6f0eadd9902c4e76ecc060fee9c",
    remote = "https://github.com/google/glog.git",
)

bind(
    name = "glog",
    actual = "@com_github_glog_glog//:glog",
)
