# PrivateLearn (UNDER CONSTRUCTION, CANNOT BUILD YET)

This is fork from https://github.com/google/asylo

## Motivation is in the end

## High level idea:
1. Use tensorflow highlevel python api with differential privacy added to generate .pb file (this is the dataflow graph embedded in protocolbuf)
2. build c api using asylo sgx toolchain and adding it to enclave runtime support
3. pass .pb file into enclave and use c api to start learning process
(encryption for entering and leaving asylo enclave boundary)

## Detail idea:

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

To reproduce:

first change tensorflow path to your path in WORKSPACE

E.g. My path is as following

local_repository(
    name = "org_tensorflow",
    path = "/home/rd/tensorflow",
)

Bazel build --config=enc-sim /hello_world:hello_world


## FINISHED:

1. Learn the software stack of Tensorflow and select out what to port into enclave. Turns out Tensorflow use high level Python code to generate dataflow graph and send the dataflow graph to low level worker code. Low level worker code will start learning using the received dataflow graph. Thus, for PrivateLearn, we need to port necessary low level c/c++ api into enclave.

2. Modify WORKSPACE in Ayslo Bazel build system to include Tensorflow as an externel dependencies. Because Bazel does not support package well yet, so need to add transitive dependencies manually.

PS. Ayslo version -- v0.3
    Tensorflow version -- v1.12


## TODO:

1. Now working on using Asylo SGX enclave toolchain to build c/c++ api of tensorflow and include into enclave runtime. Because the toolchain for building enclave dynamic library miss a good number of runtime support, now I will have to get ridof the missing headers and provide workarounds to deal with the side effects of getting rid of these headers.

2. And at the same time, I will trim the BUILD files related to c_api and get rid of the unnecessary features so that I can get around several missing posix call errors.



## Motivation

There is no doubt that the coming of IoT and AI will give birth to tons of new applications. For example, on Google IO 2018, Google promised to bring personalized restaurant recommendations on Google map taking advantage of location data, user reviews and AI. These types of recommendation systems typically learn their models from user data. However, users might want to keep their data used for learning model private. Although Google have no intent on hurting users' privacy, users cannot be sure of that. Because servers can be taken down by malicious actors or insider and thus there is a concern for users when sharing their private data. Is there a way to be sure that the learning process is privacy-preserving and has strong security guarantee? Yes, and PrivateLearn provides a potential solution. There are two phases where leakage may happen on the server side. One is data leakage during the training phase and the other is data leakage from learnt model. Data leakage during training phase is because that during training, server can see user private data all in plaintext. Data leakage from learnt model is for the cases when a malicious actors get the learnt model, they can reconstruct training samples used to train this model. PrivateLearn put its eye on two advancing technology - Trusted Execution Environment (TEE) and Privacy-preserving Machine Learning for defending against this two scenarios. With TEE, PrivateLearn can guarantee users that their data collected will remain confidential even if the service provider's infrastructure is compromised. And users can verify the processing of their data through remote attestation. This defends against data leakage in training phase. On the other hand, with Privacy-preserving Machine Learning, PrivateLearn can guarantee users that models learnt can not be used to recover their private data. This defenses against data leakage from learnt model. To be specific, the TEE architecture PrivateLearn adopts is Intel SGX. And the privacy-preserving mechanism PrivateLearn chooses is differential privacy. In addition, PrivateLearn is built on top of combination of Asylos and Tensorflow. Asylo provides the TEE support to PrivateLearn and Tensorflow provides the machine learning backend and differential privacy feature to PrivateLearn. To sum up, PrivateLearn mitigates the privacy concern of users when they share their data to cloud service providers for learning. Users now can have faith in their data privacy because now even if the cloud infrastructure is compromised, their data still has certain level of protection. Cloud service provider can adopt PrivateLearn and claim that they have a strong security guarantee and they have no intention on stealing users' private data. Through remote attestation of remote cloud server, users can be sure of that the cloud service provider is not trying to perform malicious behavior on their confidential data.

## Additional Motivation

As IoT and AI steadily coming to people's daily lives, security and privacy concerns has emerged. With recent big data breakage incidents happening, people are losing their faith in the security and privacy guarantee claimed by cloud providers. To prevent slowdown of adoption of new IoT and AI applications, there is a need for privacy-preserving machine learning frameworks. PrivateLearn smells this need and aim at solving this concern. The privacy-preserving machine learning framework PrivateLearn presents is general, which means it can be extended to different application scenarios and provide the same strong level of security guarantee. PrivateLearn also shows that porting existing application into Asylo framework is practical. Although currently most deep learning computation happens on GPUs instead of CPU, PrivateLearn's design of processing dataflow graph inside enclave can be used to support GPUs when TEE on GPUs is possible. (There has already been several research work on secure system built on GPUs.) Tensorflow adopted by PrivateLearn supports GPU computation itself. And I believe when the secure GPU TEE come out, this can be ported into PrivateLearn framework also. The most interesting achievement of PrivateLearn is that, if implemented correctly, users can be sure of the privacy of their data even if the cloud infrastructure is compromised. And this can be achieved with a very minimal amount of performance overhead. Since the main training phases happens all in ring3 enclave memory and does not need to call out from enclave. I find this application of Asylo framework interesting.





