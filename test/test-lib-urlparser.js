//var url = require('./../lib/url');
//var url = require('url')
var url = require("fast-url-parser")

start = process.hrtime();
url.parse('//google.com?a=b');
end = process.hrtime(start);
console.log(end[0] + end[1] / 1e9);

//console.log(urlParser.parse("http:\\\\evil-phisher\\foo.html#h\\a\\s\\h", true, false));
//console.log(urlParser.parse("http:\\\\evil-phisher\\foo.html", true, false));
//console.log(urlParser.parse("HTTP://www.example.com", true, false));

//console.log(urlParser.parse("//www.example.com", true, false));

//console.log(urlParser.parse('//google.com?a=b', true, false));
//console.log(urlParser.parse('http://google.com?a=b', true, false));
//console.log(urlParser.parse('http://ليهمابتكلموشعربي؟.ي؟/', true, false));
//console.log(urlParser.parse('//ليهمابتكلموشعربي؟.ي؟?a=b', true, false));