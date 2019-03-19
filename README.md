# PrivateLearn

first change tensorflow path to your path in WORKSPACE

E.g. My path is as following

local_repository(
    name = "org_tensorflow",
    path = "/home/rd/tensorflow",
)


High level plan:
1. Use tensorflow code to generate .pb file
2. build c api using asylo sgx toolchain
3. pass .pb file into enclave and use c api to start learning process


FINISHED:
Learn the software stack of Tensorflow and select out what to port into enclave.
Turns out Tensorflow use high level Python code to generate dataflow graph and send the dataflow graph to low level worker code. Low level worker code will start learning using the received dataflow graph. Thus, for PrivateLearn, we need to port necessary low level c/c++ api into enclave.

Modify WORKSPACE in Ayslo Bazel build system to include Tensorflow as an externel dependencies. Because Bazel does not support package well yet, so need to add transitive dependencies manually.


TODO:
Now working on using Asylo SGX enclave toolchain to build c/c++ api of tensorflow and include into enclave runtime. Because the toolchain for building enclave dynamic library miss a good number of runtime support, now I will have to get ridof the missing headers and provide workarounds to deal with the side effects of getting rid of these headers.

And at the same time, I will trim the BUILD files related to c_api and get rid of the unnecessary features so that I can get around several missing posix call errors.
