#ifndef network_h_INCLUDED
#define network_h_INCLUDED
#include "string.h"
#include <stdbool.h>

struct server_connection {
  int sock;
  struct string port;
  struct string hostname;
};

int send_msg(struct server_connection *conn, struct string *msg);
int recv_msg(struct server_connection *conn, struct string *msg);
void server_connect(struct server_connection *conn);
#endif // network_h_INCLUDED

