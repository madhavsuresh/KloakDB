# Copyright 2015 gRPC authors.
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
"""The Python implementation of the GRPC helloworld.Greeter server."""

from concurrent import futures
import time

import grpc

import logger_pb2
import logger_pb2_grpc

_ONE_DAY_IN_SECONDS = 60 * 60 * 24


class LogServer(logger_pb2_grpc.LogsServicer):

    def Log(self, request, context):
        print(request.logmessage.strip())
        return logger_pb2.LogReply()


def serve():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=20))
    logger_pb2_grpc.add_LogsServicer_to_server(LogServer(), server)
    server.add_insecure_port('[::]:60000')
    server.start()
    try:
        while True:
            time.sleep(_ONE_DAY_IN_SECONDS)
    except KeyboardInterrupt:
        server.stop(0)


if __name__ == '__main__':
    serve()
