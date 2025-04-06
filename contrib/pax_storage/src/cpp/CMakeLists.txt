cmake_minimum_required (VERSION 3.11.0)

# protobuf

set(protobuf_files
  ${CMAKE_CURRENT_SOURCE_DIR}/storage/proto/micro_partition_stats.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/storage/proto/pax.proto
  ${CMAKE_CURRENT_SOURCE_DIR}/storage/proto/orc_proto.proto
)

link_directories($ENV{GPHOME}/lib)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
## build pax_format.so
include(pax_format)
## build pax.so
include(pax)
