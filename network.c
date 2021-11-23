#define _GNU_SOURCE
#include "string.h"
#include "network.h"
#include "log.h"
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <errno.h>

int send_msg(struct server_connection *conn, struct string *msg){
  int result = 0;

  result = send(conn->sock, msg->data, msg->size, 0);
  if(result == -1) {
    log_error("%s", strerror(errno));
  }  return result;
}

int recv_msg(struct server_connection *conn, struct string *msg) {
  int result = 0;
  result = recv(conn->sock, msg->data, msg->capacity, 0);
  if(result > -1) {
    msg->size = result;
  } else msg->size = 0;
  return result;
}

void server_connect(struct server_connection *conn) {

  int s = -1;
  int status;
  struct addrinfo hints;
  struct addrinfo *servinfo;
  char ipstr[INET6_ADDRSTRLEN];

  memset(&hints, 0, sizeof hints);

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  printf("%lu\n", conn->port.size);
  char *host = calloc(100, 1);
  char *port = calloc(100, 1);
  for(size_t i = 0; i < conn->hostname.size; ++i)
    host[i] = conn->hostname.data[i];
  host[conn->hostname.size] = '\0';
  for(size_t i = 0; i < conn->port.size; ++i)
    port[i] = conn->port.data[i];
  port[conn->port.size] = '\0';

  if ((status = getaddrinfo(host, port, &hints, &servinfo)) != 0) {
    const char *msg = gai_strerror(status);
    log_info("%s", conn->hostname.data);
    printf("%s", msg);
  } else {
  for (struct addrinfo *p = servinfo; p != 0; p = p->ai_next) {
    void *addr;

    if (p->ai_family == AF_INET) {
      struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
      addr = &(ipv4->sin_addr);
    } else {
      struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
      addr = &(ipv6->sin6_addr);
    }
    inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
    s = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (s != -1)
      if (connect(s, p->ai_addr, p->ai_addrlen) != -1) {
        conn->sock = s;
        char buf[256];
        wchar_t fmt[] = L"Connected to: %ls\n";
        int size = sizeof(fmt) + conn->hostname.size;
        snprintf(buf, size, "%s", conn->hostname.data);
        log_info("Connected to: %s\n", buf);
        break;
      }
  }
    }
  freeaddrinfo(servinfo);
  free(port);
  free(host);

  if (s == -1) {
    printf("No socket %d!!!!\n", s);
  }
}
