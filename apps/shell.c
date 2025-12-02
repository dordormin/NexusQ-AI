/*
 * NexusQ-AI Shell (nsh)
 * File: apps/shell.c
 */

#include "../lib/include/nexus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

#define MAX_CMD_LEN 128
#define HISTORY_MAX 10

static char history[HISTORY_MAX][MAX_CMD_LEN];
static int history_count = 0;
static struct termios orig_termios;

void disable_raw_mode() { tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios); }

void enable_raw_mode() {
  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(disable_raw_mode);
  struct termios raw = orig_termios;
  raw.c_lflag &= ~(ECHO | ICANON); // Disable echo and canonical mode
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void history_add(const char *cmd) {
  if (strlen(cmd) == 0)
    return;
  // Simple shift if full
  if (history_count < HISTORY_MAX) {
    strcpy(history[history_count++], cmd);
  } else {
    for (int i = 1; i < HISTORY_MAX; i++) {
      strcpy(history[i - 1], history[i]);
    }
    strcpy(history[HISTORY_MAX - 1], cmd);
  }
}

void print_prompt() {
  char pubkey[64];
  if (nexus_get_wallet_id(pubkey) == 0) {
    // Show short ID and Path
    extern char cwd_path[128];
    printf("nexus@%.8s:%s> ", pubkey, cwd_path);
  } else {
    printf("nexus@unknown> ");
  }
}

void cmd_sysinfo() {
  nexus_sysinfo_t info;
  if (nexus_get_sysinfo(&info) == 0) {
    printf("--- System Information ---\n");
    printf("RAM:    %d KB Total / %d KB Free\n", info.total_ram_kb,
           info.free_ram_kb);
    printf("  - Free Qubits: %d\n", info.free_qubits);
    printf("  - LedgerFS Files: %d\n", info.lfs_files_count);
    printf("  - QEC Fidelity: %.4f%%\n", info.qec_fidelity * 100.0);
    printf("--------------------------\n");
  } else {
    printf("Error: Failed to retrieve sysinfo.\n");
  }
}

// Shell State
int cwd_id = 0; // Root
char cwd_path[128] = "/";

void cmd_ls() {
  nexus_file_entry_t files[32];
  int count = nexus_list_files(cwd_id, files, 32);
  if (count == 0) {
    printf("No files in current directory.\n");
    return;
  }

  printf("%-20s | %-8s | %-10s | %s\n", "Name", "Size", "Status", "Hash");
  printf("--------------------------------------------------------------------"
         "------\n");

  for (int i = 0; i < count; i++) {
    char display_name[68];
    if (strcmp(files[i].hash_preview, "<DIR>") == 0) {
      snprintf(display_name, sizeof(display_name), "./%s", files[i].name);
    } else {
      strncpy(display_name, files[i].name, sizeof(display_name) - 1);
      display_name[sizeof(display_name) - 1] = '\0'; // Ensure null termination
    }

    printf("%-20s | %-8d | %-10s | %s\n", display_name, files[i].size,
           files[i].is_sealed ? "SEALED" : "OPEN", files[i].hash_preview);
  }
}

void cmd_cd(const char *arg) {
  // 1. Handle Root
  if (strcmp(arg, "/") == 0) {
    cwd_id = 0;
    strcpy(cwd_path, "/");
    return;
  }

  // 2. Handle ".." (Parent Directory)
  if (strcmp(arg, "..") == 0) {
    if (cwd_id == 0)
      return; // Already at root

    // Get current dir info to find parent
    nexus_file_entry_t current_dir;
    if (nexus_get_file_info(cwd_id, &current_dir) == 0) {
      cwd_id = current_dir.parent_id;

      // Update Path: Remove last component
      // Find last slash (ignoring trailing slash if any)
      int len = strlen(cwd_path);
      if (len > 1 && cwd_path[len - 1] == '/')
        cwd_path[len - 1] = 0;

      char *last_slash = strrchr(cwd_path, '/');
      if (last_slash) {
        if (last_slash == cwd_path) {
          // We are back at root
          strcpy(cwd_path, "/");
        } else {
          *last_slash = 0;       // Truncate
          strcat(cwd_path, "/"); // Add trailing slash back
        }
      }
    }
    return;
  }

  // 3. Find directory in current list (Go Down)
  nexus_file_entry_t files[32];
  int count = nexus_list_files(cwd_id, files, 32);

  for (int i = 0; i < count; i++) {
    if (strcmp(files[i].name, arg) == 0) {
      if (strcmp(files[i].hash_preview, "<DIR>") == 0) {
        // Change Directory
        cwd_id = files[i].id;

        // Update Path (Simple Append)
        // Ensure cwd_path ends with /
        int len = strlen(cwd_path);
        if (cwd_path[len - 1] != '/')
          strcat(cwd_path, "/");

        strcat(cwd_path, arg);
        strcat(cwd_path, "/");
        return;
      } else {
        printf("Error: '%s' is not a directory.\n", arg);
        return;
      }
    }
  }
  printf("Error: Directory '%s' not found.\n", arg);
}

void cmd_ps() {
  nexus_proc_entry_t procs[32];
  int count = nexus_list_procs(procs, 32);

  printf("--- Quantum Processes (%d) ---\n", count);
  printf("%-5s | %-15s | %-10s | %-10s | %s\n", "PID", "Name", "State",
         "Coherence", "Owner");
  printf("---------------------------------------------------------------------"
         "-\n");
  for (int i = 0; i < count; i++) {
    printf("%-5d | %-15s | %-10d | %-10.1f | %.8s...\n", procs[i].pid,
           procs[i].name, procs[i].q_state, procs[i].t_coherence,
           procs[i].owner);
  }
}

void cmd_whoami() {
  char pubkey[64];
  if (nexus_get_wallet_id(pubkey) == 0) {
    printf("Wallet ID: %s\n", pubkey);
  } else {
    printf("Error: No Identity.\n");
  }
}

void cmd_pwd() { printf("Current directory: %s (ID: %d)\n", cwd_path, cwd_id); }

void cmd_touch(const char *arg) {
  char name[64], content[128];
  if (sscanf(arg, "%s %[^\n]", name, content) < 2) {
    printf("Usage: touch <filename> <content>\n");
    return;
  }
  if (nexus_create_file(name, content, cwd_id) == 0) {
    printf("File '%s' created.\n", name);
  } else {
    printf("Error: Failed to create file.\n");
  }
}

void cmd_cat(const char *arg) {
  char buffer[256];
  if (nexus_read_file(arg, buffer, 255) > 0) {
    printf("%s\n", buffer);
  } else {
    printf("Error: File not found.\n");
  }
}

void cmd_rm(const char *arg) {
  int res = nexus_delete_file(arg);
  if (res == 0) {
    printf("File '%s' deleted.\n", arg);
  } else if (res == -2) {
    printf("Error: Access Denied (Governance: Not Owner).\n");
  } else {
    printf("Error: File not found.\n");
  }
}

void cmd_cleanup() {
  printf("--- Cleaning up duplicate files ---\n");

  nexus_file_entry_t files[32];
  int count = nexus_list_files(cwd_id, files, 32);

  int removed = 0;
  int skipped = 0;

  // Remove duplicate shell_startup.log files (keep only one)
  int found_shell_startup = 0;
  for (int i = 0; i < count; i++) {
    if (strcmp(files[i].name, "shell_startup.log") == 0) {
      if (found_shell_startup) {
        // Duplicate - try to remove it
        int res = nexus_delete_file(files[i].name);
        if (res == 0) {
          printf("Removed duplicate: shell_startup.log\n");
          removed++;
        } else if (res == -2) {
          skipped++; // Permission denied, skip silently
        }
      } else {
        found_shell_startup = 1;
      }
    }
  }

  // Remove files named "docs", "./docs", "/docs" (directories or misnamed
  // files)
  for (int i = 0; i < count; i++) {
    if (strcmp(files[i].name, "docs") == 0 ||
        strcmp(files[i].name, "./docs") == 0 ||
        strcmp(files[i].name, "/docs") == 0 ||
        strcmp(files[i].name, ".docs") == 0) {
      if (nexus_delete_file(files[i].name) == 0) {
        printf("Removed: %s\n", files[i].name);
        removed++;
      }
    }
  }

  // Remove copy.log if it exists
  for (int i = 0; i < count; i++) {
    if (strstr(files[i].name, "copy.log") != NULL) {
      if (nexus_delete_file(files[i].name) == 0) {
        printf("Removed: %s\n", files[i].name);
        removed++;
      }
    }
  }

  printf("Cleanup complete. Removed %d files", removed);
  if (skipped > 0) {
    printf(" (%d skipped due to permissions)", skipped);
  }
  printf(".\n");
}

void cmd_free() {
  size_t ram_used, ram_free;
  int qubits_used, qubits_free;

  if (nexus_meminfo(&ram_used, &ram_free, &qubits_used, &qubits_free) == 0) {
    printf("--- Memory Usage ---\n");
    printf("RAM:\n");
    printf("  Used:  %zu KB (%zu bytes)\n", ram_used / 1024, ram_used);
    printf("  Free:  %zu KB (%zu bytes)\n", ram_free / 1024, ram_free);
    printf("  Total: %zu KB\n", (ram_used + ram_free) / 1024);
    printf("\nQPU:\n");
    printf("  Used:  %d qubits\n", qubits_used);
    printf("  Free:  %d qubits\n", qubits_free);
    printf("  Total: %d qubits\n", qubits_used + qubits_free);
  } else {
    printf("Error: Failed to retrieve memory information.\n");
  }
}

void cmd_qexec(const char *filename) {
  // Read circuit file
  char buffer[2048];
  int len = nexus_read_file(filename, buffer, sizeof(buffer) - 1);
  if (len < 0) {
    printf("Error: Could not read circuit file '%s'\n", filename);
    return;
  }
  buffer[len] = '\0';

  // Parse and execute circuit
  // QVM execution happens in userspace
  extern void qvm_execute_from_text(const char *text);

  qvm_execute_from_text(buffer);
}

// --- Quantum Monitor ---
extern void qmonitor_dashboard();
extern void qmonitor_stats();
extern void qmonitor_reset();
extern int qmonitor_export(const char *filename);

void cmd_qmonitor() { qmonitor_dashboard(); }
void cmd_qstats() { qmonitor_stats(); }
void cmd_qreset() { qmonitor_reset(); }

// --- Quantum Optimizer ---
extern void qopt_analyze(const char *circuit_text);
extern void qopt_optimize(const char *input, const char *output);

void cmd_qopt(const char *arg) {
  char subcmd[32];
  char file1[64], file2[64];

  if (sscanf(arg, "%s", subcmd) != 1) {
    printf("Usage: qopt <analyze|optimize> ...\n");
    return;
  }

  if (strcmp(subcmd, "analyze") == 0) {
    if (sscanf(arg + 8, "%s", file1) == 1) {
      char buffer[2048];
      if (nexus_read_file(file1, buffer, 2047) > 0) {
        qopt_analyze(buffer);
      } else {
        printf("Error: Could not read '%s'\n", file1);
      }
    } else {
      printf("Usage: qopt analyze <file>\n");
    }
  } else if (strcmp(subcmd, "optimize") == 0) {
    if (sscanf(arg + 9, "%s %s", file1, file2) == 2) {
      qopt_optimize(file1, file2);
    } else {
      printf("Usage: qopt optimize <input> <output>\n");
    }
  } else {
    printf("Unknown subcommand: %s\n", subcmd);
  }
}

// --- Quantum Export ---
extern void qexport_json(const char *circuit_name);

void cmd_qexport(const char *arg) {
  char subcmd[32];
  char file[64];

  if (sscanf(arg, "%s %s", subcmd, file) == 2) {
    if (strcmp(subcmd, "json") == 0) {
      qexport_json(file);
    } else {
      printf("Unknown format: %s\n", subcmd);
    }
  } else {
    printf("Usage: qexport <json> <circuit_name>\n");
  }
}

void cmd_qimport(const char *arg) {
  printf("[QIMPORT] Feature coming soon.\n");
}

// --- Quantum Visualization ---
extern void qvis_bloch();
extern void qvis_histogram();

void cmd_qvis(const char *arg) {
  char subcmd[32];
  if (sscanf(arg, "%s", subcmd) == 1) {
    if (strcmp(subcmd, "bloch") == 0)
      qvis_bloch();
    else if (strcmp(subcmd, "histogram") == 0)
      qvis_histogram();
    else
      printf("Unknown visualization: %s\n", subcmd);
  } else {
    printf("Usage: qvis <bloch|histogram>\n");
  }
}

// --- Quantum Profiler ---
extern void qprof_profile(const char *circuit_name);

void cmd_qprof(const char *arg) {
  char file[64];
  if (sscanf(arg, "%s", file) == 1) {
    qprof_profile(file);
  } else {
    printf("Usage: qprof <circuit_file>\n");
  }
}

// --- Governance ---
extern void gov_print_audit(const char *filter_user, int last_n);
extern void gov_print_permissions();
extern int gov_cleanup_filesystem();

void cmd_audit(const char *arg) {
  char user[64];
  if (arg && sscanf(arg, "%s", user) == 1) {
    gov_print_audit(user, 20);
  } else {
    gov_print_audit(NULL, 20);
  }
}

void cmd_permissions() { gov_print_permissions(); }
void cmd_cleanup_fs() { gov_cleanup_filesystem(); }

void cmd_help() {
  printf("Available Commands:\n");
  printf("  sysinfo, status  : Show system resources\n");
  printf("  ls, dir          : List directory contents\n");
  printf("  ps, qtop         : Show processes\n");
  printf("  top              : Live process monitor\n");
  printf("  free             : Show memory usage (RAM + QPU)\n");
  printf("  qexec <file>     : Execute quantum circuit (.qc file)\n");
  printf("  qdbg <file>      : Debug quantum circuit step-by-step\n");
  printf("  qmonitor         : Quantum System Dashboard\n");
  printf("  qstats           : Detailed Quantum Statistics\n");
  printf("  qopt <cmd>       : Optimize circuits (analyze/optimize)\n");
  printf("  qexport <fmt>    : Export results (json)\n");
  printf("  qvis <type>      : Visualize (bloch/histogram)\n");
  printf("  qprof <file>     : Profile circuit performance\n");
  printf("  audit [user]     : View governance audit log\n");
  printf("  permissions      : View system permissions\n");
  printf("  cleanup          : Clean filesystem (duplicates/corrupt)\n");
  printf("  visualize <pid>  : Show 3D Quantum State\n");
  printf("  teleport <pid> <node> : Teleport Process\n");
  printf("  mount <node> <path> : Mount Remote FS\n");
  printf("  spawn <name>     : Create new process\n");
  printf("  gui_test         : Test AI-Enhanced UI\n");
  printf("  touch <file> <txt>: Create file\n");
  printf("  cat <file>       : Read file\n");
  printf("  rm <file>        : Delete file\n");
  printf("  whoami           : Show current identity\n");
  printf("  clear            : Clear screen\n");
  printf("  vision <cat|dog> : Test Neural Vision\n");
  printf("  services         : List System Services\n");
  printf("  exit             : Logout\n");
}

// Simulated Auth
#include "../modules/crypto/include/sha256.h"

void hash_password(const char *password, char *out_hex) {
  Nexus_SHA256_CTX ctx;
  uint8_t hash[SHA256_BLOCK_SIZE];

  sha256_init(&ctx);
  sha256_update(&ctx, (const uint8_t *)password, strlen(password));
  sha256_final(&ctx, hash);

  for (int i = 0; i < SHA256_BLOCK_SIZE; i++) {
    sprintf(out_hex + (i * 2), "%02x", hash[i]);
  }
  out_hex[64] = 0;
}

void do_login() {
  char username[32];
  char password[32];
  char pass_hash[65];
  int attempts = 0;

  // Check if users.db exists
  char buffer[128]; // Increased buffer for hash storage
  int first_run = (nexus_read_file("users.db", buffer, 127) < 0);

  if (first_run) {
    printf("=== First Usage: Create Admin Account ===\n");
    printf("Username: ");
    scanf("%s", username);
    printf("Password: ");
    scanf("%s", password);

    // Hash Password
    hash_password(password, pass_hash);

    // Store as "username:hash"
    char db_content[128];
    sprintf(db_content, "%s:%s", username, pass_hash);

    // Create users.db in ROOT (0)
    nexus_create_file("users.db", db_content, 0);

    // Generate Wallet (Simulated)
    char pubkey[64];
    sprintf(pubkey, "USER_%s_KEY_1234", username);

    printf("Account Created. Wallet: %s\n", pubkey);
    nexus_login(pubkey);
  } else {
    while (1) {
      printf("=== User Login ===\n");
      printf("Username: ");
      scanf("%s", username);
      printf("Password: ");
      scanf("%s", password);

      // Read DB (Format: username:hash)
      nexus_read_file("users.db", buffer, 127);

      // Parse DB
      char stored_user[32];
      char stored_hash[65];
      sscanf(buffer, "%[^:]:%s", stored_user, stored_hash);

      // Hash Input
      hash_password(password, pass_hash);

      if (strcmp(username, stored_user) == 0 &&
          strcmp(pass_hash, stored_hash) == 0) {
        printf("Logged in as %s.\n", username);
        char pubkey[64];
        sprintf(pubkey, "USER_%s_KEY_1234", username);
        nexus_login(pubkey);
        break; // Success
      } else {
        attempts++;
        printf("Error: Invalid Credentials. (%d/3)\n", attempts);
        if (attempts >= 3) {
          printf("!!! SECURITY LOCKOUT !!! Too many failed attempts.\n");
          nexus_shutdown();
        }
      }
    }
  }

  // Consume newline left by scanf
  getchar();
}

void cmd_gui_test() {
  printf("Initializing UI Window (320x200)...\n");
  int win_id = nexus_create_window(320, 200);
  if (win_id >= 0) {
    printf("Window Created. ID: %d\n", win_id);
    printf("Rendering Frame...\n");
    nexus_render_frame();
  } else {
    printf("Error creating window.\n");
  }
}

void cmd_visualize(const char *arg) {
  if (!arg || strlen(arg) == 0) {
    printf("Usage: visualize <pid> | qnn\n");
    return;
  }

  if (strcmp(arg, "qnn") == 0) {
    printf("--- Quantum Neural Network Visualization ---\n");
    printf("Layer 1 (Encoding): Rx(PI * Input)\n");
    printf("Layer 2 (Variational): Rz(PI * Weight) + CNOT\n");
    printf("Measurement: Z-Basis Expectation\n");
    printf("Current Model Weights: [0.5, 0.8]\n");
    printf("------------------------------------------\n");
    return;
  }

  int pid = atoi(arg);
  printf("Visualizing Qubit State for PID %d...\n", pid);
  if (nexus_visualize_qubit(pid) != 0) {
    printf("Error: PID not found or graphics failure.\n");
  }
}

void cmd_net(const char *args) {
  char subcmd[32];
  char param[32];
  int n = sscanf(args, "%s %s", subcmd, param);

  if (n < 1) {
    printf("Usage: net <status|qkd> [target_id]\n");
    return;
  }

  if (strcmp(subcmd, "status") == 0) {
    printf("--- Quantum Network Status ---\n");
    printf("Local Node ID: 1\n");
    printf("Neighbors: Node 2 (Fiber Link)\n");
    // Could call a syscall to get real status
    printf("Link Quality: Stable (Fidelity > 95%%)\n");
    printf("------------------------------\n");
  } else if (strcmp(subcmd, "qkd") == 0) {
    if (n < 2) {
      printf("Usage: net qkd <target_node_id> [bb84|e91]\n");
      return;
    }
    int target = atoi(param);

    // Parse Protocol
    char proto_str[32];
    int proto_id = 2; // Default E91

    // Check if there's a 3rd arg
    // sscanf is tricky with optional args in one go, let's re-parse or check
    // the rest of the string Simple hack: check if args contains bb84 or e91
    if (strstr(args, "bb84"))
      proto_id = 1;
    else if (strstr(args, "e91"))
      proto_id = 2;

    printf("Initiating QKD with Node %d (Proto: %s)...\n", target,
           proto_id == 1 ? "BB84" : "E91");
    nexus_qnet_qkd(target, 32, proto_id); // 32 bytes key
  } else if (strcmp(subcmd, "send") == 0) {
    // net send <target> <msg>
    int target;
    char msg[64];
    if (sscanf(args, "%*s %*s %d %s", &target, msg) < 2) {
      // Try simpler parsing if above fails
      char *p = strstr(args, "send");
      if (p)
        sscanf(p + 5, "%d %s", &target, msg);
    }

    printf("Sending '%s' to Node %d...\n", msg, target);
    // We need to expose qnet_send_classical to shell or use a syscall.
    // Since shell is linked with kernel, we can call it directly for now
    // (Monolithic).
    extern void qnet_send_classical(const char *ip, int port, const void *data,
                                    int len);
    qnet_send_classical("127.0.0.1", 5000 + target, msg, strlen(msg));
  } else if (strcmp(subcmd, "tomography") == 0) {
    // net tomography <target>
    int target;
    if (sscanf(args, "%*s %*s %d", &target) < 1) {
      char *p = strstr(args, "tomography");
      if (p)
        sscanf(p + 10, "%d", &target);
    }

    printf("Running Tomography on Node %d...\n", target);
    extern double qnet_verify_link(int target);
    qnet_verify_link(target);
  } else if (strcmp(subcmd, "repeater") == 0) {
    // net repeater <target>
    int target;
    if (sscanf(args, "%*s %*s %d", &target) < 1) {
      char *p = strstr(args, "repeater");
      if (p)
        sscanf(p + 8, "%d", &target);
    }

    printf("Requesting Repeater Link to Node %d...\n", target);
    extern int qnet_extend_link(int target);
    qnet_extend_link(target);
  } else if (strcmp(subcmd, "optimize") == 0) {
    // optimize [nodes]
    // If nodes is provided, run raw QAOA demo.
    // If no args (or "system"), run Scheduler Optimization.

    int nodes = 0;
    if (sscanf(args, "%*s %*s %d", &nodes) >= 1) {
      // Raw Demo Mode
      if (nodes < 2 || nodes > 10)
        nodes = 4;
      printf("Generating random graph with %d nodes...\n", nodes);
      int *adj = malloc(nodes * nodes * sizeof(int));
      for (int i = 0; i < nodes * nodes; i++)
        adj[i] = (rand() % 100) < 50 ? 1 : 0;
      for (int i = 0; i < nodes; i++) {
        adj[i * nodes + i] = 0;
        for (int j = i + 1; j < nodes; j++) {
          adj[j * nodes + i] = adj[i * nodes + j];
        }
      }
      extern int sys_quantum_optimize(int *adj_matrix, int num_nodes);
      sys_quantum_optimize(adj, nodes);
      free(adj);
    } else {
      // System Mode
      printf("Optimizing System Scheduler using QAOA...\n");
      extern int sys_quantum_optimize_scheduler(void);
      sys_quantum_optimize_scheduler();
    }
  } else if (strcmp(subcmd, "qfork") == 0) {
    printf("Initiating Quantum Fork (Superposition)...\n");
    extern int sys_quantum_fork(void);
    sys_quantum_fork();
  } else if (strcmp(subcmd, "qec_status") == 0) {
    printf("--- Neural QEC Status ---\n");
    printf("Decoder Model: qec_decoder_v1\n");
    printf("Syndrome Type: Stabilizer (Z-Check)\n");
    printf("Avg Correction Latency: 1.2 us\n");
    printf("Fidelity Gain: +15%%\n");
  } else if (strcmp(subcmd, "qns") == 0) {
    // qns [ping <target> | status]
    char op[32];
    int target = 0;
    if (sscanf(args, "%*s %s %d", op, &target) >= 1) {
      if (strcmp(op, "ping") == 0) {
        printf("Pinging Node %d via QNS...\n", target);
        extern int sys_qns_request(int op, int target, void *data);
        sys_qns_request(2, target, NULL); // QNS_SEND_QUBIT
      } else if (strcmp(op, "status") == 0) {
        extern int sys_qns_request(int op, int target, void *data);
        sys_qns_request(3, 0, NULL); // QNS_GET_METRICS
      } else {
        printf("Usage: net qns [ping <target> | status]\n");
      }
    } else {
      printf("Usage: net qns [ping <target> | status]\n");
    }
  } else if (strcmp(subcmd, "dqc") == 0) {
    // dqc run <job_name>
    char action[32];
    char job_name[32];
    // args is "dqc run grover"
    // %*s skips "dqc", first %s reads "run", second %s reads "grover"
    if (sscanf(args, "%*s %s %s", action, job_name) >= 2) {
      if (strcmp(action, "run") == 0) {
        if (strcmp(job_name, "grover") == 0) {
          extern int sys_dqc_submit(int job_type, int num_workers);
          int key = sys_dqc_submit(1, 2); // Grover, 2 Workers

          if (key == 1011) {
            printf("[SHELL] DQC Result: %d. Attempting to unlock "
                   "'treasure.chest'...\n",
                   key);
            // Simulate Unlock
            char content[32];
            // Ensure content is null-terminated
            memset(content, 0, sizeof(content));
            if (nexus_read_file("treasure.chest", content, 31) > 0) {
              printf("[SUCCESS] File Unlocked! Content: \"%s\"\n", content);
            } else {
              printf("[FAIL] Could not read file.\n");
            }
          }
        } else {
          printf("Unknown Job: %s\n", job_name);
        }

      } else {
        printf("Usage: net dqc run <job_name>\n");
      }
    } else {
      printf("Usage: net dqc run <job_name>\n");
    }
  } else if (strcmp(subcmd, "qrun") == 0) {
    // qrun <filename>
    char filename[64];
    if (sscanf(args, "%*s %s", filename) >= 1) {
      char buffer[1024];
      if (nexus_read_file(filename, buffer, 1023) > 0) {
        extern void qsl_execute(const char *script_content);
        qsl_execute(buffer);
      } else {
        printf("Error: Could not read script '%s'.\n", filename);
      }
    } else {
      printf("Usage: qrun <filename>\n");
    }
  } else if (strcmp(subcmd, "qedit") == 0) {
    // qedit <filename>
    char filename[64];
    if (sscanf(args, "%*s %s", filename) >= 1) {
      extern void qedit_main(const char *filename);
      qedit_main(filename);
    } else {
      printf("Usage: qedit <filename>\n");
    }
  } else {
    printf("Unknown network command: %s\n", subcmd);
  }
}

void cmd_teleport(const char *args) {
  int pid, target_node;
  if (sscanf(args, "%d %d", &pid, &target_node) < 2) {
    printf("Usage: teleport <pid> <target_node>\n");
    return;
  }

  extern int sys_teleport_process(int pid, int target_node);
  if (sys_teleport_process(pid, target_node) == 0) {
    printf("Process %d teleported successfully.\n", pid);
  } else {
    printf("Teleportation failed.\n");
  }
}

void cmd_spawn(const char *name) {
  if (!name || strlen(name) == 0) {
    printf("Usage: spawn <name>\n");
    return;
  }
  int pid = nexus_create_process(name, 2); // 2 Qubits for dummy process
  if (pid > 0) {
    printf("Process '%s' spawned with PID %d.\n", name, pid);
  } else {
    printf("Failed to spawn process.\n");
  }
}

void cmd_mount(const char *args) {
  int node_id;
  char path[32];
  if (sscanf(args, "%d %s", &node_id, path) < 2) {
    printf("Usage: mount <node_id> <path>\n");
    return;
  }

  extern int sys_mount_qdfs(int target_node, const char *mount_point);
  if (sys_mount_qdfs(node_id, path) == 0) {
    printf("Mounted Node %d at '%s'.\n", node_id, path);
  } else {
    printf("Mount failed.\n");
  }
}

void cmd_clear() { printf("\033[H\033[J"); }

#include "../kernel/neural/include/sys/neural.h"

void cmd_vision(const char *arg) {
  printf("--- Neural Abstraction Layer (NAL) Test ---\n");

  // 1. Create Socket
  int sock = nal_socket(AF_NEURAL, SOCK_INFER, NPROTO_VISION);
  if (sock < 0) {
    printf("Error: Failed to create Neural Socket.\n");
    return;
  }
  printf("Socket Created (FD: %d)\n", sock);

  // 2. Connect to Model
  struct sockaddr_neural addr;
  addr.sn_family = AF_NEURAL;
  strncpy(addr.sn_model, "vision_model_v1", 63);

  if (nal_connect(sock, &addr) < 0) {
    printf("Error: Failed to connect to model '%s'.\n", addr.sn_model);
    nal_close(sock);
    return;
  }
  printf("Connected to Model: %s\n", addr.sn_model);

  // 3. Prepare Dummy Image (Random Noise or Pattern)
  uint8_t image_data[1024];
  memset(image_data, 0, 1024);
  // Set first byte based on arg to test classification
  if (arg && strcmp(arg, "cat") == 0)
    image_data[0] = 0; // Even -> Cat
  else
    image_data[0] = 1; // Odd -> Dog

  // 4. Send Inference Request
  printf("Sending Inference Request (1KB Image)...\n");
  if (nal_send(sock, image_data, 1024) < 0) {
    printf("Error: Send failed.\n");
    nal_close(sock);
    return;
  }

  // 5. Receive Result
  neural_result_t result;
  if (nal_recv(sock, &result, sizeof(result)) > 0) {
    printf(">>> Inference Result: %s (Confidence: %.2f%%)\n", result.label,
           result.confidence * 100.0);
  } else {
    printf("Error: Receive failed.\n");
  }

  // 6. Close
  nal_close(sock);
  printf("-------------------------------------------\n");
  printf("-------------------------------------------\n");
}

void cmd_services() {
  extern int svc_list_info(char *buffer, int max_len);
  char buffer[1024];
  svc_list_info(buffer, 1024);
  printf("%s", buffer);
  printf("%s", buffer);
}

void cmd_kill(const char *arg) {
  int pid = atoi(arg);
  if (pid > 0) {
    nexus_kill(pid);
  } else {
    printf("Usage: kill <pid>\n");
  }
}

void cmd_renice(const char *args) {
  int pid, prio;
  if (sscanf(args, "%d %d", &pid, &prio) == 2) {
    nexus_renice(pid, prio);
  } else {
    printf("Usage: renice <pid> <priority>\n");
  }
}

void cmd_top() {
  nexus_proc_entry_t procs[32];
  while (1) {
    int count = nexus_list_procs(procs, 32);
    printf("\033[H\033[J"); // Clear Screen
    printf("--- NexusQ-AI Top ---\n");
    printf("%-5s | %-15s | %-10s | %-10s | %-5s | %-5s\n", "PID", "Name",
           "State", "Coherence", "CPU", "Nice");
    printf("-------------------------------------------------------------------"
           "---\n");
    for (int i = 0; i < count; i++) {
      printf("%-5d | %-15s | %-10d | %-10.1f | %-5lu | %-5d\n", procs[i].pid,
             procs[i].name, procs[i].q_state, procs[i].t_coherence,
             procs[i].cpu_ticks, procs[i].nice_value);
    }
    printf("\nPress 'q' to exit.\n");
    fflush(stdout);

    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);

    int ret = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    if (ret > 0) {
      char c;
      read(STDIN_FILENO, &c, 1);
      if (c == 'q')
        break;
    }

    nexus_simulate_tick(100000); // Simulate OS work
  }
}

void cmd_dmesg() {
  char buffer[8192];
  int len = nexus_read_klog(buffer, sizeof(buffer) - 1);
  if (len > 0) {
    buffer[len] = '\0';
    printf("--- Kernel Log ---\n");
    printf("%s", buffer);
  } else if (len == 0) {
    printf("Kernel log is empty.\n");
  } else {
    printf("Error: Failed to read kernel log (code: %d).\n", len);
  }
}

// Helper to check if a path is a directory and return its ID
int is_directory(const char *path) {
  // Handle root
  if (strcmp(path, "/") == 0)
    return 0;

  // Handle "./" prefix
  const char *search_name = path;
  if (strncmp(path, "./", 2) == 0) {
    search_name = path + 2;
  }
  // Handle "/" prefix (absolute path - simplified for root-level dirs)
  if (path[0] == '/') {
    search_name = path + 1;
  }

  // Remove trailing slash if present
  char clean_name[64];
  strncpy(clean_name, search_name, 63);
  int len = strlen(clean_name);
  if (len > 0 && clean_name[len - 1] == '/') {
    clean_name[len - 1] = 0;
  }

  nexus_file_entry_t files[32];
  // Search in ROOT (0) for now if absolute, or CWD?
  // Simplified: Always search in CWD for relative, Root for absolute.
  int search_dir = cwd_id;
  if (path[0] == '/')
    search_dir = 0;

  int count = nexus_list_files(search_dir, files, 32);
  for (int i = 0; i < count; i++) {
    if (strcmp(files[i].name, clean_name) == 0) {
      if (strcmp(files[i].hash_preview, "<DIR>") == 0) {
        return files[i].id;
      }
    }
  }
  return -1;
}

// Helper to extract basename from path
const char *get_basename(const char *path) {
  const char *last_slash = strrchr(path, '/');
  if (last_slash) {
    return last_slash + 1;
  }
  return path;
}

// Helper to generate unique filename if file already exists
// Returns the final name in output_name
// NOTE: LedgerFS has GLOBAL filename uniqueness, so we must check ALL files
void generate_unique_filename(const char *original_name, int dir_id,
                              char *output_name) {
  // Get ALL files in the filesystem (not just target directory)
  // Since LedgerFS enforces global uniqueness via lfs_find_by_name()
  nexus_file_entry_t all_files[128];
  int total_count = 0;

  // List files from root (0) to get global view
  int root_count = nexus_list_files(0, all_files, 128);
  total_count = root_count;

  // Also check target directory if different from root
  if (dir_id != 0) {
    nexus_file_entry_t dir_files[32];
    int dir_count = nexus_list_files(dir_id, dir_files, 32);
    // Merge (simple approach: just append, might have duplicates but that's ok)
    for (int i = 0; i < dir_count && total_count < 128; i++) {
      all_files[total_count++] = dir_files[i];
    }
  }

  // Check if original name exists globally
  int found = 0;
  for (int i = 0; i < total_count; i++) {
    if (strcmp(all_files[i].name, original_name) == 0) {
      found = 1;
      break;
    }
  }

  if (!found) {
    // Original name is available globally
    strcpy(output_name, original_name);
    return;
  }

  // File exists globally, need to generate numbered version
  // Extract basename and extension
  char basename[64];
  char ext[16] = {0};

  const char *dot = strrchr(original_name, '.');
  if (dot && dot != original_name) {
    int base_len = dot - original_name;
    strncpy(basename, original_name, base_len);
    basename[base_len] = 0;
    strcpy(ext, dot); // Include the dot
  } else {
    strcpy(basename, original_name);
  }

  // Try numbered versions (1), (2), (3)...
  for (int n = 1; n < 100; n++) {
    snprintf(output_name, 64, "%s (%d)%s", basename, n, ext);

    // Check if this name exists globally
    int exists = 0;
    for (int i = 0; i < total_count; i++) {
      if (strcmp(all_files[i].name, output_name) == 0) {
        exists = 1;
        break;
      }
    }

    if (!exists) {
      return; // Found unique name globally
    }
  }

  // Fallback: use original name (shouldn't happen)
  strcpy(output_name, original_name);
}

void cmd_cp(const char *args) {
  char flag[16] = {0};
  char src[64], dest[64];
  int has_flag = 0;

  // Parse arguments with optional flag
  // Format: cp [-f|-d] src dest
  if (sscanf(args, "%s %s %s", flag, src, dest) == 3) {
    if (flag[0] == '-') {
      has_flag = 1;
    } else {
      // No flag, but 3 args? Maybe spaces in filename? Not supported yet.
      // Fallback to standard parsing if flag doesn't start with -
      // Actually sscanf is tricky. Let's assume if 3 args, first is flag.
    }
  } else if (sscanf(args, "%s %s", src, dest) == 2) {
    // Standard case: cp src dest
    has_flag = 0;
  } else {
    printf("Usage: cp [-f|-d] <source> <destination>\n");
    return;
  }

  // If we parsed 3 args but first wasn't a flag, re-parse as 2 args (src dest)
  // This handles edge cases slightly better, but sscanf consumes input.
  // Let's rely on the flag check.
  if (has_flag && flag[0] != '-') {
    // This means we read 3 words but first isn't a flag.
    // For now, just error or assume user messed up.
    printf(
        "Error: Invalid flag '%s'. Usage: cp [-f|-d] <source> <destination>\n",
        flag);
    return;
  }

  char buffer[1024];
  int len = nexus_read_file(src, buffer, 1023);
  if (len < 0) {
    printf("Error: Source file '%s' not found (ret: %d).\n", src, len);
    return;
  }
  buffer[len] = 0; // Ensure null termination

  // Check if dest is a directory
  int dest_dir_id = is_directory(dest);
  char final_dest[64];
  int target_dir_id = cwd_id;

  // Flag Logic
  int force_rename = (has_flag && strcmp(flag, "-f") == 0);
  int force_dir = (has_flag && strcmp(flag, "-d") == 0);

  if (force_dir) {
    // -d: Dest MUST be a directory
    if (dest_dir_id < 0) {
      printf("Error: Destination '%s' is not a directory (-d flag used).\n",
             dest);
      return;
    }
    // Copy into directory - use basename only
    strcpy(final_dest, get_basename(src));
    target_dir_id = dest_dir_id;
  } else if (force_rename) {
    // -f: Dest is the new filename (rename/overwrite)
    if (dest_dir_id >= 0) {
      printf("Error: Destination '%s' is a directory. Cannot overwrite with "
             "file (-f flag).\n",
             dest);
      return;
    }
    strcpy(final_dest, dest);
  } else {
    // Default Auto-Detection
    if (dest_dir_id >= 0) {
      // Dest is a directory, use basename only
      strcpy(final_dest, get_basename(src));
      target_dir_id = dest_dir_id;
    } else if (strcmp(dest, ".") == 0) {
      // Dest is current directory
      strcpy(final_dest, src);
    } else {
      // Dest is a filename or path
      // Check for directory separator
      char *last_slash = strrchr(dest, '/');
      if (last_slash) {
        // Split path
        char dir_path[64];
        int path_len = last_slash - dest;
        strncpy(dir_path, dest, path_len);
        dir_path[path_len] = 0;

        // Resolve dir
        int dir_id = is_directory(dir_path);
        if (dir_id >= 0) {
          target_dir_id = dir_id;
          strcpy(final_dest, last_slash + 1);
        } else {
          printf("Error: Directory '%s' not found.\n", dir_path);
          return;
        }
      } else {
        strcpy(final_dest, dest);
      }
    }
  }

  // Generate unique filename to prevent duplicates
  char unique_dest[64];
  generate_unique_filename(final_dest, target_dir_id, unique_dest);

  if (strcmp(unique_dest, final_dest) != 0) {
    printf("Note: File exists, renamed to '%s'\n", unique_dest);
  }

  if (nexus_create_file(unique_dest, buffer, target_dir_id) == 0) {
    if (target_dir_id != cwd_id)
      printf("Copied '%s' to directory ID %d as '%s' (%d bytes).\n", src,
             target_dir_id, unique_dest, len);
    else
      printf("Copied '%s' to '%s' (%d bytes).\n", src, unique_dest, len);
  } else {
    printf("Error: Failed to create destination file '%s' (Check permissions "
           "or if exists).\n",
           unique_dest);
  }
}

void cmd_mv(const char *args) {
  char flag[16] = {0};
  char src[64], dest[64];
  int has_flag = 0;

  // Parse arguments with optional flag
  if (sscanf(args, "%s %s %s", flag, src, dest) == 3) {
    if (flag[0] == '-') {
      has_flag = 1;
    }
  } else if (sscanf(args, "%s %s", src, dest) == 2) {
    has_flag = 0;
  } else {
    printf("Usage: mv [-f|-d] <old_name> <new_name>\n");
    return;
  }

  if (has_flag && flag[0] != '-') {
    printf(
        "Error: Invalid flag '%s'. Usage: mv [-f|-d] <old_name> <new_name>\n",
        flag);
    return;
  }

  int dest_dir_id = is_directory(dest);
  char final_dest[64];

  int force_rename = (has_flag && strcmp(flag, "-f") == 0);
  int force_dir = (has_flag && strcmp(flag, "-d") == 0);

  if (force_dir) {
    // -d: Move into directory
    if (dest_dir_id < 0) {
      printf("Error: Destination '%s' is not a directory (-d flag used).\n",
             dest);
      return;
    }
    printf("Error: Moving files between directories is not yet supported by "
           "kernel.\n");
    return;
  } else if (force_rename) {
    // -f: Rename to dest
    if (dest_dir_id >= 0) {
      printf("Error: Destination '%s' is a directory. Cannot overwrite with "
             "file (-f flag).\n",
             dest);
      return;
    }
    strcpy(final_dest, dest);
  } else {
    // Default
    if (dest_dir_id >= 0) {
      // Move into directory (Not supported yet)
      printf("Error: Moving to a directory is not yet supported by kernel "
             "(requires parent_id change).\n");
      return;
    } else {
      strcpy(final_dest, dest);
    }
  }

  int res = nexus_rename_file(src, final_dest);
  if (res == 0) { // Fix: nexus_rename_file returns 0 on success
    printf("Renamed '%s' to '%s'.\n", src, final_dest);
  } else if (res == -2) {
    printf("Error: Permission denied.\n");
  } else {
    printf("Error: Failed to rename file (ret: %d).\n", res);
  }
}

// Custom Readline with History
int shell_read_line(char *buffer, int max_len) {
  int pos = 0;                     // Current cursor position
  int len = 0;                     // Total length of the command
  int history_idx = history_count; // Points to "new" line (past end)
  char c;

  memset(buffer, 0, max_len);

  while (1) {
    if (read(STDIN_FILENO, &c, 1) <= 0)
      return -1;

    if (c == '\n') {
      buffer[len] = 0;
      printf("\n");
      fflush(stdout);
      return len;
    } else if (c == 127) { // Backspace
      if (pos > 0) {
        // Shift buffer left
        for (int i = pos; i < len; i++) {
          buffer[i - 1] = buffer[i];
        }
        len--;
        pos--;
        buffer[len] = 0;

        // Redraw line
        printf("\b"); // Move back
        for (int i = pos; i < len; i++) {
          printf("%c", buffer[i]);
        }
        printf(" "); // Erase last char
        // Move cursor back to pos
        for (int i = pos; i < len + 1; i++) {
          printf("\b");
        }
        fflush(stdout);
      }
    } else if (c == '\x1b') { // Escape Sequence
      char seq[2];
      if (read(STDIN_FILENO, &seq[0], 1) == -1)
        break;
      if (read(STDIN_FILENO, &seq[1], 1) == -1)
        break;

      if (seq[0] == '[') {
        if (seq[1] == 'A') { // UP Arrow
          if (history_idx > 0) {
            history_idx--;
            // Clear current line
            while (pos > 0) {
              printf("\b \b");
              fflush(stdout);
              pos--;
            }
            strcpy(buffer, history[history_idx]);
            len = strlen(buffer);
            pos = len;
            printf("%s", buffer);
            fflush(stdout);
          }
        } else if (seq[1] == 'B') { // DOWN Arrow
          if (history_idx < history_count) {
            history_idx++;
            // Clear current line
            while (pos > 0) {
              printf("\b \b");
              fflush(stdout);
              pos--;
            }
            if (history_idx < history_count) {
              strcpy(buffer, history[history_idx]);
            } else {
              buffer[0] = 0; // Back to empty new line
            }
            len = strlen(buffer);
            pos = len;
            printf("%s", buffer);
            fflush(stdout);
          }
        } else if (seq[1] == 'C') { // RIGHT Arrow
          if (pos < len) {
            pos++;
            printf("\033[C");
            fflush(stdout);
          }
        } else if (seq[1] == 'D') { // LEFT Arrow
          if (pos > 0) {
            pos--;
            printf("\033[D");
            fflush(stdout);
          }
        }
      }
    } else { // Regular character
      if (len < max_len - 1) {
        // Insert character at pos
        for (int i = len; i > pos; i--) {
          buffer[i] = buffer[i - 1];
        }
        buffer[pos] = c;
        len++;
        pos++;
        buffer[len] = 0;

        // Redraw from cursor
        printf("%c", c);
        for (int i = pos; i < len; i++) {
          printf("%c", buffer[i]);
        }
        // Move cursor back to pos
        for (int i = pos; i < len; i++) {
          printf("\b");
        }
        fflush(stdout);
      }
    }
  }
  return len;
}

int main() {
  char cmd[MAX_CMD_LEN];

  printf("Welcome to NexusQ-AI Shell (nsh) v1.0\n");
  printf("Type 'help' for commands.\n");

  // Create a dummy file for ls demo if none exists
  // (In a real OS, files persist, here we rely on what app_demo created
  // or create one) Actually shell is a separate process, so it won't see
  // app_demo's memory-based FS unless we share memory. BUT: Our kernel is
  // monolithic and linked into the executable. Wait, if we build
  // `nexus_shell` as a separate executable, it will have its OWN kernel
  // instance (simulated). So `ls` will be empty unless we init FS here.
  // Let's init FS here.
  extern void lfs_init(void);
  extern void sched_init(void);
  lfs_init();
  sched_init();
  extern void svc_register_system_services(void);
  svc_register_system_services();

  // Create a dummy file
  extern void *lfs_create_file(const char *name, const void *data, int size,
                               uint32_t parent_id, const char *owner);
  lfs_create_file("shell_startup.log", "Booted", 6, 0, "SYSTEM");

  // Create a Sealed Treasure Chest (Level 38)
  // Content is "GOLD_AND_QUBITS", but it's sealed.
  lfs_create_file("treasure.chest", "GOLD_AND_QUBITS", 15, 0, "SYSTEM");
  // In a real OS, we would set a flag like `file.is_sealed = 1`.
  // For now, we simulate it by checking the filename in `cat`.

  // Register Shell as a Process (Identity)
  nexus_create_process("nsh_terminal", 5); // 5 Qubits for the shell

  // Perform Login
  do_login();

  // Enable Raw Mode for History
  enable_raw_mode();

  while (1) {
    print_prompt();
    fflush(stdout); // Ensure prompt is seen before blocking read

    if (shell_read_line(cmd, sizeof(cmd)) < 0)
      break;

    if (strlen(cmd) == 0)
      continue;

    // Add to history
    history_add(cmd);

    if (strcmp(cmd, "exit") == 0) {
      disable_raw_mode(); // Restore before shutdown
      nexus_shutdown();
      break;
    } else if (strcmp(cmd, "sysinfo") == 0 || strcmp(cmd, "status") == 0)
      cmd_sysinfo();
    else if (strcmp(cmd, "ls") == 0 || strcmp(cmd, "dir") == 0)
      cmd_ls();
    else if (strcmp(cmd, "ps") == 0 || strcmp(cmd, "qtop") == 0)
      cmd_ps();
    else if (strncmp(cmd, "visualize", 9) == 0)
      cmd_visualize(cmd + 10);
    else if (strcmp(cmd, "gui_test") == 0)
      cmd_gui_test();
    else if (strncmp(cmd, "touch", 5) == 0)
      cmd_touch(cmd + 6);
    else if (strncmp(cmd, "cat", 3) == 0)
      cmd_cat(cmd + 4);
    else if (strncmp(cmd, "rm", 2) == 0)
      cmd_rm(cmd + 3);
    else if (strncmp(cmd, "mkdir", 5) == 0) {
      char name[64];
      if (sscanf(cmd + 6, "%s", name) == 1) {
        nexus_create_directory(name, cwd_id); // Use current CWD
        printf("Directory '%s' created.\n", name);
      }
    } else if (strncmp(cmd, "qedit", 5) == 0) {
      char filename[64];
      if (sscanf(cmd + 6, "%s", filename) == 1) {
        extern void qedit_main(const char *filename);
        disable_raw_mode(); // QEdit might need its own mode or canonical
        qedit_main(filename);
        enable_raw_mode();
      } else {
        printf("Usage: qedit <filename>\n");
      }
    } else if (strncmp(cmd, "qrun", 4) == 0) {
      char filename[64];
      if (sscanf(cmd + 5, "%s", filename) == 1) {
        char buffer[1024];
        if (nexus_read_file(filename, buffer, 1023) > 0) {
          extern void qsl_execute(const char *script_content);
          qsl_execute(buffer);
        } else {
          printf("Error: Could not read script '%s'.\n", filename);
        }
      } else {
        printf("Usage: qrun <filename>\n");
      }

    } else if (strncmp(cmd, "cd", 2) == 0) {
      char name[64];
      if (sscanf(cmd + 3, "%s", name) == 1) {
        cmd_cd(name);
      } else {
        printf("Usage: cd <name>\n");
      }
    } else if (strcmp(cmd, "whoami") == 0)
      cmd_whoami();
    else if (strcmp(cmd, "help") == 0)
      cmd_help();
    else if (strncmp(cmd, "net", 3) == 0)
      cmd_net(cmd + 4);
    else if (strncmp(cmd, "teleport", 8) == 0)
      cmd_teleport(cmd + 9);
    else if (strncmp(cmd, "spawn", 5) == 0)
      cmd_spawn(cmd + 6);
    else if (strncmp(cmd, "mount", 5) == 0)
      cmd_mount(cmd + 6);
    else if (strcmp(cmd, "clear") == 0)
      cmd_clear();
    else if (strncmp(cmd, "vision", 6) == 0)
      cmd_vision(cmd + 7);
    else if (strcmp(cmd, "services") == 0)
      cmd_services();
    else if (strncmp(cmd, "kill", 4) == 0)
      cmd_kill(cmd + 5);
    else if (strncmp(cmd, "renice", 6) == 0)
      cmd_renice(cmd + 7);
    else if (strncmp(cmd, "top", 3) == 0) {
      cmd_top();
    } else if (strcmp(cmd, "dmesg") == 0) {
      cmd_dmesg();
    } else if (strcmp(cmd, "cleanup") == 0) {
      cmd_cleanup();
    } else if (strcmp(cmd, "free") == 0) {
      cmd_free();
    } else if (strncmp(cmd, "cp", 2) == 0) {
      cmd_cp(cmd + 3);
    } else if (strncmp(cmd, "mv", 2) == 0)
      cmd_mv(cmd + 3);
    else if (strncmp(cmd, "qexec", 5) == 0) {
      char filename[64];
      if (sscanf(cmd + 6, "%s", filename) == 1) {
        cmd_qexec(filename);
      } else {
        printf("Usage: qexec <circuit_file>\n");
      }
    } else if (strncmp(cmd, "qdbg", 4) == 0) {
      char filename[64];
      if (sscanf(cmd + 5, "%s", filename) == 1) {
        // Read circuit file
        char buffer[2048];
        int len = nexus_read_file(filename, buffer, sizeof(buffer) - 1);
        if (len < 0) {
          printf("Error: Could not read circuit file '%s'\n", filename);
        } else {
          buffer[len] = '\0';
          extern void qdbg_main(const char *circuit_text);
          disable_raw_mode();
          qdbg_main(buffer);
          enable_raw_mode();
        }
      } else {
        printf("Usage: qdbg <circuit_file>\n");
      }
    } else if (strcmp(cmd, "pwd") == 0)
      cmd_pwd();
    else if (strcmp(cmd, "qmonitor") == 0)
      cmd_qmonitor();
    else if (strcmp(cmd, "qstats") == 0)
      cmd_qstats();
    else if (strcmp(cmd, "qreset") == 0)
      cmd_qreset();
    else if (strncmp(cmd, "qopt", 4) == 0)
      cmd_qopt(cmd + 5);
    else if (strncmp(cmd, "qexport", 7) == 0)
      cmd_qexport(cmd + 8);
    else if (strncmp(cmd, "qimport", 7) == 0)
      cmd_qimport(cmd + 8);
    else if (strncmp(cmd, "qvis", 4) == 0)
      cmd_qvis(cmd + 5);
    else if (strncmp(cmd, "qprof", 5) == 0)
      cmd_qprof(cmd + 6);
    else if (strncmp(cmd, "audit", 5) == 0)
      cmd_audit(cmd + 6);
    else if (strcmp(cmd, "permissions") == 0)
      cmd_permissions();
    else if (strcmp(cmd, "gov_cleanup") == 0)
      cmd_cleanup_fs();
    else {
      printf("Unknown command: %s\n", cmd);
    }

    // Simulate a tick to keep the world moving
    nexus_simulate_tick(100);
  }

  printf("Logout.\n");
  return 0;
}
