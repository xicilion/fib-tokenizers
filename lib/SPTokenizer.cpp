#include "SPTokenizer.h"
#include "sentencepiece.pb.h"
#include <third_party/absl/flags/flag.h>

ABSL_FLAG(std::string, test_tmpdir, "test_tmp", "Temporary directory.");

napi_value SPTokenizer::from(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2];
    NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NODE_API_ASSERT(env, argc == 1 || argc == 2, "Wrong number of arguments");

    bool is_typedarray;
    NODE_API_CALL(env, napi_is_typedarray(env, args[0], &is_typedarray));
    NODE_API_ASSERT(env, is_typedarray, "Wrong argument type, must be typedarray");

    napi_typedarray_type type;
    napi_value input_buffer;
    size_t byte_offset;
    size_t i, length;
    NODE_API_CALL(env, napi_get_typedarray_info(env, args[0], &type, &length, NULL, &input_buffer, &byte_offset));
    NODE_API_ASSERT(env, type == napi_uint8_array, "Wrong argument type, must be Uint8Array");

    void* data;
    size_t byte_length;
    NODE_API_CALL(env, napi_get_arraybuffer_info(env, input_buffer, &data, &byte_length));

    if (argc == 2) {
        napi_valuetype valuetype1;
        NODE_API_CALL(env, napi_typeof(env, args[1], &valuetype1));
        if (valuetype1 == napi_undefined)
            argc = 1;
        else {
            NODE_API_ASSERT(env, valuetype1 == napi_object, "Wrong argument type, must be object");
        }
    }

    SPTokenizer* tok = new SPTokenizer();
    auto status = tok->sentence_piece_.LoadFromSerializedProto(std::string_view((char*)data + byte_offset, byte_length - byte_offset));
    if (!status.ok()) {
        delete tok;
        napi_throw_error(env, NULL, "Failed to load model");
    }

    if (argc == 2) {
        try {
            napi_value k, v;
            napi_valuetype valuetype2;

            NODE_API_CALL(env, napi_create_string_utf8(env, "offset", NAPI_AUTO_LENGTH, &k));
            NODE_API_CALL(env, napi_get_property(env, args[1], k, &v));

            NODE_API_CALL(env, napi_typeof(env, v, &valuetype2));
            if (valuetype2 != napi_undefined) {
                NODE_API_ASSERT(env, valuetype2 == napi_number, "Wrong argument type, `offset` must be number");
                NODE_API_CALL(env, napi_get_value_int32(env, v, &tok->offset));
            }

            NODE_API_CALL(env, napi_create_string_utf8(env, "extra_options", NAPI_AUTO_LENGTH, &k));
            NODE_API_CALL(env, napi_get_property(env, args[1], k, &v));

            NODE_API_CALL(env, napi_typeof(env, v, &valuetype2));
            if (valuetype2 != napi_undefined) {
                NODE_API_ASSERT(env, valuetype2 == napi_string, "Wrong argument type, `extra_options` must be string");
                tok->sentence_piece_.SetEncodeExtraOptions(to_string(env, v)).IgnoreError();
            }

            NODE_API_CALL(env, napi_create_string_utf8(env, "added_tokens", NAPI_AUTO_LENGTH, &k));
            NODE_API_CALL(env, napi_get_property(env, args[1], k, &v));

            NODE_API_CALL(env, napi_typeof(env, v, &valuetype2));
            if (valuetype2 != napi_undefined) {
                uint32_t tokens_length;
                NODE_API_ASSERT(env, valuetype2 == napi_object, "Wrong argument type, `added_tokens` must be object");
                NODE_API_CALL(env, napi_get_array_length(env, v, &tokens_length));

                for (uint32_t i = 0; i < tokens_length; i++) {
                    napi_value e;
                    std::string token;

                    NODE_API_CALL(env, napi_get_element(env, v, i, &e));
                    NODE_API_CALL(env, napi_typeof(env, e, &valuetype2));
                    NODE_API_ASSERT(env, valuetype2 == napi_string, "Wrong argument type, `extra_options[]` must be string");

                    tok->added_tokens.push_back(to_string(env, e));
                }
            }
        } catch (...) {
            delete tok;
            throw;
        }
    }

    return tok->wrap(env);
}

void SPTokenizer::encode(const std::string& txt, std::vector<int>& ids)
{
    sentencepiece::SentencePieceText spt;
    sentence_piece_.Encode(txt, &spt);

    ids.resize(spt.pieces_size());
    for (int i = 0; i < spt.pieces_size(); i++) {
        int j;
        auto piece = spt.pieces(i);
        std::string txt = piece.piece();

        for (j = 0; j < added_tokens.size(); j++) {
            if (txt == added_tokens[j]) {
                ids[i] = j;
                break;
            }
        }

        if (j == added_tokens.size())
            ids[i] = piece.id() + offset;
    }
}

void SPTokenizer::decode(const std::vector<int>& ids, std::string& txt)
{
    std::vector<std::string> pieces;
    const int num_pieces = sentence_piece_.GetPieceSize();
    pieces.reserve(ids.size());

    for (const int id : ids) {
        if (id < 0 || id >= num_pieces + offset)
            pieces.emplace_back("");
        else if (id < added_tokens.size())
            pieces.emplace_back(added_tokens[id]);
        else
            pieces.emplace_back(sentence_piece_.IdToPiece(id - offset));
    }

    sentence_piece_.Decode(pieces, &txt);
}
