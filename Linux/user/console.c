#include "console.h"

void puts_c( char* x, int n ) {
  for( int i = 0; i < n; i++ ) {
    PL011_putc( UART1, x[ i ], true );
  }
}

void gets_c( char* x, int n ) {
  for( int i = 0; i < n; i++ ) {
    x[ i ] = PL011_getc( UART1, true );

    if( x[ i ] == '\x0A' ) {
      x[ i ] = '\x00'; break;
    }
  }
}

/* Since we lack a *real* loader (as a result of also lacking a storage
 * medium to store program images), the following function approximates
 * one: given a program name from the set of programs statically linked
 * into the kernel image, it returns a pointer to the entry point.
 */

extern void main_P3();
extern void main_P4();
extern void main_P5();
extern void main_pipe();
extern void main_philosophers();

void* load_c( char* x ) {
  if     ( 0 == strcmp( x, "P3"           ) ) {
      return &main_P3;
  }
  else if( 0 == strcmp( x, "P4"           ) ) {
      return &main_P4;
  }
  else if( 0 == strcmp( x, "P5"           ) ) {
      return &main_P5;
  }
  else if( 0 == strcmp( x, "pipe"         ) ) {
      return &main_pipe;
  }
  else if( 0 == strcmp( x, "philosophers" ) ) {
      return &main_philosophers;
  }

  return NULL;
}

/* The behaviour of a console process can be summarised as an infinite
 * loop over three main steps, namely
 *
 * 1. write a command prompt then read a command,
 * 2. split the command into space-separated tokens using strtok, then
 * 3. execute whatever steps the command dictates.
 *
 * As is, the console only recognises the following commands:
 *
 * a. execute <program name>
 *
 *    This command will use fork to create a new process; the parent
 *    (i.e., the console) will continue as normal, whereas the child
 *    uses exec to replace the process image and thereby execute a
 *    different (named) program.  For example,
 *
 *    execute P3
 *
 *    would execute the user program named P3, with priority 120, niceness 0.
 *
 * b. nice <niceness> <program name>
 *
 *    Same as execute, but sets niceness value to <niceness>
 *
 * c. renice <niceness> <process ID>
 *
 *    Resets the niceness value of the process with given PID
 *    to the value given.
 *
 * d. terminate <process ID>
 *
 *    This command uses kill to send a terminate or SIG_TERM signal
 *    to a specific process (identified via the PID provided); this
 *    acts to forcibly terminate the process, vs. say that process
 *    using exit to terminate itself.  For example,
 *
 *    terminate 3
 *
 *    would terminate the process whose PID is 3.
 *
 *    terminate 0
 *
 *    would terminate all the processes except the console
 *
 */

void main_console() {
  char* p, x[ 1024 ];

  while( 1 ) {
    puts_c( "shell$ ", 7 ); gets_c( x, 1024 ); p = strtok( x, " " );

    if( 0 == strcmp( p, "execute" ) )
    {
      void* program = load_c( strtok( NULL, " " ) );
      if( NULL == program ) {
          puts_c( "Invalid program\n", 16 );
      }
      else {
          pid_t pid = fork();

          if( 0 == pid ) {
            exec( program );
          }
      }
    }
    else if( 0 == strcmp( p, "terminate" ) )
    {
      pid_t pid = atoi( strtok( NULL, " " ) );
      int   s   = atoi( strtok( NULL, " " ) );

      int r = kill( pid, s );
      if( r == 0 ) {
          if( pid == 0) {
              puts_c( "All processes terminated successfully\n", 38 );
          }
          else {
              puts_c( "Process with PID ", 17 );
              char* m; itoa( m, pid );
              if( pid < 10) {
                  puts_c( m, 1 );
              }
              else if( pid >= 10 ) {
                  puts_c( m, 2 );
              }
              puts_c( " terminated successfully\n", 25 );
          }
      }
      else if( r == -1) {
          puts_c( "Kill failed\n", 13 );
      }
    }
    else if( 0 == strcmp( p, "nice" ) )
    {
        int pr    = atoi( strtok( NULL, " " ) );
        void* program = load_c( strtok( NULL, " " ) );
        if( (pr < -20) || (pr > 19) )
        {
            puts_c( "Invalid nice value\n", 19 );
            puts_c( "Please enter a value from -20 to 19\n", 36 );
        }
        else if( NULL == program ) {
            puts_c( "Invalid program\n", 16 );
        }
        else {
            pid_t pid = fork();

            if( 0 == pid ) {
              exec( program );
            }
            else {
                nice( pid, pr );
                // -20 .. 19
            }
        }
    }
    else if( 0 == strcmp( p, "renice" ) )
    {
        int pr    = atoi( strtok( NULL, " " ) );
        pid_t pid = atoi( strtok( NULL, " " ) );
        if( (pr < -20) || (pr > 19) ) {
            puts_c( "Invalid nice value\n", 19 );
            puts_c( "Please enter a value from -20 to 19\n", 36 );
        }
        else {
            nice( pid, pr );
            // -20 .. 19
        }
    }
    else if( 0 == strcmp( p, "top" ) ) {
        top();
    }
    else if( 0 == strcmp( p, "help" ) )
    {
        puts_c( "*** add help page here ***\n", 27 );
    }
    else
    {
      puts_c( "unknown command\n", 16 );
    }
  }

  exit( EXIT_SUCCESS );
}
