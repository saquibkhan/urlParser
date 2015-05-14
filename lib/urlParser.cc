// urlParser.cc
#include <node.h>
#include <v8.h>

using namespace v8;

Handle<Value> parse(const Arguments& args) {
  HandleScope scope;
  return scope.Close(String::New("Hello world"));
}

void Init(Handle<Object> exports) {
  exports->Set(String::NewSymbol("parse"),
      FunctionTemplate::New(parse)->GetFunction());
}

NODE_MODULE(urlParser, Init)