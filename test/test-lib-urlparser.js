var urlParser = require('bindings')('./../lib/build/Release/urlParser');

//console.log(urlParser.parse("http:\\\\evil-phisher\\foo.html#h\\a\\s\\h", true, false));
//console.log(urlParser.parse("http:\\\\evil-phisher\\foo.html", true, false));
//console.log(urlParser.parse("HTTP://www.example.com", true, false));

//console.log(urlParser.parse("//www.example.com", true, false));
console.log(urlParser.parse("//google.com?a=b", true, false));