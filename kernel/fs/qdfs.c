/*
 * NexusQ-AI Kernel - Quantum Distributed File System (QDFS)
 * File: kernel/fs/qdfs.c
 */

#include "../memory/include/sys/kalloc.h"
#include "../net/include/qtcp.h"
#include <stdio.h>
#include <string.h>

// Mock Mount Table
#define MAX_MOUNTS 4
struct qdfs_mount {
  int node_id;
  char mount_point[32];
  int active;
};

static struct qdfs_mount mounts[MAX_MOUNTS];

void qdfs_init(void) {
  for (int i = 0; i < MAX_MOUNTS; i++) {
    mounts[i].active = 0;
  }
  printf("[QDFS] Distributed File System Initialized.\n");
}

int sys_mount_qdfs(int target_node, const char *mount_point) {
  // Find free slot
  for (int i = 0; i < MAX_MOUNTS; i++) {
    if (!mounts[i].active) {
      mounts[i].node_id = target_node;
      strncpy(mounts[i].mount_point, mount_point, 31);
      mounts[i].active = 1;
      printf("[QDFS] Mounted Node %d at '%s'.\n", target_node, mount_point);
      return 0;
    }
  }
  return -1; // Table full
}

// Helper to check if a path belongs to a mount point
// Returns node_id if found, -1 otherwise.
// Updates 'relative_path' to point to the part after the mount point.
int qdfs_resolve_mount(const char *path, const char **relative_path) {
  for (int i = 0; i < MAX_MOUNTS; i++) {
    if (mounts[i].active) {
      int len = strlen(mounts[i].mount_point);
      if (strncmp(path, mounts[i].mount_point, len) == 0) {
        // Match found. Check if it's exact match or subdirectory
        if (path[len] == '/' || path[len] == '\0') {
          *relative_path = path + len;
          if (*relative_path[0] == '/')
            (*relative_path)++; // Skip leading slash
          return mounts[i].node_id;
        }
      }
    }
  }
  return -1;
}

// Read file from remote node via Q-TCP
int qdfs_read_remote(int node_id, const char *filename, char *buffer,
                     int size) {
  printf("[QDFS] Requesting '%s' from Node %d...\n", filename, node_id);

  // 1. Connect to QNet Daemon on Remote Node
  // We use the correct API from qtcp.h

  // IP is localhost for simulation, Port is 5000 + node_id
  int port = 5000 + node_id;
  qtcp_socket_t *sock =
      qtcp_connect("127.0.0.1", port, (qnet_node_id_t)node_id);

  if (!sock) {
    printf("[QDFS] Error: Connection failed.\n");
    return -1;
  }

  // Prepare Request
  char request[128];
  // Format: "READ <filename>"
  snprintf(request, 128, "READ %s", filename);

  qtcp_send(sock, request, strlen(request) + 1); // +1 for null terminator

  // Receive Response
  int bytes = qtcp_recv(sock, buffer, size);
  if (bytes < 0) {
    printf("[QDFS] Error: Read failed.\n");
  } else {
    printf("[QDFS] Received %d bytes from Node %d.\n", bytes, node_id);
  }

  qtcp_close(sock);
  return bytes;
}
