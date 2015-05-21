// urlParser.cc
#include <node.h>
#include <v8.h>
#include <iostream>
#include <regex.h>

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

#define SPACE ' '
#define TAB '\t'
#define FORM_FEED '\f'
#define VTAB '\v'
#define NEWLINE '\n'
#define CARRIAGE_RETURN '\r'

bool _isSpace(const char &str)
{
  //Note: /s is =>
  //space character
  //tab character
  //carriage return character
  //new line character
  //vertical tab character
  //form feed character
  
  return (str == SPACE || str == TAB || str == FORM_FEED || str == VTAB || str == NEWLINE || str == CARRIAGE_RETURN);
}

bool _simplePath(const std::string &strRest, std::string &strPath, std::string &strSearch)
{
  //simplePathPattern = /^(\/\/?(?!\/)[^\?\s]*)(\?[^\s]*)?$/
  // [1]: string starting from (zero or one occurences // 
  //AND not followed by / AND zero or more occurences of (string other than ? and spaces) )
  //[2]: any string matching ? and not containing spaces
  //[3] Zero or one match of full [1]+[2] 

  //Note: /s is =>
  //space character
  //tab character
  //carriage return character
  //new line character
  //vertical tab character
  //form feed character

  int i = 0;
  int len = strRest.length();

  if (0 == len)
  {
    //no match
    return false;
  }  

  if (strRest[i] != FORWARD_SLASH)
  {
    //no match
  }

  //[1] - zero or one ?
  ++i;
  if (strRest[i] == FORWARD_SLASH)
  {
    ++i;
    if (strRest[i] == FORWARD_SLASH)
    {
      //no match
      return false;
    }
    else
    {
      ++i;
    }
  }

  //[^\?\s]*
  while (i < len && strRest[i] != QUOTION_MARK && !_isSpace(strRest[i]))
  {
    ++i;
  }

  //match ?
  if (i < len && strRest[i] == QUOTION_MARK)
  {
    strPath = std::string(strRest, 0, i);
  }
  else if (_isSpace(strRest[i]))
  {
    //no match
    return false;
  }
  else
  {
    //end of string - no search string
    strPath = std::string(strRest, 0, len);
    return true;
  }

  //(\?[^\s]*)
  int iSearchPos = i;
  while (i < len && !_isSpace(strRest[i]))
  {
    ++i;
  }

  if (_isSpace(strRest[i]))
  {
    //no match
    return false;
  }
  else
  {
    //end of string - search string also found
    strSearch = std::string(strRest, iSearchPos, i-iSearchPos);
  }

  return true;
}

void _parse(std::string strUrl, bool bParseQueryString, bool bSlashesDenoteHost)
{
  printf("Url:%s\n", strUrl.c_str());
  int iUrlLen = strUrl.length();
  for (int i = 0; i < iUrlLen && (strUrl[i] != QUOTION_MARK && strUrl[i] != HASH_MARK); i++)
  {
    if (strUrl[i] == BACK_SLASH)
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

  if (!bSlashesDenoteHost && strUrl.find(HASH_MARK) == std::string::npos)
  {
    std::string strPath, strSearch;
    if (_simplePath(strRest, strPath, strSearch))
    {
      printf("Path: %s\n", strPath.c_str());
      printf("Search: %s\n", strSearch.c_str());
    }

  }

}


void parse(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  Local<Object> obj = Object::New(isolate);

  if (args.Length() != FUCTION_PARAM) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, ERR_INVALID_NUM_ARGS)));
  }

  if (!args[0]->IsString() || !args[1]->IsBoolean() || !args[2]->IsBoolean()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, ERR_INVALID_ARGS)));
  }

  v8::String::Utf8Value param1(args[0]->ToString());
  std::string strUrl = std::string(*param1);

  bool bParseQueryString = args[1]->ToBoolean()->BooleanValue();
  bool bSlashesDenoteHost = args[2]->ToBoolean()->BooleanValue();

  _parse(strUrl, bParseQueryString, bSlashesDenoteHost);

  obj->Set(String::NewFromUtf8(isolate, "url"), String::NewFromUtf8(isolate, strUrl.c_str()));
  obj->Set(String::NewFromUtf8(isolate, "parseQueryString"), Boolean::New(isolate, bParseQueryString));
  obj->Set(String::NewFromUtf8(isolate, "slashesDenoteHost"), Boolean::New(isolate, bSlashesDenoteHost));

  args.GetReturnValue().Set(obj);
}

void Init(Handle<Object> exports) {
  NODE_SET_METHOD(exports, FUCTION_NAME, parse);
}

NODE_MODULE(urlParser, Init)