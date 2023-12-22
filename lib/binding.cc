#include "Tokenizer.h"
#include "WordpieceTokenizer.h"

NAPI_MODULE_INIT()
{
    Tokenizer::Init(env);

    const napi_property_descriptor exports_set[] = {
        { "WordpieceTokenizer", nullptr, nullptr, nullptr, nullptr, JSWordpieceTokenizer::Init(env), napi_enumerable, nullptr },
        { "from", nullptr, Tokenizer::from, nullptr, nullptr, nullptr, napi_enumerable, nullptr }
    };

    NODE_API_CALL(env, napi_define_properties(env, exports, sizeof(exports_set) / sizeof(napi_property_descriptor), exports_set));
    return exports;
}
