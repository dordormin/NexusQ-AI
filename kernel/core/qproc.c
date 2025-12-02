#include "../memory/include/sys/qproc.h"
#include "../memory/include/sys/kalloc.h" // For kmalloc if needed, or malloc for now
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct qproc *qproc_list = NULL;

struct qproc *qproc_create(const char *name, int qubits) {
  struct qproc *p = (struct qproc *)malloc(sizeof(struct qproc));
  if (!p)
    return NULL;

  static int next_pid = 100;
  p->pid = next_pid++;
  strncpy(p->name, name, 31);
  p->num_qubits = qubits;
  p->t_coherence = 10000.0; // Increased for stability (was 100us)
  p->q_state = QSTATE_IDLE;
  p->burst_prediction = 0.5f;

  // Init Resources
  p->reg_count = 0;
  p->active_reg = -1;
  for (int i = 0; i < MAX_Q_REGS; i++)
    p->allocated_regs[i] = -1;

  // Default Owner (System)
  // Dans un vrai OS, cela viendrait de l'utilisateur qui lance le processus.
  strncpy(p->owner_pubkey, "SYSTEM_ROOT_KEY_0000000000000000", 63);

  // Link to global list
  p->next = qproc_list;
  qproc_list = p;

  return p;
}

void qproc_update_coherence(struct qproc *p, double delta_time) {
  if (p->q_state == QSTATE_RUNNING || p->num_qubits > 0) {
    p->t_coherence -= delta_time;
    if (p->t_coherence <= 0) {
      p->t_coherence = 0;
      p->q_state = QSTATE_DECOHERED;
    }
  }
}

void qproc_destroy(int pid) {
  struct qproc *current = qproc_list;
  struct qproc *prev = NULL;

  while (current) {
    if (current->pid == pid) {
      if (prev) {
        prev->next = current->next;
      } else {
        qproc_list = current->next;
      }
      free(current);
      return;
    }
    prev = current;
    current = current->next;
  }
}
