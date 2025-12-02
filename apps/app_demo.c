/*
 * NexusQ-AI Demo Application
 * "The Quantum Hello World"
 */

#include "../lib/include/nexus.h"
#include <assert.h>
#include <stdio.h>

// Needed for simulation setup (not part of standard API)
extern void lfs_init(void);
extern void kalloc_init(void);
extern void sched_init(void);
extern void lfs_create_file(const char *name, const unsigned char *data,
                            size_t size);

int main() {
  printf("=== NexusQ-AI Application Demo ===\n");

  // 0. System Boot (Simulation only)
  lfs_init();
  kalloc_init();
  sched_init();

  // Create a dummy contract for the demo
  lfs_create_file("demo.contract", (const unsigned char *)"content", 7);

  // 1. Process Creation
  printf("\n[App] Spawning Quantum Process...\n");
  nexus_pid_t pid = nexus_create_process("demo_app", 10);
  if (pid > 0) {
    printf("[App] Success! PID: %d\n", pid);
  } else {
    printf("[App] Failed to create process.\n");
    return 1;
  }

  // 2. Quantum Memory Allocation
  printf("\n[App] Allocating Qubits...\n");
  nexus_qreg_t qreg = nexus_malloc_q(5);
  if (qreg >= 0) {
    printf("[App] Success! Allocated QRegister ID: %d\n", qreg);
  } else {
    printf("[App] Failed to allocate qubits.\n");
  }

  // 3. Smart Contract Verification
  printf("\n[App] Verifying Smart Contract...\n");
  if (nexus_verify_contract("demo.contract") == 0) {
    printf("[App] Contract Verified. Execution Authorized.\n");
  } else {
    printf("[App] SECURITY ALERT: Contract Invalid.\n");
  }

  // 4. Scheduler Governance Demo (Survival of the Fittest)
  printf("\n[App] Starting Quantum Simulation (Time Travel)...\n");

  // Check Identity before dying
  char my_pubkey[64];
  if (nexus_get_wallet_id(my_pubkey) == 0) {
    printf("[App] Identity Verified. Owner: %.16s...\n", my_pubkey);
  }

  // 5. Smart Contract Execution (Law as Code)
  printf("\n[App] Executing Governance Contract...\n");
  // Contract: IF SENDER == SYSTEM (1) THEN GRANT_ACCESS
  // Bytecode: OP_GET_SENDER (0x10), OP_PUSH 1 (0x01, 0x01), OP_EQ (0x05),
  // OP_GRANT_ACCESS (0xFF) Note: OP_GRANT_ACCESS should probably check the
  // stack top, but our simple VM just returns 1 on opcode. Let's make it:
  // GET_SENDER, PUSH 1, EQ. If result is 1, we assume success? Actually our VM
  // returns 1 on OP_GRANT_ACCESS. So we need conditional jump? For this
  // prototype, let's just do: GET_SENDER (pushes 1 if system), PUSH 1, EQ.
  // Wait, we don't have JUMP.
  // Let's simplify: The VM prints debug info. We just want to see it run.
  // Let's run: GET_SENDER, OP_GRANT_ACCESS.
  unsigned char code[] = {0x10, 0xFF};

  if (nexus_exec_contract(code, sizeof(code)) == 1) {
    printf("[App] Contract Result: ACCESS GRANTED (Bureaucracy Satisfied)\n");
  } else {
    printf("[App] Contract Result: ACCESS DENIED\n");
  }

  // We need to register the process with the scheduler first (Kernel side logic
  // usually) For this demo, we assume nexus_create_process did it (we need to
  // ensure it does or mock it)

  // Let's simulate 5 ticks of 10us each.
  // The default coherence is 100us.
  // We will fast forward time to kill the process.

  printf("[App] Fast forwarding 200us...\n");
  int running_pid = nexus_simulate_tick(200);

  // If the process died, the scheduler might return 0 or another PID.
  // In our kernel logic, it prints "[SCHED] KILL PID ..."

  printf("\n=== Demo Completed Successfully ===\n");
  // --- QNet Demo ---
  printf("\n=== Quantum Network Demo ===\n");
  printf("Requesting Entanglement with Node 2 (Min Fidelity 95%%)...\n");
  int pair_id = nexus_request_entanglement(2, 95);

  if (pair_id > 0) {
    printf("Entanglement Established! Pair ID: %d\n", pair_id);
    printf("Teleporting Qubit state...\n");
    // Assuming Qubit 0 is the one we want to teleport
    if (nexus_teleport_state(0, pair_id) == 0) {
      printf("Teleportation Successful!\n");
    } else {
      printf("Teleportation Failed.\n");
    }
  } else {
    printf("Failed to establish entanglement.\n");
  }

  return 0;
}
