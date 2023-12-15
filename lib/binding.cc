#include <string.h>
#include <locale>
#include "HFTokenizer.h"

napi_ref HFTokenizer::constructor;
static RE2 re(
    "('s|'t|'re|'ve|'m|'ll|'d| ?\\p{L}+| ?\\p{N}+| "
    "?[^\\s\\p{L}\\p{N}]+|\\s+\\(?!\\S\\)|\\s+)");
static std::unordered_map<uint8_t, wchar_t> b2u;
static std::unordered_map<wchar_t, uint8_t> u2b;

void HFTokenizer::Init(napi_env env)
{
    napi_property_descriptor properties[] = {
        { "json", nullptr, json, nullptr, nullptr, nullptr, napi_enumerable, nullptr },
        { "encode", nullptr, encode, nullptr, nullptr, nullptr, napi_enumerable, nullptr },
        { "decode", nullptr, decode, nullptr, nullptr, nullptr, napi_enumerable, nullptr }
    };

    napi_value cons;
    NODE_API_CALL_RETURN_VOID(env, napi_define_class(env, "HFTokenizer", -1, New, nullptr, sizeof(properties) / sizeof(napi_property_descriptor), properties, &cons));
    NODE_API_CALL_RETURN_VOID(env, napi_create_reference(env, cons, 1, &constructor));

    bytes_to_unicode(&b2u, &u2b);
}

std::string to_string(napi_env env, napi_value value)
{
    size_t sz;
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

napi_value HFTokenizer::from(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2];
    NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NODE_API_ASSERT(env, argc == 2, "Wrong number of arguments");

    napi_valuetype valuetype0;
    NODE_API_CALL(env, napi_typeof(env, args[0], &valuetype0));
    NODE_API_ASSERT(env, valuetype0 == napi_object, "Wrong argument type, must be object");

    bool is_typedarray;
    NODE_API_CALL(env, napi_is_typedarray(env, args[1], &is_typedarray));
    NODE_API_ASSERT(env, is_typedarray, "Wrong type of arguments. Expects a typed array as first argument.");

    napi_typedarray_type type;
    napi_value input_buffer;
    size_t byte_offset;
    size_t merge_length;
    NODE_API_CALL(env, napi_get_typedarray_info(env, args[1], &type, &merge_length, NULL, &input_buffer, &byte_offset));
    NODE_API_ASSERT(env, type == napi_uint8_array, "Wrong type of arguments. Expects a typed array as first argument.");

    char* merge_data;
    NODE_API_CALL(env, napi_get_arraybuffer_info(env, input_buffer, (void**)&merge_data, &merge_length));

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

            int64_t value;
            NODE_API_CALL(env, napi_get_value_int64(env, v, &value));
            std::string key = to_string(env, k);

            tok->t2i.insert({ key, value });
            tok->i2t.insert({ value, key });
        }

        int nmerge_count = 0;
        while (merge_length) {
            char* t1 = merge_data;
            char* spc = (char*)memchr(merge_data, ' ', merge_length);
            if (!spc)
                break;
            merge_length -= (spc - merge_data) + 1;
            merge_data = spc + 1;

            char* t2 = merge_data;
            char* endl = (char*)memchr(merge_data, '\n', merge_length);
            if (!endl) {
                endl = merge_data + merge_length;
                merge_length = 0;
            } else {
                merge_length -= (endl - merge_data) + 1;
                merge_data = endl + 1;
            }

            if (nmerge_count > 0) {
                tok->bpe_ranks.insert({ { utf8_to_wstring(t1, spc),
                                            utf8_to_wstring(t2, endl) },
                    nmerge_count - 1 });
            }
            nmerge_count++;
        }
    } catch (...) {
        delete tok;
        throw;
    }

    return tok->wrap(env);
}

napi_value HFTokenizer::json(napi_env env, napi_callback_info info)
{
    napi_value _this;
    NODE_API_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, nullptr));

    HFTokenizer* obj;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&obj)));

    napi_value object;
    NODE_API_CALL(env, napi_create_object(env, &object));

    for (auto& e : obj->t2i) {
        napi_value key, value;
        NODE_API_CALL(env, napi_create_string_utf8(env, e.first.c_str(), e.first.size(), &key));
        NODE_API_CALL(env, napi_create_int64(env, e.second, &value));
        NODE_API_CALL(env, napi_set_property(env, object, key, value));
    }

    return object;
}

napi_value HFTokenizer::encode(napi_env env, napi_callback_info info)
{
    napi_value _this;
    size_t argc = 1;
    napi_value args[1];
    NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));

    NODE_API_ASSERT(env, argc == 1, "Wrong number of arguments");

    HFTokenizer* obj;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&obj)));

    std::string txt = to_string(env, args[0]);

    std::vector<int> ids;

    try {
        ::encode(txt, re, obj->bpe_ranks, b2u, obj->t2i, &ids);
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

napi_value HFTokenizer::decode(napi_env env, napi_callback_info info)
{
    napi_value _this;
    size_t argc = 1;
    napi_value args[1];
    NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));

    NODE_API_ASSERT(env, argc == 1, "Wrong number of arguments");

    HFTokenizer* obj;
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
        txt = ::decode(ids, u2b, obj->i2t);
    } catch (std::exception& e) {
        napi_throw_error(env, NULL, e.what());
    }

    napi_value ret;
    NODE_API_CALL(env, napi_create_string_utf8(env, txt.c_str(), txt.size(), &ret));

    return ret;
}

NAPI_MODULE_INIT()
{
    HFTokenizer::Init(env);

    const napi_property_descriptor exports_set = {
        "from", nullptr, HFTokenizer::from, nullptr, nullptr, nullptr, napi_enumerable, nullptr
    };

    NODE_API_CALL(env, napi_define_properties(env, exports, 1, &exports_set));
    return exports;
}
