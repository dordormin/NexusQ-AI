#ifndef QTCP_H
#define QTCP_H

#include "../../memory/include/sys/qsocket.h"
#include <stdint.h>

// --- Q-TCP Socket Structure ---
typedef struct {
  int tcp_fd;
  qnet_node_id_t target_node;
  uint8_t session_key[32];
  int is_secure;
} qtcp_socket_t;

// --- API ---
qtcp_socket_t *qtcp_connect(const char *ip, int port,
                            qnet_node_id_t target_node);
int qtcp_send(qtcp_socket_t *sock, const void *data, int len);
int qtcp_recv(qtcp_socket_t *sock, void *buffer, int len);
void qtcp_close(qtcp_socket_t *sock);

#endif // QTCP_H
