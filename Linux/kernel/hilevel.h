#ifndef __HILEVEL_H
#define __HILEVEL_H

#include <stdlib.h>

// Include functionality relating to newlib (the standard C library).
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <string.h>

// Include functionality relating to the platform.
#include   "GIC.h"
#include "PL011.h"
#include "PL050.h"
#include "PL111.h"
#include "SP804.h"
#include   "SYS.h"

// Include functionality relating to the   kernel.
#include "lolevel.h"
#include "int.h"

// Include functionality relating to the graphic interface
#include "fonts.h"
#include "interface.h"

/* status = captures the status of a process.
 * ctx    = captures each component of an execution context (processor state)
 *          in a compatible order wrt. the low-level handler
 *          preservation and restoration prologue and epilogue.
 * PCB    = a type that captures a process control block.
 * PID    = Process Identifier (integer).
 */

#define READ  0
#define WRITE 1

typedef int pid_t;

typedef enum {
    CREATED,
    READY,
    EXECUTING,
    WAITING_READ,
    WAITING_WRITE,
    TERMINATED
} status_t;

typedef struct {
    uint32_t cpsr, pc, gpr[ 13 ], sp, lr;
} ctx_t;

typedef struct {
    pid_t    pid;
    int      age;
    int      priority;
    int      niceness;
    float    time;
    status_t status;
    ctx_t    ctx;
    uint32_t tos;
} pcb_t;

// pipe inode info
typedef struct {
    char buffer[ 100 ];
    int  size;
    int  readers;
    int  writers;
} pipe_inode_t;

typedef struct {
    pipe_inode_t* pipe;
    int           free;
    int           access;
} fildes_t;

#endif
