const path = require('path');
var tokenizers = require(`./addon/${path.basename(__dirname)}.node`);

const fs = require("fs");

const added_tokens = [
    "<s>",
    "<pad>",
    "</s>",
    "<unk>"
];

const LlamaTokenizer = { extra_options: "bos" };
const RobertaTokenizer = {
    extra_options: "bos:eos",
    offset: 1,
    added_tokens
};

const def_opts = {
    CodeLlamaTokenizer: LlamaTokenizer,
    LlamaTokenizer: LlamaTokenizer,
    XLMRobertaTokenizer: RobertaTokenizer,
    PreTrainedTokenizer: {}
};

tokenizers.from_file = function (vocab, merges) {
    const extname = path.extname(vocab);
    if (extname === '.model') {
        if (merges) {
            const opt = {
                type: merges.type || "PreTrainedTokenizer"
            };

            const def_opt = def_opts[opt.type] || {};

            opt.extra_options = merges.extra_options === undefined ? def_opt.extra_options : merges.extra_options;
            opt.offset = merges.offset === undefined ? def_opt.offset : merges.offset;
            opt.added_tokens = merges.added_tokens === undefined ? def_opt.added_tokens : merges.added_tokens;

            merges = opt;
        }

        return tokenizers.from("sentencepiece", fs.readFileSync(vocab), merges);
    }

    if (extname === '.tiktoken')
        return tokenizers.from("tiktoken", fs.readFileSync(vocab).toString().split('\n'), merges);

    if (path.extname(vocab) !== '.json')
        throw new Error(`Unknown file extension: ${vocab}`);

    vocab = JSON.parse(fs.readFileSync(vocab, "utf8"));
    if (merges) {
        if (path.extname(merges) !== '.txt')
            throw new Error(`Unknown file extension: ${merges}`);

        return tokenizers.from("bpe", vocab, fs.readFileSync(merges, "utf8").split("\n"));
    }

    return tokenizers.from("bpe", vocab.model.vocab, vocab.model.merges);
};

tokenizers.from_model = function (folder) {
    const vocab = path.join(folder, "vocab.json");
    const merges = path.join(folder, "merges.txt");

    return tokenizers.from_file(vocab, merges);

};

module.exports = tokenizers;
