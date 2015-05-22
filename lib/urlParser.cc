// urlParser.cc
#include <node.h>
#include <v8.h>
#include <iostream>
#include <regex.h>
#include <regex>

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

#define MAX_ERROR_MSG 0x1000

/* Compile the regular expression described by "regex_text" into
   "r". */
static int compile_regex (regex_t * r, const char * regex_text)
{
    int status = regcomp (r, regex_text, REG_EXTENDED|REG_NEWLINE);
    if (status != 0) {
  char error_message[MAX_ERROR_MSG];
  regerror (status, r, error_message, MAX_ERROR_MSG);
        printf ("Regex error compiling '%s': %s\n",
                 regex_text, error_message);
        return 1;
    }
    return 0;
}

/*
  Match the string in "to_match" against the compiled regular
  expression in "r".
 */

static int match_regex (regex_t * r, const char * to_match)
{
    /* "P" is a pointer into the string which points to the end of the
       previous match. */
    const char * p = to_match;
    /* "N_matches" is the maximum number of matches allowed. */
    const int n_matches = 10;
    /* "M" contains the matches found. */
    regmatch_t m[n_matches];

    while (1) {
        int i = 0;
        int nomatch = regexec (r, p, n_matches, m, 0);
        if (nomatch) {
            printf ("No more matches.\n");
            return nomatch;
        }
        for (i = 0; i < n_matches; i++) {
            int start;
            int finish;
            if (m[i].rm_so == -1) {
                break;
            }
            start = m[i].rm_so + (p - to_match);
            finish = m[i].rm_eo + (p - to_match);
            if (i == 0) {
                printf ("$& is ");
            }
            else {
                printf ("$%d is ", i);
            }
            printf ("'%.*s' (bytes %d:%d)\n", (finish - start),
                    to_match + start, start, finish);
        }
        p += m[0].rm_eo;
    }
    return 0;
}

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

  if(!bSlashesDenoteHost && strUrl.find(HASH_MARK) == std::string::npos)
  {
    //regex_t regExp;
    //const char * cstrSimplePathPattern = "^(\\/\\/?(?!\\/)[^\\\?\\s]*)(\\\?[^[:space:]]*)?$";
    //const char * cstrSimplePathPattern = "^(//?)";
    //compile_regex(&regExp, cstrSimplePathPattern);
    //match_regex(&regExp, strRest.c_str());
    //regfree (&regExp);

    std::regex re("^(//?(?!/)[^\?[:space:]]*)(\?[^[:space:]]*)?$");
    std::smatch match;
    if (/*std::regex_search(strRest, match, re)*/ std::regex_match(strRest, match, re) && match.size() > 1) {
      printf("match found subexp=%d\n",match.size());
        for (unsigned i=0; i<match.size(); ++i) {
          printf("%s", match[i]);
        }
    } else {
      printf("not found");
    } 

    // int ret = 0;
    // char msgbuf[100];
    // //std::string strSimplePathPattern = "^\\(//?\\(?!/\\)\\[^\?\\s\\]*\\)";
    // std::string strSimplePathPattern = "\\(//\\)*";
    // regex_t regExp;

    // int nMatch = 2;
    // regmatch_t regMatch[nMatch];

    // //Compile regular expression
    // //ret = regcomp(&simplePathPattern, "^(//?(?!/)[^\?\\s]*)(\?[^\\s]*)?$", 0);
    // //ret = regcomp(&simplePathPattern, "^(\\/\\/?(?!\\/)[^\?\\s]*)(\?[^\\s]*)?$", 0);
    // ret = regcomp(&regExp, strSimplePathPattern.c_str(), REG_NOSUB);
    // if (ret) {
    //     fprintf(stderr, "Could not compile regex simplePathPattern\n");
    //     return;
    // }

    // /* Execute regular expression */
    // ret = regexec(&regExp, strRest.c_str(), nMatch, &regMatch[0], REG_NOSUB);
    // if (!ret) {
    //     printf("Match");
    // }
    // else if (ret == REG_NOMATCH) {
    //     printf("No match");
    // }
    // else {
    //     regerror(ret, &regExp, msgbuf, sizeof(msgbuf));
    //     fprintf(stderr, "Regex match failed: %s\n", msgbuf);
    //     return;
    // }

  }

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