#pragma once

#include <node_api.h>
#include <string>
#include <unordered_map>
#include "common.h"
#include "bpe.h"

class HFTokenizer {
public:
    HFTokenizer()
        : env_(nullptr)
        , wrapper_(nullptr)
    {
    }

    ~HFTokenizer()
    {
        napi_delete_reference(env_, wrapper_);
    }

public:
    napi_value wrap(napi_env env)
    {
        env_ = env;

        napi_value cons;
        NODE_API_CALL(env, napi_get_reference_value(env, constructor, &cons));

        napi_value _this;
        NODE_API_CALL(env, napi_new_instance(env, cons, 0, nullptr, &_this));

        NODE_API_CALL(env, napi_wrap(env, _this, this, Destructor, nullptr, &wrapper_));

        return _this;
    }

public:
    static void Init(napi_env env);
    static napi_value from(napi_env env, napi_callback_info info);

    static void Destructor(napi_env env, void* nativeObject, void*)
    {
        HFTokenizer* obj = static_cast<HFTokenizer*>(nativeObject);
        delete obj;
    }

private:
    static napi_value New(napi_env env, napi_callback_info info)
    {
        return napi_value();
    }

    static napi_value json(napi_env env, napi_callback_info info);
    static napi_value encode(napi_env env, napi_callback_info info);
    static napi_value decode(napi_env env, napi_callback_info info);

    static napi_ref constructor;
    napi_env env_;
    napi_ref wrapper_;

public:
    std::unordered_map<std::string, int> t2i;
    std::unordered_map<int, std::string> i2t;
    BPERanks bpe_ranks;
};
