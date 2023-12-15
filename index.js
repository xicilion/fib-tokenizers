const path = require('path');
var tokenizers = require(`./addon/${path.basename(__dirname)}.node`);

tokenizers.from_file = function () {

};

module.exports = tokenizers;