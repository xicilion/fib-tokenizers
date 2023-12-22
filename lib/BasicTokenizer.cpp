#include "BasicTokenizer.h"

napi_ref JSBasicTokenizer::constructor;

napi_value JSBasicTokenizer::Init(napi_env env)
{
    napi_property_descriptor properties[] = {
        { "tokenize", nullptr, tokenize, nullptr, nullptr, nullptr, napi_enumerable, nullptr }
    };

    napi_value cons;
    NODE_API_CALL(env, napi_define_class(env, "BasicTokenizer", -1, New, nullptr, sizeof(properties) / sizeof(napi_property_descriptor), properties, &cons));
    NODE_API_CALL(env, napi_create_reference(env, cons, 1, &constructor));

    return cons;
}

napi_value JSBasicTokenizer::tokenize(napi_env env, napi_callback_info info)
{
    napi_value _this;
    size_t argc = 1;
    napi_value args[1];
    NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));

    JSBasicTokenizer* obj;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&obj)));

    ustring text = NodeValue(env, args[0]);
    std::vector<ustring> tokens = obj->tokenizer_->Tokenize(text);

    return NodeValue(env, tokens);
}