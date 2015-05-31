build:urlParser.cc
	node-gyp build
rebuild:urlParser.cc
	node-gyp rebuild
configure:
	node-gyp configure
test:
	node ./../test/test-lib-urlparser.js
.PHONY: clean
clean:
	node-gyp clean