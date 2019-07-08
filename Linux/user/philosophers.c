
#include "philosophers.h"

#define N_PHILOSOPHERS 16

#define AVAILABLE   0
#define RESERVED    1
#define UNAVAILABLE 2

/*
 * ----------------------------
 * DINING PHILOSOPHERS PROBLEM:
 * ----------------------------
 *
 *           0   P0   1   P1   2   P1   3   P3   4
 *     ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
 *     ┃     \   ⬤   |   ⬤   |   ⬤   |   ⬤   /    ┃
 *     ┃                                              ┃
 * P15 ┃ ⬤   ░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░   ⬤ ┃ P4
 *     ┃      ▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒      ┃
 * 15  ┃ --   ░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░   -- ┃ 5
 *     ┃      ▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒      ┃
 * P14 ┃ ⬤   ░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░   ⬤ ┃ P5
 *     ┃      ▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒      ┃
 *  14 ┃ --   ░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░   -- ┃ 6
 *     ┃      ▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒      ┃
 * P13 ┃ ⬤   ░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░   ⬤ ┃ P6
 *     ┃      ▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒      ┃
 *  13 ┃ --   ░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░   -- ┃ 7
 *     ┃      ▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒      ┃
 * P12 ┃ ⬤   ░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░▒▒░░   ⬤ ┃ P7
 *     ┃                                              ┃
 *     ┃     /   ⬤   |   ⬤   |   ⬤   |   ⬤   \    ┃
 *     ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
 *          12   P11  11  P10  10  P9   9   P8   8
 *
 * In this configuration:
 * - fork number (i+1)%N is to the left  of philosopher i
 * - fork number  i      is to the right of philosopher i
 */

void sleep( int msec );
void print_index( int id );

void philosopher(    int read_fd, int write_fd, int id );
void pick_up_forks(  int read_fd, int write_fd, int id );
void put_down_forks( int read_fd, int write_fd, int id );
void eat(   int id );
void think( int id );

void main_philosophers() {
    write( STDOUT_FILENO, "\n", 1 );
    int waiter_read_fd[ N_PHILOSOPHERS ];
    int waiter_write_fd[ N_PHILOSOPHERS ];

    for( int i = 0; i < N_PHILOSOPHERS; i++ ) {
        int fildes_w_p[ 2 ], fildes_p_w[ 2 ];
        int status;
        status = pipe( fildes_w_p ); // waiter -> philosopher pipe
        status = pipe( fildes_p_w ); // philosopher -> waiter pipe

        waiter_read_fd[ i ]      = fildes_p_w[ 0 ];
        waiter_write_fd[ i ]     = fildes_w_p[ 1 ];
        int philosopher_read_fd  = fildes_w_p[ 0 ];
        int philosopher_write_fd = fildes_p_w[ 1 ];

        switch( fork() ) {
            case 0: /* Child = philosopher */
                philosopher( philosopher_read_fd, philosopher_write_fd, i );

            default: /* Parent             */
                break;
        }
    }

    // parent = waiter process:
    int forks[ N_PHILOSOPHERS ];
    for( int i = 0; i < N_PHILOSOPHERS; i++ )
        forks[ i ] = AVAILABLE;
    char msg[ 100 ];

    while(1)
    {
        for( int i = 0; i < N_PHILOSOPHERS; i++ )
        {
            int nbytes = read_nb( waiter_read_fd[ i ], msg, 100);
            if( nbytes ) {
                if     ( 0 == strcmp( msg, "request left fork" ) )
                    if( forks[ (i+1)%N_PHILOSOPHERS ] == AVAILABLE &&
                        forks[ i ] == AVAILABLE )
                    {
                        forks[ (i+1)%N_PHILOSOPHERS ] = UNAVAILABLE;
                        forks[ i ] = RESERVED;
                        write( waiter_write_fd[ i ], "yes", 4);
                    }
                    else {
                        write( waiter_write_fd[ i ], "no", 4);

                    }
                else if( 0 == strcmp( msg, "request right fork" ) )
                    if( forks[ i ] == AVAILABLE ||
                        forks[ i ] == RESERVED )
                    {
                        forks[ i ] = UNAVAILABLE;
                        write( waiter_write_fd[ i ], "yes", 4);
                    }
                    else {
                        write( waiter_write_fd[ i ], "no", 3);
                    }
                else if( 0 == strcmp( msg, "put down left fork" ) ) {
                    forks[ (i+1)%N_PHILOSOPHERS ] = AVAILABLE;
                    write( waiter_write_fd[ i ], "ok", 3);
                }
                else if( 0 == strcmp( msg, "put down right fork" ) ) {
                    forks[ i ] = AVAILABLE;
                    write( waiter_write_fd[ i ], "ok", 3);
                }
            }
            else {
                sleep( 20 );
            }
        }
    }

    exit( EXIT_SUCCESS );
}

void philosopher( int read_fd, int write_fd, int id ) {
    while(1)
    {
        pick_up_forks( read_fd, write_fd, id );
        eat( id );
        put_down_forks( read_fd, write_fd, id );
        think( id );
    }

    exit( EXIT_SUCCESS );
}

void pick_up_forks(  int read_fd, int write_fd, int id ) {
    char msg[ 100 ];

    // request left fork
    int picked = 0;
    while( picked == 0 ) {
        write( write_fd, "request left fork", 18 );
        read( read_fd, msg, 100);
        if     ( 0 == strcmp( msg, "yes" ) )
            picked = 1;
        else if( 0 == strcmp( msg, "no" ) )
            sleep(100);
    }

    // delay between picking up the first and second fork
    // encourages deadlock
    sleep( 800 );

    // request right fork
    picked = 0;
    while( picked == 0 ) {
        write( write_fd, "request right fork", 19 );
        read( read_fd, msg, 100);
        if     ( 0 == strcmp( msg, "yes" ) )
            picked = 1;
        else if( 0 == strcmp( msg, "no" ) )
            sleep(100);
    }
}

void put_down_forks( int read_fd, int write_fd, int id ) {
    char msg[ 100 ];

    // release left fork
    write( write_fd, "put down left fork", 19 );
    read( read_fd, msg, 100);

    // delay between putting down the first and second fork
    // encourages deadlock and race conditions
    sleep( 900 );

    // release right fork
    write( write_fd, "put down right fork", 20 );
    read( read_fd, msg, 100);
}

void eat( int id ) {
     print_index( id );
     write( STDOUT_FILENO, " started eating.\n", 17 );

     // EAT
     sleep( 1000 );

     print_index( id );
     write( STDOUT_FILENO, " finished eating.\n", 18 );
}

void think( int id ) {
     print_index( id );
     write( STDOUT_FILENO, " started thinking.\n", 19 );

     // THINK
     sleep( 1000 );

     print_index( id );
     write( STDOUT_FILENO, " finished thinking.\n", 20 );
}

void sleep( int msec ) {
    for ( int c = 1 ; c <= 468951; c++ )
        for ( int d = 1 ; d <= msec ; d++ )
        {
            asm ( "nop" );
        }
}

void print_index( int id ) {
    char   str_id[ 3 ];
    int length_id = itoa( str_id, id );
    write( STDOUT_FILENO, "Philosopher ", 12 );
    write( STDOUT_FILENO, str_id, length_id );
}
