/*
 * NexusQ-AI Kernel - Service Manager
 * Manages background daemons and critical system services.
 * File: kernel/core/svc_manager.c
 */

#include "../memory/include/sys/qproc.h"
#include <stdio.h>
#include <string.h>

#define MAX_SERVICES 16

typedef struct {
  char name[32];
  int pid;
  int status; // 0=Stopped, 1=Running
  int auto_restart;
} service_t;

static service_t services[MAX_SERVICES];
static int svc_count = 0;

void svc_init(void) {
  svc_count = 0;
  printf("[SVC] Service Manager Initialized.\n");
}

int svc_register(const char *name, int auto_restart) {
  if (svc_count >= MAX_SERVICES)
    return -1;

  strncpy(services[svc_count].name, name, 31);
  services[svc_count].pid = -1;
  services[svc_count].status = 0;
  services[svc_count].auto_restart = auto_restart;

  printf("[SVC] Registered Service: %s\n", name);
  return svc_count++;
}

int svc_start(const char *name) {
  for (int i = 0; i < svc_count; i++) {
    if (strcmp(services[i].name, name) == 0) {
      if (services[i].status == 1)
        return 0; // Already running

      // In a real OS, we would load the binary.
      // Here we spawn a qproc with a special flag.
      extern struct qproc *qproc_create(const char *name, int qubits);
      struct qproc *p = qproc_create(name, 4); // Default 4 qubits for services
      if (p) {
        services[i].pid = p->pid;
        services[i].status = 1;
        printf("[SVC] Started Service '%s' (PID %d)\n", name, p->pid);
        return p->pid;
      }
    }
  }
  return -1;
}

void svc_register_system_services(void) {
  // Register Core Modules as Services
  svc_register("QNet Daemon", 1);
  svc_register("QEC Engine", 1);
  svc_register("LedgerFS Monitor", 1);
  svc_register("Neural Sentinel", 1);

  // Mark them as running (since they are init'd in kmain)
  for (int i = 0; i < svc_count; i++) {
    services[i].status = 1;
    services[i].pid = 100 + i; // Fake PID
  }
}

int svc_list_info(char *buffer, int max_len) {
  int offset = 0;
  offset +=
      snprintf(buffer + offset, max_len - offset, "--- System Services ---\n");
  for (int i = 0; i < svc_count; i++) {
    offset +=
        snprintf(buffer + offset, max_len - offset,
                 "[%d] %-20s | PID: %d | Status: %s\n", i, services[i].name,
                 services[i].pid, services[i].status ? "RUNNING" : "STOPPED");
  }
  return offset;
}

void svc_check_health(void) {
  // Called by Scheduler Tick
  // Check if services are still alive. If not, restart if auto_restart is set.
  // (Mock implementation: we assume they stay alive unless killed)
}
