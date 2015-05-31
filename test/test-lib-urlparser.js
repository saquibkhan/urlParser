var urlParser = require('bindings')('urlParser');

//console.log(urlParser.parse("http:\\\\evil-phisher\\foo.html#h\\a\\s\\h", true, false));
//console.log(urlParser.parse("http:\\\\evil-phisher\\foo.html", true, false));
//console.log(urlParser.parse("HTTP://www.example.com", true, false));

//console.log(urlParser.parse("//www.example.com", true, false));
console.log(urlParser.parse('//google.com?a=b', true, false));
console.log(urlParser.parse('http://google.com?a=b', true, false));
console.log(urlParser.parse('http://ليهمابتكلموشعربي؟.ي؟/', true, false));
console.log(urlParser.parse('//ليهمابتكلموشعربي؟.ي؟?a=b', true, false));