#include "Tokenizer.h"
#include "HFTokenizer.h"
#include "SPTokenizer.h"

napi_ref Tokenizer::constructor;

void Tokenizer::Init(napi_env env)
{
    napi_property_descriptor properties[] = {
        { "encode", nullptr, encode, nullptr, nullptr, nullptr, napi_enumerable, nullptr },
        { "decode", nullptr, decode, nullptr, nullptr, nullptr, napi_enumerable, nullptr }
    };

    napi_value cons;
    NODE_API_CALL_RETURN_VOID(env, napi_define_class(env, "Tokenizer", -1, New, nullptr, sizeof(properties) / sizeof(napi_property_descriptor), properties, &cons));
    NODE_API_CALL_RETURN_VOID(env, napi_create_reference(env, cons, 1, &constructor));
}

napi_value Tokenizer::from(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1];
    NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NODE_API_ASSERT(env, argc == 1, "Wrong number of arguments");

    bool is_typedarray;
    NODE_API_CALL(env, napi_is_typedarray(env, args[0], &is_typedarray));

    return is_typedarray ? SPTokenizer::from(env, info) : HFTokenizer::from(env, info);
}

napi_value Tokenizer::encode(napi_env env, napi_callback_info info)
{
    napi_value _this;
    size_t argc = 1;
    napi_value args[1];
    NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));

    NODE_API_ASSERT(env, argc == 1, "Wrong number of arguments");

    Tokenizer* obj;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&obj)));

    std::string txt = to_string(env, args[0]);
    std::vector<int> ids;
    try {
        obj->encode(txt, ids);
    } catch (std::exception& e) {
        napi_throw_error(env, NULL, e.what());
    }

    napi_value output_buffer;
    float* output_ptr = NULL;
    NODE_API_CALL(env, napi_create_arraybuffer(env, ids.size() * sizeof(float), (void**)&output_ptr, &output_buffer));

    napi_value ret;
    NODE_API_CALL(env, napi_create_typedarray(env, napi_float32_array, ids.size(), output_buffer, 0, &ret));

    for (size_t i = 0; i < ids.size(); i++)
        output_ptr[i] = ids[i];

    return ret;
}

napi_value Tokenizer::wrap(napi_env env)
{
    env_ = env;

    napi_value cons;
    NODE_API_CALL(env, napi_get_reference_value(env, constructor, &cons));

    napi_value _this;
    NODE_API_CALL(env, napi_new_instance(env, cons, 0, nullptr, &_this));

    NODE_API_CALL(env, napi_wrap(env, _this, this, Destructor, nullptr, &wrapper_));

    return _this;
}

napi_value Tokenizer::decode(napi_env env, napi_callback_info info)
{
    napi_value _this;
    size_t argc = 1;
    napi_value args[1];
    NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));

    NODE_API_ASSERT(env, argc == 1, "Wrong number of arguments");

    Tokenizer* obj;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&obj)));

    napi_valuetype valuetype0;
    NODE_API_CALL(env, napi_typeof(env, args[0], &valuetype0));
    NODE_API_ASSERT(env, valuetype0 == napi_object, "Wrong argument type, must be object");

    bool is_typedarray;
    NODE_API_CALL(env, napi_is_typedarray(env, args[0], &is_typedarray));

    std::vector<int> ids;
    uint32_t i, length;
    if (is_typedarray) {
        napi_typedarray_type type;
        napi_value input_buffer;
        size_t byte_offset;
        size_t item_length;
        NODE_API_CALL(env, napi_get_typedarray_info(env, args[0], &type, &item_length, NULL, &input_buffer, &byte_offset));
        length = item_length;
    } else {
        NODE_API_CALL(env, napi_get_array_length(env, args[0], &length));
    }

    for (i = 0; i < length; i++) {
        napi_value e;
        NODE_API_CALL(env, napi_get_element(env, args[0], i, &e));
        int64_t value;
        NODE_API_CALL(env, napi_get_value_int64(env, e, &value));
        ids.push_back(value);
    }

    std::string txt;
    try {
        obj->decode(ids, txt);
    } catch (std::exception& e) {
        napi_throw_error(env, NULL, e.what());
    }

    napi_value ret;
    NODE_API_CALL(env, napi_create_string_utf8(env, txt.c_str(), txt.size(), &ret));

    return ret;
}
