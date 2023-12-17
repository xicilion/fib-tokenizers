const test = require("test");
test.setup();

const path = require("path");
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
