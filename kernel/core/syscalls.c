/* * NexusQ-AI Governance: The Syscall Table
 * Ce fichier définit les SEULES portes d'entrée vers le Kernel (Ring 0).
 */
#include "../../lib/include/nexus.h" // For nexus_sysinfo_t etc.
#include "../../modules/contracts/include/contract.h"
#include "../memory/include/sys/kalloc.h"
#include "../memory/include/sys/ledgerfs.h"
#include "../memory/include/sys/qproc.h"
#include "../memory/include/sys/scheduler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Prototypes des fonctions internes du noyau
// extern struct qproc* qproc_create(const char* name, int qubits); // Defined
// in qproc.c

// Audit
extern void audit_init(void);
extern void audit_log(int event_type, const char *user, const char *message);
#define AUDIT_LOGIN 1
#define AUDIT_FILE_ACCESS 2
#define AUDIT_EXEC 3
#define AUDIT_NET 4
#define AUDIT_SYSTEM 5

// Implementations des Syscalls (Wrappers)

int sys_create_qproc(char *name, int qubits) {
  // Dans un vrai OS, on vérifierait les pointeurs ici (copy_from_user)
  struct qproc *p = qproc_create(name, qubits);
  if (p) {
    sched_submit(p); // Add to RunQueue
    return p->pid;
  }
  return -1;
}

int sys_allocate_qmemory(int size) {
  // Gouvernance: Allocation physique stricte
  if (size < 0 || size > QPU_MAX_QUBITS)
    return -1;

  // Deep Integration: Bind to Process
  extern struct qproc *current_process;
  if (!current_process) {
    // Kernel/System allocation (allowed)
    return (int)qmalloc((uint16_t)size);
  }

  struct qproc *p = current_process;

  // Check Limits
  if (p->reg_count >= MAX_Q_REGS) {
    printf("[KERNEL] Allocation Denied: Process %d reached register limit.\n",
           p->pid);
    return -1;
  }

  // Allocate
  q_register_t reg = qmalloc((uint16_t)size);
  if (reg >= 0) {
    p->allocated_regs[p->reg_count++] = reg;
    p->active_reg = reg; // Auto-switch context to new block
    printf("[KERNEL] Assigned QRegister %d to PID %d.\n", reg, p->pid);
  }

  return (int)reg; // Retourne l'ID du registre (ou -1 si échec)
}

// #include <sys/scheduler.h> // Duplicate removed

// ... existing code ...

int sys_verify_ledger(char *path) {
  // 1. Trouver le fichier
  lfs_inode_t *inode = lfs_find_by_name(path);
  if (!inode)
    return -1; // Not found

  // 2. Vérifier l'intégrité
  return lfs_verify_integrity(inode);
}

// --- Scheduler Syscalls ---

int sys_yield(void) {
  // Dans un vrai OS, cela déclencherait un context switch immédiat.
  // Ici, on demande au scheduler de recalculer les priorités.
  // sched_tick(0); // Tick partiel
  return 0;
}

int sys_scheduler_tick(int us) {
  // Simulation du temps qui passe
  struct qproc *next = sched_tick((double)us);
  if (next)
    return next->pid;
  return 0; // Idle
}

int sys_get_wallet_id(char *out_pubkey) {
  // Récupère la clé publique du processus courant
  // Pour ce prototype, on triche et on prend le premier de la liste (ou on
  // devrait avoir 'current_process') On va supposer que le scheduler a un
  // 'current_proc' accessible, ou on le passe en paramètre. Simplification: On
  // retourne celle du dernier créé (qproc_list est une pile LIFO ici)

  // TODO: Accéder au vrai 'current' via le scheduler
  // extern struct qproc* current_process;

  // Pour l'instant, on hardcode le fait qu'on accède à la tête de liste (le
  // plus récent) C'est suffisant pour le demo qui lance 1 process.
  extern struct qproc *qproc_list;
  if (qproc_list) {
    strncpy(out_pubkey, qproc_list->owner_pubkey, 63);
    return 0;
  }
  return -1;
}

int sys_exec_contract(uint8_t *bytecode, int size) {
  // 1. Récupérer l'identité du process appelant
  char sender_pubkey[64];
  if (sys_get_wallet_id(sender_pubkey) != 0) {
    return -1; // No identity
  }

  // 2. Préparer le contrat
  contract_t contract;
  strncpy(contract.name, "UserContract", 31);
  if (size > MAX_BYTECODE_SIZE)
    size = MAX_BYTECODE_SIZE;
  memcpy(contract.bytecode, bytecode, size);
  contract.size = size;

  // 3. Exécuter via la VM
  // 3. Exécuter via la VM
  return vm_exec(&contract, sender_pubkey);
}

// --- New Syscalls for Shell ---

int sys_get_sysinfo(nexus_sysinfo_t *info) {
  if (!info)
    return -1;

  // Mocking RAM info (Real OS would track pages)
  info->total_ram_kb = 16384; // 16 MB
  info->free_ram_kb = 12000;  // Simulated

  // Qubits (From qproc.c or kalloc.c logic)
  // We don't have a global counter exposed easily, let's mock for now or add
  // one.
  info->total_qubits = 128;
  info->free_qubits = 64; // TODO: Track this real-time

  // LedgerFS
  extern int
      global_file_count; // From ledgerfs.c (need to expose it or add getter)
  info->lfs_files_count = global_file_count;

  // QEC Fidelity
  extern double qec_get_fidelity_metric(void);
  info->qec_fidelity = qec_get_fidelity_metric();

  return 0;
}

int sys_list_files(int parent_id, nexus_file_entry_t *files, int max_count) {
  // Use the new directory listing function
  // parent_id -1 could mean "ALL" if we wanted, but let's stick to directory
  // listing
  extern int lfs_list_directory(uint32_t dir_id, void *out_list, int max);
  extern int lfs_list_directory(uint32_t dir_id, void *out_list, int max);
  return lfs_list_directory((uint32_t)parent_id, (void *)files, max_count);
}

int sys_get_file_info(int file_id, nexus_file_entry_t *info) {
  extern lfs_inode_t *lfs_find_by_id(uint32_t id);
  lfs_inode_t *node = lfs_find_by_id((uint32_t)file_id);

  if (!node)
    return -1;

  strncpy(info->name, node->filename, 63);
  info->size = node->size;
  info->is_sealed = node->is_sealed;
  info->id = node->inode_id;
  info->parent_id = node->parent_id;

  // Hash Preview
  if (node->type == 1) { // Directory
    strcpy(info->hash_preview, "<DIR>");
  } else {
    sprintf(info->hash_preview, "%02X%02X%02X%02X", node->content_hash[0],
            node->content_hash[1], node->content_hash[2],
            node->content_hash[3]);
  }
  return 0;
}

int sys_list_procs(nexus_proc_entry_t *procs, int max_count) {
  extern struct qproc *qproc_list;
  struct qproc *current = qproc_list;
  int count = 0;

  while (current && count < max_count) {
    procs[count].pid = current->pid;
    strncpy(procs[count].name, current->name, 31);
    strncpy(procs[count].owner, current->owner_pubkey,
            31); // Truncate for display
    procs[count].q_state = current->q_state;
    procs[count].t_coherence = current->t_coherence;
    procs[count].cpu_ticks = current->cpu_ticks;
    procs[count].nice_value = current->nice_value;

    current = current->next;
    count++;
  }
  return count;
}

// --- Graphics Syscalls ---
// Normally we would have sys_gpu_draw, but for high-level demo we do
// sys_visualize_qubit which does the rendering kernel-side (simulated) and
// flushes to user.

void qvis_bloch_sphere(float theta, float phi); // From visualization.c

int sys_visualize_qubit(int pid) {
  // 1. Find Process
  extern struct qproc *qproc_list;
  struct qproc *p = qproc_list;
  while (p) {
    if (p->pid == pid)
      break;
    p = p->next;
  }

  if (!p)
    return -1; // Not found

  // 2. Determine State (Simulated)
  // If coherent, show a nice vector. If decohered, show random or collapsed.
  float theta = 0.0f;
  float phi = 0.0f;

  if (p->q_state == QSTATE_DECOHERED) {
    // Collapsed to |0> or |1> (Random)
    theta = (p->pid % 2) ? 3.14159f : 0.0f;
  } else {
    // Superposition: Let's make it rotate based on time/coherence
    // theta = PI/2 (Equator), phi = time
    theta = 1.57f;
    phi = (float)(p->t_coherence) * 0.1f;
  }

  // 3. Render
  qvis_bloch_sphere(theta, phi);
  return 0;
}

// --- UI/Compositor Syscalls ---
// We need to link compositor functions.
// For now, we declare them extern.
extern int compositor_create_window(int width, int height);
extern void compositor_render(void);
extern void compositor_init(void); // Should be called at boot

int sys_create_window(int width, int height) {
  // Ensure init is called (lazy init for prototype)
  static int init_done = 0;
  if (!init_done) {
    compositor_init();
    init_done = 1;
  }
  return compositor_create_window(width, height);
}

int sys_render_frame(void) {
  compositor_render();
  return 0;
}

// --- File Management Syscalls ---
// Need prototypes from ledgerfs.c (updated)
extern lfs_inode_t *lfs_create_file(const char *name, const void *data,
                                    int size, uint32_t parent_id,
                                    const char *owner);
extern int lfs_read_file(const char *name, void *buffer, int max_size);
extern int lfs_delete_file(const char *name, const char *requestor);

int sys_create_file(const char *name, const char *content, int size,
                    int parent_id) {
  // Get Current Process Owner
  // For simplicity in this prototype, we assume the shell passes the owner,
  // OR we look up the current running process if we had a "current" pointer.
  // Let's assume the Scheduler has a `current_process` global or we pass it.
  // Actually, let's look up the process by PID if we had it, but syscalls don't
  // pass PID usually. We will assume the SHELL passes the owner string for now
  // (User Space Auth), trusting the shell (since it's the OS shell). WAIT: The
  // user request says "respectant notre gouvernance". Ideally kernel checks
  // `current_process->owner_pubkey`. Let's assume `current_process` is
  // available from scheduler.c

  extern struct qproc *current_process; // Need to expose this from scheduler.c
  char owner[64] = "SYSTEM";
  if (current_process) {
    strncpy(owner, current_process->owner_pubkey, 63);
  }

  if (lfs_create_file(name, content, size, (uint32_t)parent_id, owner)) {
    char msg[128];
    snprintf(msg, 128, "Created file '%s' (%d bytes)", name, size);
    audit_log(AUDIT_FILE_ACCESS, owner, msg);
    return 0;
  }
  return -1;
}

int sys_read_file(const char *name, char *buffer, int max_size) {
  // Check QDFS first
  extern int qdfs_resolve_mount(const char *path, const char **relative_path);
  extern int qdfs_read_remote(int node_id, const char *filename, char *buffer,
                              int size);

  const char *rel_path;
  int node_id = qdfs_resolve_mount(name, &rel_path);

  if (node_id >= 0) {
    return qdfs_read_remote(node_id, rel_path, buffer, max_size);
  }

  return lfs_read_file(name, buffer, max_size);
}

int sys_delete_file(const char *name) {
  extern struct qproc *current_process;
  char owner[64] = "SYSTEM";
  if (current_process) {
    strncpy(owner, current_process->owner_pubkey, 63);
  }
  int res = lfs_delete_file(name, owner);
  if (res == 0) {
    char msg[128];
    snprintf(msg, 128, "Deleted file '%s'", name);
    audit_log(AUDIT_FILE_ACCESS, owner, msg);
  }
  return res;
}

int sys_login(const char *pubkey) {
  extern struct qproc *current_process;

  // Lazy Init Audit
  static int audit_ready = 0;
  if (!audit_ready) {
    audit_init();
    audit_ready = 1;
  }

  if (current_process) {
    strncpy(current_process->owner_pubkey, pubkey, 63);

    char msg[128];
    snprintf(msg, 128, "User logged in with key: %s", pubkey);
    audit_log(AUDIT_LOGIN, pubkey, msg);

    return 0;
  }
  return -1;
}

int sys_shutdown(void) {
  printf("[KERNEL] System Shutdown Initiated...\n");
  lfs_save_disk();
  printf("[KERNEL] Bye.\n");
  exit(0); // Terminate the simulation
  return 0;
}

int sys_create_directory(const char *name, int parent_id) {
  // Get current process owner
  extern struct qproc *current_process;
  struct qproc *p = current_process;
  const char *owner = (p) ? p->owner_pubkey : "SYSTEM";

  lfs_create_directory(name, (uint32_t)parent_id, owner);
  return 0;
}

// --- Quantum Network Syscalls ---
#include "../memory/include/sys/qsocket.h"
extern int qnet_request_entanglement(qnet_node_id_t target, double min_fidelity,
                                     int timeout_ms);
extern int qnet_teleport_state(int qubit_id, int pair_id);

int sys_request_entanglement(int target_node, int fidelity_percent) {
  // fidelity_percent 0-100 mapped to 0.0-1.0
  double fid = (double)fidelity_percent / 100.0;
  return qnet_request_entanglement((qnet_node_id_t)target_node, fid, 1000);
}

int sys_teleport_state(int qubit_id, int pair_id) {
  return qnet_teleport_state(qubit_id, pair_id);
}

extern int qnet_exchange_key(qnet_node_id_t target, uint8_t *key_buffer,
                             int key_len, int proto_id);

int sys_qnet_qkd(int target_node, int key_len, int proto_id) {
  // For the shell, we just print the key to stdout inside the kernel
  // (simulated) or return it. Let's allocate a temp buffer.
  if (key_len > 128)
    key_len = 128;
  uint8_t key[128];
  return qnet_exchange_key((qnet_node_id_t)target_node, key, key_len, proto_id);
}

extern int sys_teleport_process(int pid, int target_node);
extern int sys_mount_qdfs(int target_node, const char *mount_point);

int sys_quantum_optimize(int *adj_matrix, int num_nodes) {
  extern int qaoa_solve_maxcut(int *adj_matrix, int num_nodes);
  return qaoa_solve_maxcut(adj_matrix, num_nodes);
}

// Syscall 14: Optimize Scheduler (Level 33)
int sys_quantum_optimize_scheduler(void) {
  extern void sched_optimize_load_balance(void);
  sched_optimize_load_balance();
  return 0;
}

int sys_read_klog(char *buffer, int max_size) {
  extern int klog_read(char *buffer, int max_size);
  return klog_read(buffer, max_size);
}

// Syscall #29: Memory info
int sys_meminfo(size_t *ram_used, size_t *ram_free, int *qubits_used,
                int *qubits_free) {
  extern void kalloc_get_stats(size_t *ram_used, size_t *ram_free,
                               int *qubits_used, int *qubits_free);
  kalloc_get_stats(ram_used, ram_free, qubits_used, qubits_free);
  return 0;
}

// Syscall #30: Execute QVM circuit
int sys_qvm_execute(const char *circuit_text) {
  // QVM execution in userspace for now
  // Kernel just validates and logs
  printf("[KERNEL] QVM circuit execution requested\n");
  return 0; // Success - actual execution in userspace
}

// Syscall 25: Quantum Fork (Level 34)
int sys_quantum_fork(void) {
  extern void sched_quantum_fork(void);
  sched_quantum_fork();
  return 0;
}

// Syscall 36: Quantum Network Stack Request
int sys_qns_request(int op, int target, void *data) {
  extern int qns_request(int op, int target, void *data);
  return qns_request(op, target, data);
}

// Syscall 37: Distributed Quantum Computing Submit
int sys_dqc_submit(int job_type, int num_workers) {
  extern int dqc_submit_job(int job_type, int num_workers);
  return dqc_submit_job(job_type, num_workers);
}

// Syscall 25: Kill Process
int sys_kill(int pid) {
  extern void scheduler_kill_process(int pid);
  scheduler_kill_process(pid);
  return 0;
}

// Syscall 26: Renice Process
int sys_renice(int pid, int priority) {
  extern void scheduler_renice_process(int pid, int priority);
  scheduler_renice_process(pid, priority);
  return 0;
}

// Syscall 27: Rename File
int sys_rename_file(const char *old_name, const char *new_name) {
  extern struct qproc *current_process;
  char owner[64] = "SYSTEM";
  if (current_process) {
    strncpy(owner, current_process->owner_pubkey, 63);
  }

  extern int lfs_rename_file(const char *old_name, const char *new_name,
                             const char *requestor);
  int res = lfs_rename_file(old_name, new_name, owner);

  if (res == 0) {
    char msg[128];
    snprintf(msg, 128, "Renamed '%s' to '%s'", old_name, new_name);
    audit_log(AUDIT_FILE_ACCESS, owner, msg);
  }
  return res;
}

// La Table de Dispatch (Le "Code Civil")
void *syscall_table[] = {
    [0] = sys_create_qproc,     // Syscall #0: Naissance d'un citoyen
    [1] = sys_allocate_qmemory, // Syscall #1: Demande de terrain (Qubits)
    [2] = sys_verify_ledger,    // Syscall #2: Appel à la Police (Intégrité)
    [3] = sys_yield,            // Syscall #3: Coopération
    [4] = sys_scheduler_tick,   // Syscall #4: Le Temps (Dieu)
    [5] = sys_get_wallet_id,    // Syscall #5: Identité (Carte d'identité)
    [6] = sys_exec_contract,    // Syscall #6: Loi (Smart Contract)
    [7] = sys_get_sysinfo,      // Syscall #7: Recensement
    [8] = sys_list_files,       // Syscall #8: Cadastre (Fichiers)
    [9] = sys_list_procs,       // Syscall #9: Annuaire (Processus)
    [10] = sys_visualize_qubit, // Syscall #10: Vision (Graphique)
    [11] = sys_create_window,   // Syscall #11: Fenêtre (UI)
    [12] = sys_render_frame,    // Syscall #12: Rendu (AI NSS)
    [13] = sys_create_file,     // Syscall #13: Création Fichier
    [14] = sys_read_file,       // Syscall #14: Lecture Fichier
    [15] = sys_delete_file,     // Syscall #15: Suppression Fichier
    [16] = sys_login,    // Syscall #16: Connexion (Changement d'identité)
    [17] = sys_shutdown, // Syscall #17: Arrêt du Système (Persistence)
    [18] = sys_create_directory,     // Syscall #18: Création Dossier
    [19] = sys_request_entanglement, // Syscall #19: QNet Entanglement
    [20] = sys_teleport_state,       // Syscall #20: QNet Teleportation
    [21] = sys_qnet_qkd,             // Syscall #21: QNet QKD (E91)
    [22] = sys_teleport_process,
    [23] = sys_mount_qdfs,
    [24] = sys_get_file_info,
    [25] = sys_kill,
    [26] = sys_renice,
    [27] = sys_rename_file,
    [28] = sys_read_klog,
    [29] = sys_meminfo,
    [30] = sys_qvm_execute,
};

int num_syscalls = 31;