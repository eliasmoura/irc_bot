#include "lexer.h"
#include "irc.h"
#include "string.h"
#include <string.h>

#define IRC_CMD(str, enum) str,
const char *irc_cmd_str[] = {IRC_CMDS "IRC_CMD_UNKNOWN"};
#undef IRC_CMD

struct string token(const char *str, int size, char *t) {
  struct string result = {0};
  while (*str != 0 && (*str == ' ' || *str == '\n' || *str == '\r'))
    ++str;
  for (int i = 0; i <= size; ++i) {
    if (str[i] == *t) {
      result.capacity = i;
      result.size = i;
      result.data = (char*)str;
      break;
    }
  }
  return result;
}

struct irc_msg lexer_msg_parse(const char * const str, size_t size) {
  struct irc_msg result = {0};
  struct string user = {0};
  struct string cmd_token = {0};
  char const *it = str;
  if (*it == ':') {
    user = token(++it, size, " ");
    it = it + user.size + 1;
  } else
    cmd_token = token(it, size, " ");

  if (user.size) {
    result.sender.user = token(user.data, user.size, "!");
    // offset +2 e.g kotto!~kotto@....
    //                   ^^
    result.sender.ip =
        token(user.data + result.sender.user.size + 2, user.size, " ");
    cmd_token = token(
        it, (result.sender.ip.data + result.sender.ip.size) - str, " ");
    //TODO(elias): capture the channel of the message being received.
  }
  result.type = IRC_CMD_UNKNOWN;
  if(cmd_token.data[1] == '5')
    cmd_token.data[1] = '5';
  if (cmd_token.data)
    for (enum irc_cmd i = 0; i < IRC_CMD_UNKNOWN - 1; ++i) {
      if (!strncmp(cmd_token.data, irc_cmd_str[i], cmd_token.size)) {
        result.type = i;
        break;
      }
    }

  result.args.data = cmd_token.data + cmd_token.size + 1;
  result.args.capacity = size - (result.args.data - str);
  result.args.size =
      result.args.capacity - 1;
  return result;
}
