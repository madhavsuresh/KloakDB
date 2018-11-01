licenses(["notice"])

load("@bazel_rules//:config.bzl", "cc_fix_config")

cc_fix_config(
    name = "gen_config",
    cmake = True,
    files = {"src/config.h.cmake.in": "config.h"},
    values = {
        "_START_GOOGLE_NAMESPACE_": "namespace google {",
        "_END_GOOGLE_NAMESPACE_": "}",
        "GOOGLE_NAMESPACE": "google",
        "GOOGLE_GLOG_DLL_DECL": "",
        "HAVE_DLADDR": "1",
        "HAVE_SNPRINTF": "1",
        "HAVE_DLFCN_H": "1",
        "HAVE_FCNTL": "1",
        "HAVE_GLOB_H": "1",
        "HAVE_INTTYPES_H": "1",
        "HAVE_LIBPTHREAD": "1",
        #"HAVE_LIBUNWIND_H": "1",
        "HAVE_LIB_GFLAGS": "1",
        #"HAVE_LIB_UNWIND": "1",
        "HAVE_MEMORY_H": "1",
        "HAVE_NAMESPACES": "1",
        "HAVE_PREAD": "1",
        "HAVE_PTHREAD": "1",
        "HAVE_PWD_H": "1",
        "HAVE_PWRITE": "1",
        "HAVE_RWLOCK": "1",
        "HAVE_SIGACTION": "1",
        "HAVE_SIGALTSTACK": "1",
        "HAVE_STDINT_H": "1",
        "HAVE_STRING_H": "1",
        "HAVE_SYS_SYSCALL_H": "1",
        "HAVE_SYS_TIME_H": "1",
        "HAVE_SYS_TYPES_H": "1",
        "HAVE_SYS_UCONTEXT_H": "1",
        "HAVE_SYS_UTSNAME_H": "1",
        "HAVE_UNISTD_H": "1",
        "HAVE_USING_OPERATOR": "1",
        "HAVE_HAVE___ATTRIBUTE___": "1",
        "HAVE_HAVE___BUILTIN_EXPECT": "1",
        #"NO_FRAME_POINTER": "1",
        "_GNU_SOURCE": "1",
    },
    visibility = ["//visibility:private"],
)

cc_fix_config(
    name = "gen_headers",
    files = {
        "src/glog/logging.h.in": "glog/logging.h",
        "src/glog/raw_logging.h.in": "glog/raw_logging.h",
        "src/glog/stl_logging.h.in": "glog/stl_logging.h",
        "src/glog/vlog_is_on.h.in": "glog/vlog_is_on.h",
    },
    values = {
        "ac_cv_have_unistd_h": "1",
        "ac_cv_have_stdint_h": "1",
        "ac_cv_have_systypes_h": "1",
        "ac_cv_have_libgflags_h": "1",
        "ac_cv_have_libgflags": "1",
        "ac_cv_have_uint16_t": "1",
        "ac_cv_have___builtin_expect": "1",
        "ac_google_start_namespace": "namespace google {",
        "ac_google_end_namespace": "}",
        "ac_google_namespace": "google",
        "ac_cv___attribute___noinline": "__attribute__((noinline))",
        "ac_cv___attribute___noreturn": "__attribute__((noreturn))",
        "ac_cv___attribute___printf_4_5": "__attribute__((__format__ (__printf__, 4, 5)))",
    },
    visibility = ["//visibility:private"],
)

cc_library(
    name = "glog",
    deps = [
        "//external:gflags",
        ":gen_config",
        ":gen_headers",
    ],
    includes = [
        "src",
    ],
    copts = [
        "-isystem $(GENDIR)/external/gflags_repo/",
    ],
    srcs = [
        "src/demangle.cc",
        "src/logging.cc",
        "src/raw_logging.cc",
        "src/signalhandler.cc",
        "src/symbolize.cc",
        "src/utilities.cc",
        "src/vlog_is_on.cc",
    ],
    hdrs = [
        "src/demangle.h",
        "src/mock-log.h",
        "src/stacktrace.h",
        "src/symbolize.h",
        "src/utilities.h",
        "src/base/googleinit.h",
        "src/base/mutex.h",
        "src/glog/log_severity.h",
    ],
    linkopts = [
        "-pthread",
    ],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "vaultdb",
    srcs = [
        "VaultDB.cpp",
    ],
    deps = [
        "//glog",
        "//operators",
        "//rpc:rpc_lib",
    ],
)
