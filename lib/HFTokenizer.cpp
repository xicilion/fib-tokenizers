#include <locale>
#include "HFTokenizer.h"

static RE2 re(
    "('s|'t|'re|'ve|'m|'ll|'d| ?\\p{L}+| ?\\p{N}+| "
    "?[^\\s\\p{L}\\p{N}]+|\\s+\\(?!\\S\\)|\\s+)");
static std::unordered_map<uint8_t, wchar_t> b2u;
static std::unordered_map<wchar_t, uint8_t> u2b;

class __init {
public:
    __init()
    {
        bytes_to_unicode(&b2u, &u2b);
    }
};
static __init s_init;

std::wstring utf8_to_wstring(const char* str, const char* str_end)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
    return myconv.from_bytes(str, str_end);
}

napi_value HFTokenizer::from(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2];
    NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NODE_API_ASSERT(env, argc == 2, "Wrong number of arguments");

    napi_valuetype valuetype0;
    NODE_API_CALL(env, napi_typeof(env, args[0], &valuetype0));
    NODE_API_ASSERT(env, valuetype0 == napi_object, "Wrong argument type, must be object");

    NODE_API_CALL(env, napi_typeof(env, args[1], &valuetype0));
    NODE_API_ASSERT(env, valuetype0 == napi_object, "Wrong argument type, must be object");

    napi_value keys;
    NODE_API_CALL(env, napi_get_property_names(env, args[0], &keys));

    uint32_t i, length;
    NODE_API_CALL(env, napi_get_array_length(env, keys, &length));

    HFTokenizer* tok = new HFTokenizer();
    try {
        for (i = 0; i < length; i++) {
            napi_value k, v;
            NODE_API_CALL(env, napi_get_element(env, keys, i, &k));
            NODE_API_CALL(env, napi_get_property(env, args[0], k, &v));

            int32_t value;
            NODE_API_CALL(env, napi_get_value_int32(env, v, &value));
            std::string key = to_string(env, k);

            tok->t2i.insert({ key, value });
            tok->i2t.insert({ value, key });
        }

        uint32_t merge_length;
        NODE_API_CALL(env, napi_get_array_length(env, args[1], &merge_length));

        for (int merge_count = 1; merge_count < merge_length; merge_count++) {
            napi_value e;
            NODE_API_CALL(env, napi_get_element(env, args[1], merge_count, &e));
            std::string line = to_string(env, e);
            if (line.length() == 0)
                continue;

            int d = line.find(" ");
            if (d == std::string::npos)
                throw std::runtime_error("Wrong format of merge file");

            tok->bpe_ranks.insert({ { utf8_to_wstring(line.substr(0, d)),
                                        utf8_to_wstring(line.substr(d + 1)) },
                merge_count - 1 });
        }

    } catch (...) {
        delete tok;
        throw;
    }

    return tok->wrap(env);
}

void HFTokenizer::encode(const std::string& txt, std::vector<int>& ids)
{
    ::encode(txt, re, bpe_ranks, b2u, t2i, &ids);
}

void HFTokenizer::decode(const std::vector<int>& ids, std::string& txt)
{
    txt = ::decode(ids, u2b, i2t);
}
