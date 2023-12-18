const test = require("test");
test.setup();

const path = require("path");
const fs = require("fs");

const tokenizers = require("..");

describe("tokenizer", () => {
    it("gpt-2", () => {
        const tokenizer = tokenizers.from_file(
            path.join(__dirname, "models/vocab.json"),
            path.join(__dirname, "models/merges.txt")
        );

        const toks = tokenizer.encode("lower newer");
        assert.deepEqual(toks, [0, 1, 2, 15, 10, 9, 3, 2, 15]);
        assert.equal(tokenizer.decode(toks), "lower newer");
    });

    describe("tiktoken", () => {
        const models = {
            cl100k_base: tokenizers.from_file(path.join(__dirname, "models/cl100k_base.tiktoken"), "cl100k_base"),
            p50k_base: tokenizers.from_file(path.join(__dirname, "models/p50k_base.tiktoken"), "p50k_base"),
            r50k_base: tokenizers.from_file(path.join(__dirname, "models/r50k_base.tiktoken"), "r50k_base"),
        };

        function get_value(line) {
            const pos = line.indexOf(": ");
            return line.substr(pos + 2);
        }

        Object.keys(models).forEach((model) => {
            it(model, () => {
                const tokenizer = models[model];

                const lines = fs.readFileSync(path.join(__dirname, "models/TestPlans.txt"), "utf-8").split("\n");
                for (var i = 0; i < lines.length; i += 4) {
                    var line = lines[i];
                    if (line.length == 0)
                        break;
                    if (get_value(line) == model) {
                        const txt = get_value(lines[i + 1]);
                        const tok = models[model].encode(txt);
                        assert.deepEqual(tok, JSON.parse(get_value(lines[i + 2])));
                        assert.equal(models[model].decode(tok), txt);
                    }
                }
            });
        });
    });

    describe("sentencepiece", () => {
        const sentencepiece_cases = {
            CodeLlamaTokenizer: [
                [1, 285, 46, 10, 170, 382],
                [1, 8, 21, 84, 55, 24, 19, 7, 0, 602, 347, 347, 347, 3, 12, 66, 46, 72, 80, 6, 0, 4]
            ],
            LlamaTokenizer: [
                [1, 285, 46, 10, 170, 382],
                [1, 8, 21, 84, 55, 24, 19, 7, 0, 602, 347, 347, 347, 3, 12, 66, 46, 72, 80, 6, 0, 4]
            ],
            XLMRobertaTokenizer: [
                [0, 286, 47, 11, 171, 383, 2],
                [0, 9, 22, 85, 56, 25, 20, 8, 3, 603, 348, 348, 348, 4, 13, 67, 47, 73, 81, 7, 3, 5, 2]
            ],
        };

        function test_tokenizer(type, _toks) {
            it(type, () => {
                const tokenizer = tokenizers.from_file(
                    path.join(__dirname, "models/test_sentencepiece.model"),
                    { type: type }
                );

                const toks = tokenizer.encode("This is a test");
                assert.deepEqual(toks, _toks[0]);
                assert.equal(tokenizer.decode(toks), "This is a test");

                var toks1 = tokenizer.encode('I was born in 92000, and this is falsé.');
                assert.deepEqual(toks1, _toks[1]);
                assert.equal(tokenizer.decode(toks1), 'I was born in  ⁇ 2000, and this is fals ⁇ .');
            });
        }

        for (var type in sentencepiece_cases)
            test_tokenizer(type, sentencepiece_cases[type]);
    });
});

test.run();
