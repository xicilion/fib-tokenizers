#pragma once

#include <node_api.h>
#include "common.h"
#include <string>
#include <vector>

class Tokenizer {
public:
    Tokenizer()
        : env_(nullptr)
        , wrapper_(nullptr)
    {
    }

    virtual ~Tokenizer()
    {
        napi_delete_reference(env_, wrapper_);
    }

public:
    virtual void encode(const std::string& txt, std::vector<int>& ids) = 0;
    virtual void decode(const std::vector<int>& ids, std::string& txt) = 0;

public:
    static void Init(napi_env env);
    static napi_value from(napi_env env, napi_callback_info info);
    napi_value wrap(napi_env env);

    static std::string to_string(napi_env env, napi_value value)
    {
        size_t sz = 0;
        NODE_API_CALL(env, napi_get_value_string_utf8(env, value, nullptr, 0, &sz));
        std::string str;
        str.resize(sz);
        char* data = &str[0];
        NODE_API_CALL(env, napi_get_value_string_utf8(env, value, data, sz + 1, nullptr));
        return str;
    }

private:
    static napi_value New(napi_env env, napi_callback_info info)
    {
        return napi_value();
    }

    static void Destructor(napi_env env, void* nativeObject, void*)
    {
        Tokenizer* obj = static_cast<Tokenizer*>(nativeObject);
        delete obj;
    }

    static napi_value encode(napi_env env, napi_callback_info info);
    static napi_value decode(napi_env env, napi_callback_info info);

private:
    static napi_ref constructor;
    napi_env env_;
    napi_ref wrapper_;
};
