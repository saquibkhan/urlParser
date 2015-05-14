var urlParser = require('bindings')('./../lib/build/Release/urlParser');

console.log(urlParser.parse("http:\\\\evil-phisher\\foo.html#h\\a\\s\\h", true, false));