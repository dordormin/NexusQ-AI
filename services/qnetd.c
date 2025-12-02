/*
 * NexusQ-AI Service - QNet Daemon (qnetd)
 * Manages Quantum Network connections and entanglement maintenance.
 * File: services/qnetd.c
 */

#include "../lib/include/nexus.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// Mock QProc structure for deserialization
struct qproc_mock {
  int pid;
  char name[32];
  int priority;
  int num_qubits;
  int q_state;
  // ... other fields ignored for mock reception
};

int main(int argc, char *argv[]) {
  int node_id = 1;
  if (argc > 1) {
    node_id = atoi(argv[1]);
  }

  printf("[qnetd] Daemon Started (PID: %d) as Node %d\n", getpid(), node_id);

  // 1. Initialize Network Listener (Classical Channel for Q-TCP)
  int port = 5000 + node_id;
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    perror("[qnetd] Socket failed");
    return 1;
  }

  struct sockaddr_in address;
  int opt = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("[qnetd] Bind failed");
    return 1;
  }

  if (listen(server_fd, 3) < 0) {
    perror("[qnetd] Listen failed");
    return 1;
  }

  printf("[qnetd] Listening for Q-TCP connections on port %d...\n", port);

  while (1) {
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    int new_socket =
        accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);

    if (new_socket < 0) {
      perror("[qnetd] Accept failed");
      continue;
    }

    printf("[qnetd] Incoming Connection from %s\n",
           inet_ntoa(client_addr.sin_addr));

    // 2. Receive Teleported Process Data
    // In a real implementation, we would perform the QKD handshake here first.
    // For this test, we assume the kernel handled the handshake and we just get
    // the data. (Actually, our qtcp implementation in kernel does the
    // handshake, but here we are in user space acting as the receiver. We
    // should simulate the handshake or just read the data).

    // Let's just read the raw struct for this demo.
    struct qproc_mock received_proc;
    int valread = read(new_socket, &received_proc, sizeof(received_proc));

    if (valread > 0) {
      // Check Request Type
      if (strncmp((char *)&received_proc, "FORWARD ", 8) == 0) {
        // Forwarding Logic
        int target_node;
        char msg[256];
        sscanf((char *)&received_proc, "FORWARD %d %[^\n]", &target_node, msg);

        printf("[qnetd] Routing: Forwarding message to Node %d...\n",
               target_node);

        // Send to Target
        int target_port = 5000 + target_node;
        int fwd_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (fwd_sock >= 0) {
          struct sockaddr_in target_addr;
          target_addr.sin_family = AF_INET;
          target_addr.sin_port = htons(target_port);
          inet_pton(AF_INET, "127.0.0.1", &target_addr.sin_addr);

          if (connect(fwd_sock, (struct sockaddr *)&target_addr,
                      sizeof(target_addr)) >= 0) {
            send(fwd_sock, msg, strlen(msg) + 1, 0);
            printf("[qnetd] Forwarded successfully.\n");
          } else {
            perror("[qnetd] Forwarding failed");
          }
          close(fwd_sock);
        }
      } else if (strncmp((char *)&received_proc, "READ ", 5) == 0) {
        // File Read Request
        char *filename = ((char *)&received_proc) + 5;
        printf("[qnetd] READ Request for '%s'\n", filename);

        char response[256];
        if (strcmp(filename, "secret.txt") == 0) {
          snprintf(response, 256, "This is a secret from Node %d!", node_id);
        } else {
          snprintf(response, 256, "File not found on Node %d.", node_id);
        }

        send(new_socket, response, strlen(response) + 1, 0);

      } else {
        // Teleport Request (Default)
        printf("[qnetd] RECEIVED TELEPORTED PROCESS!\n");
        printf("        PID: %d\n", received_proc.pid);
        printf("        Name: %s\n", received_proc.name);
        printf("        Qubits: %d\n", received_proc.num_qubits);
        printf("[qnetd] Process Resurrected on Node %d.\n", node_id);
      }
    }

    close(new_socket);
  }

  return 0;
}
