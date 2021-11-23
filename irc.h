#ifndef irc_h_INCLUDED
#define irc_h_INCLUDED
#include "string.h"
#include "lexer.h"
#include "network.h"
#include <stdbool.h>

struct irc_channel {
  struct string name;
  struct string *users;
  int user_count;
};

struct origin {
  struct string ip;
  struct string user;
};

struct irc_msg {
  enum irc_cmd type;
  struct origin sender;
  struct irc_channel channel;
  struct string args;
};

struct irc_msg_list {
  size_t size;
  size_t capacity;
  struct irc_msg *msgs;
};

struct irc_connection {
  struct string pass;
  struct string nick;
  struct server_connection server;
  struct string local_user;
  bool logged;
  struct irc_channel *channels;
  struct irc_msg last_receive_msg;
  struct irc_msg last_sent_msg;
};


/*
 * Send the message to the server
 * @conn: the connection to the server
 */
void login(struct irc_connection *conn);
/*
 * Send the message to the server
 * @conn: the connection to the server
 * @msg: the processed messages from the server
 */
size_t irc_send_msg(struct irc_connection *conn, struct irc_msg *m);
///
/// Recieves the messages from and process them.
/// @param: conn the connection to the server
/// @param: buf the buffer used to store the umprocessed data
/// @param: msgs the processed messages from the server
///
int irc_recv_msg(struct irc_connection *conn, struct string *bbuf, int offset);
int irc_parse_buffer(struct irc_msg_list *msgs, struct string bbuf, int *offset);
#endif // irc_h_INCLUDED
