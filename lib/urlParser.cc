// urlParser.cc
#include <node.h>
#include <v8.h>
#include <iostream>
#include <regex.h>
#include <algorithm>

using namespace v8;
using namespace std;

#define FUCTION_NAME 'parse'
#define FUCTION_PARAM 3

#define ERR_INVALID_ARGS  'Wrong arguments'
#define ERR_INVALID_NUM_ARGS  'Wrong number of arguments'

#define QUOTION_MARK '?'
#define HASH_MARK '#'
#define BACK_SLASH '\\'
#define FORWARD_SLASH '/'
#define AT_RATE '@'
#define TRAILING_WHITESPACES ' \t\f\v\n\r'

#define SPACE ' '
#define TAB '\t'
#define FORM_FEED '\f'
#define VTAB '\v'
#define NEWLINE '\n'
#define CARRIAGE_RETURN '\r'

#define PROTO_JAVASCRIPT 'javascript'
#define PROTO_JAVASCRIPT_COLON 'javascript:'

#define PROTO_HTTP 'http'
#define PROTO_HTTPS 'https'
#define PROTO_FTP 'ftp'
#define PROTO_GOPHER 'gopher'
#define PROTO_FILE 'file'
#define PROTO_HTTP_COLON 'http:'
#define PROTO_HTTPS_COLON 'https:'
#define PROTO_FTP_COLON 'ftp:'
#define PROTO_GOPHER_COLON 'gopher:'
#define PROTO_FILE_COLON 'file:'



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

bool _protoPattern(const std::string &strRest, std::string &strProto)
{
  // /^([a-z0-9.+-]+:)/i
  int i = 0;
  int len = strRest.length();
  if (0 == len)
  {
    //no match
    return false;
  }

  while ( i < len && ((strRest[i] >= 'A' && strRest[i] <= 'Z') || (strRest[i] >= 'a' && strRest[i] <= 'z')
      || strRest[i] == '.' || strRest[i] == '+' || strRest[i] == '-'))
  {
    ++i;
  }

  // at least one match is required.
  if (0 == i)
  {
    //no match
    return false;
  }

  if (i < len && strRest[i] == ':')
  {
    //match found
    ++i;
    strProto = strRest.substr(0, i);
    return true;
  }

  return false;
}

bool _simplePath(const std::string &strRest, std::string &strPath,
    std::string &strSearch, std::string &strQuery)
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
    return false;
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
    strPath = strRest.substr(0, i);
  }
  else if (_isSpace(strRest[i]))
  {
    //no match
    return false;
  }
  else
  {
    //end of string - no search string
    strPath = strRest.substr(0, len);
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
    strSearch = strRest.substr(iSearchPos, i-iSearchPos);
    strQuery = strRest.substr(iSearchPos+1, i-iSearchPos); //skipping ?
  }

  return true;
}

class CUrl
{
  public:
    std::string path;
    std::string href;
    std::string pathname;
    std::string search;
    std::string query;
    std::string protocol;
    std::string slashes;
    std::string auth;
    std::string host;
    std::string parseHost;
    std::string hostname;
    std::string port;
    std::string hash;
    std::string format;
};

bool isHostless(const std::string &strRest)
{
  // /^\/\/[^@\/]+@[^@\/]+/)*/
  int len = strRest.length();
  int i = 0;

  if (0 == len)
  {
    return false;
  }

  if (strRest[i] == FORWARD_SLASH)
  {
    ++i;
    if (i < len && strRest[i] == FORWARD_SLASH)
    {
      ++i;
      bool atLeastOne = false;
      while (i < len && strRest[i] != AT_RATE && strRest[i] != FORWARD_SLASH)
      {
        ++i;
        atLeastOne = true;
      }

      if (i < len && atLeastOne && strRest[i] == AT_RATE)
      {
        ++i;
        atLeastOne = false;
        while (i < len && strRest[i] != AT_RATE && strRest[i] != FORWARD_SLASH)
        {
          ++i;
          atLeastOne = true;
        }

        if ( i < len && atLeastOne && strRest[i] == FORWARD_SLASH)
        {
          return true;
        }
      }
    }
  }

  return false;
}

bool isHostlessProtocol(const std::string &strProtocol)
{
  //   hostlessProtocol = {
  //   'javascript': true,
  //   'javascript:': true
  // },


  return (strProtocol == PROTO_JAVASCRIPT || strProtocol == PROTO_JAVASCRIPT_COLON);
}

bool isSlashedProtocol(const std::string &strProtocol)
{
  // protocols that always contain a // bit.
  // slashedProtocol = {
  //   'http': true,
  //   'https': true,
  //   'ftp': true,
  //   'gopher': true,
  //   'file': true,
  //   'http:': true,
  //   'https:': true,
  //   'ftp:': true,
  //   'gopher:': true,
  //   'file:': true
  // }

  return (strProtocol == PROTO_HTTP || strProtocol == PROTO_HTTP_COLON
      || strProtocol == PROTO_HTTPS || strProtocol == PROTO_HTTPS_COLON
      || strProtocol == PROTO_FTP || strProtocol == PROTO_FTP_COLON
      || strProtocol == PROTO_FILE || strProtocol == PROTO_FILE_COLON
      || strProtocol == PROTO_GOPHER || strProtocol == PROTO_GOPHER_COLON);
}

void _parse(std::string strUrl, CUrl &outUrl, bool bParseQueryString, bool bSlashesDenoteHost)
{
  int iUrlLen = strUrl.length();
  for (int i = 0; i < iUrlLen && (strUrl[i] != QUOTION_MARK && strUrl[i] != HASH_MARK); i++)
  {
    if (strUrl[i] == BACK_SLASH)
    {
      strUrl[i] = FORWARD_SLASH;
    }
  }

  std::string strRest = strUrl;

  //Trim
  std::string whitespaces (TRAILING_WHITESPACES);
  std::size_t found = strRest.find_last_not_of(whitespaces);
  if (found != std::string::npos)
    strRest.erase(found+1);
  else
    strRest.clear();// str is all whitespace


  if (!bSlashesDenoteHost && strUrl.find(HASH_MARK) == std::string::npos)
  {
    if (_simplePath(strRest, outUrl.pathname, outUrl.search, outUrl.query))
    {
      outUrl.path = strRest;
      outUrl.href = strRest;
      return;
    }
  }

  bool bProto = _protoPattern(strRest, outUrl.protocol);
  if (bProto)
  {
    std::transform(outUrl.protocol.begin(), outUrl.protocol.end(), outUrl.protocol.begin(), ::tolower);
    strRest = strRest.substr(outUrl.protocol.length());
  }

  // figure out if it's got a host
  // user@server is *always* interpreted as a hostname, and url
  // resolution will treat //foo/bar as host=foo,path=bar because that's
  // how the browser resolves relative URLs.

  bool slashes = (strRest[0] == FORWARD_SLASH && strRest[1] == FORWARD_SLASH);
  if (slashesDenoteHost || bProto || isHostless(strRest)) 
  {
    if (slashes && !(bProto && isHostlessProtocol(outUrl.protocol)) {
      strRest = strRest.substr(2);
      outUrl.slashes = true;
    }
  }

//TODO: isSlashedProtocol looks like a bug, capital letter proo will not enter
  if (!isHostlessProtocol(outUrl.protocol) &&
    (slashes || (bProto && !isSlashedProtocol(outUrl.protocol))))
  {
      
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

  CUrl outUrl;
  _parse(strUrl, outUrl, bParseQueryString, bSlashesDenoteHost);

  //Output
  //this.path
  //this.href
  //this.pathname
  //this.search
  //this.query
  //this.protocol
  //this.slashes
  //this.auth
  //this.host
  //this.parseHost
  //this.hostname
  //this.port
  //this.hash
  //this.format

  obj->Set(String::NewFromUtf8(isolate, 'path'), String::NewFromUtf8(isolate, outUrl.path.c_str()));
  obj->Set(String::NewFromUtf8(isolate, 'href'), String::NewFromUtf8(isolate, outUrl.href.c_str()));
  obj->Set(String::NewFromUtf8(isolate, 'pathname'), String::NewFromUtf8(isolate, outUrl.pathname.c_str()));
  obj->Set(String::NewFromUtf8(isolate, 'search'), String::NewFromUtf8(isolate, outUrl.search.c_str()));
  obj->Set(String::NewFromUtf8(isolate, 'query'), String::NewFromUtf8(isolate, outUrl.query.c_str()));
  obj->Set(String::NewFromUtf8(isolate, 'protocol'), String::NewFromUtf8(isolate, outUrl.protocol.c_str()));
  obj->Set(String::NewFromUtf8(isolate, 'slashes'), String::NewFromUtf8(isolate, outUrl.slashes.c_str()));
  obj->Set(String::NewFromUtf8(isolate, 'auth'), String::NewFromUtf8(isolate, outUrl.auth.c_str()));
  obj->Set(String::NewFromUtf8(isolate, 'host'), String::NewFromUtf8(isolate, outUrl.host.c_str()));
  obj->Set(String::NewFromUtf8(isolate, 'parseHost'), String::NewFromUtf8(isolate, outUrl.parseHost.c_str()));
  obj->Set(String::NewFromUtf8(isolate, 'hostname'), String::NewFromUtf8(isolate, outUrl.hostname.c_str()));
  obj->Set(String::NewFromUtf8(isolate, 'port'), String::NewFromUtf8(isolate, outUrl.port.c_str()));
  obj->Set(String::NewFromUtf8(isolate, 'hash'), String::NewFromUtf8(isolate, outUrl.hash.c_str()));
  obj->Set(String::NewFromUtf8(isolate, 'format'), String::NewFromUtf8(isolate, outUrl.format.c_str()));

  args.GetReturnValue().Set(obj);
}

void Init(Handle<Object> exports) {
  NODE_SET_METHOD(exports, FUCTION_NAME, parse);
}

NODE_MODULE(urlParser, Init)