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