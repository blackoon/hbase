/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
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
#include <wangle/channel/AsyncSocketHandler.h>
#include <wangle/channel/EventBaseHandler.h>
#include <wangle/codec/LengthFieldBasedFrameDecoder.h>
#include <wangle/codec/LengthFieldPrepender.h>
#include <wangle/service/ServerDispatcher.h>

#include "connection/rpc-test-server-handler.h"
#include "connection/rpc-test-server.h"
#include "if/test.pb.h"

namespace hbase {

RpcTestServerSerializePipeline::Ptr RpcTestServerPipelineFactory::newPipeline(
    std::shared_ptr<AsyncTransportWrapper> sock) {
  auto pipeline = RpcTestServerSerializePipeline::create();
  pipeline->addBack(AsyncSocketHandler(sock));
  // ensure we can write from any thread
  pipeline->addBack(EventBaseHandler());
  pipeline->addBack(LengthFieldBasedFrameDecoder());
  pipeline->addBack(RpcTestServerSerializeHandler());
  pipeline->addBack(
      MultiplexServerDispatcher<std::unique_ptr<Request>, std::unique_ptr<Response>>(&service_));
  pipeline->finalize();

  return pipeline;
}

Future<std::unique_ptr<Response>> RpcTestService::operator()(std::unique_ptr<Request> request) {
  /* build Response */
  auto response = std::make_unique<Response>();
  response->set_call_id(request->call_id());
  std::string method_name = request->method();

  if (method_name == "ping") {
    auto pb_resp_msg = std::make_shared<EmptyResponseProto>();
    response->set_resp_msg(pb_resp_msg);
  } else if (method_name == "echo") {
    auto pb_resp_msg = std::make_shared<EchoResponseProto>();
    auto pb_req_msg = std::static_pointer_cast<EchoRequestProto>(request->req_msg());
    pb_resp_msg->set_message(pb_req_msg->message());
    response->set_resp_msg(pb_resp_msg);
  } else if (method_name == "error") {
    // TODO:
  } else if (method_name == "pause") {
    // TODO:
  } else if (method_name == "addr") {
    // TODO:
  }

  return folly::makeFuture<std::unique_ptr<Response>>(std::move(response));
}
}  // namespace hbase