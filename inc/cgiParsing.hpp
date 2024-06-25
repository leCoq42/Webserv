#pragma once

// welke moet ik even kijken
#include "request.hpp"
#include <memory>
#include <unordered_map>

// REFERENCE :( chapter 4:
// http://www.faqs.org/rfcs/rfc3875.html

class CgiParsing {
private:
  std::vector<std::string> meta_variables; // envpp;	// meta variables and
                                           // header variables and shiznitz
  std::vector<std::string> uri;            // argv;		// 4.4 The Script
                                           // Command Line
  std::vector<const char *> return_uri;
  std::string body; // stdin;	//body should go here unless otherwise defined.
  std::vector<std::string> customizable_variables_names; // accepted post_fixes
  bool add_to_envpp(std::string name, std::string value, std::string additive);
  bool add_to_uri(std::string name, std::string value, std::string additive);
  bool dismantle_body(std::string body, std::string boundary);

public:
  CgiParsing(std::unordered_map<std::string, std::string> headers,
             char **environ, std::shared_ptr<Request> _request,
             const std::string &path,
             const std::string &interpreter); // ServerStruct &serverinfo
  ~CgiParsing(void);
  // char	**get_envpp();
  std::vector<std::string> &get_argv();
  std::vector<std::string> &get_envp();
  std::string &get_stdin();
};

// Accepted env_variables: http://www.faqs.org/rfcs/rfc3875.html  chapter: 4.1
//  std::vector<std::string>	meta_variables_names = {"AUTH_TYPE" ,
//  "CONTENT_LENGTH" ,
//                             "CONTENT_TYPE" , "GATEWAY_INTERFACE" ,
//                             "PATH_INFO" , "PATH_TRANSLATED" ,
//                             "QUERY_STRING" , "REMOTE_ADDR" ,
//                             "REMOTE_HOST" , "REMOTE_IDENT" ,
//                             "REMOTE_USER" , "REQUEST_METHOD" ,
//                             "SCRIPT_NAME" , "SERVER_NAME" ,
//                             "SERVER_PORT" , "SERVER_PROTOCOL" ,
//                             "SERVER_SOFTWARE" }; // server passed vraiables,
//                             envpp
//  customizable_variables_names = { "scheme", "protocol-var-name",
//  "extension-var-name"}; request_variables = "HTTP_"; //HTTP_var, request
//  passed variables, envpp
/*RULES
      protocol-var-name  = ( protocol | scheme ) "_" var-name
      scheme             = alpha *( alpha | digit | "+" | "-" | "." )
      var-name           = token
      extension-var-name = token}
        meta-variables with the same name as a scheme, and names beginning
   with the name of a protocol or scheme (e.g., HTTP_ACCEPT) are also
   defined.  The number and meaning of these variables may change
   independently of this specification.  (See also section 4.1.18.)

   The server MAY set additional implementation-defined extension meta-
   variables, whose names SHOULD be prefixed with "X_".

   This specification does not distinguish between zero-length (NULL)
   values and missing values.  For example, a script cannot distinguish
   between the two requests http://host/script and http://host/script?
   as in both cases the QUERY_STRING meta-variable would be NULL.

      meta-variable-value = "" | 1*<TEXT, CHAR or tokens of value>

   An optional meta-variable may be omitted (left unset) if its value is
   NULL.  Meta-variable values MUST be considered case-sensitive except
   as noted otherwise.  The representation of the characters in the
   meta-variables is system-defined; the server MUST convert values to
   that representation. */
