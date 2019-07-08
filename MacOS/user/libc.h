
#ifndef __LIBC_H
#define __LIBC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef int pid_t;

/* The definitions below capture symbolic constants within these classes:
 *
 * 1. system call identifiers (i.e., the constant used by a system call
 *    to specify which action the kernel should take),
 * 2. signal identifiers (as used by the kill system call),
 * 3. status codes for exit,
 * 4. standard file descriptors (e.g., for read and write system calls),
 * 5. platform-specific constants, which may need calibration (wrt. the
 *    underlying hardware QEMU is executed on).
 *
 * They don't *precisely* match the standard C library, but are intended
 * to act as a limited model of similar concepts.
 */

#define SYS_YIELD     ( 0x00 )
#define SYS_PIPE      ( 0x01 )
#define SYS_WRITE     ( 0x02 )
#define SYS_READ      ( 0x03 )
#define SYS_WRITE_NB  ( 0x04 )
#define SYS_READ_NB   ( 0x05 )
#define SYS_CLOSE     ( 0x06 )
#define SYS_FORK      ( 0x07 )
#define SYS_EXIT      ( 0x08 )
#define SYS_EXEC      ( 0x09 )
#define SYS_KILL      ( 0x0A )
#define SYS_NICE      ( 0x0B )
#define SYS_TOP       ( 0x0C )

#define SIG_TERM      ( 0x00 )
#define SIG_QUIT      ( 0x01 )

#define EXIT_SUCCESS  ( 0 )
#define EXIT_FAILURE  ( 1 )

#define  STDIN_FILENO ( 0 )
#define STDOUT_FILENO ( 1 )
#define STDERR_FILENO ( 2 )

// convert ASCII string x into integer r
extern int atoi( char* x        );
// convert integer x into ASCII string r
extern int itoa( char* r, int x );

// cooperatively yield control of processor, i.e., invoke the scheduler
extern void yield();

// create an interprocess channel
extern int  pipe( int fildes[2] );
// write n bytes from x to   the file descriptor fd; return bytes written
extern int write( int fildes, const void* x, size_t n );
// read  n bytes into x from the file descriptor fd; return bytes read
extern int  read( int fildes,       void* x, size_t n );
// non-blocking version of write
extern int write_nb( int fildes, const void* x, size_t n );
// non-blocking version of read
extern int  read_nb( int fildes,       void* x, size_t n );
//
extern int close( int fildes );

// perform fork, returning 0 iff. child or > 0 iff. parent process
extern int  fork();
// perform exit, i.e., terminate process with status x
extern void exit(       int   x );
// perform exec, i.e., start executing program at address x
extern void exec( const void* x );

// for process identified by pid, send signal of x
extern int  kill( pid_t pid, int x );
// for process identified by pid, set  priority to x
extern void nice( pid_t pid, int x );

//
extern void top();

#endif
