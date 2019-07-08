#include "pipe.h"

/* a simple example of 2 processes that communicate through pipes
 * The parent writes the second message after a delay of 3s,
 * demonstrating the use of blocking read / write operations
 */
void main_pipe() {
    int fildes[ 2 ];
    int status = pipe( fildes );

    switch ( fork() ) {
        case -1:
        {/* Handle error */
            break;
        }
        case 0:
        { /* Child - reads from pipe */
            char buf[ 100 ];
            int  nbytes;

            write( STDOUT_FILENO, "Child reads\n", 12 );

            nbytes = read(fildes[0], buf, 14);

            write( STDOUT_FILENO, "Child: received ", 16 );
            write( STDOUT_FILENO, buf, nbytes );

            nbytes = read(fildes[0], buf, 18);

            write( STDOUT_FILENO, "Child: received ", 16 );
            write( STDOUT_FILENO, buf, nbytes );

            close( fildes[0] );
            write( STDOUT_FILENO, "Child: Shutting down.\n", 22 );
            exit( EXIT_SUCCESS );
        }

        default:
        { /* Parent - writes to pipe */
            const char msg1[] = "\"Hello world\"\n";
            const char msg2[] = "\"Another Message\"\n";

            write( STDOUT_FILENO, "Parent: sending \"Hello world\"\n", 30 );
            write(fildes[1], msg1, 14);

            for ( int c = 1 ; c <= 468951; c++ )
                for ( int d = 1 ; d <= 3000; d++ )
                {
                    asm ( "nop" );
                }

            write( STDOUT_FILENO, "Parent: sending \"Another Message\"\n", 34 );
            write( fildes[1], msg2, 18 );

            close( fildes[1]);
            write( STDOUT_FILENO, "Parent: Shutting down.\n", 23 );
            exit( EXIT_SUCCESS );
        }
    }
}
