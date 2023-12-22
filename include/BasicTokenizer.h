#pragma once

#include "napi_value.h"
#include <node_api.h>
#include <memory>
#include "common.h"
#include "basic_tokenizer.hpp"

class JSBasicTokenizer {
public:
    JSBasicTokenizer()
        : env_(nullptr)
        , wrapper_(nullptr)
    {
    }

    ~JSBasicTokenizer()
    {
        napi_delete_reference(env_, wrapper_);
    }

public:
    static napi_value Init(napi_env env);

private:
    static napi_value New(napi_env env, napi_callback_info info)
    {
        napi_value new_target;
        NODE_API_CALL(env, napi_get_new_target(env, info, &new_target));
        NODE_API_ASSERT(env, new_target != nullptr, "Not a constructor call");

        napi_value _this;
        size_t argc = 1;
        napi_value args[1];
        NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));

        std::unique_ptr<JSBasicTokenizer> obj(new JSBasicTokenizer());

        NodeOpt opt(env, args[0]);
        obj->tokenizer_ = std::make_shared<BasicTokenizer>(
            opt.Get("do_lower_case", true), opt.Get("tokenize_chinese_chars", true),
            opt.Get("strip_accents", true), opt.Get("do_split_on_punc", true), true);

        obj->env_ = env;
        NODE_API_CALL(env, napi_wrap(env, _this, obj.get(), Destructor, nullptr, &obj->wrapper_));

        obj.release();
        return _this;
    }

    static void Destructor(napi_env env, void* nativeObject, void*)
    {
        JSBasicTokenizer* obj = static_cast<JSBasicTokenizer*>(nativeObject);
        delete obj;
    }

private:
    static napi_value tokenize(napi_env env, napi_callback_info info);

private:
    static napi_ref constructor;
    napi_env env_;
    napi_ref wrapper_;

private:
    std::shared_ptr<BasicTokenizer> tokenizer_;
};