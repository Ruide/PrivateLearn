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



LOG:
I currently am able to create the dataflow model i want out from tensorflow high level api. And I get the .pb file prepared for transmitting inside enclave.

My plan is to port the c or c++ backend into enclave. And use that backend to read the .pb file and start training. In my imagination, it should work like https://gist.github.com/asimshankar/7c9f8a9b04323e93bb217109da8c7ad2. 

I suppose that to achieve this, i need to:
1. get .pb file prepared (done)
2. create static c/c++ backend library.
3. link the static library with enclave runtime and include the header files.  (Or compile from source code and statically linked in enclave_unsigned.so)
4. port the c/c++ code into enclave.
5. encrypt training data and call into enclave



Differential Privacy Reference:
https://github.com/tensorflow/privacy. Their adding differential privacy module is a dataflow graph defined on top of tensorflow.

Tensorflow backend Reference:
https://gist.github.com/asimshankar/7c9f8a9b04323e93bb217109da8c7ad2. Split backend out and load dataflow graph .pb file and start learning.
