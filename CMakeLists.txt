cmake_minimum_required(VERSION 2.6)

file(GLOB src_list
"${PROJECT_SOURCE_DIR}/lib/*.c*"
"${PROJECT_SOURCE_DIR}/huggingface-tokenizer-in-cxx/tokenizer/bpe.cc"
"${PROJECT_SOURCE_DIR}/huggingface-tokenizer-in-cxx/re2/re2/*.c*"
"${PROJECT_SOURCE_DIR}/huggingface-tokenizer-in-cxx/re2/util/*.c*"
)

include_directories(
    "${PROJECT_SOURCE_DIR}/huggingface-tokenizer-in-cxx/tokenizer/"
    "${PROJECT_SOURCE_DIR}/huggingface-tokenizer-in-cxx/re2"
)

include(fib-addon/common.cmake)
