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

#include "grpc_server/translator_server.h"

#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "include/grpcpp/grpcpp.h"

namespace examples {
namespace grpc_server {

TranslatorServer::TranslatorServer(absl::Notification *shutdown_requested)
    : Service(),
      // Initialize the translation map with a few known translations.
      translation_map_({{"asylo", "sanctuary"},
                        {"istio", "sail"},
                        {"kubernetes", "helmsman"}}),
      shutdown_requested_(shutdown_requested) {}

::grpc::Status TranslatorServer::GetTranslation(
    ::grpc::ServerContext *context, const GetTranslationRequest *request,
    GetTranslationResponse *response) {
  // Confirm that |*request| has an |input_word| field.
  if (!request->has_input_word()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          "No input word given");
  }

  // Confirm that the translation map has a translation for the input word.
  auto response_iterator =
      translation_map_.find(absl::AsciiStrToLower(request->input_word()));
  if (response_iterator == translation_map_.end()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          absl::StrCat("No known translation for \"",
                                       request->input_word(), "\""));
  }

  // Return the translation.
  response->set_translated_word(response_iterator->second);
  return ::grpc::Status::OK;
}

::grpc::Status TranslatorServer::Shutdown(::grpc::ServerContext *context,
                                          const ShutdownRequest *query,
                                          ShutdownResponse *response)
    LOCKS_EXCLUDED(shutdown_requested_mutex_) {
  // Lock shutdown_requested_mutex_ and request a shutdown.
  absl::MutexLock lock(&shutdown_requested_mutex_);
  if (!shutdown_requested_->HasBeenNotified()) {
    shutdown_requested_->Notify();
  }
  return ::grpc::Status::OK;
}

}  // namespace grpc_server
}  // namespace examples
