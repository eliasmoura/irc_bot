#include "irc.h"
#include "lexer.h"
#include "log.h"
#include "network.h"
#include "string.h"
#include <errno.h>
#include <locale.h>
#include <poll.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

int main(int argc, char *argv[]) {
  setlocale(LC_CTYPE, "en_US.utf8");
  struct server_connection sconn = {0};
  struct irc_connection conn = {0};

  if (argc >= 2)
    sconn.hostname.data = (char *)argv[1];
  else
    sconn.hostname.data = "irc.libera.chat";
  if (argc == 3)
    sconn.port.data = (char *)argv[2];
  else
    sconn.port.data = "6667";
  sconn.hostname.size = strlen(sconn.hostname.data);
  sconn.port.size = strlen(sconn.port.data);

  conn.server = sconn;
  server_connect(&conn.server);
  if (!conn.server.sock) {
    log_error("%s", "FAIL to connect\n");
    return 255;
  }

  conn.pass.data = "hello";
  conn.pass.capacity = strlen(conn.pass.data);
  conn.pass.size = conn.pass.capacity;

  conn.nick.data = "kotto_bot";
  conn.nick.capacity = strlen(conn.nick.data);
  conn.nick.size = conn.nick.capacity;

  conn.local_user.data = "kotto2 127.0.0.1 chat.freenode.net :I O";
  conn.local_user.capacity = strlen(conn.local_user.data);
  conn.local_user.size = conn.local_user.capacity;

  login(&conn);

  struct irc_channel channel = {0};
  channel.name.data = "#kotto";
  channel.name.capacity = strlen(channel.name.data);
  channel.name.size = channel.name.capacity;
  conn.channels = &channel;
  // if (conn.logged)
  //   return 1;
  conn.last_sent_msg.args = channel.name;
  conn.last_sent_msg.type = IRC_CMD_JOIN;
  irc_send_msg(&conn, &conn.last_sent_msg);

  struct pollfd fds[2] = {0};
  fds[0].fd = STDIN_FILENO;
  fds[0].events = POLLIN;
  fds[1].fd = conn.server.sock;
  fds[1].events = POLLIN;
  int r = 0;

  struct string local_input = {0};
  string_init_capacity(&local_input, 256);
  struct string buf = {0};
  string_init_capacity(&buf, 2048);

  struct irc_msg_list list_msgs = {0};
  list_msgs.capacity = 100;
  list_msgs.size = 0;
  list_msgs.msgs = malloc(sizeof(list_msgs.msgs[0]) * list_msgs.capacity);
  int parse_offset = 0;

  while (conn.server.sock > 0) {
    int polled = poll(fds, ARRAY_SIZE(fds), -1); // 500);
    if (polled > 0) {
      if (fds[0].revents & POLLIN) {
        r = read(STDIN_FILENO, local_input.data, local_input.capacity);
        if (r > 0 && r <= (int)(local_input.capacity - 2) &&
            strcmp((char *)local_input.data, "!quit")) {
          local_input.data[r] = '\0';
          if (local_input.capacity) {
            struct irc_msg msg =
                lexer_msg_parse(local_input.data, local_input.size);
            r = irc_send_msg(&conn, &msg);
          };
          // memset(local_input, 0, r);
        } else
          break;
      }

      if (fds[1].revents & POLLIN || parse_offset) {
        irc_recv_msg(&conn, &buf, parse_offset);
        irc_parse_buffer(&list_msgs, buf, &parse_offset);
        size_t index = 0;
        while (index < list_msgs.size) {
          struct irc_msg msg = list_msgs.msgs[index];
          if (msg.type) { /*&& msg.type != IRC_CMD_MOTD && msg.type != RPL_MOTD
               && msg.type != RPL_ENDOFMOTD && msg.type != RPL_NAMREPLY &&
               msg.type != RPL_WELCOME && msg.type != RPL_INFO &&
               msg.type != RPL_CREATED) {*/
            // RPL_MYIINFO BOUNCE LUSERCLIENT   RPL_LUSERCLIENT)
            // RPL_LUSEROP)\ RPL_LUSERUNKNOWN)\ RPL_LUSERCHANNELS)
            // RPL_LUSERME)

            switch (msg.type) {
            case IRC_CMD_NOTICE:
              break;
            case IRC_CMD_INFO:
              break;
            case IRC_CMD_JOIN: {
              char buf[256];
              int inf_size = msg.args.size;
              if (inf_size > (int)sizeof buf)
                inf_size = sizeof buf;
              inf_size = snprintf(buf, inf_size, "%.*s", (int)msg.args.size,
                                  msg.args.data);
              log_info("JOINned: %s", buf);

              struct irc_msg m = {0};
              m.type = IRC_CMD_PRIVMSG;
              char hey[] = "#kotto :Hey";
              int size = sizeof hey;
              m.args.data = hey;
              m.args.size = size - 1;
              m.args.capacity = size;
              int r = irc_send_msg(&conn, &m);
              if (r < 0) {
                log_error("%s", strerror(errno));
              } else
                log_info("%s", hey);
            } break;
            case IRC_CMD_PING: {
              struct irc_msg m = {0};
              m.type = IRC_CMD_PONG;
              int r = irc_send_msg(&conn, &m);
              if (r < 0) {
                log_error("%s", strerror(errno));
              }
            } break;
            case IRC_CMD_PONG:
              break;
            case ERR_NOTREGISTERED: {
              log_error("User <%s> not authenticated!", conn.nick.data);
            } break;
            case RPL_WELCOME:
            case RPL_YOURHOST:
            case RPL_MYINFO:
            case RPL_BOUNCE:
            case RPL_LUSERCLIENT:
            case RPL_LUSEROP:
            case RPL_LUSERUNKNOWN:
            case RPL_LUSERCHANNELS:
            case RPL_ENDOFNAMES:
            case RPL_MOTDSTART:
            case RPL_LUSERME: {
              char buf[256];
              int size = msg.args.size;
              if (size > (int)sizeof(buf))
                size = sizeof buf;
              size = snprintf(buf, size, "%.*s", (int)msg.args.size,
                              msg.args.data);
              log_info("> %s", buf);
            } break;
            case IRC_CMD_PRIVMSG: {
              char buf[256];
              if (strcmp(msg.sender.user.data, conn.nick.data)) {
                snprintf(buf, sizeof(buf), "%.*s| %.*s",
                         (int)msg.sender.user.size, msg.sender.user.data,
                         (int)msg.args.size, msg.args.data);
                log_info("> %s", buf);
              }
            } break;
            default: {
              char buf[256];
              char fmt[] = "Command \"%s\" not implemented!";
              int size = sizeof(irc_cmd_str[msg.type]);
              size += sizeof(fmt);
              snprintf(buf, sizeof(buf), "%s", irc_cmd_str[msg.type]);
              log_warn("Command \"%s\" not implemented!", buf);
              log_warn("%.*s", (int)msg.args.size, msg.args.data);
            }
            }
            fflush(stderr);
          }
          ++index;
        }
      } else if (fds[1].revents & POLLHUP) {
        log_error("Connection closed by perr: %s", conn.server.hostname.data);
        conn.server.sock = 0;
      } else
        conn.server.sock = 0;
      list_msgs.size = 0;
    } // if polled > 0
  }   // while s > 0
  if (list_msgs.msgs)
    free(list_msgs.msgs);
  string_free(&buf);

  return 0;
}
