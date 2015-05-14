var urlParser = require('bindings')('./../lib/build/Release/urlParser');

console.log(urlParser.parse("url", "queryString", "host"));