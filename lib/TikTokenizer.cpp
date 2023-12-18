#include "TikTokenizer.h"
#include "emdedded_resource_reader.h"

class LinesReader : public IResourceReader {
public:
    LinesReader(std::vector<std::string>& lines_)
        : lines(lines_)
    {
    }

    std::vector<std::string> readLines() override
    {
        return lines;
    }

private:
    std::vector<std::string>& lines;
};

napi_value TikTokenizer::from(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2];
    NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NODE_API_ASSERT(env, argc == 1 || argc == 2, "Wrong number of arguments");

    bool is_array;
    NODE_API_CALL(env, napi_is_array(env, args[0], &is_array));
    NODE_API_ASSERT(env, is_array, "Wrong argument type, must be array");

    LanguageModel base_model = LanguageModel::CL100K_BASE;
    if (argc == 2) {
        napi_valuetype valuetype1;
        NODE_API_CALL(env, napi_typeof(env, args[1], &valuetype1));
        if (valuetype1 != napi_undefined) {
            NODE_API_ASSERT(env, valuetype1 == napi_string, "Wrong argument type, must be string");
            std::string str_model = to_string(env, args[1]);
            if (str_model == "cl100k_base")
                base_model = LanguageModel::CL100K_BASE;
            else if (str_model == "r50k_base")
                base_model = LanguageModel::R50K_BASE;
            else if (str_model == "p50k_base")
                base_model = LanguageModel::P50K_BASE;
            else if (str_model == "p50k_edit")
                base_model = LanguageModel::P50K_EDIT;
            else
                NODE_API_ASSERT(env, false, "Unknown base model");
        }
    }

    uint32_t i, length;
    NODE_API_CALL(env, napi_get_array_length(env, args[0], &length));

    std::vector<std::string> lines;
    lines.reserve(length);

    for (i = 0; i < length; i++) {
        napi_value e;
        NODE_API_CALL(env, napi_get_element(env, args[0], i, &e));
        lines.push_back(to_string(env, e));
    }

    LinesReader lines_reader(lines);
    TikTokenizer* tok = new TikTokenizer();

    try {
        tok->encoder_ = GptEncoding::get_encoding(base_model, &lines_reader);
    } catch (...) {
        delete tok;
        throw;
    }

    return tok->wrap(env);
}

void TikTokenizer::encode(const std::string& txt, std::vector<int>& ids)
{
    ids = encoder_->encode(txt);
}

void TikTokenizer::decode(const std::vector<int>& ids, std::string& txt)
{
    txt = encoder_->decode(ids);
}
