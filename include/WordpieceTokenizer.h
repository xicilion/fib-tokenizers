#pragma once

#include "napi_value.h"
#include <node_api.h>
#include <memory>
#include "common.h"

class JSWordpieceTokenizer {
public:
    JSWordpieceTokenizer()
        : env_(nullptr)
        , wrapper_(nullptr)
    {
    }

    ~JSWordpieceTokenizer()
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
        size_t argc = 2;
        napi_value args[2];
        NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));

        std::unique_ptr<JSWordpieceTokenizer> obj(new JSWordpieceTokenizer());

        obj->vocab_array = NodeValue(env, args[0]);
        for (int i = 0; i < obj->vocab_array.size(); i++)
            obj->vocab_[obj->vocab_array[i]] = i;

        NodeOpt opt(env, args[1]);
        obj->unk_token_ = opt.Get("unk_token", std::u16string(u"UNK"));
        obj->max_input_chars_per_word_ = opt.Get("max_input_chars_per_word", obj->max_input_chars_per_word_);

        obj->env_ = env;
        NODE_API_CALL(env, napi_wrap(env, _this, obj.get(), Destructor, nullptr, &obj->wrapper_));

        obj.release();
        return _this;
    }

    static void Destructor(napi_env env, void* nativeObject, void*)
    {
        JSWordpieceTokenizer* obj = static_cast<JSWordpieceTokenizer*>(nativeObject);
        delete obj;
    }

private:
    static napi_value tokenize(napi_env env, napi_callback_info info);

private:
    static napi_ref constructor;
    napi_env env_;
    napi_ref wrapper_;

private:
    std::vector<std::u16string> vocab_array;
    std::unordered_map<std::u16string, int32_t> vocab_;
    std::u16string unk_token_;
    uint32_t max_input_chars_per_word_ = 100;
};