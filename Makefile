build:src/urlParser.cc
	node-gyp build
rebuild:src/urlParser.cc
	node-gyp rebuild
configure:
	node-gyp configure
test:
	node ./test/test-url.js
.PHONY: rebuild configure test clean
clean:
	node-gyp clean
