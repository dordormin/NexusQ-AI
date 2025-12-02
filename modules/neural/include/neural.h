/*
 * NexusQ-AI Neural Engine Interface
 * File: modules/neural/include/neural.h
 */

#ifndef _NEURAL_H_
#define _NEURAL_H_

#include <stdint.h>

// --- Types ---
typedef struct {
    float weights[10]; // Modèle simple (Perceptron multicouche simulé)
    float bias;
    float learning_rate;
    int generation;
} neural_model_t;

// --- API ---

// Initialisation du moteur neuronal
void neural_init(void);

// Inférence : Prédit la charge CPU (Burst) d'un processus (0.0 - 1.0)
// pid: Process ID
// history_metric: Une métrique historique (ex: temps CPU passé)
float neural_predict_burst(int pid, float history_metric);

// Entraînement : Effectue une étape de backpropagation (Simulé)
void neural_train_step(void);

#endif // _NEURAL_H_
