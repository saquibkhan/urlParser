// urlParser.cc
#include <node.h>
#include <v8.h>
#include <iostream>

using namespace v8;
using namespace std;

#define FUCTION_NAME "parse"
#define FUCTION_PARAM 3

#define ERR_INVALID_ARGS  "Wrong arguments"
#define ERR_INVALID_NUM_ARGS  "Wrong number of arguments"


Handle<Value> parse(const Arguments& args) {
  HandleScope scope;

  Local<Object> obj = Object::New();

  if (args.Length() != FUCTION_PARAM) {
    ThrowException(Exception::TypeError(String::New(ERR_INVALID_NUM_ARGS)));
    return scope.Close(obj);
  }

  if (!args[0]->IsString() || !args[1]->IsString() || !args[2]->IsString()) {
    ThrowException(Exception::TypeError(String::New(ERR_INVALID_ARGS)));
    return scope.Close(obj);
  }

  // Local<v8::String> url = args[0]->ToString();
  // Local<v8::String> parseQueryString = args[1]->ToString();
  // Local<v8::String> slashesDenoteHost = args[2]->ToString();

  v8::String::Utf8Value param1(args[0]->ToString());
  std::string url = std::string(*param1);

  v8::String::Utf8Value param2(args[1]->ToString());
  std::string parseQueryString = std::string(*param2);

  v8::String::Utf8Value param3(args[2]->ToString());
  std::string slashesDenoteHost = std::string(*param3);





  // std::string something("hello world"); 
  // Handle<Value> something_else = String::New( something.c_str() );

  obj->Set(String::NewSymbol("url"), String::New(url.c_str()));
  obj->Set(String::NewSymbol("parseQueryString"), String::New(parseQueryString.c_str()));
  obj->Set(String::NewSymbol("slashesDenoteHost"), String::New(slashesDenoteHost.c_str()));

  return scope.Close(obj);
}

void Init(Handle<Object> exports) {
  exports->Set(String::NewSymbol(FUCTION_NAME),
      FunctionTemplate::New(parse)->GetFunction());
}

NODE_MODULE(urlParser, Init)