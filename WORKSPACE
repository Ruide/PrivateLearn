workspace(name = "asylo_examples")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")


local_repository(
    name = "org_tensorflow",
    path = "/home/rd/tensorflow",
)

#load("@mytensorflow//third_party/gpus:cuda_configure.bzl", "cuda_configure")
#cuda_configure(name = "local_config_cuda")

#load("@mytensorflow//third_party/gpus:rocm_configure.bzl", "rocm_configure")
#rocm_configure(name = "local_config_rocm")

#load("@mytensorflow//third_party/tensorrt:tensorrt_configure.bzl", "tensorrt_configure")
#tensorrt_configure(name = "local_config_tensorrt")

#load("@tensorflow//tensorflow:tensorflow.bzl", "")

#load("@tensorflow//third_party/gpus:rocm_configure.bzl", "rocm_configure")
#rocm_configure(name = "local_config_rocm")

# To update TensorFlow to a new revision.
# 1. Update the 'git_commit' args below to include the new git hash.
# 2. Get the sha256 hash of the archive with a command such as...
#    curl -L https://github.com/tensorflow/tensorflow/archive/<git hash>.tar.gz | sha256sum
#    and update the 'sha256' arg with the result.
# 3. Request the new archive to be mirrored on mirror.bazel.build for more
#    reliable downloads.

#load("//hello_world:repo.bzl", "tensorflow_http_archive")

#tensorflow_http_archive(
#    name = "org_tensorflow",
#    sha256 = "95f9897d008b3bed0ba34610dd39f0c36168ca00de54ebf08f827b5a37cab93f",
#    git_commit = "8fa58e776283de8d1e877fe6d57d6a3f6e4dc556",
#)



load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# TensorFlow depends on "io_bazel_rules_closure" so we need this here.
# Needs to be kept in sync with the same target in TensorFlow's WORKSPACE file.
http_archive(
    name = "io_bazel_rules_closure",
    sha256 = "a38539c5b5c358548e75b44141b4ab637bba7c4dc02b46b1f62a96d6433f56ae",
    strip_prefix = "rules_closure-dbb96841cc0a5fb2664c37822803b06dab20c7d1",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/rules_closure/archive/dbb96841cc0a5fb2664c37822803b06dab20c7d1.tar.gz",
        "https://github.com/bazelbuild/rules_closure/archive/dbb96841cc0a5fb2664c37822803b06dab20c7d1.tar.gz",  # 2018-04-13
    ],
)



load("@org_tensorflow//tensorflow:workspace.bzl", "tf_workspace")

tf_workspace(path_prefix = "", tf_repo_name = "org_tensorflow")

# Download and use the Asylo SDK.

http_archive(
    name = "com_google_asylo",
    urls = ["https://github.com/google/asylo/archive/v0.3.3.tar.gz"],
    strip_prefix = "asylo-0.3.3",
    sha256 = "55eaf1a2511a3ba5d1f5042a38b1129caaceb41088618454ed68abc8591a75a6",
)
# asylo_deps and asylo_testonly_deps are two functions. which load more http archives.
load("@com_google_asylo//asylo/bazel:asylo_deps.bzl", "asylo_deps",
     "asylo_testonly_deps")


asylo_deps()
bind(
    name = "python_headers",
    actual = "@com_google_protobuf//util/python:python_headers",
)
asylo_testonly_deps()

load("@com_google_asylo//asylo/bazel:sgx_deps.bzl", "sgx_deps")
sgx_deps()

load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")
grpc_deps()
