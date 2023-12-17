#pragma once

#include "Tokenizer.h"
#include "sentencepiece_processor.h"

class SPTokenizer : public Tokenizer {
public:
    static napi_value from(napi_env env, napi_callback_info info);

public:
    virtual void encode(const std::string& txt, std::vector<int>& ids);
    virtual void decode(const std::vector<int>& ids, std::string& txt);

private:
    sentencepiece::SentencePieceProcessor sentence_piece_;

private:
    std::vector<std::string> added_tokens;
    int32_t offset = 0;
    uint32_t unk_id = 0;
};
