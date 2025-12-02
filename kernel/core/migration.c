/*
 * NexusQ-AI Kernel - Process Migration (Teleportation)
 * File: kernel/core/migration.c
 */

#include "../../memory/include/sys/qproc.h"
#include "../../memory/include/sys/scheduler.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// --- Forward Declarations from Network Layer ---
typedef struct {
  int tcp_fd;
  int target_node;
  uint8_t session_key[32];
  int is_secure;
} qtcp_socket_t;

extern qtcp_socket_t *qtcp_connect(const char *ip, int port, int target_node);
extern int qtcp_send(qtcp_socket_t *sock, const void *data, int len);
extern void qtcp_close(qtcp_socket_t *sock);
extern int qnet_teleport_state(int qubit_id, int pair_id);

// --- Forward Declarations from Kernel ---
extern struct qproc *scheduler_get_process(int pid);
extern void scheduler_kill_process(int pid);

// --- Implementation ---

int sys_teleport_process(int pid, int target_node) {
  printf("[KERNEL] Initiating Teleportation for PID %d to Node %d...\n", pid,
         target_node);

  // 1. Find Process
  struct qproc *proc = scheduler_get_process(pid);
  if (!proc) {
    printf("[KERNEL] Error: PID %d not found.\n", pid);
    return -1;
  }

  // 2. Suspend Process (Simulated by just not scheduling it, or we assume it's
  // running) In a real OS, we would remove it from the runqueue here.
  printf("[KERNEL] Suspending PID %d (State: %d)...\n", pid, proc->q_state);

  // 3. Connect to Target Node via Q-TCP
  // Assuming Node 2 is at 127.0.0.1:5002 for simulation
  char target_ip[16];
  sprintf(target_ip, "127.0.0.1");
  int target_port = 5000 + target_node;

  qtcp_socket_t *sock = qtcp_connect(target_ip, target_port, target_node);
  if (!sock) {
    printf("[KERNEL] Error: Failed to establish Q-TCP connection to Node %d.\n",
           target_node);
    return -2;
  }

  // 4. Teleport Quantum State (Qubits)
  // Iterate over allocated registers and teleport "virtual" qubits
  printf("[KERNEL] Teleporting Quantum Registers...\n");
  for (int i = 0; i < proc->reg_count; i++) {
    int reg_id = proc->allocated_regs[i];
    // Simulate teleporting the qubits associated with this register
    // We assume 1 register = 1 logical qubit block for this demo
    // We use a mock pair_id based on the register
    int pair_id = 100 + (i % 10);

    if (qnet_teleport_state(reg_id, pair_id) != 0) {
      printf("[KERNEL] Warning: Failed to teleport Register %d.\n", reg_id);
    }
  }

  // 5. Transfer Classical State (The Struct)
  printf("[KERNEL] Serializing Classical State (%lu bytes)...\n",
         sizeof(struct qproc));
  if (qtcp_send(sock, proc, sizeof(struct qproc)) > 0) {
    printf("[KERNEL] Classical State Transferred Successfully.\n");
  } else {
    printf("[KERNEL] Error: Failed to send Classical State.\n");
    qtcp_close(sock);
    return -3;
  }

  // 6. Cleanup
  qtcp_close(sock);

  // 7. Terminate Local Instance
  // We kill the local process effectively "moving" it.
  printf("[KERNEL] Terminating Local Instance of PID %d...\n", pid);
  scheduler_kill_process(pid);

  printf("[KERNEL] Teleportation Complete. Process %d is now on Node %d.\n",
         pid, target_node);
  return 0;
}
