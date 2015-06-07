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

#define HOSTNAME_MAX_LEN 255

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

    std::string tmpRest;
    bool tmpAtSign;
    bool tmpbIsipv6;
    bool tmpbHostNamePresent;
    bool tmpbHostPresent;

    CUrl()
    {
      this->slashes = false;

      this->tmpAtSign = false;
      this->tmpbIsipv6 = false;
      this->tmpbHostNamePresent = false;
      this->tmpbHostPresent = false;
    }

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
  //printf("_findHostEndingChars called strRest=%s\n", strRest.c_str());

  //hostEndingChars = ['/', '?', '#']
  int ifoundIndex = -1;
  int len = strRest.length();
  int i = 0;
  while (i < len)
  {
    if (strRest[i] == FORWARD_SLASH || strRest[i] == QUOTION_MARK || strRest[i] == HASH_MARK)
    {
      ifoundIndex = i;
      break;
    }
    ++i;
  }

  //printf("_findHostEndingChars ifoundIndex=%d\n", ifoundIndex);
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
        //autoEscape Chars below
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
  //printf("CUrl::_parseHost - this->host=%s\n", this->host.c_str());
  std::string strHost = this->host;
  int len = strHost.length();
  if (0 == len)
  {
    return;
  }

  int i = 0;
  int iColonPos = -1;
  while (i < len)
  {
    if (strHost[i] == ':')
    {
      iColonPos = i;
      ++i;
      break;
    }
    ++i;
  }
  if (iColonPos != -1)
  {
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

  if (iColonPos != -1)
  {
    //printf("colon found iColonPos=%d, i=%d\n", iColonPos, i);
    std::string strPort = strHost.substr(iColonPos, i);
    //printf("strPort=%s\n", strPort.c_str());

    if ( strPort != ":")
    {
      this->port = strPort.substr(1);
      //printf("this->port=%s\n", this->port.c_str());
    }

    strHost = strHost.substr(0, iColonPos);
    //printf("CUrl::_parseHost - strHost=%s\n", strHost.c_str());
  }

  if (strHost.length())
  {
    //printf("CUrl::_parseHost - strHost=%s\n", strHost.c_str());
    this->hostname = strHost;
  }
}

bool _matchHostNamePartPattern(const std::string &strPart)
{
  //TODO: Cross check and test For emty string also it return true
  //hostnamePartPattern = /^[+a-z0-9A-Z_-]{0,63}$/

  bool bFound = true;
  int len = strPart.length();
  if (len > 0)
  {
    int i = 0;
    int iCount = 0;
    while (i < len)
    {
      if (iCount > 63)
      {
        bFound = false;
        break;
      }
      if ((strPart[i] >= 'a' && strPart[i] <= 'z')
          || (strPart[i] >= 'A' && strPart[i] <= 'Z')
          || (strPart[i] >= '0' && strPart[i] <= '9')
          || (strPart[i] == '+')
          || (strPart[i] == '_')
          || (strPart[i] == '-')
          )
      {
        ++iCount;
      }
      else
      {
        bFound = false;
        break;
      }
      ++i;
    }
  }
  return bFound;
}

bool _matchHostnamePartStart(const std::string &strPart,
    std::string &bit1, std::string &bit2)
{
  //TODO: Cross check and test For emty string also it return true
  //hostnamePartStart = /^([+a-z0-9A-Z_-]{0,63})(.*)$/
  bool bFound = true;
  int len = strPart.length();
  if (len > 0)
  {
    int i = 0;
    int iPart1 = 0;
    int iPart2 = 0;
    while (i < len)
    {
      if (iPart1 > 63)
      {
        bFound = false;
        break;
      }
      if ((strPart[i] >= 'a' && strPart[i] <= 'z')
          || (strPart[i] >= 'A' && strPart[i] <= 'Z')
          || (strPart[i] >= '0' && strPart[i] <= '9')
          || (strPart[i] == '+')
          || (strPart[i] == '_')
          || (strPart[i] == '-')
          )
      {
        ++iPart1;
        iPart2 = iPart1;
      }
      else
      {
        if (iPart1 > 63)
        {
          bFound = false;
        }
        else
        {
          ++iPart2;
        }
        break;
      }
      ++i;
    }
    if (bFound)
    {
      bit1 = strPart.substr(0, iPart1);
      if (iPart2 > iPart1)
      {
        bit2 = strPart.substr(iPart1, (iPart2 - iPart1));
      }
    }
  }
  return bFound;
}

void _validateHostName(std::string &strHostName, std::string &strNotHost)
{
  int len = strHostName.length();
  if (len > 0)
  {
    int i = 0;
    int iPartStart = i;
    std::string strNonAscHostName = strHostName;
    while (i < len)
    {
      if (strHostName[i] == '.')
      {
        if (strNonAscHostName[i] > 127)
        {
          strNonAscHostName[i] = 'x';
        }
        if ( i != iPartStart)
        {
          //Part found match pattern
          std::string strPart = strHostName.substr(iPartStart, (i-iPartStart));
          if (!_matchHostNamePartPattern(strPart))
          {
            // we test again with ASCII char only
            std::string strNonAscPart = strNonAscHostName.substr(iPartStart, (i-iPartStart));
            if (!_matchHostNamePartPattern(strNonAscPart))
            {
              std::string strValidParts = strHostName.substr(0, i);
              if (i+1 < len)
              {
                strNotHost = strHostName.substr(i+1);
              }
              std::string bit1,bit2;
              if (_matchHostnamePartStart(strPart, bit1, bit2))
              {
                if (bit1.length() != 0 && bit2.length()!=0)
                {
                  strValidParts += bit1;
                  strNotHost = bit2 + strNotHost;
                }
                strHostName = strValidParts;
                break;
              }
            }
          }
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
  std::size_t first = strRest.find_first_not_of(whitespaces);
  std::size_t last = strRest.find_last_not_of(whitespaces);
  if (first != std::string::npos || last != std::string::npos)
  {
    if (last == strRest.length())
    {
      last = strRest.length()-1;
    }
    strRest = strRest.substr(first, last-first+1);
  }
  else
  {
    strRest.clear();
  }

  if (!bSlashesDenoteHost && (strUrl.find(HASH_MARK) == std::string::npos))
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

  //TODO: isSlashedProtocol looks like a bug, capital letter protocol will not pass
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
      atSign = strRest.find_last_of(AT_RATE);
    }
    else
    {
      atSign = strRest.find_last_of(AT_RATE, hostEnd);
    }

    //printf("atSign: %d \n",atSign);
    //printf("hostEnd: %d \n",hostEnd);
    //printf("strRest before: %s\n", strRest.c_str());

    // Now we have a portion which is definitely the auth.
    // Pull that off.
    if (atSign != -1)
    {
      outUrl.auth = strRest.substr(0, atSign);
      strRest = strRest.substr(atSign+1);

      outUrl.tmpAtSign = true;
      //TODO: Call this from javascript layer
      //this.auth = decodeURIComponent(auth);
    }

    //printf("outUrl.auth: %s\n", outUrl.auth.c_str());
    //printf("strRest: %s\n", strRest.c_str());

    hostEnd = _findNonHostChars(strRest);

    // if we still have not hit it, then the entire thing is a host.
    if (-1 == hostEnd)
    {
      hostEnd = strRest.length();
    }

    outUrl.host = strRest.substr(0, hostEnd);
    if (outUrl.host.length() == 0)
    {
      outUrl.tmpbHostPresent = true;
    }
    strRest = strRest.substr(hostEnd);

    outUrl.tmpbHostNamePresent = true;

    //pull out port
    outUrl._parseHost();

    // if hostname begins with [ and ends with ]
    // assume that it's an IPv6 address.
    bool bIpv6Hostname = false;
    std::string strHostName = outUrl.hostname;
    int iHostNameLen = strHostName.length();
    if (iHostNameLen > 0)
    {
      bIpv6Hostname = strHostName[0] == '['
          && strHostName[iHostNameLen - 1] == ']';
    }

    if (!bIpv6Hostname)
    {
      //validate hostname
      std::string strNotHost;
      _validateHostName(outUrl.hostname, strNotHost);
      if (strNotHost.length() != 0)
      {
        strRest = '/' + strNotHost + strRest;
      }
    }

    if (outUrl.hostname.length() > HOSTNAME_MAX_LEN)
    {
      outUrl.hostname = "";
    }
    else
    {
      // hostnames are always lower case.
      std::transform(outUrl.hostname.begin(), outUrl.hostname.end(), outUrl.hostname.begin(), ::tolower);
    }

    if (!bIpv6Hostname)
    {
      // IDNA Support: Returns a punycoded representation of "domain".
      // It only converts parts of the domain name that
      // have non-ASCII characters, i.e. it doesn't matter if
      // you call it with a domain that already is ASCII-only.

      outUrl.tmpbIsipv6 = true;
      //TODO: Call from Javascript layer
      //this.hostname = punycode.toASCII(this.hostname);
    }

    if (outUrl.port.length() > 0)
    {
      outUrl.host = outUrl.hostname + ":" + outUrl.port;
    }
    else
    {
      outUrl.host = outUrl.hostname;
    }
    outUrl.href += outUrl.host;

    // strip [ and ] from the hostname
    // the host field still retains them, though
    if (bIpv6Hostname)
    {
      outUrl.hostname = outUrl.hostname.substr(1, outUrl.hostname.length() - 2);
      if (strRest[0] != FORWARD_SLASH) {
        strRest = "/" + strRest;
      }
    }
  } //End of not hostless protocol

  //Capture strRest in temporary variable for further processing in JS layer
  outUrl.tmpRest = strRest;

  // now rest is set to the post-host stuff.
  // chop off any delim chars.
  if (outUrl.protocol != PROTO_JAVASCRIPT && outUrl.protocol != PROTO_JAVASCRIPT) {

    // First, make 100% sure that any "autoEscape" chars get
    // escaped, even if encodeURIComponent doesn't think they
    // need to be.

    // for (var i = 0, l = autoEscape.length; i < l; i++) {
    //   var ae = autoEscape[i];
    //   if (rest.indexOf(ae) === -1)
    //     continue;
    //   var esc = encodeURIComponent(ae);
    //   if (esc === ae) {
    //     esc = escape(ae);
    //   }
    //   rest = rest.split(ae).join(esc);
    // }
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

  obj->Set(String::NewFromUtf8(isolate, "rest"), String::NewFromUtf8(isolate, outUrl.tmpRest.c_str()));
  obj->Set(String::NewFromUtf8(isolate, "isATSign"), Boolean::New(isolate, outUrl.tmpAtSign));
  obj->Set(String::NewFromUtf8(isolate, "isIpv6"), Boolean::New(isolate, outUrl.tmpbIsipv6));
  obj->Set(String::NewFromUtf8(isolate, "tmpbHostNamePresent"), Boolean::New(isolate, outUrl.tmpbHostNamePresent));
  obj->Set(String::NewFromUtf8(isolate, "tmpbHostPresent"), Boolean::New(isolate, outUrl.tmpbHostPresent));

  

  

  args.GetReturnValue().Set(obj);
}

void Init(Handle<Object> exports) {
  NODE_SET_METHOD(exports, FUCTION_NAME, parse);
}

NODE_MODULE(urlParser, Init)