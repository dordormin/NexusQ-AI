/*
 * NexusQ-AI Neural Inference Module
 * File: modules/neural/inference/predict.c
 */

#include "../include/neural.h"
#include <stdio.h>

// État global du modèle (partagé avec le module d'entraînement)
extern neural_model_t global_model;

float neural_predict_burst(int pid, float history_metric) {
  // Simulation d'un Forward Pass
  // Dans un vrai système, ce serait un calcul matriciel sur NPU.

  // Entrée : PID (normalisé) + Historique
  float input1 = (pid % 100) / 100.0f;
  float input2 = history_metric;

  // Calcul simple : w1*i1 + w2*i2 + bias
  // float activation = (input1 * global_model.weights[0]) +
  //                    (input2 * global_model.weights[1]) +
  //                    global_model.bias;

  // ASM Optimized Version
  extern float neural_dot_product_asm(float *weights, float *inputs, int count);
  float inputs[2] = {input1, input2};
  float activation_classical =
      neural_dot_product_asm(global_model.weights, inputs, 2) +
      global_model.bias;

  // --- QUANTUM LAYER (QNN) ---
  extern float neural_pqc_run(float *inputs, float *weights);
  float activation_quantum = neural_pqc_run(inputs, global_model.weights);

  // Hybrid Fusion: Classical + Quantum
  float activation =
      (activation_classical * 0.7f) + (activation_quantum * 0.3f);

  // Fonction d'activation (Sigmoid simplifiée)
  if (activation > 1.0f)
    activation = 1.0f;
  if (activation < 0.0f)
    activation = 0.0f;

  // Log de debug (pour voir l'IA "réfléchir")
  // printf("[NEURAL] Inference PID %d -> Prediction: %.2f (Hybrid)\n", pid,
  // activation);

  return activation;
}
