/*
 * NexusQ-AI Kernel - Neural Abstraction Layer (NAL) Interface
 * File: include/sys/neural.h
 *
 * BSD-style Sockets for Neural Networks.
 */

#ifndef _SYS_NEURAL_H_
#define _SYS_NEURAL_H_

#include <stddef.h>
#include <stdint.h>

// Address Family for Neural Networks
#define AF_NEURAL 42

// Socket Types
#define SOCK_NEURAL_STREAM 1 // Standard Stream
#define SOCK_INFER 2         // Inference Request (One-shot)

// Protocols
#define NPROTO_VISION 1
#define NPROTO_NLP 2
#define NPROTO_QNN 3
#define NPROTO_SCHED 4
#define NPROTO_SEC 5

// Neural Socket Address Structure
struct sockaddr_neural {
  uint16_t sn_family; // AF_NEURAL
  char sn_model[64];  // Model Name (e.g., "resnet50", "gpt-nano")
};

// IOCTL Commands for Neural Sockets
#define NIOC_LOAD_MODEL 0xN001
#define NIOC_GET_STATUS 0xN002
#define NIOC_SET_PRECISION 0xN003

// Data Structures for Inference
typedef struct {
  uint32_t width;
  uint32_t height;
  uint32_t channels;
  uint32_t format; // 0=RGB, 1=Grayscale
  uint8_t *data;   // Pointer to pixel data
} neural_image_t;

typedef struct {
  char label[64];
  float confidence;
} neural_result_t;

// Kernel API (Internal)
int nal_socket(int domain, int type, int protocol);
int nal_connect(int sockfd, const struct sockaddr_neural *addr);
int nal_send(int sockfd, const void *buf, size_t len);
int nal_recv(int sockfd, void *buf, size_t len);
int nal_close(int sockfd);

#endif // _SYS_NEURAL_H_
