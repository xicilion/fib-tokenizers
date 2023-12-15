const path = require('path');
var tokenizers = require(`./addon/${path.basename(__dirname)}.node`);

const fs = require("fs");

tokenizers.from_file = function (vocab, merges) {
    if (merges === undefined) {
        var tokenizer = JSON.parse(fs.readFileSync(vocab, "utf8"));
        return tokenizers.from(tokenizer.model.vocab, tokenizer.model.merges);
    } else
        return tokenizers.from(JSON.parse(fs.readFileSync(vocab, "utf8")),
            fs.readFileSync(merges, "utf8").split("\n"));
};

module.exports = tokenizers;