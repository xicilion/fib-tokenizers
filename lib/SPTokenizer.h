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
};
