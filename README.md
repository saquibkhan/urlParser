#Url parser for Node.js - ***Work In Progress***

#Benchmark - implementation in progress, half done!
```
  saquib@ubuntu:/media/saquib/Ubuntu/gitHub/urlParser$ node benchmark/url.js 
  misc/url.js parse(): 3393.1
  misc/url.js format(): 3243.0
  misc/url.js resolve("../foo/bar?baz=boom"): 1710.6
  misc/url.js resolve("foo/bar"): 1721.6
  misc/url.js resolve("http://nodejs.org"): 1627.4
  misc/url.js resolve("./foo/bar?baz"): 1738.1
  
  saquib@ubuntu:/media/saquib/Ubuntu/gitHub/urlParser$ node benchmark/url.js 
  misc/url.js parse(): 10522
  misc/url.js format(): 11470
  misc/url.js resolve("../foo/bar?baz=boom"): 4286.9
  misc/url.js resolve("foo/bar"): 4931.8
  misc/url.js resolve("http://nodejs.org"): 4227.5
  misc/url.js resolve("./foo/bar?baz"): 4618.2
  saquib@ubuntu:/media/saquib/Ubuntu/gitHub/urlParser$ ^C
```

##Compiling and Running
1. cd lib
2. make configure
3. make build
4. make test

###References:
1. joyent/node [lib/url.js](https://github.com/joyent/node/blob/master/lib/url.js)
2. RFC [1738](https://tools.ietf.org/html/rfc1738), [3986](https://tools.ietf.org/html/rfc3986)
