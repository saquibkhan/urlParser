// urlParser.cc
//Author: Saquib Khan
//Email: saquibofficial@gmail.com

#include <node.h>
#include <v8.h>
#include <iostream>
#include <regex.h>
#include <algorithm>

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
#define AT_RATE '@'
#define TRAILING_WHITESPACES " \t\f\v\n\r"

#define SPACE ' '
#define TAB '\t'
#define FORM_FEED '\f'
#define VTAB '\v'
#define NEWLINE '\n'
#define CARRIAGE_RETURN '\r'

#define PROTO_JAVASCRIPT "javascript"
#define PROTO_JAVASCRIPT_COLON "javascript:"

#define PROTO_HTTP "http"
#define PROTO_HTTPS "https"
#define PROTO_FTP "ftp"
#define PROTO_GOPHER "gopher"
#define PROTO_FILE "file"
#define PROTO_HTTP_COLON "http:"
#define PROTO_HTTPS_COLON "https:"
#define PROTO_FTP_COLON "ftp:"
#define PROTO_GOPHER_COLON "gopher:"
#define PROTO_FILE_COLON "file:"

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
    bool slashes;
    std::string auth;
    std::string host;
    std::string hostname;
    std::string port;
    std::string hash;

    void _parseHost();
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

int _findHostEndingChars(const std::string &strRest)
{
  //hostEndingChars = ['/', '?', '#']
  int ifoundIndex = -1;
  int len = strRest.length();
  int i = 0;
  while (i < len)
  {
    if (strRest[i] == BACK_SLASH || strRest[i] == QUOTION_MARK || strRest[i] == HASH_MARK)
    {
      ifoundIndex = i;
      break;
    }
    ++i;
  }
  return ifoundIndex;
}

int _findNonHostChars(const std::string &strRest)
{
  // RFC 2396: characters reserved for delimiting URLs.
  // We actually just auto-escape these.
  //delims = ['<', '>', '"', '`', ' ', '\r', '\n', '\t'],

  // RFC 2396: characters not allowed for various reasons.
  //unwise = ['{', '}', '|', '\\', '^', '`'].concat(delims),

  // Allowed by RFCs, but cause of XSS attacks.  Always escape these.
  //autoEscape = ['\''].concat(unwise),

  // Characters that are never ever allowed in a hostname.
  // Note that any invalid chars are also handled, but these
  // are the ones that are *expected* to be seen, so we fast-path
  // them.
  //nonHostChars = ['%', '/', '?', ';', '#'].concat(autoEscape),

  int ifoundIndex = -1;
  int len = strRest.length();
  int i = 0;
  while (i < len)
  {
    if (strRest[i] == '%'
        || strRest[i] == '/'
        || strRest[i] == '?'
        || strRest[i] == ';'
        || strRest[i] == '#'
        || strRest[i] == '{'
        || strRest[i] == '}'
        || strRest[i] == '|'
        || strRest[i] == '\\'
        || strRest[i] == '^'
        || strRest[i] == '`'
        || strRest[i] == '<'
        || strRest[i] == '>'
        || strRest[i] == '"'
        || strRest[i] == ' '
        || strRest[i] == '\r'
        || strRest[i] == '\n'
        || strRest[i] == '\t'
        || strRest[i] == '\'')
    {
      ifoundIndex = i;
      break;
    }
    ++i;
  }
  return ifoundIndex;
}

void CUrl::_parseHost()
{
  // portPattern = /:[0-9]*$/
  std::string strHost = this->host;
  int len = strHost.length();
  if (0 == len)
  {
    return;
  }

  int i = 0;
  if (strHost[i] == ':')
  {
    ++i;
    while (i < len)
    {
      if(strHost[i] >= '0' && strHost[i] <= '9')
      {
        ++i;
      }
      else
      {
        break;
      }
    }
  }

  if (i != 0)
  {
    std::string strPort = strHost.substr(0, i);
    if ( strPort != ":")
    {
      this->port = strPort.substr(1);
    }
    strHost = strHost.substr(0, (strHost.length() - this->port.length()));
  }

  if (strHost.length())
  {
    this->hostname = strHost;
  }
}

void _validateHostName(std::string &strHostName)
{
  int len = strHostName.length();
  if (len > 0)
  {
    int i = 0;
    int iPartStart = i;
    while (i < len)
    {
      if (strHostName[i] == '.')
      {
        //match pattern
        std::string strPart = strHostName.substr(iPartStart, (i-iPartStart));
        if (!_matchHostNamePattern(strPart))
        {

        }

        iPartStart = i+1;
      }
      ++i;
    }
  }
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
  if (bSlashesDenoteHost || bProto || isHostless(strRest))
  {
    if (slashes && !(bProto && isHostlessProtocol(outUrl.protocol))) {
      strRest = strRest.substr(2);
      outUrl.slashes = true;
    }
  }

  //TODO: isSlashedProtocol looks like a bug, capital letter proo will not enter
  if (!isHostlessProtocol(outUrl.protocol) &&
    (slashes || (bProto && !isSlashedProtocol(outUrl.protocol))))
  {
    // there's a hostname.
    // the first instance of /, ?, ;, or # ends the host.
    //
    // If there is an @ in the hostname, then non-host chars *are* allowed
    // to the left of the last @ sign, unless some host-ending character
    // comes *before* the @-sign.
    // URLs are obnoxious.
    //
    // ex:
    // http://a@b@c/ => user:a@b host:c
    // http://a@b?@c => user:a host:c path:/?@c

    // v0.12 TODO: This is not quite how Chrome does things.
    // Review our test case against browsers more comprehensively.

    // find the first instance of any hostEndingChars
    //hostEndingChars = ['/', '?', '#']
    int hostEnd = _findHostEndingChars(strRest);

    // at this point, either we have an explicit point where the
    // auth portion cannot go past, or the last @ char is the decider.
    int atSign = -1;
    if (-1 == hostEnd)
    {
      atSign = strUrl.find_last_of(AT_RATE);
    }
    else
    {
      atSign = strUrl.find_last_of(AT_RATE, hostEnd);
    }

    // Now we have a portion which is definitely the auth.
    // Pull that off.
    if (-1 == atSign)
    {
      outUrl.auth = strRest.substr(0, atSign);
      strRest = strRest.substr(atSign+1);
    }

    hostEnd = _findNonHostChars(strRest);

    // if we still have not hit it, then the entire thing is a host.
    if (-1 == hostEnd)
    {
      hostEnd = strRest.length();
    }

    outUrl.host = strRest.substr(hostEnd);

    //pull out port
    outUrl._parseHost();

    // if hostname begins with [ and ends with ]
    // assume that it's an IPv6 address.
    bool bipv6Hostname = false;
    std::string strHostName = outUrl.hostname;
    int iHostNameLen = strHostName.length();
    if (iHostNameLen > 0)
    {
      bipv6Hostname = strHostName[0] == '['
          && strHostName[iHostNameLen - 1] == ']';
    }

    if (!bipv6Hostname)
    {
      //validate hostname
      _validateHostName(outUrl.hostname)
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
  //this.hostname
  //this.port
  //this.hash
  //this.format

  obj->Set(String::NewFromUtf8(isolate, "path"), String::NewFromUtf8(isolate, outUrl.path.c_str()));
  obj->Set(String::NewFromUtf8(isolate, "href"), String::NewFromUtf8(isolate, outUrl.href.c_str()));
  obj->Set(String::NewFromUtf8(isolate, "pathname"), String::NewFromUtf8(isolate, outUrl.pathname.c_str()));
  obj->Set(String::NewFromUtf8(isolate, "search"), String::NewFromUtf8(isolate, outUrl.search.c_str()));
  obj->Set(String::NewFromUtf8(isolate, "query"), String::NewFromUtf8(isolate, outUrl.query.c_str()));
  obj->Set(String::NewFromUtf8(isolate, "protocol"), String::NewFromUtf8(isolate, outUrl.protocol.c_str()));
  obj->Set(String::NewFromUtf8(isolate, "slashes"), Boolean::New(isolate, outUrl.slashes));
  obj->Set(String::NewFromUtf8(isolate, "auth"), String::NewFromUtf8(isolate, outUrl.auth.c_str()));
  obj->Set(String::NewFromUtf8(isolate, "host"), String::NewFromUtf8(isolate, outUrl.host.c_str()));
  obj->Set(String::NewFromUtf8(isolate, "hostname"), String::NewFromUtf8(isolate, outUrl.hostname.c_str()));
  obj->Set(String::NewFromUtf8(isolate, "port"), String::NewFromUtf8(isolate, outUrl.port.c_str()));
  obj->Set(String::NewFromUtf8(isolate, "hash"), String::NewFromUtf8(isolate, outUrl.hash.c_str()));

  args.GetReturnValue().Set(obj);
}

void Init(Handle<Object> exports) {
  NODE_SET_METHOD(exports, FUCTION_NAME, parse);
}

NODE_MODULE(urlParser, Init)