/*
 * NexusQ-AI Standard Library (libnexus)
 * The official API for developing Quantum-AI Applications.
 */

#ifndef _NEXUS_H_
#define _NEXUS_H_

#include <stddef.h>
#include <stdint.h>

// --- Types ---
typedef int32_t nexus_pid_t;
typedef int32_t nexus_qreg_t;

// --- Process Management ---
// Spawns a new quantum-aware process.
nexus_pid_t nexus_create_process(const char *name, int required_qubits);

// --- Quantum Memory Management ---
// Allocates a contiguous block of qubits on the QPU.
// Returns a handle (register ID) or -1 on failure.
nexus_qreg_t nexus_malloc_q(int num_qubits);

// --- LedgerFS / Smart Contracts ---
// Verifies the integrity of a file/contract against the blockchain ledger.
// Returns 0 if valid, -1 if compromised.
int nexus_verify_contract(const char *path);

// --- Scheduler ---
// Voluntarily yield the processor.
void nexus_yield(void);

// Simulate system time progression (for demo purposes).
// Returns the PID of the process chosen to run.
int nexus_simulate_tick(int microseconds);

// --- Identity ---
// Retrieve the Public Key of the current process owner.
int nexus_get_wallet_id(char *out_pubkey);

// --- Smart Contracts ---
// Execute a contract bytecode. Returns 1 if authorized, 0 otherwise.
int nexus_exec_contract(const unsigned char *bytecode, int size);

// --- System Info & Shell Support ---

typedef struct {
  int total_ram_kb;
  int free_ram_kb;
  int total_qubits;
  int free_qubits;
  int lfs_files_count;
  double qec_fidelity; // New: QEC Metric
} nexus_sysinfo_t;

typedef struct {
  char name[64];
  int size;
  int is_sealed;
  char hash_preview[16]; // First few chars of hash
  int id;                // Added ID for directory navigation
  int parent_id;         // Added Parent ID for ".." navigation
} nexus_file_entry_t;

typedef struct {
  int pid;
  char name[32];
  char owner[32]; // Short owner key
  int q_state;
  double t_coherence;
  uint64_t cpu_ticks; // CPU Usage
  int nice_value;     // Priority
} nexus_proc_entry_t;

// Syscalls Wrappers
int nexus_get_sysinfo(nexus_sysinfo_t *info);
int nexus_list_files(int parent_id, nexus_file_entry_t *files, int max_count);
int nexus_get_file_info(int file_id, nexus_file_entry_t *info); // New

// Process Management
int nexus_kill(int pid);
int nexus_renice(int pid, int priority);

int nexus_list_procs(nexus_proc_entry_t *procs, int max_count);
int nexus_visualize_qubit(int pid);

// --- UI / AI Graphics ---
int nexus_create_window(int width, int height);
int nexus_render_frame(void);

// --- File Management ---
int nexus_create_file(const char *name, const char *content, int parent_id);
int nexus_read_file(const char *name, char *buffer, int max_size);
int nexus_delete_file(const char *name);
int nexus_rename_file(const char *old_name, const char *new_name);
int nexus_read_klog(char *buffer, int max_size);

// Memory info
int nexus_meminfo(size_t *ram_used, size_t *ram_free, int *qubits_used,
                  int *qubits_free);

// QVM
int nexus_qvm_execute(const char *circuit_text);

int nexus_login(const char *pubkey);
void nexus_shutdown(void);
int nexus_create_directory(const char *name, int parent_id);

// --- Quantum Network ---
int nexus_request_entanglement(int target_node, int fidelity_percent);
int nexus_teleport_state(int qubit_id, int pair_id);
int nexus_qnet_qkd(int target_node, int key_len, int proto_id);

#endif // _NEXUS_H_
