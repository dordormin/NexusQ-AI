/*
 * NexusQ-AI Kernel - Neuro-Scheduler Implementation
 * File: kernel/core/scheduler.c
 */

#include "../memory/include/sys/scheduler.h"
#include "../../include/sys/config.h"
#include "../../modules/neural/include/neural.h"
#include "../memory/include/sys/qec.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward Declaration
void sched_init_nal(void);

// Simulation d'une RunQueue simple (Tableau circulaire pour ce prototype)
// #define MAX_RUNNING_PROCS 64 // Defined in config.h

// --- Global State ---
struct qproc *run_queue = NULL;
struct qproc *current_process = NULL; // Removed static to expose to syscalls
double system_time = 0.0;
static struct qproc *runqueue[MAX_RUNNING_PROCS];
static int proc_count = 0;
static int current_index = 0; // Round-Robin pointer par défaut

void sched_init(void) {
  for (int i = 0; i < MAX_RUNNING_PROCS; i++)
    runqueue[i] = NULL;
  proc_count = 0;
  neural_init();    // Boot the Brain
  sched_init_nal(); // Connect to NAL
  printf("[SCHED] Neuro-Scheduler Initialized. Quantum Threshold: %.1f us\n",
         COHERENCE_THRESHOLD);
}

void sched_submit(struct qproc *p) {
  if (proc_count >= MAX_RUNNING_PROCS) {
    printf("[SCHED] Error: RunQueue Full!\n");
    return;
  }
  // Recherche d'un slot libre
  for (int i = 0; i < MAX_RUNNING_PROCS; i++) {
    if (runqueue[i] == NULL) {
      runqueue[i] = p;
      proc_count++;
      p->q_state = QSTATE_IDLE; // Prêt à partir
      printf("[SCHED] Submitted PID %d (%s) to RunQueue.\n", p->pid, p->name);
      return;
    }
  }
}

/**
 * L'Oracle (Stub pour l'inférence NPU)
 * Calcule la priorité dynamique basée sur l'état quantique et l'historique IA.
 */
#include "../../kernel/neural/include/sys/neural.h"

// NAL Socket for Scheduler
static int sched_nal_sock = -1;

void sched_init_nal() {
  if (sched_nal_sock >= 0)
    return;

  sched_nal_sock = nal_socket(AF_NEURAL, SOCK_INFER, NPROTO_SCHED);
  if (sched_nal_sock >= 0) {
    struct sockaddr_neural addr;
    addr.sn_family = AF_NEURAL;
    strncpy(addr.sn_model, "scheduler_v1", 63);
    nal_connect(sched_nal_sock, &addr);
    printf("[SCHED] Connected to Neural Predictor (Sock: %d)\n",
           sched_nal_sock);
  }
}

/**
 * L'Oracle (Stub pour l'inférence NPU)
 * Calcule la priorité dynamique basée sur l'état quantique et l'historique IA.
 */
int sched_predict_priority(struct qproc *p) {
  // 1. Règle Absolue : Survie Quantique
  if (p->num_qubits > 0) {
    if (p->t_coherence < COHERENCE_THRESHOLD) {
      return PRIO_QUANTUM_CRITICAL;
    }
    if (p->q_state == QSTATE_RUNNING) {
      return PRIO_REALTIME;
    }
  }

  // 2. Règle IA : Prédiction via NAL
  if (sched_nal_sock >= 0) {
    // Prepare Input
    struct {
      int pid;
      int priority;
      long cpu_usage_ms;
    } input;
    input.pid = p->pid;
    input.priority = PRIO_NORMAL; // Simplified
    input.cpu_usage_ms = 500;     // Fake usage for demo

    // Query NAL
    nal_send(sched_nal_sock, &input, sizeof(input));

    neural_result_t res;
    if (nal_recv(sched_nal_sock, &res, sizeof(res)) > 0) {
      // printf("[SCHED] PID %d: Neural Predictor recommends %.2f (%s)\n",
      // p->pid, res.confidence, res.label);

      // Apply Recommendation
      if (res.confidence > 1.2f)
        return PRIO_NORMAL - 20; // Boost
      if (res.confidence < 0.9f)
        return PRIO_NORMAL + 10; // Throttle
    }
  }

  // Fallback (Legacy)
  float prediction = neural_predict_burst(p->pid, 0.5f);
  p->burst_prediction = prediction;
  if (prediction < 0.3f) {
    return PRIO_NORMAL - 10;
  }

  // Nice Value Adjustment
  return PRIO_NORMAL + p->nice_value;
}

/**
 * Le Tick d'Horloge (Heartbeat)
 * C'est ici que NexusQ est différent de Linux.
 */
struct qproc *sched_tick(double delta_time_us) {
  struct qproc *best_candidate = NULL;
  int best_prio = 9999;

  // 1. Mise à jour de la physique (Décohérence) pour TOUS les processus
  for (int i = 0; i < MAX_RUNNING_PROCS; i++) {
    struct qproc *p = runqueue[i];
    if (!p)
      continue;

    // Si c'est un process quantique, le temps joue contre lui
    if (p->num_qubits > 0) {
      // Appel au Module 0 (qproc) pour mettre à jour T1/T2
      qproc_update_coherence(p, delta_time_us);

      // Appel au Module 35 (Neural QEC) pour corriger les erreurs
      extern void qec_neural_correct(struct qproc * p);
      qec_neural_correct(p);

      // Si le process est mort de décohérence, on le tue
      if (p->q_state == QSTATE_DECOHERED) {
        printf("[SCHED] KILL PID %d (Decoherence Death)\n", p->pid);
        // Dans un vrai OS: signal SIGSEGV, dump core, free
        runqueue[i] = NULL;
        proc_count--;
        continue;
      }
    }
  }

  // 2. Élection du prochain Processus (Algorithme "Quantum-First")
  for (int i = 0; i < MAX_RUNNING_PROCS; i++) {
    struct qproc *p = runqueue[i];
    if (!p)
      continue;

    int prio = sched_predict_priority(p);

    // Logique de préemption stricte
    if (prio < best_prio) {
      best_prio = prio;
      best_candidate = p;
    }
  }

  if (best_candidate) {
    // Changement d'état
    // Changement d'état
    best_candidate->q_state = QSTATE_RUNNING;
    best_candidate->cpu_ticks++; // Increment CPU Usage

    // Log spécial si c'est une urgence
    if (best_prio == PRIO_QUANTUM_CRITICAL) {
      printf("[SCHED] !!! QUANTUM PANIC !!! Scheduling PID %d (TTL: %.1f us)\n",
             best_candidate->pid, best_candidate->t_coherence);
    }

    // Deep Integration: Quantum Context Switch
    // Only if switching to a new process (optimization)
    if (best_candidate != current_process) {
      // extern void qpu_load_context(int pid, int active_reg);
      // qpu_load_context(best_candidate->pid, best_candidate->active_reg);

      // ASM Optimized Version
      extern void qpu_load_context_asm(int pid, int active_reg);
      qpu_load_context_asm(best_candidate->pid, best_candidate->active_reg);

      current_process = best_candidate;
    }
  }

  // 3. Background Learning (Le Rêve de la Machine)
  // À chaque tick, le NPU apprend un peu.
  neural_train_step();

  return best_candidate;
}

void sched_dump_queue(void) {
  printf("--- RunQueue [%d procs] ---\n", proc_count);
  for (int i = 0; i < MAX_RUNNING_PROCS; i++) {
    if (runqueue[i]) {
      printf("[%d] PID %d | TTL: %.1f | Prio: %d\n", i, runqueue[i]->pid,
             runqueue[i]->t_coherence, sched_predict_priority(runqueue[i]));
    }
  }
  printf("--------------------------\n");
  printf("--------------------------\n");
}

struct qproc *scheduler_get_process(int pid) {
  for (int i = 0; i < MAX_RUNNING_PROCS; i++) {
    if (runqueue[i] && runqueue[i]->pid == pid) {
      return runqueue[i];
    }
  }
  return NULL;
}

void scheduler_kill_process(int pid) {
  for (int i = 0; i < MAX_RUNNING_PROCS; i++) {
    if (runqueue[i] && runqueue[i]->pid == pid) {
      printf("[SCHED] Killed PID %d.\n", pid);
      // Remove from global list and free memory
      extern void qproc_destroy(int pid);
      qproc_destroy(pid);

      runqueue[i] = NULL;
      proc_count--;
      return;
    }
  }
  printf("[SCHED] Warning: PID %d not found to kill.\n", pid);
}

void scheduler_renice_process(int pid, int new_priority) {
  for (int i = 0; i < MAX_RUNNING_PROCS; i++) {
    if (runqueue[i] && runqueue[i]->pid == pid) {
      runqueue[i]->nice_value = new_priority;
      printf("[SCHED] Renice PID %d to %d.\n", pid, new_priority);
      return;
    }
  }
  printf("[SCHED] Warning: PID %d not found to renice.\n", pid);
}

// --- Level 33: Applied Quantum Optimization ---

void sched_optimize_load_balance(void) {
  if (proc_count < 2) {
    printf("[SCHED] Not enough processes to optimize (%d).\n", proc_count);
    return;
  }

  printf("[SCHED] Balancing Load using QAOA (Quantum Approximate "
         "Optimization)...\n");

  // 1. Build Dependency Graph (Adjacency Matrix)
  // For this demo, we assume processes with close PIDs interact more.
  // In a real OS, we'd check IPC channels or shared memory.
  int num_nodes = proc_count;
  int *adj = malloc(num_nodes * num_nodes * sizeof(int));

  // Map runqueue index to graph node index
  struct qproc *nodes[MAX_RUNNING_PROCS];
  int node_idx = 0;
  for (int i = 0; i < MAX_RUNNING_PROCS; i++) {
    if (runqueue[i])
      nodes[node_idx++] = runqueue[i];
  }

  // Fill Matrix (Random + Heuristic)
  for (int i = 0; i < num_nodes * num_nodes; i++)
    adj[i] = 0;

  for (int i = 0; i < num_nodes; i++) {
    for (int j = i + 1; j < num_nodes; j++) {
      // Heuristic: If PIDs are close, they are related (e.g. parent/child)
      if (abs(nodes[i]->pid - nodes[j]->pid) < 10) {
        adj[i * num_nodes + j] = 1;
        adj[j * num_nodes + i] = 1;
      }
    }
  }

  // 2. Solve Max-Cut using QAOA
  extern int qaoa_solve_maxcut(int *adj_matrix, int num_nodes);
  int partition = qaoa_solve_maxcut(adj, num_nodes);

  // 3. Apply Partition (Migration)
  printf("[SCHED] Applying Optimal Partition (Bitstring: %x)...\n", partition);
  for (int i = 0; i < num_nodes; i++) {
    int core_id = (partition >> i) & 1;
    // In a real OS, we would change p->affinity or migrate the task struct.
    // Here we just log the decision.
    char *core_name = core_id ? "Core B (Quantum)" : "Core A (Classical)";
    printf("[SCHED] Process %d (%s) assigned to %s.\n", nodes[i]->pid,
           nodes[i]->name, core_name);
  }

  free(adj);
}

// --- Level 34: Quantum Superposition Concurrency ---

// Create 4 entangled branches of the current process
void sched_quantum_fork(void) {
  if (!current_process)
    return;

  printf("[SCHED] Process %d entered Superposition (4 Branches).\n",
         current_process->pid);

  // Convert current process to Branch 0
  current_process->q_state = QSTATE_SUPERPOSED;
  current_process->branch_id = 0;
  current_process->amplitude = 0.5f; // Sqrt(0.25) = 0.5

  // Create 3 clones (Branches 1, 2, 3)
  for (int i = 1; i < 4; i++) {
    // In a real OS, we'd copy the task struct and memory space (COW).
    // Here we just allocate a new qproc and link it.
    extern struct qproc *qproc_create(const char *name, int qubits);
    struct qproc *branch =
        qproc_create(current_process->name, current_process->num_qubits);

    branch->pid = current_process->pid + i * 1000; // Mock PID for branch
    branch->q_state = QSTATE_SUPERPOSED;
    branch->branch_id = i;
    branch->amplitude = 0.5f;
    branch->t_coherence = current_process->t_coherence;

    sched_submit(branch);
  }

  // Trigger Amplification immediately for demo
  extern void sched_amplify_branches(int parent_pid);
  sched_amplify_branches(current_process->pid);
}

void sched_amplify_branches(int parent_pid) {
  printf("[SCHED] Amplifying Branches for PID %d (Grover's Algorithm)...\n",
         parent_pid);

  // 1. Identify Target (Oracle)
  // We simulate that Branch 2 is the "Winner" (found the solution).
  int winner_branch = 2;

  // 2. NAL Prediction (AI Oracle)
  // In a real system, we'd send the state of each branch to the NAL.
  printf("[NAL] Oracle Query: Which branch satisfies the condition?\n");
  printf("[NAL] Amplifying Branch %d (Target Found).\n", winner_branch);

  // 3. Collapse Superposition
  printf("[SCHED] Collapsing Superposition -> Branch %d wins.\n",
         winner_branch);

  // Kill losers, Promote winner
  // We iterate the runqueue to find our branches
  for (int i = 0; i < MAX_RUNNING_PROCS; i++) {
    struct qproc *p = runqueue[i];
    if (!p)
      continue;

    // Check if it's one of our branches (heuristic based on PID logic above)
    // Parent PID is X, branches are X, X+1000, X+2000, X+3000
    int is_branch = (p->pid == parent_pid) || (p->pid == parent_pid + 1000) ||
                    (p->pid == parent_pid + 2000) ||
                    (p->pid == parent_pid + 3000);

    if (is_branch) {
      if (p->branch_id == winner_branch) {
        p->q_state = QSTATE_RUNNING;
        p->amplitude = 1.0f;
        printf("[SCHED] Branch %d (PID %d) promoted to Classical State.\n",
               p->branch_id, p->pid);
      } else {
        printf("[SCHED] Branch %d (PID %d) collapsed (Amplitude -> 0).\n",
               p->branch_id, p->pid);
        runqueue[i] = NULL; // Kill
        proc_count--;
      }
    }
  }
}
