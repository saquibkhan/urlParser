#Url parser for Node.js - *Development Stopped*

##Performance Analysis
**Performance is very slow with C++**. Issue is while setting the object at v8 layer which is taking max time.

![](https://raw.githubusercontent.com/saquibkhan/urlParser/master/assets/urlparser_perf1.gif)
![](https://raw.githubusercontent.com/saquibkhan/urlParser/master/assets/urlparser_perf2.gif)

##Compiling and Running
1. make configure
2. make build
3. make test

###References:
1. joyent/node [lib/url.js](https://github.com/joyent/node/blob/master/lib/url.js)
2. RFC [1738](https://tools.ietf.org/html/rfc1738), [3986](https://tools.ietf.org/html/rfc3986)

## License

MIT © [Saquib Khan](https://github.com/saquibkhan)
