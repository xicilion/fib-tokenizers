cmake_minimum_required(VERSION 2.6)

set(CMAKE_CXX_STANDARD 17)

file(GLOB src_list
    "${PROJECT_SOURCE_DIR}/lib/*.c*"
    "${PROJECT_SOURCE_DIR}/huggingface-tokenizer-in-cxx/re2/re2/*.c*"
    "${PROJECT_SOURCE_DIR}/huggingface-tokenizer-in-cxx/re2/util/rune.cc"
    "${PROJECT_SOURCE_DIR}/huggingface-tokenizer-in-cxx/re2/util/strutil.cc"

    "${PROJECT_SOURCE_DIR}/sentencepiece/src/*.c*"
    "${PROJECT_SOURCE_DIR}/sentencepiece/src/builtin_pb/*.c*"

    "${PROJECT_SOURCE_DIR}/sentencepiece/third_party/protobuf-lite/*.c*"

    "${PROJECT_SOURCE_DIR}/sentencepiece/third_party/absl/flags/flag.cc"
)

list(FILTER src_list EXCLUDE REGEX ".*\_main.cc$")
list(FILTER src_list EXCLUDE REGEX ".*\_test.cc$")

include_directories(
    "${PROJECT_SOURCE_DIR}/huggingface-tokenizer-in-cxx/tokenizer"
    "${PROJECT_SOURCE_DIR}/huggingface-tokenizer-in-cxx/re2"
    "${PROJECT_SOURCE_DIR}/sentencepiece/src"
    "${PROJECT_SOURCE_DIR}/sentencepiece/src/builtin_pb"
    "${PROJECT_SOURCE_DIR}/sentencepiece/third_party/protobuf-lite"
    "${PROJECT_SOURCE_DIR}/sentencepiece"
)

include(fib-addon/common.cmake)

add_definitions(
    -DHAVE_PTHREAD=1
    -DPACKAGE_STRING
    -DVERSION
)
