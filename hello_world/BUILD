#
# Copyright 2018 Asylo authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

licenses(["notice"])  # Apache v2.0

# Hello World project for Asylo.

load(
    "@com_google_asylo//asylo/bazel:asylo.bzl",
    "enclave_loader",
    "sim_enclave",
)
load("@com_google_asylo//asylo/bazel:proto.bzl", "asylo_proto_library")

load("@com_google_asylo//asylo/bazel:asylo.bzl", "ASYLO_ALL_BACKENDS")
load("@com_google_asylo//asylo/bazel:copts.bzl", "ASYLO_DEFAULT_COPTS")


package(
    default_visibility = ["//visibility:public"],
)

# Example for using the Asylo framework.


asylo_proto_library(
    name = "hello_proto",
    srcs = ["hello.proto"],
    deps = ["@com_google_asylo//asylo:enclave_proto"],
)
'''
cc_import(
  name = "libtensorflow",
  hdrs = ["c_api.h"],
  shared_library = "libtensorflow.so",
)

cc_import(
  name = "libtensorflow_framework",
  hdrs = ["c_api.h"],
  shared_library = "libtensorflow_framework.so",
)

# Runtime library for Asylo enclaves.
#
cc_library(
    name = "enclave_runtime",
    hdrs = ["trusted_application.h"],
    copts = ASYLO_DEFAULT_COPTS,
    linkstatic = 1,
    tags = ASYLO_ALL_BACKENDS,
    visibility = ["//visibility:public"],
    deps = [
        "//asylo/platform/arch:trusted_arch",
        "//asylo/platform/core:trusted_application",
        "//asylo/platform/crypto/gcmlib:trusted_gcmlib",
        "//asylo/platform/posix:trusted_posix",
        "//asylo/platform/storage/secure:trusted_secure",
        "//asylo/platform/storage/utils:trusted_utils",
        "//asylo/platform/system",
    ],
)

@com_google_tensorflow//
'''

sim_enclave(
    name = "hello_enclave.so",
    srcs = ["hello_enclave.cc"],
    deps = [
        ":hello_proto_cc",
        "@com_google_absl//absl/strings",
        "@com_google_asylo//asylo:enclave_runtime",
        "@com_google_asylo//asylo/util:logging",
        "@com_google_asylo//asylo/util:status",
        #"@org_tensorflow//tensorflow/c:c_api_internal",
        "@org_tensorflow//tensorflow/core:core_cpu",
        #"libtensorflow",
        #"libtensorflow_framework"
    ],
)

enclave_loader(
    name = "hello_world",
    srcs = ["hello_driver.cc"],
    enclaves = {"enclave": ":hello_enclave.so"},
    loader_args = ["--enclave_path='{enclave}'"],
    deps = [
        ":hello_proto_cc",
        "@com_github_gflags_gflags//:gflags_nothreads",
        "@com_google_absl//absl/strings",
        "@com_google_asylo//asylo:enclave_client",
        "@com_google_asylo//asylo/util:logging",
    ],
)
