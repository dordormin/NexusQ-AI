/*
 * NexusQ-AI Kernel Entry Point
 * File: kernel/core/kmain.c
 */

#include "../../include/sys/qnet.h"
#include "../memory/include/sys/kalloc.h"
#include "../memory/include/sys/ledgerfs.h"
#include "../memory/include/sys/qec.h"
#include "../memory/include/sys/scheduler.h"
#include <stdio.h>
#include <unistd.h>

// External init functions
extern void svc_init(void);
extern void audit_init(void);

void kmain(void) {
  printf("\n");
  printf("   _  __                    ___       ___ \n");
  printf("  / |/ /____  __ __ __ __  / _ \\___  / _ |\n");
  printf(" /    // -_)| \\ // // // / / // // _ \\/ __ |\n");
  printf("/_/|_/ \\__/ /_\\_\\\\_,_//_/  \\___\\\\___/_/ |_|\n");
  printf("NexusQ-AI Kernel v1.0 | Quantum-Native OS\n");
  printf("-----------------------------------------\n\n");

  printf("[BOOT] Starting Kernel Initialization...\n");

  // Initialize kernel logging
  extern void klog_init(void);
  klog_init();

  // 1. Memory Management (KAlloc)
  printf("[BOOT] Initializing Memory Subsystem...\n");
  kalloc_init();

  // 2. File System (LedgerFS)
  printf("[BOOT] Mounting LedgerFS...\n");
  lfs_init();

  // 3. Quantum Error Correction (QEC)
  printf("[BOOT] Calibrating QEC Engine...\n");
  qec_init();

  // 4. Quantum Network (QNet)
  printf("[BOOT] Bringing up Quantum Network Stack...\n");
  qnet_init();

  // 5. System Services & Audit
  printf("[BOOT] Starting System Services...\n");
  svc_init();
  extern void svc_register_system_services(void);
  svc_register_system_services();
  audit_init();

  // 6. Scheduler (The Brain)
  // Note: sched_init() initializes the Neural Engine and connects to NAL.
  printf("[BOOT] Awakening Neuro-Scheduler...\n");
  sched_init();

  printf("[BOOT] Kernel Initialization Complete.\n");
  printf("[BOOT] Handing over to Init Process (Shell)...\n");
  fflush(stdout);

  // Launch the Shell
  char *args[] = {"./nexus_shell", NULL};
  execv(args[0], args);

  // If execv returns, it failed
  perror("[KERNEL PANIC] Failed to launch init process");
  // Fallback loop
  while (1) {
    sleep(1);
  }
}

int main() {
  kmain();
  return 0;
}
