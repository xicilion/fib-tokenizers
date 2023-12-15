#include <third_party/absl/flags/flag.h>
#include "Tokenizer.h"
#include "SPTokenizer.h"

ABSL_FLAG(std::string, test_tmpdir, "test_tmp", "Temporary directory.");

NAPI_MODULE_INIT()
{
    Tokenizer::Init(env);
    SPTokenizer::Init(env);

    const napi_property_descriptor exports_set = {
        "from", nullptr, Tokenizer::from, nullptr, nullptr, nullptr, napi_enumerable, nullptr
    };

    NODE_API_CALL(env, napi_define_properties(env, exports, 1, &exports_set));
    return exports;
}
