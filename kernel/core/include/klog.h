#ifndef KLOG_H
#define KLOG_H

// Log levels
#define KLOG_DEBUG 0
#define KLOG_INFO 1
#define KLOG_WARN 2
#define KLOG_ERROR 3

// Logging API
void klog_init(void);
void klog_write(int level, const char *fmt, ...);
int klog_read(char *buffer, int max_size);

// Convenience macros
#define LOG_DEBUG(...) klog_write(KLOG_DEBUG, __VA_ARGS__)
#define LOG_INFO(...) klog_write(KLOG_INFO, __VA_ARGS__)
#define LOG_WARN(...) klog_write(KLOG_WARN, __VA_ARGS__)
#define LOG_ERROR(...) klog_write(KLOG_ERROR, __VA_ARGS__)

#endif // KLOG_H
