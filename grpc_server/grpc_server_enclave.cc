/*
 *
 * Copyright 2018 Asylo authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <chrono>
#include <memory>

#include "absl/base/thread_annotations.h"
#include "absl/synchronization/mutex.h"
#include "absl/synchronization/notification.h"
#include "absl/time/time.h"
#include "asylo/trusted_application.h"
#include "asylo/util/status.h"
#include "grpc_server/grpc_server_config.pb.h"
#include "grpc_server/translator_server.h"
#include "include/grpcpp/grpcpp.h"
#include "include/grpcpp/security/server_credentials.h"
#include "include/grpcpp/server.h"
#include "include/grpcpp/server_builder.h"

namespace examples {
namespace grpc_server {

// An enclave that runs a TranslatorServer. We override the methods of
// TrustedApplication as follows:
//
// * Initialize starts the gRPC server.
// * Run waits for the server to receive the shutdown RPC or for the provided
//   timeout to expire.
// * Finalize shuts down the server.
//
// Note: Enclaves do not have a secure source of time information. Consequently,
// enclaves should not rely on timing for security. For instance, the host can
// make the server in this example run forever or shut down prematurely by
// providing the enclave with incorrect time information. However, neither of
// these possibilities would compromise the security of the server in this
// example, so it is fine to rely on a non-secure source of time here.
class GrpcServerEnclave final : public asylo::TrustedApplication {
 public:
  GrpcServerEnclave() : service_(&shutdown_requested_) {}

  asylo::Status Initialize(const asylo::EnclaveConfig &enclave_config)
      LOCKS_EXCLUDED(server_mutex_) override;

  asylo::Status Run(const asylo::EnclaveInput &enclave_input,
                    asylo::EnclaveOutput *enclave_output) override;

  asylo::Status Finalize(const asylo::EnclaveFinal &enclave_final)
      LOCKS_EXCLUDED(server_mutex_) override;

 private:
  // Guards the |server_| member.
  absl::Mutex server_mutex_;

  // A gRPC server hosting |service_|.
  std::unique_ptr<::grpc::Server> server_ GUARDED_BY(server_mutex_);

  // The translation service.
  TranslatorServer service_;

  // An object that gets notified when the server receives a shutdown RPC.
  absl::Notification shutdown_requested_;

  // The amount of time that Finalize() should wait for the shutdown RPC before
  // shutting down anyway.
  absl::Duration shutdown_timeout_;
};

asylo::Status GrpcServerEnclave::Initialize(
    const asylo::EnclaveConfig &enclave_config) LOCKS_EXCLUDED(server_mutex_) {
  // Fail if there is no server_address available.
  if (!enclave_config.HasExtension(server_address)) {
    return asylo::Status(asylo::error::GoogleError::INVALID_ARGUMENT,
                         "Expected a server_address extension on config.");
  }

  // Fail if there is no server_max_lifetime available.
  if (!enclave_config.HasExtension(server_max_lifetime)) {
    return asylo::Status(asylo::error::GoogleError::INVALID_ARGUMENT,
                         "Expected a server_max_lifetime extension on config.");
  }

  shutdown_timeout_ =
      absl::Seconds(enclave_config.GetExtension(server_max_lifetime));

  // Lock |server_mutex_| so that we can start setting up the server.
  absl::MutexLock lock(&server_mutex_);

  // Check that the server is not already running.
  if (server_) {
    return asylo::Status(asylo::error::GoogleError::ALREADY_EXISTS,
                         "Server is already started");
  }

  // Create a ServerBuilder object to set up the server.
  ::grpc::ServerBuilder builder;

  // Add a listening port to the server.
  //
  // Note: This gRPC server is hosted with InsecureServerCredentials. This
  // means that no additional security is used for channel establishment.
  // Neither the server nor its clients are authenticated, and no channels are
  // secured. This configuration is not suitable for a production environment.
  int selected_port;
  builder.AddListeningPort(enclave_config.GetExtension(server_address),
                           ::grpc::InsecureServerCredentials(), &selected_port);

  // Add the translator service to the server.
  builder.RegisterService(&service_);

  // Start the server.
  server_ = builder.BuildAndStart();
  if (!server_) {
    return asylo::Status(asylo::error::GoogleError::INTERNAL,
                         "Failed to start server");
  }

  LOG(INFO) << "Server started on port " << selected_port;

  return asylo::Status::OkStatus();
}

asylo::Status GrpcServerEnclave::Run(const asylo::EnclaveInput &enclave_input,
                                     asylo::EnclaveOutput *enclave_output) {
  // Wait until the timeout runs out or the server receives a shutdown RPC.
  shutdown_requested_.WaitForNotificationWithTimeout(shutdown_timeout_);

  return asylo::Status::OkStatus();
}

asylo::Status GrpcServerEnclave::Finalize(
    const asylo::EnclaveFinal &enclave_final) LOCKS_EXCLUDED(server_mutex_) {
  // Lock |server_mutex_| so that we can start shutting down the server.
  absl::MutexLock lock(&server_mutex_);

  // If the server exists, then shut it down. Also delete the Server object to
  // indicate that it is no longer valid.
  if (server_) {
    LOG(INFO) << "Server shutting down";

    // Give all outstanding RPC calls 500 milliseconds to complete.
    server_->Shutdown(std::chrono::system_clock::now() +
                      std::chrono::milliseconds(500));
    server_.reset(nullptr);
  }

  return asylo::Status::OkStatus();
}

}  // namespace grpc_server
}  // namespace examples

namespace asylo {

TrustedApplication *BuildTrustedApplication() {
  return new examples::grpc_server::GrpcServerEnclave;
}

}  // namespace asylo
