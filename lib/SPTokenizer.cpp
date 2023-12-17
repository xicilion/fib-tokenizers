#include "SPTokenizer.h"
#include <third_party/absl/flags/flag.h>

ABSL_FLAG(std::string, test_tmpdir, "test_tmp", "Temporary directory.");

napi_value SPTokenizer::from(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1];
    NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NODE_API_ASSERT(env, argc == 1, "Wrong number of arguments");

    bool is_typedarray;
    NODE_API_CALL(env, napi_is_typedarray(env, args[0], &is_typedarray));
    NODE_API_ASSERT(env, is_typedarray, "Wrong argument type, must be typedarray");

    napi_typedarray_type type;
    napi_value input_buffer;
    size_t byte_offset;
    size_t i, length;
    NODE_API_CALL(env, napi_get_typedarray_info(env, args[0], &type, &length, NULL, &input_buffer, &byte_offset));
    NODE_API_ASSERT(env, type == napi_uint8_array, "Wrong argument type, must be Uint8Array");

    void* data;
    size_t byte_length;
    NODE_API_CALL(env, napi_get_arraybuffer_info(env, input_buffer, &data, &byte_length));

    SPTokenizer* tok = new SPTokenizer();

    try {
        tok->sentence_piece_.LoadFromSerializedProto(std::string_view((char*)data + byte_offset, byte_length - byte_offset));
    } catch (std::exception& e) {
        napi_throw_error(env, NULL, e.what());
    }

    return tok->wrap(env);
}

void SPTokenizer::encode(const std::string& txt, std::vector<int>& ids)
{
    sentence_piece_.Encode(txt, &ids).IgnoreError();
}

void SPTokenizer::decode(const std::vector<int>& ids, std::string& txt)
{
    sentence_piece_.Decode(ids, &txt).IgnoreError();
}
