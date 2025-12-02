/*
 * NexusQ-AI Kernel - Neural Abstraction Layer (NAL)
 * File: kernel/neural/nal.c
 */

#include "../../lib/include/nexus.h" // For printf/logging
#include "include/sys/neural.h"
#include <stdio.h>
#include <stdlib.h> // For malloc/free (simulated kernel alloc)
#include <string.h>

// --- NAL Internal State ---
#define MAX_NEURAL_SOCKETS 32

typedef struct {
  int id;
  int used;
  int protocol;
  int state; // 0=CLOSED, 1=READY, 2=CONNECTED
  char connected_model[64];

  // Buffers for simulation
  void *input_buffer;
  size_t input_size;

  neural_result_t last_result;
} nal_socket_t;

static nal_socket_t socket_table[MAX_NEURAL_SOCKETS];
static int nal_initialized = 0;

void nal_init_subsystem() {
  if (nal_initialized)
    return;
  memset(socket_table, 0, sizeof(socket_table));
  nal_initialized = 1;
  printf("[NAL] Neural Abstraction Layer Initialized (AF_NEURAL).\n");
}

// --- API Implementation ---

int nal_socket(int domain, int type, int protocol) {
  if (!nal_initialized)
    nal_init_subsystem();

  if (domain != AF_NEURAL)
    return -1;

  for (int i = 0; i < MAX_NEURAL_SOCKETS; i++) {
    if (!socket_table[i].used) {
      socket_table[i].used = 1;
      socket_table[i].id = i + 100; // Fake FD
      socket_table[i].protocol = protocol;
      socket_table[i].state = 0; // CLOSED/INIT
      printf("[NAL] Socket Created (FD: %d, Proto: %d)\n", socket_table[i].id,
             protocol);
      return socket_table[i].id;
    }
  }
  return -1; // Table full
}

static nal_socket_t *get_socket(int sockfd) {
  for (int i = 0; i < MAX_NEURAL_SOCKETS; i++) {
    if (socket_table[i].used && socket_table[i].id == sockfd) {
      return &socket_table[i];
    }
  }
  return NULL;
}

int nal_connect(int sockfd, const struct sockaddr_neural *addr) {
  nal_socket_t *sock = get_socket(sockfd);
  if (!sock)
    return -1;

  if (addr->sn_family != AF_NEURAL)
    return -1;

  // "Connect" to the model (Simulated binding)
  strncpy(sock->connected_model, addr->sn_model, 63);
  sock->state = 2; // CONNECTED

  printf("[NAL] Socket %d connected to Model '%s'.\n", sockfd,
         sock->connected_model);
  return 0;
}

// Forward declaration of backend
extern void vision_process_image(const void *data, size_t len,
                                 neural_result_t *out_result);

int nal_send(int sockfd, const void *buf, size_t len) {
  nal_socket_t *sock = get_socket(sockfd);
  if (!sock || sock->state != 2)
    return -1;

  // In a real system, we might queue this.
  // Here we process immediately for "Blocking Inference".

  printf("[NAL] Sending %zu bytes to Model '%s'...\n", len,
         sock->connected_model);

  if (sock->protocol == NPROTO_VISION) {
    // Dispatch to Vision Backend
    // We assume buf contains raw image data or a struct.
    // For simplicity, let's assume it's just raw bytes we pass to the vision
    // module.
    vision_process_image(buf, len, &sock->last_result);
  } else if (sock->protocol == NPROTO_SCHED) {
    extern void scheduler_predict(const void *data, size_t len,
                                  neural_result_t *out_result);
    scheduler_predict(buf, len, &sock->last_result);
  } else if (sock->protocol == NPROTO_SEC) {
    extern void firewall_predict(const void *data, size_t len,
                                 neural_result_t *out_result);
    firewall_predict(buf, len, &sock->last_result);
  } else {
    printf("[NAL] Error: Protocol %d not supported yet.\n", sock->protocol);
    return -1;
  }

  return len;
}

int nal_recv(int sockfd, void *buf, size_t len) {
  nal_socket_t *sock = get_socket(sockfd);
  if (!sock || sock->state != 2)
    return -1;

  // Return the last result
  if (len < sizeof(neural_result_t))
    return -1;

  memcpy(buf, &sock->last_result, sizeof(neural_result_t));
  return sizeof(neural_result_t);
}

int nal_close(int sockfd) {
  nal_socket_t *sock = get_socket(sockfd);
  if (!sock)
    return -1;

  sock->used = 0;
  sock->state = 0;
  printf("[NAL] Socket %d closed.\n", sockfd);
  return 0;
}
