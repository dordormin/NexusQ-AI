/*
 * NexusQ-AI Standard Library Implementation
 * Wraps Kernel Syscalls into user-friendly functions.
 */

#include "include/nexus.h"
#include <string.h>

// --- Syscall Interface (Simulated) ---
// In a real OS, this would use inline assembly (INT 0x80)
// Here we link directly to the kernel's syscall table for simulation.

extern void *syscall_table[];
// Function pointer types for casting
typedef int (*sys_create_qproc_t)(char *, int);
typedef int (*sys_allocate_qmemory_t)(int);
typedef int (*sys_verify_ledger_t)(char *);

// --- API Implementation ---

nexus_pid_t nexus_create_process(const char *name, int required_qubits) {
  sys_create_qproc_t sys_func = (sys_create_qproc_t)syscall_table[0];
  return sys_func((char *)name, required_qubits);
}

nexus_qreg_t nexus_malloc_q(int num_qubits) {
  sys_allocate_qmemory_t sys_func = (sys_allocate_qmemory_t)syscall_table[1];
  return sys_func(num_qubits);
}

int nexus_verify_contract(const char *path) {
  sys_verify_ledger_t sys_func = (sys_verify_ledger_t)syscall_table[2];
  return sys_func((char *)path);
}

typedef int (*sys_yield_t)(void);
typedef int (*sys_scheduler_tick_t)(int);

void nexus_yield(void) {
  sys_yield_t sys_func = (sys_yield_t)syscall_table[3];
  sys_func();
}

int nexus_simulate_tick(int microseconds) {
  sys_scheduler_tick_t sys_func = (sys_scheduler_tick_t)syscall_table[4];
  return sys_func(microseconds);
}

typedef int (*sys_get_wallet_id_t)(char *);

int nexus_get_wallet_id(char *out_pubkey) {
  sys_get_wallet_id_t sys_func = (sys_get_wallet_id_t)syscall_table[5];
  return sys_func(out_pubkey);
}

typedef int (*sys_exec_contract_t)(const unsigned char *, int);

int nexus_exec_contract(const unsigned char *bytecode, int size) {
  sys_exec_contract_t sys_func = (sys_exec_contract_t)syscall_table[6];
  return sys_func(bytecode, size);
}

typedef int (*sys_get_sysinfo_t)(nexus_sysinfo_t *);
typedef int (*sys_list_files_t)(int parent_id, nexus_file_entry_t *files,
                                int max);
typedef int (*sys_list_procs_t)(nexus_proc_entry_t *, int);

int nexus_get_sysinfo(nexus_sysinfo_t *info) {
  sys_get_sysinfo_t sys_func = (sys_get_sysinfo_t)syscall_table[7];
  return sys_func(info);
}

int nexus_list_files(int parent_id, nexus_file_entry_t *files, int max) {
  sys_list_files_t sys_func = (sys_list_files_t)syscall_table[8];
  return sys_func(parent_id, files, max);
}

int nexus_list_procs(nexus_proc_entry_t *procs, int max_count) {
  sys_list_procs_t sys_func = (sys_list_procs_t)syscall_table[9];
  return sys_func(procs, max_count);
}

typedef int (*sys_visualize_qubit_t)(int);

int nexus_visualize_qubit(int pid) {
  sys_visualize_qubit_t sys_func = (sys_visualize_qubit_t)syscall_table[10];
  return sys_func(pid);
}

typedef int (*sys_create_window_t)(int, int);
typedef int (*sys_render_frame_t)(void);

int nexus_create_window(int width, int height) {
  sys_create_window_t sys_func = (sys_create_window_t)syscall_table[11];
  return sys_func(width, height);
}

int nexus_render_frame(void) {
  sys_render_frame_t sys_func = (sys_render_frame_t)syscall_table[12];
  return sys_func();
}

typedef int (*sys_create_file_t)(const char *, const char *, int, int);
typedef int (*sys_read_file_t)(const char *, char *, int);
typedef int (*sys_delete_file_t)(const char *);

int nexus_create_file(const char *name, const char *content, int parent_id) {
  sys_create_file_t sys_func = (sys_create_file_t)syscall_table[13];
  return sys_func(name, content, strlen(content), parent_id);
}

int nexus_read_file(const char *name, char *buffer, int max_size) {
  sys_read_file_t sys_func = (sys_read_file_t)syscall_table[14];
  return sys_func(name, buffer, max_size);
}

int nexus_delete_file(const char *name) {
  sys_delete_file_t sys_func = (sys_delete_file_t)syscall_table[15];
  return sys_func(name);
}

typedef int (*sys_login_t)(const char *);

int nexus_login(const char *pubkey) {
  sys_login_t sys_func = (sys_login_t)syscall_table[16];
  return sys_func(pubkey);
}

typedef int (*sys_shutdown_t)(void);

void nexus_shutdown(void) {
  sys_shutdown_t sys_func = (sys_shutdown_t)syscall_table[17];
  sys_func();
}

typedef int (*sys_create_dir_t)(const char *name, int parent_id);

int nexus_create_directory(const char *name, int parent_id) {
  sys_create_dir_t sys_func = (sys_create_dir_t)syscall_table[18];
  return sys_func(name, parent_id);
}

typedef int (*sys_req_ent_t)(int, int);
typedef int (*sys_teleport_t)(int, int);
typedef int (*sys_qkd_t)(int, int, int);

int nexus_request_entanglement(int target_node, int fidelity_percent) {
  sys_req_ent_t sys_func = (sys_req_ent_t)syscall_table[19];
  return sys_func(target_node, fidelity_percent);
}

int nexus_teleport_state(int qubit_id, int pair_id) {
  sys_teleport_t sys_func = (sys_teleport_t)syscall_table[20];
  return sys_func(qubit_id, pair_id);
}

int nexus_qnet_qkd(int target_node, int key_len, int proto_id) {
  sys_qkd_t sys_func = (sys_qkd_t)syscall_table[21];
  return sys_func(target_node, key_len, proto_id);
}

typedef int (*sys_get_file_info_t)(int, nexus_file_entry_t *);

int nexus_get_file_info(int file_id, nexus_file_entry_t *info) {
  sys_get_file_info_t sys_func = (sys_get_file_info_t)syscall_table[24];
  return sys_func(file_id, info);
}

typedef int (*sys_kill_t)(int);
typedef int (*sys_renice_t)(int, int);

int nexus_kill(int pid) {
  sys_kill_t sys_func = (sys_kill_t)syscall_table[25];
  return sys_func(pid);
}

int nexus_renice(int pid, int priority) {
  sys_renice_t sys_func = (sys_renice_t)syscall_table[26];
  return sys_func(pid, priority);
}

typedef int (*sys_rename_file_t)(const char *, const char *);

int nexus_rename_file(const char *old_name, const char *new_name) {
  sys_rename_file_t sys_func = (sys_rename_file_t)syscall_table[27];
  return sys_func(old_name, new_name);
}

typedef int (*sys_read_klog_t)(char *, int);

int nexus_read_klog(char *buffer, int max_size) {
  sys_read_klog_t sys_func = (sys_read_klog_t)syscall_table[28];
  return sys_func(buffer, max_size);
}

typedef int (*sys_meminfo_t)(size_t *, size_t *, int *,
                             int *); // Get memory info

int nexus_meminfo(size_t *ram_used, size_t *ram_free, int *qubits_used,
                  int *qubits_free) {
  sys_meminfo_t sys_func = (sys_meminfo_t)syscall_table[29];
  return sys_func(ram_used, ram_free, qubits_used, qubits_free);
}

typedef int (*sys_qvm_execute_t)(const char *); // Execute QVM circuit

int nexus_qvm_execute(const char *circuit_text) {
  sys_qvm_execute_t sys_func = (sys_qvm_execute_t)syscall_table[30];
  return sys_func(circuit_text);
}
