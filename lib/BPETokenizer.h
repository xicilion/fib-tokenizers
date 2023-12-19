#pragma once

#include "Tokenizer.h"
#include <unordered_map>
#include "bpe.h"

class BPETokenizer : public Tokenizer {
public:
    static napi_value from(napi_env env, napi_callback_info info);

public:
    virtual void encode(const std::string& txt, std::vector<int>& ids);
    virtual void decode(const std::vector<int>& ids, std::string& txt);

private:
    std::unordered_map<std::string, int> t2i;
    std::unordered_map<int, std::string> i2t;
    BPERanks bpe_ranks;
};
