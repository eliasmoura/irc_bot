#include "irc.h"
#include "log.h"
#include "network.h"
#include "string.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t irc_send_msg(struct irc_connection *conn, struct irc_msg *m) {
  size_t result = 0;
  struct string buf = {0};
  string_init_capacity(&buf, 256);
  const char fmt[] = "%s %s\r\n";
  size_t size = sizeof(fmt);
  if (m->type) {
    size += strlen(irc_cmd_str[m->type]);
    if (m->args.size && m->args.data)
      size += m->args.size;
    if (size > buf.capacity)
      size = sizeof(buf);
    buf.size =
        snprintf(buf.data, size, fmt, irc_cmd_str[m->type], m->args.data);
    if (buf.size > 0) {
      result = send_msg(&conn->server, &buf);
    }
  }
  if (result == 0 && errno) {
    char err[256];
    snprintf(err, sizeof(err), "%d %s", errno, strerror(errno));
    log_error("%s", strerror(errno));
  }
  string_free(&buf);
  return result;
}

/*
 * @brief Recieves the messages from the server.
 * @param[in] conn: the connection to the server
 * @param[out] buf: the buffer used to store the umprocessed data
 * @returns (int) number of bytes received.
 */
int irc_recv_msg(struct irc_connection *conn, struct string *bbuf, int offset) {
  int result = 0;
  if (!bbuf->capacity) {
    int buf_size = 2048;
    bbuf->capacity = buf_size * 2;
    string_init_capacity(bbuf, bbuf->capacity);
  }
  if (offset) {
    struct string tmp = {0};
    tmp.capacity = bbuf->capacity;
    string_init_capacity(&tmp, tmp.capacity);
    tmp.size = bbuf->capacity - offset;
    string_append_chars_sz(&tmp, bbuf->data + offset, tmp.size);
    string_free(bbuf);
    *bbuf = tmp;
  }
  result = recv_msg(&conn->server, bbuf);
  return result;
}

/**
 * @brief Parse the messages contained in buf`
 *
 * @param[out] msgs processed irc messages.
 * @param[in] bbuf buffer with the messages to be processed
 * @param[out] (*int) >0 there are messages left to be processed.
 *              Offset indicates where the processing stoped
 * @returns (int) 0 ok, <0 error, >0 offset of bytes that are left for
 * processing.
 */
int irc_parse_buffer(struct irc_msg_list *msgs, struct string bbuf,
                     int *offset) {
  int result = 0;
  assert(bbuf.data && bbuf.data);
  size_t max_lines = msgs->capacity;
  struct string *lines = malloc(sizeof(*lines) * max_lines);
  size_t index = 0;
  /* if(offset){ */
  /*   string_append(&static_buf, &buf); */
  /*   offset = 0; */
  /* } else static_buf = buf; */
  size_t start = 0;
  size_t nc = 0;
  for (; index < max_lines && start < bbuf.size && nc < bbuf.size;) {
    for (size_t i = start; i < bbuf.size; ++i, nc = i) {
      if (bbuf.data[i] == '\r' && bbuf.data[i + 1] == '\n') {
        /* lines[index].capacity = lines[index].size; */
        lines[index].size = i - start + 2;
        lines[index].data = bbuf.data + start;
        start = i + 2;
        ++index;
        break;
      }
    }
  }
  for (; msgs->size <= msgs->capacity && msgs->size < index; ++msgs->size) {
    msgs->msgs[msgs->size] =
        lexer_msg_parse(lines[msgs->size].data, lines[msgs->size].size);
  }
  if (start < bbuf.size) {
    *offset = start;
    /* memset(static_buf_copy.data, 0, static_buf_copy.size); */
    /* static_buf_copy.size = 0; */
    /* string_append_chars(&static_buf_copy, static_buf.data+*offset); */
    /* static_buf=static_buf_copy; */
  } else
    *offset = 0;
  free(lines);
  return result;
}

void login(struct irc_connection *conn) {
  int r = 0;
  log_info("Login %s...", conn->nick.data);
  struct irc_msg msg = {0};
  msg.type = IRC_CMD_PASSWORD;
  msg.args = conn->pass;
  irc_send_msg(conn, &msg);

  msg.type = IRC_CMD_NICK;
  msg.args = conn->nick;
  irc_send_msg(conn, &msg);

  msg.type = IRC_CMD_USER;
  msg.args = conn->local_user;
  irc_send_msg(conn, &msg);

  if (!(r & -1)) {
    conn->logged = true;
    log_info("Login of %s  done.", conn->nick.data);
  } else {
    log_warn("Failed to longin %s", conn->nick.data);
  }
}
void print_line(struct string *l, struct irc_msg *msg) {
  char buf[256];
  const char fmt[] = "Server msg(%s): %s\33[33m..\33[m\n";
  int size = strlen(irc_cmd_str[msg->type]);
  size += l->size;
  size += sizeof(fmt);
  size -= 5; // off by five?

  if (size > (int)sizeof(buf))
    size = sizeof(buf);
  snprintf(buf, size, fmt, irc_cmd_str[msg->type], l->data);
  fprintf(stdout, "%s\n", buf);
}
