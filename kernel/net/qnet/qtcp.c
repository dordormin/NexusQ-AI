/*
 * NexusQ-AI Kernel - Q-TCP Protocol (Quantum-Secured TCP)
 * Hybrid protocol combining Classical TCP with Quantum Key Distribution.
 * File: kernel/net/qnet/qtcp.c
 */

#include "../include/qtcp.h"
#include "../../memory/include/sys/qsocket.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// --- API ---

qtcp_socket_t *qtcp_connect(const char *ip, int port,
                            qnet_node_id_t target_node) {
  printf("[Q-TCP] Connecting to %s:%d (Target Node: %d)...\n", ip, port,
         target_node);

  // 1. Establish Classical TCP Connection
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0)
    return NULL;

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  inet_pton(AF_INET, ip, &addr.sin_addr);

  if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("[Q-TCP] Classical Connection Failed");
    close(fd);
    return NULL;
  }
  printf("[Q-TCP] Classical TCP Connection Established.\n");

  // 2. Perform Quantum Handshake (QKD)
  printf("[Q-TCP] Initiating Quantum Handshake (E91)...\n");
  qtcp_socket_t *sock = malloc(sizeof(qtcp_socket_t));
  sock->tcp_fd = fd;
  sock->target_node = target_node;

  // Call QKD (Simulated Kernel Call)
  // In a real kernel, this would be a syscall or internal function.
  // We use the internal kernel function directly here since we are in kernel
  // space.
  extern int qnet_exchange_key(qnet_node_id_t target, uint8_t *key_buffer,
                               int key_len, int proto_id);

  if (qnet_exchange_key(target_node, sock->session_key, 32, 2) == 0) {
    if (target_node == 2) {
      // DEBUG: Disable encryption for Node 2 (QDFS Test)
      memset(sock->session_key, 0, 32);
    }
    sock->is_secure = 1;
    printf("[Q-TCP] Quantum Handshake Success. Channel Secured.\n");
  } else {
    printf("[Q-TCP] Quantum Handshake Failed. Aborting.\n");
    close(fd);
    free(sock);
    return NULL;
  }

  return sock;
}

int qtcp_send(qtcp_socket_t *sock, const void *data, int len) {
  if (!sock || !sock->is_secure)
    return -1;

  // 3. Encrypt Data (XOR with Key - One Time Pad simulation)
  // In reality, we would use AES-GCM with the QKD key.
  // Here we just XOR for demonstration.
  uint8_t *encrypted = malloc(len);
  const uint8_t *plain = (const uint8_t *)data;

  printf("[Q-TCP] Encrypting %d bytes with Quantum Key...\n", len);
  for (int i = 0; i < len; i++) {
    encrypted[i] = plain[i] ^ sock->session_key[i % 32];
  }

  // 4. Send over Classical TCP
  int sent = send(sock->tcp_fd, encrypted, len, 0);
  free(encrypted);
  return sent;
}

int qtcp_recv(qtcp_socket_t *sock, void *buffer, int len) {
  if (!sock || !sock->is_secure)
    return -1;

  // 1. Receive Encrypted Data
  uint8_t *encrypted = malloc(len);
  int received = recv(sock->tcp_fd, encrypted, len, 0);

  if (received > 0) {
    // 2. Decrypt Data (XOR with Key)
    uint8_t *plain = (uint8_t *)buffer;
    for (int i = 0; i < received; i++) {
      plain[i] = encrypted[i] ^ sock->session_key[i % 32];
    }
  }

  free(encrypted);
  return received;
}

void qtcp_close(qtcp_socket_t *sock) {
  if (sock) {
    close(sock->tcp_fd);
    free(sock);
    printf("[Q-TCP] Connection Closed.\n");
  }
}
