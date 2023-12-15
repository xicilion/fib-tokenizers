const path = require('path');
var tokenizers = require(`./addon/${path.basename(__dirname)}.node`);

const fs = require("fs");

tokenizers.from_file = function (vocab, merges) {
    if (merges) {
        if (path.extname(vocab) !== '.json')
            throw new Error(`Unknown file extension: ${vocab}`);

        if (path.extname(merges) !== '.txt')
            throw new Error(`Unknown file extension: ${merges}`);

        return tokenizers.from(JSON.parse(fs.readFileSync(vocab, "utf8")),
            fs.readFileSync(merges, "utf8").split("\n"));
    }

    const extname = path.extname(vocab);

    if (extname === '.model')
        return tokenizers.from(fs.readFileSync(vocab));

    if (extname === '.json') {
        var tokenizer = JSON.parse(fs.readFileSync(vocab, "utf8"));
        return tokenizers.from(tokenizer.model.vocab, tokenizer.model.merges);
    }

    throw new Error(`Unknown file extension: ${vocab}`);
};

module.exports = tokenizers;