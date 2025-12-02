/*
 * NexusQ-AI System Configuration
 * File: include/sys/config.h
 *
 * Centralized configuration for system tuning.
 */

#ifndef _SYS_CONFIG_H_
#define _SYS_CONFIG_H_

// --- Scheduler Configuration ---
#define QUANTUM_TIME_SLICE 10    // Max ticks for a QPU process
#define COHERENCE_THRESHOLD 20.0 // Coherence warning threshold (µs)
#define MAX_RUNNING_PROCS 64     // Max processes in RunQueue

// --- Quantum Process Configuration ---
#define MAX_Q_REGS 8 // Max quantum registers per process

// --- Quantum Network Configuration ---
#define MAX_ENTANGLEMENT_BANK 64 // Max EPR pairs in local bank
#define LOCAL_NODE_ID 1          // Current Node ID (Static for now)

// --- Fidelity Thresholds ---
#define QNET_FIDELITY_LOW 0.80   // Good for QKD
#define QNET_FIDELITY_HIGH 0.95  // Required for Teleportation
#define QNET_FIDELITY_ULTRA 0.99 // Required for Distributed Computing

#endif // _SYS_CONFIG_H_
