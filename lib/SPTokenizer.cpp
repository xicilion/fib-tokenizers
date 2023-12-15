#include <string.h>
#include <locale>
#include "SPTokenizer.h"

napi_ref SPTokenizer::constructor;

inline std::string to_string(napi_env env, napi_value value)
{
    size_t sz = 0;
    NODE_API_CALL(env, napi_get_value_string_utf8(env, value, nullptr, 0, &sz));
    std::string str;
    str.resize(sz);
    char* data = &str[0];
    NODE_API_CALL(env, napi_get_value_string_utf8(env, value, data, sz + 1, nullptr));
    return str;
}

void SPTokenizer::Init(napi_env env)
{
    napi_property_descriptor properties[] = {
        { "encode", nullptr, encode, nullptr, nullptr, nullptr, napi_enumerable, nullptr },
        { "decode", nullptr, decode, nullptr, nullptr, nullptr, napi_enumerable, nullptr }
    };

    napi_value cons;
    NODE_API_CALL_RETURN_VOID(env, napi_define_class(env, "SPTokenizer", -1, New, nullptr, sizeof(properties) / sizeof(napi_property_descriptor), properties, &cons));
    NODE_API_CALL_RETURN_VOID(env, napi_create_reference(env, cons, 1, &constructor));
}

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

napi_value SPTokenizer::encode(napi_env env, napi_callback_info info)
{
    napi_value _this;
    size_t argc = 1;
    napi_value args[1];
    NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));

    NODE_API_ASSERT(env, argc == 1, "Wrong number of arguments");

    SPTokenizer* obj;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&obj)));

    std::string txt = to_string(env, args[0]);

    std::vector<int> ids;

    try {
        obj->sentence_piece_.Encode(txt, &ids).IgnoreError();
    } catch (std::exception& e) {
        napi_throw_error(env, NULL, e.what());
    }

    napi_value ret;
    NODE_API_CALL(env, napi_create_array(env, &ret));

    for (size_t i = 0; i < ids.size(); i++) {
        napi_value e;
        NODE_API_CALL(env, napi_create_int64(env, ids[i], &e));
        NODE_API_CALL(env, napi_set_element(env, ret, i, e));
    }

    return ret;
}

napi_value SPTokenizer::decode(napi_env env, napi_callback_info info)
{
    napi_value _this;
    size_t argc = 1;
    napi_value args[1];
    NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));

    NODE_API_ASSERT(env, argc == 1, "Wrong number of arguments");

    SPTokenizer* obj;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&obj)));

    napi_valuetype valuetype0;
    NODE_API_CALL(env, napi_typeof(env, args[0], &valuetype0));
    NODE_API_ASSERT(env, valuetype0 == napi_object, "Wrong argument type, must be object");

    uint32_t i, length;
    NODE_API_CALL(env, napi_get_array_length(env, args[0], &length));

    std::vector<int> ids;
    for (i = 0; i < length; i++) {
        napi_value e;
        NODE_API_CALL(env, napi_get_element(env, args[0], i, &e));
        int64_t value;
        NODE_API_CALL(env, napi_get_value_int64(env, e, &value));
        ids.push_back(value);
    }

    std::string txt;

    try {
        obj->sentence_piece_.Decode(ids, &txt).IgnoreError();
    } catch (std::exception& e) {
        napi_throw_error(env, NULL, e.what());
    }

    napi_value ret;
    NODE_API_CALL(env, napi_create_string_utf8(env, txt.c_str(), txt.size(), &ret));

    return ret;
}
