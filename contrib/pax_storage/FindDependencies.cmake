# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
find_package(protobuf REQUIRED CONFIG)
find_package(zstd REQUIRED zstd::libzstd_static)
find_package(tabulate REQUIRED CONFIG)
find_package(ZLIB REQUIRED)
find_package(lz4 REQUIRED CONFIG)
find_package(tabulate REQUIRED CONFIG)

if (USE_MANIFEST_API)
    find_package(yyjson REQUIRED CONFIG)
endif ()

if (VEC_BUILD)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLIB REQUIRED glib-2.0)
    pkg_check_modules(UUID REQUIRED uuid)

    find_package(gRPC REQUIRED CONFIG)
    find_package(opentelemetry-cpp REQUIRED CONFIG)
    set(OPENTELEMETRY_LIBS
            opentelemetry-cpp::otlp_grpc_exporter
            opentelemetry-cpp::otlp_http_exporter
            opentelemetry-cpp::ostream_span_exporter
    )

    find_package(Arrow REQUIRED CONFIG)
    find_package(ArrowDataset REQUIRED CONFIG)
endif ()

find_package(orc REQUIRED CONFIG)
