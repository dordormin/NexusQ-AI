/*
 * NexusQ-AI - Quantum System Monitor
 * File: modules/quantum/qmonitor.c
 *
 * Real-time monitoring and statistics for quantum operations
 */

#include "include/qvm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_HISTORY 100

// Execution statistics
typedef struct {
  char circuit_name[64];
  int num_qubits;
  int num_gates;
  double execution_time_ms;
  time_t timestamp;
  int success;
} exec_history_t;

// Global statistics
static exec_history_t history[MAX_HISTORY];
static int history_count = 0;
static int total_executions = 0;
static int successful_executions = 0;
static double total_execution_time = 0.0;

// Gate usage statistics
static int gate_usage[10] = {0}; // One counter per gate type

// Record execution
void qmonitor_record_execution(const char *name, int qubits, int gates,
                               double time_ms, int success) {
  if (history_count >= MAX_HISTORY) {
    // Rotate history
    for (int i = 0; i < MAX_HISTORY - 1; i++) {
      history[i] = history[i + 1];
    }
    history_count = MAX_HISTORY - 1;
  }

  exec_history_t *entry = &history[history_count++];
  strncpy(entry->circuit_name, name, 63);
  entry->num_qubits = qubits;
  entry->num_gates = gates;
  entry->execution_time_ms = time_ms;
  entry->timestamp = time(NULL);
  entry->success = success;

  total_executions++;
  if (success)
    successful_executions++;
  total_execution_time += time_ms;
}

// Record gate usage
void qmonitor_record_gate(int gate_type) {
  if (gate_type >= 0 && gate_type < 10) {
    gate_usage[gate_type]++;
  }
}

// External Getters
extern void sched_get_stats(int *active_procs, double *avg_coherence);
extern void qec_get_stats(int *detected, int *corrected);
extern void qkd_get_stats(int *keys, float *qber);

// Print dashboard
void qmonitor_dashboard() {
  // Fetch Stats
  int active_procs = 0;
  double avg_coherence = 0.0;
  sched_get_stats(&active_procs, &avg_coherence);

  int qec_detected = 0, qec_corrected = 0;
  qec_get_stats(&qec_detected, &qec_corrected);

  int qkd_keys = 0;
  float qkd_qber = 0.0f;
  qkd_get_stats(&qkd_keys, &qkd_qber);

  printf("\n");
  printf("╔════════════════════════════════════════════════════════════════════"
         "══╗\n");
  printf("║                  QUANTUM SYSTEM MONITOR - MISSION CONTROL          "
         "  ║\n");
  printf("╚════════════════════════════════════════════════════════════════════"
         "══╝\n");

  // Row 1: System Health & Network
  printf("┌─── System Health ──────────────────┐  ┌─── Network Status "
         "─────────────────┐\n");
  printf("│ Active Processes:     %-5d        │  │ Keys Generated:       %-5d  "
         "      │\n",
         active_procs, qkd_keys);
  printf("│ Avg Coherence:        %-5.1f us    │  │ Last QBER:            "
         "%-5.2f%%       │\n",
         avg_coherence, qkd_qber);
  printf("└────────────────────────────────────┘  "
         "└────────────────────────────────────┘\n");

  // Row 2: Reliability & Execution
  printf("┌─── Reliability ────────────────────┐  ┌─── Execution Stats "
         "────────────────┐\n");
  printf("│ Errors Detected:      %-5d        │  │ Total Executions:     %-5d  "
         "      │\n",
         qec_detected, total_executions);
  printf("│ Errors Corrected:     %-5d        │  │ Success Rate:         "
         "%-5.1f%%       │\n",
         qec_corrected,
         total_executions > 0
             ? (100.0 * successful_executions / total_executions)
             : 0.0);
  printf("└────────────────────────────────────┘  "
         "└────────────────────────────────────┘\n");

  // Gate Usage
  printf("\n┌─── Gate Usage Statistics "
         "─────────────────────────────────────────┐\n");
  const char *gate_names[] = {"H", "X",    "Y",  "Z",    "T",
                              "S", "CNOT", "CZ", "SWAP", "M"};
  int max_usage = 0;
  for (int i = 0; i < 10; i++) {
    if (gate_usage[i] > max_usage)
      max_usage = gate_usage[i];
  }

  for (int i = 0; i < 10; i++) {
    if (gate_usage[i] > 0) {
      printf("│ %-6s: %4d  ", gate_names[i], gate_usage[i]);
      int bar_len = max_usage > 0 ? (gate_usage[i] * 40 / max_usage) : 0;
      for (int j = 0; j < bar_len; j++)
        printf("█");
      printf("\n");
    }
  }
  printf("└───────────────────────────────────────────────────────────────────┘"
         "\n");

  // Recent Executions
  printf("\n┌─── Recent Executions (Last 10) "
         "──────────────────────────────────┐\n");
  printf("│ %-20s | %6s | %6s | %10s | %s\n", "Circuit", "Qubits", "Gates",
         "Time(ms)", "Status");
  printf("├───────────────────────────────────────────────────────────────────┤"
         "\n");

  int start = history_count > 10 ? history_count - 10 : 0;
  for (int i = start; i < history_count; i++) {
    printf("│ %-20s | %6d | %6d | %10.2f | %s\n", history[i].circuit_name,
           history[i].num_qubits, history[i].num_gates,
           history[i].execution_time_ms, history[i].success ? "✓" : "✗");
  }

  if (history_count == 0) {
    printf("│ No executions recorded yet.                                      "
           "│\n");
  }
  printf("└───────────────────────────────────────────────────────────────────┘"
         "\n");
  printf("\n");
}

// Print detailed statistics
void qmonitor_stats() {
  printf("\n╔═══════════════════════════════════════════════════════════╗\n");
  printf("║            DETAILED QUANTUM STATISTICS                   ║\n");
  printf("╚═══════════════════════════════════════════════════════════╝\n\n");

  if (history_count == 0) {
    printf("No execution history available.\n");
    return;
  }

  // Calculate statistics
  int qubit_usage[17] = {0}; // 0-16 qubits
  double min_time = history[0].execution_time_ms;
  double max_time = history[0].execution_time_ms;

  for (int i = 0; i < history_count; i++) {
    if (history[i].num_qubits >= 0 && history[i].num_qubits <= 16) {
      qubit_usage[history[i].num_qubits]++;
    }
    if (history[i].execution_time_ms < min_time)
      min_time = history[i].execution_time_ms;
    if (history[i].execution_time_ms > max_time)
      max_time = history[i].execution_time_ms;
  }

  printf("Execution Time Statistics:\n");
  printf("  Min: %.2f ms\n", min_time);
  printf("  Max: %.2f ms\n", max_time);
  printf("  Avg: %.2f ms\n",
         total_executions > 0 ? total_execution_time / total_executions : 0);
  printf("\n");

  printf("Qubit Distribution:\n");
  for (int i = 0; i <= 16; i++) {
    if (qubit_usage[i] > 0) {
      printf("  %2d qubits: %3d circuits (%.1f%%)\n", i, qubit_usage[i],
             100.0 * qubit_usage[i] / history_count);
    }
  }
  printf("\n");

  printf("Success Rate: %.1f%% (%d/%d)\n",
         total_executions > 0 ? 100.0 * successful_executions / total_executions
                              : 0,
         successful_executions, total_executions);
}

// Export statistics to file
int qmonitor_export(const char *filename) {
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    printf("[QMONITOR] Error: Cannot write to '%s'\n", filename);
    return -1;
  }

  fprintf(fp, "# NexusQ-AI Quantum Monitor Export\n");
  fprintf(fp, "# Generated: %s\n", ctime(&(time_t){time(NULL)}));
  fprintf(fp, "\n");
  fprintf(fp, "[Summary]\n");
  fprintf(fp, "total_executions=%d\n", total_executions);
  fprintf(fp, "successful_executions=%d\n", successful_executions);
  fprintf(fp, "total_time_ms=%.2f\n", total_execution_time);
  fprintf(fp, "\n");

  fprintf(fp, "[Gate_Usage]\n");
  const char *gate_names[] = {"H", "X",    "Y",  "Z",    "T",
                              "S", "CNOT", "CZ", "SWAP", "M"};
  for (int i = 0; i < 10; i++) {
    fprintf(fp, "%s=%d\n", gate_names[i], gate_usage[i]);
  }
  fprintf(fp, "\n");

  fprintf(fp, "[History]\n");
  for (int i = 0; i < history_count; i++) {
    fprintf(fp, "%s,%d,%d,%.2f,%ld,%d\n", history[i].circuit_name,
            history[i].num_qubits, history[i].num_gates,
            history[i].execution_time_ms, history[i].timestamp,
            history[i].success);
  }

  fclose(fp);
  printf("[QMONITOR] Statistics exported to '%s'\n", filename);
  return 0;
}

// Reset statistics
void qmonitor_reset() {
  history_count = 0;
  total_executions = 0;
  successful_executions = 0;
  total_execution_time = 0.0;
  memset(gate_usage, 0, sizeof(gate_usage));
  printf("[QMONITOR] Statistics reset.\n");
}
