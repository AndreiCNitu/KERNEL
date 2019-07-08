#include "hilevel.h"

#include <sys/types.h>

// integer to string, returns size of string
int itoas( char* r, int x );

// float to string, returns size of string
int ftoas( char *res, float fVal );

void show( char* x, int n );
void show_console( char* x, int n );

void try_writing( int pcb_index );
void try_reading( int pcb_index );

extern uint32_t end;
caddr_t _sbrk( int incr ) {
    return ( caddr_t )( &end );
}

extern void     main_console();
extern uint32_t tos_console;
extern uint32_t tos_general;
uint32_t stack_offset = 0x2000;

// interrupt time quantum for real time processes:
uint32_t rt_quantum  = 0x000028F5; // ~ 0.01 sec
// interrupt time quantum for non real time processes:
uint32_t nrt_quantum = 0x00080000; // ~ 0.50 sec

fildes_t fd_table[ 300 ];
int n_pid  = 0;  // last PID used
int n_pcb  = 0;  // last pcb entry used
pcb_t pcb[ 32 ];
int executing = 0;


void scheduler( ctx_t* ctx ) {
    // display when a context switch is made
    // show( "-s-", 3 );

    // TODO
    for( int i = 0; i <= n_pcb; i++) {
        if( pcb[ i ].status == WAITING_WRITE ) {
            try_writing( i );
        }
        else if( pcb[ i ].status == WAITING_READ ) {
            try_reading( i );
        }
    }

    // priority-based + ageing, timer pre-emption
    // Priority( x ) = base priority( x ) + age( x ) + niceness( x )
    int min_priority = 139;
    int next = 0;
    int pr;
    for( int i = 0; i <= n_pcb; i++) {
        pr = pcb[ i ].priority + pcb[ i ].age + pcb[ i ].niceness;
        if( (pr < min_priority) &&
            ( (pcb[ i ].status == READY) ||
              (pcb[ i ].status == EXECUTING) ) ) {
            min_priority = pr;
            next = i;
        }
    }

    // "deage" the process that will execute next
    pcb[ next ].age = 0;
    // age all other processes
    for( int i = 0; i <= n_pcb; i++ )
        if( i != next )
            pcb[ i ].age -= 2;

    memcpy( &pcb[ executing ].ctx, ctx, sizeof( ctx_t ) ); // preserve executing P
    if ( pcb[ executing ].status == EXECUTING )
         pcb[ executing ].status  = READY;                 // update   P status
    memcpy( ctx, &pcb[ next ].ctx, sizeof( ctx_t ) );      // restore  P next
    pcb[ next ].status = EXECUTING;                        // update   P status
    executing = next;

    if( executing == 0 ) {
        // execute console process for ~ 0.01 sec
        TIMER0->Timer1Load  = rt_quantum;
        // record CPU time
        pcb[ next ].time += 0.01f;
    }
    else {
        // execute other processes for ~ 0.50 sec
        TIMER0->Timer1Load  = nrt_quantum;
        // record CPU time
        pcb[ next ].time += 0.50f;
    }

    return;
}

void hilevel_handler_rst( ctx_t *ctx ) {
    // Configure the mechanism for interrupt handling:

    TIMER0->Timer1Load  = 0x00100000; // initial period = 2^20 ticks ~= 1 sec
    TIMER0->Timer1Ctrl  = 0x00000002; // select 32-bit   timer
    TIMER0->Timer1Ctrl |= 0x00000040; // select periodic timer
    TIMER0->Timer1Ctrl |= 0x00000020; // enable          timer interrupt
    TIMER0->Timer1Ctrl |= 0x00000080; // enable          timer

    GICC0->PMR          = 0x000000F0; // unmask all            interrupts
    GICD0->ISENABLER1  |= 0x00000010; // enable timer          interrupt
    GICC0->CTLR         = 0x00000001; // enable GIC interface
    GICD0->CTLR         = 0x00000001; // enable GIC distributor

    int_enable_irq();

    // initialize file descriptor table
    for( int i = 0; i < 3; i++ )
        fd_table[ i ].free = 0;
    for( int i = 3; i < 300; i++ )
        fd_table[ i ].free = 1;

    memset( &pcb[ 0 ], 0, sizeof( pcb_t ) );
    pcb[ 0 ].pid      = 0;
    pcb[ 0 ].status   = READY;
    // TODO set console priority (from 0 to 99):
    pcb[ 0 ].priority = 99;
    pcb[ 0 ].niceness = 0;
    pcb[ 0 ].age      = 0;
    pcb[ 0 ].ctx.cpsr = 0x50;
    pcb[ 0 ].ctx.pc   = ( uint32_t )( &main_console );
    pcb[ 0 ].ctx.sp   = ( uint32_t )( &tos_console  );
    pcb[ 0 ].tos      = ( uint32_t )( &tos_console  );

    // start executing the console
    executing = 0;
    memcpy( ctx, &pcb[ executing ].ctx, sizeof( ctx_t ) );
    pcb[ executing ].status = EXECUTING;
    pcb[ executing ].time   = 0.0f;

    return;
}

void hilevel_handler_irq( ctx_t *ctx ) {
    // read the interrupt identifier so we know the source.
    uint32_t id = GICC0->IAR;

    // handle the interrupt, then clear (or reset) the source.
    if( id == GIC_SOURCE_TIMER0 ) {
      scheduler( ctx );
      TIMER0->Timer1IntClr = 0x01;
    }

    // write the interrupt identifier to signal we're done.
    GICC0->EOIR = id;

  return;
}

void hilevel_handler_svc( ctx_t *ctx, uint32_t id ) {

    switch( id ) {
        case 0x00 : // yield()
        {
            scheduler( ctx );

            break;
        }

        // TODO
        case 0x01 : // pipe( int fildes[2] )
        {
            int* fildes;
            fildes = ( int* )( ctx->gpr[ 0 ] );
            // create new pipe object
            pipe_inode_t* pipe_inode = malloc( sizeof(pipe_inode_t) );
            while( pipe_inode == NULL ) {
                free( pipe_inode );
                pipe_inode = malloc( sizeof(pipe_inode_t) );
            }

            pipe_inode->size    = 0;
            pipe_inode->readers = 1;
            pipe_inode->writers = 1;

            // find the first 2 free entries in the file descriptor table
            // and allocate them for the read and write ends of the pipe
            int read_fd = -1;//TODO check if stays -1
            for( int i = 3; i < 300; i++ ) {
                if( fd_table[ i ].free == 1 )
                {
                    fd_table[ i ].free   = 0;
                    fd_table[ i ].access = READ;
                    fd_table[ i ].pipe   = pipe_inode;
                    read_fd = i; // read end
                    break;
                }
            }
            int write_fd = -1; //TODO check if stays -1
            for( int i = 3; i < 300; i++ ) {
                if( fd_table[ i ].free == 1 )
                {
                    fd_table[ i ].free   = 0;
                    fd_table[ i ].access = WRITE;
                    fd_table[ i ].pipe   = pipe_inode;
                    write_fd = i; // write end
                    break;
                }
            }

            if( (read_fd == -1) || (write_fd == -1) ) {
                ctx->gpr[ 0 ] = -1; // fail
            }
            else {
                *( fildes )     = read_fd;  // return fildes for reading
                *( fildes + 1 ) = write_fd; // return fildes for writing

                ctx->gpr[ 0 ] = 0; // success
            }
            break;
        }

        // TODO
        case 0x02 : // write( fd, x, n )
        {
            int   fildes = ( int   )( ctx->gpr[ 0 ] );
            char*  x     = ( char* )( ctx->gpr[ 1 ] );
            int    n     = ( int   )( ctx->gpr[ 2 ] );

            if( 1 == fildes ) { // STDOUT_FILENO
                for( int i = 0; i < n; i++ ) {
                    PL011_putc( UART0, *x++, true );
                }
                ctx->gpr[ 0 ] = n;
            }
            else if( WRITE == fd_table[ fildes ].access ) {
                pipe_inode_t* pipe = fd_table[ fildes ].pipe;

                // if there are still bytes left to be read then
                // mark process as waiting and invoke scheduler
                if( pipe->size != 0 ) {
                    pcb[ executing ].status = WAITING_WRITE;
                    scheduler( ctx );
                    break;
                }
                // else start writing
                else {
                    for( int i = 0; i < n; i++ ) {
                        pipe->buffer[ i ] = *x;
                        pipe->size++;
                        x++;
                    }
                    ctx->gpr[ 0 ] = n;
                }
            }
            else {
                // error
                ctx->gpr[ 0 ] = -1;
            }

            break;
        }

        // TODO
        case 0x03 : // read( fd, x, n )
        {
            int   fildes = ( int   )( ctx->gpr[ 0 ] );
            char*  x     = ( char* )( ctx->gpr[ 1 ] );
            int    n     = ( int   )( ctx->gpr[ 2 ] );

            if( READ == fd_table[ fildes ].access ) {
                pipe_inode_t* pipe = fd_table[ fildes ].pipe;
                int size           = pipe->size;

                // if there is nothing to read then
                // mark process as waiting and invoke scheduler
                if( pipe->size == 0 ) {
                    pcb[ executing ].status = WAITING_READ;
                    scheduler( ctx );
                    break;
                }
                // else start reading
                else {
                    for( int i = 0; i < size; i++ ) {
                        *( x + i ) = pipe->buffer[ i ];
                        pipe->size--;
                    }
                    ctx->gpr[ 0 ] = size; // might be less than n
                }
            }
            else {
                // error
                ctx->gpr[ 0 ] = -1;
            }

            break;
        }

        case 0x04 : // write_nb( fd, x, n )
        {
            int   fildes = ( int   )( ctx->gpr[ 0 ] );
            char*  x     = ( char* )( ctx->gpr[ 1 ] );
            int    n     = ( int   )( ctx->gpr[ 2 ] );

            if( 1 == fildes ) { // STDOUT_FILENO
                for( int i = 0; i < n; i++ ) {
                    PL011_putc( UART0, *x++, true );
                }
                ctx->gpr[ 0 ] = n;
            }
            else if( WRITE == fd_table[ fildes ].access ) {
                pipe_inode_t* pipe = fd_table[ fildes ].pipe;

                // if there are still bytes left to be read
                // then return 0 bytes written
                if( pipe->size != 0 ) {
                    ctx->gpr[ 0 ] = 0;
                }
                // else start writing
                else {
                    for( int i = 0; i < n; i++ ) {
                        pipe->buffer[ i ] = *x;
                        pipe->size++;
                        x++;
                    }
                    ctx->gpr[ 0 ] = n;
                }
            }
            else {
                // error
                ctx->gpr[ 0 ] = -1;
            }

            break;
        }

        case 0x05 : // read_nb( fd, x, n )
        {
            int   fildes = ( int   )( ctx->gpr[ 0 ] );
            char*  x     = ( char* )( ctx->gpr[ 1 ] );
            int    n     = ( int   )( ctx->gpr[ 2 ] );

            if( READ == fd_table[ fildes ].access ) {
                pipe_inode_t* pipe = fd_table[ fildes ].pipe;
                int size           = pipe->size;

                // if there is nothing to read
                // then return 0 bytes read
                if( pipe->size == 0 ) {
                    ctx->gpr[ 0 ] = 0;
                }
                // else start reading
                else {
                    for( int i = 0; i < size; i++ ) {
                        *( x + i ) = pipe->buffer[ i ];
                        pipe->size--;
                    }
                    ctx->gpr[ 0 ] = size; // might be less than n
                }
            }
            else {
                // error
                ctx->gpr[ 0 ] = -1;
            }

            break;
        }

        // TODO
        case 0x06 : // close( int fildes )
        {
            int fildes = ctx->gpr[ 0 ];
            pipe_inode_t* pipe = fd_table[ fildes ].pipe;
            if( READ == fd_table[ fildes ].access )
                pipe->readers--;
            if( WRITE == fd_table[ fildes ].access )
                pipe->writers--;

            if( (pipe->readers==0) && (pipe->writers==0) )
                free(pipe);

            fd_table[ fildes ].free = 1;
            ctx->gpr[ 0 ] = 0;

            break;
        }

        case 0x07 : // fork()
        {
            n_pid++;
            int newEntry = -1;

            // search for a free gap in the pcb
            for( int i = 0; i <= n_pcb; i++ )
                if( pcb[ i ].status == TERMINATED ) {
                    newEntry = i;
                    break;
                }

            if( newEntry == -1 ) {
                // add new entry to pcb
                n_pcb++;
                newEntry = n_pcb;
                memset( &pcb[ newEntry ], 0, sizeof( pcb_t ) );

                // allocate new stack
                pcb[ newEntry ].tos = ( uint32_t )( &tos_general ) - (stack_offset * (n_pcb-1));
            }

            // create new child process
            pcb[ newEntry ].pid      = n_pid;
            pcb[ newEntry ].status   = READY;
            pcb[ newEntry ].priority = 120;
            pcb[ newEntry ].age      = 0;
            pcb[ newEntry ].niceness = 0;
            pcb[ newEntry ].time     = 0.0f;

            // replicate state of parent in child
            memcpy( &pcb[ newEntry ].ctx, ctx, sizeof( ctx_t ) );

            // copy the stack of the parent
            volatile uint32_t size = pcb[ executing ].tos - (uint32_t) ctx->sp;
            pcb[ newEntry ].ctx.sp = pcb[ newEntry ].tos - size;
            memcpy( (uint32_t*) (pcb[ newEntry ].ctx.sp), (uint32_t*) ctx->sp , size);

            pcb[ newEntry ].ctx.gpr[ 0 ] = 0; // return  0  to child
            ctx->gpr[ 0 ] = n_pid;            // return PID to parent

            break;
        }

        case 0x08 : // exit( x )
        {
            int sig = ctx->gpr[ 0 ];
            pcb[ executing ].status = TERMINATED;

            // show( " Process finished with exit code ", 33 );
            // char* code = malloc( sizeof(char) * 4);
            // int scode = itoas( code, sig );
            // show( code, scode );
            // free( code );
            // if( sig == 0 )
            //     show( " (EXIT SUCCESS) ", 16 );
            // else if( sig == 1 )
            //     show( " (EXIT FAILURE) ", 16 );
            // else {
            //     show( " (unknown exit code) ", 21 );
            // }

            scheduler( ctx );

            break;
        }

        case 0x09 : // exec( x )
        {
            // read pointer to the entry point
            ctx->pc = ( uint32_t )( ctx->gpr[ 0 ] );

            // reset stack pointer
            ctx->sp = pcb[ executing ].tos;

            break;
        }

        case 0x0A : // kill( pid, x )
        {
            pid_t pid = ctx->gpr[ 0 ];
            volatile int sig = ctx->gpr[ 1 ];

            // find PCB entry of given PID
            int pcb_entry = -1;
            for( int i = 0; i <= n_pcb; i++)
                if( (pid == pcb[ i ].pid) && (pcb[ i ].status != TERMINATED) ) {
                    pcb_entry = i;
                    break;
                }

            // PID > 0 => kill process with PID
            if ( pcb_entry > 0 ) {
                pcb[ pcb_entry ].status = TERMINATED;
                ctx->gpr[ 0 ] = 0; // success
            }
            // PID = 0 => kill all processes except console
            else if( pcb_entry == 0) {
                for( int i = 1; i <= n_pcb; i++)
                    pcb[ i ].status = TERMINATED;

                ctx->gpr[ 0 ] = 0; // success
            }
            else {
                ctx->gpr[ 0 ] = -1; // fail
            }

            break;
        }

        case 0x0B : // nice( pid, n )
        {
            pid_t pid = ctx->gpr[ 0 ];
            int  nice = ctx->gpr[ 1 ];

            // find PCB entry of given PID
            int pcb_entry = -1;
            for( int i = 0; i <= n_pcb; i++)
                if( (pid == pcb[ i ].pid) && (pcb[ i ].status != TERMINATED) ) {
                    pcb_entry = i;
                    break;
                }
            if( pcb_entry == 0 )
                show_console( "Cannot renice the console", 25 );
            else if( pcb_entry > 0 )
                pcb[ pcb_entry ].niceness = nice;
            else
                show_console( "No process found with given PID\n", 32 );

            break;
        }

        case 0x0C : // top()
        {
            show_console( "PID   PR    NI    TIME    STATUS\n", 33 );

            for( int entry = 0; entry <= n_pcb; entry++ ) {
                if( pcb[ entry ].status != TERMINATED )
                {
                    char id[ 4 ];
                    int sid = itoas( id, pcb[ entry ].pid );
                    char pr[ 4 ];
                    int spr = itoas( pr, pcb[ entry ].priority );
                    char ni[ 4 ];
                    int sni = itoas( ni, pcb[ entry ].niceness );
                    char tm[ 8 ];
                    int stm = ftoas( tm, pcb[ entry ].time );

                    show_console( id, sid );
                    show_console( "      ", 6 - sid );
                    show_console( pr, spr );
                    if( pcb[ entry ].niceness < 0 ) {
                        show_console( "      ", 5 - spr );
                        show_console( ni, sni );
                        show_console( "      ", 7 - sni );
                    }
                    else {
                        show_console( "      ", 6 - spr );
                        show_console( ni, sni );
                        show_console( "      ", 6 - sni );
                    }
                    show_console( tm, stm );
                    show_console( "      ", 8 - stm );

                    if( pcb[ entry ].status == CREATED )
                        show_console( "created" , 7 );
                    else if( pcb[ entry ].status == READY )
                        show_console( "ready" , 5 );
                    else if( pcb[ entry ].status == EXECUTING )
                        show_console( "executing" , 9 );
                    else if( pcb[ entry ].status == WAITING_READ ||
                             pcb[ entry ].status == WAITING_WRITE )
                        show_console( "waiting" , 7 );
                    else if( pcb[ entry ].status == TERMINATED )
                        show_console( "terminated" , 10 );
                    show_console("\n", 1);

                }
            }
            break;
        }

        default   : // unknown/unsupported
        {
            break;
        }
    }

    return;
}

void try_writing( int pcb_index ) {
    int   fildes = ( int   )( pcb[ pcb_index ].ctx.gpr[ 0 ] );
    char*  x     = ( char* )( pcb[ pcb_index ].ctx.gpr[ 1 ] );
    int    n     = ( int   )( pcb[ pcb_index ].ctx.gpr[ 2 ] );

    pipe_inode_t* pipe = fd_table[ fildes ].pipe;

    // if there are no bytes left to be read then
    // resume and finish writing, set process to ready
    if( pipe->size == 0 ) {
        for( int i = 0; i < n; i++ ) {
            pipe->buffer[ i ] = *x;
            pipe->size++;
            x++;
        }
        pcb[ pcb_index ].ctx.gpr[ 0 ] = n;
        pcb[ pcb_index ].status = READY;
    }
}

void try_reading( int pcb_index ) {
    int   fildes = ( int   )( pcb[ pcb_index ].ctx.gpr[ 0 ] );
    char*  x     = ( char* )( pcb[ pcb_index ].ctx.gpr[ 1 ] );
    int    n     = ( int   )( pcb[ pcb_index ].ctx.gpr[ 2 ] );

    pipe_inode_t* pipe = fd_table[ fildes ].pipe;
    int size           = pipe->size;

    // Tttt start reading
    if( pipe->size != 0 ) {
        for( int i = 0; i < size; i++ ) {
            *( x + i ) = pipe->buffer[ i ];
            pipe->size--;
        }
        pcb[ pcb_index ].ctx.gpr[ 0 ] = size;
        pcb[ pcb_index ].status = READY;
    }
}

int ftoas( char *res, float f )
{
    char r[100];
    int d, dec, i;
    int s = 3;
    f += 0.005;
    d = f;
    dec = (int)(f * 100) % 100;
    memset(r, 0, 100);
    r[0] = (dec % 10) + '0';
    r[1] = (dec / 10) + '0';
    r[2] = '.';
    i = 3;
    while (d > 0)
    {
        r[i] = (d % 10) + '0';
        d /= 10;
        i++;
        s++;
    }
    for (i=strlen(r)-1; i>=0; i--)
        res[strlen(r)-i-1] = r[i];

    return s;
}

int itoas( char* r, int x )
{
  char* p = r; int t, n;
  int size = 0;

  if( x < 0 ) {
     size++;
     p++; t = -x; n = t;
  }
  else {
          t = +x; n = t;
  }

  do {
     p++;                    n /= 10;
     size++;
  } while( n );

    *p-- = '\x00';

  do {
    *p-- = '0' + ( t % 10 ); t /= 10;
  } while( t );

  if( x < 0 ) {
    *p-- = '-';
  }

  return size;
}

void show( char* x, int n )
{
  for( int i = 0; i < n; i++ ) {
    PL011_putc( UART0, x[ i ], true );
  }
}

void show_console( char* x, int n )
{
  for( int i = 0; i < n; i++ ) {
    PL011_putc( UART1, x[ i ], true );
  }
}
