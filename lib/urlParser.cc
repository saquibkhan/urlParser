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

#define QUOTION_MARK '?'
#define HASH_MARK '#'
#define BACK_SLASH '\\'
#define FORWARD_SLASH '/'
#define TRAILING_WHITESPACES " \t\f\v\n\r"

void _parse(std::string strUrl, bool bParseQueryString, bool bSlashesDenoteHost)
{
  printf("Url:%s\n", strUrl.c_str());
  int iUrlLen = strUrl.length();
  for(int i = 0; i < iUrlLen && (strUrl[i] != QUOTION_MARK && strUrl[i] != HASH_MARK); i++)
  {
    if(strUrl[i] == BACK_SLASH)
    {
      strUrl[i] = FORWARD_SLASH;
    }
  }
  printf("%s\n", strUrl.c_str());

  std::string strRest = strUrl;

  //Trim
  std::string whitespaces (TRAILING_WHITESPACES);

  std::size_t found = strRest.find_last_not_of(whitespaces);
  if (found != std::string::npos)
    strRest.erase(found+1);
  else
    strRest.clear();            // str is all whitespace

  printf("strRest = %s\n", strRest.c_str());

}


Handle<Value> parse(const Arguments& args) {
  HandleScope scope;

  Local<Object> obj = Object::New();

  if (args.Length() != FUCTION_PARAM) {
    ThrowException(Exception::TypeError(String::New(ERR_INVALID_NUM_ARGS)));
    return scope.Close(obj);
  }

  if (!args[0]->IsString() || !args[1]->IsBoolean() || !args[2]->IsBoolean()) {
    ThrowException(Exception::TypeError(String::New(ERR_INVALID_ARGS)));
    return scope.Close(obj);
  }

  // Local<v8::String> url = args[0]->ToString();
  // Local<v8::String> parseQueryString = args[1]->ToString();
  // Local<v8::String> slashesDenoteHost = args[2]->ToString();

  v8::String::Utf8Value param1(args[0]->ToString());
  std::string strUrl = std::string(*param1);

  bool bParseQueryString = args[1]->ToBoolean()->BooleanValue();
  bool bSlashesDenoteHost = args[2]->ToBoolean()->BooleanValue();

  _parse(strUrl, bParseQueryString, bSlashesDenoteHost);

  obj->Set(String::NewSymbol("url"), String::New(strUrl.c_str()));
  obj->Set(String::NewSymbol("parseQueryString"), Boolean::New(bParseQueryString));
  obj->Set(String::NewSymbol("slashesDenoteHost"), Boolean::New(bSlashesDenoteHost));

  return scope.Close(obj);
}

void Init(Handle<Object> exports) {
  exports->Set(String::NewSymbol(FUCTION_NAME),
      FunctionTemplate::New(parse)->GetFunction());
}

NODE_MODULE(urlParser, Init)