#include <string.h>
#include <locale>
#include "Tokenizer.h"
#include "SPTokenizer.h"

napi_ref Tokenizer::constructor;
static RE2 re(
    "('s|'t|'re|'ve|'m|'ll|'d| ?\\p{L}+| ?\\p{N}+| "
    "?[^\\s\\p{L}\\p{N}]+|\\s+\\(?!\\S\\)|\\s+)");
static std::unordered_map<uint8_t, wchar_t> b2u;
static std::unordered_map<wchar_t, uint8_t> u2b;

void Tokenizer::Init(napi_env env)
{
    napi_property_descriptor properties[] = {
        { "encode", nullptr, encode, nullptr, nullptr, nullptr, napi_enumerable, nullptr },
        { "decode", nullptr, decode, nullptr, nullptr, nullptr, napi_enumerable, nullptr }
    };

    napi_value cons;
    NODE_API_CALL_RETURN_VOID(env, napi_define_class(env, "Tokenizer", -1, New, nullptr, sizeof(properties) / sizeof(napi_property_descriptor), properties, &cons));
    NODE_API_CALL_RETURN_VOID(env, napi_create_reference(env, cons, 1, &constructor));

    bytes_to_unicode(&b2u, &u2b);
}

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

std::wstring utf8_to_wstring(const char* str, const char* str_end)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
    return myconv.from_bytes(str, str_end);
}

napi_value Tokenizer::from(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2];
    NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    if (argc == 1)
        return SPTokenizer::from(env, info);

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

    Tokenizer* tok = new Tokenizer();
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
        ::encode(txt, re, obj->bpe_ranks, b2u, obj->t2i, &ids);
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
        txt = ::decode(ids, u2b, obj->i2t);
    } catch (std::exception& e) {
        napi_throw_error(env, NULL, e.what());
    }

    napi_value ret;
    NODE_API_CALL(env, napi_create_string_utf8(env, txt.c_str(), txt.size(), &ret));

    return ret;
}
