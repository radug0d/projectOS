#ifndef MONITOR_STATE_H__
#define MONITOR_STATE_H__

#include <sys/types.h>

typedef enum {OFFLINE, RUNNING, SHUTTING_DOWN} monitor_state;

typedef struct {
    pid_t pid;
    monitor_state state;
} Monitor_t;

extern Monitor_t *monitor_ex;

#endif