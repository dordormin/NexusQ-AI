/*
 * NexusQ-AI Governance Test Suite
 * Simulates User Space -> Kernel Space transitions via Syscalls
 */

#include "../kernel/memory/include/sys/kalloc.h"
#include "../kernel/memory/include/sys/ledgerfs.h"
#include "../kernel/memory/include/sys/qproc.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Mocking the syscall table access (since we are not in a real OS environment
// with interrupts)
extern void *syscall_table[];
extern int num_syscalls;

typedef int (*sys_create_qproc_t)(char *, int);
typedef int (*sys_verify_ledger_t)(char *);

void test_governance_flow() {
  printf("=== Testing Governance Architecture ===\n");

  // 1. Initialize Subsystems
  lfs_init();
  kalloc_init();

  // 2. Create a file in LedgerFS (Kernel Side)
  const char *filename = "contract.qasm";
  const char *content = "H q0; CX q0, q1;";
  lfs_create_file(filename, (uint8_t *)content, strlen(content), NULL);

  // 3. Simulate User Space Call: sys_verify_ledger (Syscall #2)
  printf("\n[User] Requesting Ledger Verification for '%s'...\n", filename);

  // Manual Dispatch (Simulating INT 0x80)
  int syscall_id = 2;
  if (syscall_id >= num_syscalls) {
    printf("Security Violation: Invalid Syscall\n");
    return;
  }

  sys_verify_ledger_t sys_verify =
      (sys_verify_ledger_t)syscall_table[syscall_id];
  int result = sys_verify((char *)filename);

  if (result == 0) {
    printf("[User] Result: Verified (Integrity OK)\n");
  } else {
    printf("[User] Result: Verification FAILED\n");
  }
  assert(result == 0);

  // 4. Simulate User Space Call: sys_create_qproc (Syscall #0)
  printf("\n[User] Requesting Quantum Process Creation...\n");
  sys_create_qproc_t sys_create = (sys_create_qproc_t)syscall_table[0];
  int pid = sys_create("quantum_app", 5);
  printf("[User] Result: PID %d created\n", pid);
  assert(pid >= 100);

  // 5. Simulate User Space Call: sys_allocate_qmemory (Syscall #1)
  printf("\n[User] Requesting Quantum Memory (3 Qubits)...\n");
  typedef int (*sys_allocate_qmemory_t)(int);
  sys_allocate_qmemory_t sys_alloc = (sys_allocate_qmemory_t)syscall_table[1];

  // Test Valid Allocation
  int q_addr = sys_alloc(3);
  printf("[User] Result: Allocated at QPU Address %d\n", q_addr);
  assert(q_addr >= 0);

  // Test Governance (Oversized Request)
  printf("\n[User] Requesting Massive Quantum Memory (200 Qubits)...\n");
  int error = sys_alloc(200);
  printf("[User] Result: %d (Expected -1)\n", error);
  assert(error == -1);

  printf("\n=== Governance Tests Passed ===\n");
}

int main() {
  test_governance_flow();
  return 0;
}
