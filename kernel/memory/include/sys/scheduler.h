/*
 * NexusQ-AI Kernel - Neuro-Quantum Scheduler Interface
 * File: include/sys/scheduler.h
 */

#ifndef _SYS_SCHEDULER_H_
#define _SYS_SCHEDULER_H_

#include "../../../../include/sys/config.h"
#include "qproc.h"

// --- Niveaux de Priorité (Ring -1 à Ring 3) ---
#define PRIO_QUANTUM_CRITICAL                                                  \
  0                      // Urgence Absolue (T < 5µs) - Preempts everything
#define PRIO_REALTIME 10 // Kernel RT tasks
#define PRIO_NORMAL 50   // User apps
#define PRIO_IDLE 100    // Background AI Training

// --- Configuration ---
// Defined in sys/config.h
// #define QUANTUM_TIME_SLICE 10
// #define COHERENCE_THRESHOLD 20.0

// --- API Publique ---

// Initialisation du sous-système
void sched_init(void);

// Ajout d'un processus à la RunQueue
void sched_submit(struct qproc *p);

// Le Cœur : Appelé à chaque interruption d'horloge (Tick)
// Retourne le PID du processus qui DOIT s'exécuter maintenant.
struct qproc *sched_tick(double delta_time_us);

// Fonction de décision (L'IA simulée)
int sched_predict_priority(struct qproc *p);

// Debug
void sched_dump_queue(void);

// Process Management
struct qproc *scheduler_get_process(int pid);
void scheduler_kill_process(int pid);

#endif // _SYS_SCHEDULER_H_
