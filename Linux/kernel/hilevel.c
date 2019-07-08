#include "hilevel.h"

void try_writing( int pcb_index );
void try_reading( int pcb_index );

void show( char* x, int n );
void display_dashboard();

extern void     main_gui();
extern void     main_console();
extern uint32_t tos_console;
extern uint32_t tos_general;
uint32_t stack_offset = 0x2000;

// interrupt time quantum for real time processes:
uint32_t rt_quantum  = 0x000051EB; // ~ 0.02 sec
// interrupt time quantum for non real time processes:
uint32_t nrt_quantum = 0x00080000; // ~ 0.50 sec

// frame buffer
uint16_t      fb[ 600 ][ 800 ];
// previous frame buffer
uint16_t prev_fb[ 600 ][ 800 ];

// pointer position on the screen
int pos_line      = 0;
int pos_col       = 0;
int prev_pos_line = 0;
int prev_pos_col  = 0;

// input buffer
char input_buffer[ 100 ];
// size of buffer
int  buffer_pointer = 0;

// a command written as a line in the shell
char shell_line[ 100 ];
int  line_size = 0;

// history of inputted commands
char command_hist[ 10 ][ 100 ];
int  command_size_hist[ 10 ] = {0,0,0,0,0,0,0,0,0,0};
int  hist_pos = -1;

// caps lock / hold shift
int caps_lock = CAPS_OFF;
// current menu( console / top )
int menu = IN_CONSOLE;

// file descriptor table
fildes_t fd_table[ 100 ];

int n_pid  = 0;  // last PID used
int n_pcb  = 0;  // last pcb entry used
pcb_t pcb[ 32 ];
int executing = 0;


void scheduler( ctx_t* ctx ) {

    // check if any of the processes that were waiting on
    // read / write operations can now continue excution
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

    memcpy( &pcb[ executing ].ctx, ctx, sizeof( ctx_t ) ); // preserve executing process
    if ( pcb[ executing ].status == EXECUTING )
         pcb[ executing ].status  = READY;                 // update  previous process status
    memcpy( ctx, &pcb[ next ].ctx, sizeof( ctx_t ) );      // restore next process
    pcb[ next ].status = EXECUTING;                        // update  next process status
    executing = next;

    if( executing == 0 ) {
        // execute console process for ~ 0.02 sec
        TIMER0->Timer1Load  = rt_quantum;
        // record CPU time
        pcb[ next ].time += 0.02f;
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
    // Configure the timer:

    TIMER0->Timer1Load  = 0x00100000; // initial period = 2^20 ticks ~= 1 sec
    TIMER0->Timer1Ctrl  = 0x00000002; // select 32-bit   timer
    TIMER0->Timer1Ctrl |= 0x00000040; // select periodic timer
    TIMER0->Timer1Ctrl |= 0x00000020; // enable          timer interrupt
    TIMER0->Timer1Ctrl |= 0x00000080; // enable          timer

    GICC0->PMR          = 0x000000F0; // unmask all            interrupts
    GICD0->ISENABLER1  |= 0x00000010; // enable timer          interrupt
    GICC0->CTLR         = 0x00000001; // enable GIC interface
    GICD0->CTLR         = 0x00000001; // enable GIC distributor

    // Configure the LCD display into 800x600 SVGA @ 36MHz resolution.

    SYSCONF->CLCD      = 0x2CAC;     // per per Table 4.3 of datasheet
    LCD->LCDTiming0    = 0x1313A4C4; // per per Table 4.3 of datasheet
    LCD->LCDTiming1    = 0x0505F657; // per per Table 4.3 of datasheet
    LCD->LCDTiming2    = 0x071F1800; // per per Table 4.3 of datasheet

    LCD->LCDUPBASE     = ( uint32_t )( &fb );

    LCD->LCDControl    = 0x00000020; // select TFT   display type
    LCD->LCDControl   |= 0x00000008; // select 16BPP display mode
    LCD->LCDControl   |= 0x00000800; // power-on LCD controller
    LCD->LCDControl   |= 0x00000001; // enable   LCD controller

    /* Configure the mechanism for interrupt handling by
     *
     * - configuring then enabling PS/2 controllers st. an interrupt is
     *   raised every time a byte is subsequently received,
     * - configuring GIC st. the selected interrupts are forwarded to the
     *   processor via the IRQ interrupt signal, then
     * - enabling IRQ interrupts.
     */

    PS20->CR           = 0x00000010; // enable PS/2    (Rx) interrupt
    PS20->CR          |= 0x00000004; // enable PS/2 (Tx+Rx)
    PS21->CR           = 0x00000010; // enable PS/2    (Rx) interrupt
    PS21->CR          |= 0x00000004; // enable PS/2 (Tx+Rx)

    uint8_t ack;

          PL050_putc( PS20, 0xF4 );  // transmit PS/2 enable command
    ack = PL050_getc( PS20       );  // receive  PS/2 acknowledgement
          PL050_putc( PS21, 0xF4 );  // transmit PS/2 enable command
    ack = PL050_getc( PS21       );  // receive  PS/2 acknowledgement

    GICC0->PMR         = 0x000000F0; // unmask all          interrupts
    GICD0->ISENABLER1 |= 0x00300000; // enable PS2          interrupts
    GICC0->CTLR        = 0x00000001; // enable GIC interface
    GICD0->CTLR        = 0x00000001; // enable GIC distributor

    int_enable_irq();

    // initialize file descriptor table
    for( int i = 0; i < 4; i++ )
        fd_table[ i ].free = 0;
    for( int i = 4; i < 100; i++ )
        fd_table[ i ].free = 1;

    memset( &pcb[ 0 ], 0, sizeof( pcb_t ) );
    pcb[ 0 ].pid      = 0;
    pcb[ 0 ].status   = READY;
    // TODO set console priority (from 0 to 99):
    pcb[ 0 ].priority = 80;
    pcb[ 0 ].niceness = 0;
    pcb[ 0 ].age      = 0;
    pcb[ 0 ].ctx.cpsr = 0x50;
    // TODO Choose console version ( gui / console ):
    pcb[ 0 ].ctx.pc   = ( uint32_t )( &main_gui );
    pcb[ 0 ].ctx.sp   = ( uint32_t )( &tos_console  );
    pcb[ 0 ].tos      = ( uint32_t )( &tos_console  );

    // start executing the console
    executing = 0;
    memcpy( ctx, &pcb[ executing ].ctx, sizeof( ctx_t ) );
    pcb[ executing ].status = EXECUTING;
    pcb[ executing ].time   = 0.0f;

    set_background( BLACK );
    show_console( "shell$ ", 7, BLACK, WHITE );

    return;
}

void hilevel_handler_irq( ctx_t *ctx ) {
    // read the interrupt identifier so we know the source.
    uint32_t id = GICC0->IAR;

    // handle the interrupt, then clear (or reset) the source.
    if( id == GIC_SOURCE_TIMER0 ) {
        scheduler( ctx );

        if( menu == IN_TOP ) {
            display_dashboard();
        }

        TIMER0->Timer1IntClr = 0x01;
    }
    else if( id == GIC_SOURCE_PS20 ) {
        // keyboard
        uint8_t x = PL050_getc( PS20 );
        handle_press( x );
    }
    else if( id == GIC_SOURCE_PS21 ) {
        // mouse
        uint8_t x = PL050_getc( PS21 );
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

        case 0x01 : // pipe( int fildes[2] )
        {
            int* fildes;
            fildes = ( int* )( ctx->gpr[ 0 ] );

            // create new pipe object
            pipe_inode_t* pipe_inode = malloc( sizeof(pipe_inode_t) );
            if( pipe_inode == NULL ) {
                show( "malloc failed!\n", 16 );
            }
            pipe_inode->size    = 0;
            pipe_inode->readers = 1;
            pipe_inode->writers = 1;

            // find the first 2 free entries in the file descriptor table
            // and allocate them for the read and write ends of the pipe
            int read_fd = -1;
            for( int i = 4; i < 100; i++ ) {
                if( fd_table[ i ].free == 1 )
                {
                    fd_table[ i ].free   = 0;
                    fd_table[ i ].access = READ;
                    fd_table[ i ].pipe   = pipe_inode;
                    read_fd = i; // read end
                    break;
                }
            }
            int write_fd = -1;
            for( int i = 4; i < 100; i++ ) {
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

        case 0x03 : // read( fd, x, n )
        {
            int   fildes = ( int   )( ctx->gpr[ 0 ] );
            char*  x     = ( char* )( ctx->gpr[ 1 ] );
            int    n     = ( int   )( ctx->gpr[ 2 ] );

            if( READ == fd_table[ fildes ].access ) {
                pipe_inode_t* pipe = fd_table[ fildes ].pipe;
                int           size = pipe->size;

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
                    ctx->gpr[ 0 ] = size;
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

            if     ( 1 == fildes ) { // STDOUT_FILENO
                for( int i = 0; i < n; i++ ) {
                    PL011_putc( UART0, *x++, true );
                }
                ctx->gpr[ 0 ] = n;
            }
            else if( 3 == fildes ) { // LCD_DISPLAY
                show_console( x, n, BLACK, WHITE );
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

            if( 0 == fildes ) { // STDIN_FILENO
                if( line_size > 0 ) {
                    int bytes = line_size;
                    for( int i = 0; i < bytes; i++ ) {
                        *( x + i ) = shell_line[ i ];
                        line_size--;
                    }
                    ctx->gpr[ 0 ] = bytes;
                }
                else
                    ctx->gpr[ 0 ] = 0;
            }
            else if( READ == fd_table[ fildes ].access ) {
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
                    ctx->gpr[ 0 ] = size;
                }
            }
            else {
                // error
                ctx->gpr[ 0 ] = -1;
            }

            break;
        }

        case 0x06 : // close( int fildes )
        {
            int fildes = ctx->gpr[ 0 ];
            pipe_inode_t* pipe = fd_table[ fildes ].pipe;
            if( READ == fd_table[ fildes ].access )
                pipe->readers--;
            if( WRITE == fd_table[ fildes ].access )
                pipe->writers--;

            // if there aren't any processes using the pipe then close it
            if( (pipe->readers==0) && (pipe->writers==0) )
                free( pipe );

            fd_table[ fildes ].free = 1;
            ctx->gpr[ 0 ] = 0; // success

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
                pcb[ newEntry ].tos = ( uint32_t )( &tos_general ) - ( stack_offset * (n_pcb-1) );
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
            uint32_t size = pcb[ executing ].tos - (uint32_t) ctx->sp;
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
            for( int i = 0; i <= n_pcb; i++ )
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
                for( int i = 1; i <= n_pcb; i++ )
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
            for( int i = 0; i <= n_pcb; i++ )
                if( (pid == pcb[ i ].pid) && (pcb[ i ].status != TERMINATED) ) {
                    pcb_entry = i;
                    break;
                }

            if( pcb_entry == 0 )
                show_console( "Modifying the nice value of the console not allowed\n", 52, BLACK, WHITE );
            else if( pcb_entry > 0 )
                pcb[ pcb_entry ].niceness = nice;
            else
                show_console( "No process found with given PID\n", 32, BLACK, WHITE );

            break;
        }

        case 0x0C : // top()
        {
            for( int i = 0; i < 600; i++ ) {
                for( int j = 0; j< 800; j++ ) {
                    prev_fb[ i ][ j ] = fb[ i ][ j ];
                }
            }
            prev_pos_col  = pos_col;
            prev_pos_line = pos_line;
            set_background( BLACK );
            menu = IN_TOP;

            break;
        }

        default   : // unknown / unsupported
        {
            break;
        }
    }

    return;
}

void try_writing( int pcb_index )
{
    int   fildes = ( int   )( pcb[ pcb_index ].ctx.gpr[ 0 ] );
    char*  x     = ( char* )( pcb[ pcb_index ].ctx.gpr[ 1 ] );
    int    n     = ( int   )( pcb[ pcb_index ].ctx.gpr[ 2 ] );

    pipe_inode_t* pipe = fd_table[ fildes ].pipe;

    // if there are no more bytes left to be read then
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

void try_reading( int pcb_index )
{
    int   fildes = ( int   )( pcb[ pcb_index ].ctx.gpr[ 0 ] );
    char*  x     = ( char* )( pcb[ pcb_index ].ctx.gpr[ 1 ] );
    int    n     = ( int   )( pcb[ pcb_index ].ctx.gpr[ 2 ] );

    pipe_inode_t* pipe = fd_table[ fildes ].pipe;
    int size           = pipe->size;

    // if any bytes have now been written then
    // resume and finish reading, set process to ready
    if( pipe->size != 0 ) {
        for( int i = 0; i < size; i++ ) {
            *( x +i ) = pipe->buffer[ i ];
            pipe->size--;
        }
        pcb[ pcb_index ].ctx.gpr[ 0 ] = size;
        pcb[ pcb_index ].status = READY;
    }
}

void show( char* x, int n )
{
  for( int i = 0; i < n; i++ ) {
    PL011_putc( UART0, x[ i ], true );
  }
}

void display_dashboard() {
    pos_line = 0;
    pos_col  = 0;
    show_console( " ___________________________________________________________________ \n", 71 , BLACK, GREEN );
    show_console( "| PID   |    PR    |     NI    |    TIME     |    STATUS            |\n", 71 , BLACK, GREEN );
    show_console( "|___________________________________________________________________|\n", 71 , BLACK, GREEN );

    for( int entry = 0; entry <= n_pcb; entry++ ) {
        if( pcb[ entry ].status != TERMINATED )
        {
            char id[ 4 ]; int sid = itoas( id, pcb[ entry ].pid );
            char pr[ 4 ]; int spr = itoas( pr, pcb[ entry ].priority );
            char ni[ 4 ]; int sni = itoas( ni, pcb[ entry ].niceness );
            char tm[ 8 ]; int stm = ftoas( tm, pcb[ entry ].time );

            show_console( "| ",    2,       BLACK, GREEN );
            show_console( id,          sid, BLACK, GREEN );
            show_console( "     ", 6 - sid, BLACK, GREEN );
            show_console( "|    ", 5,       BLACK, GREEN );
            show_console( pr, spr,          BLACK, GREEN );
            if( pcb[ entry ].niceness < 0 ) {
                show_console( "     ",   6 - spr, BLACK, GREEN );
                show_console( "|    ",   5 ,      BLACK, GREEN );
                show_console( ni,            sni, BLACK, GREEN );
                show_console( "       ", 7 - sni, BLACK, GREEN );
                show_console( "|    ",   5,       BLACK, GREEN );
            }
            else {
                show_console( "      ", 6 - spr, BLACK, GREEN );
                show_console( "|     ", 6,       BLACK, GREEN );
                show_console( ni,           sni, BLACK, GREEN );
                show_console( "      ", 6 - sni, BLACK, GREEN );
                show_console( "|    ",  5,       BLACK, GREEN );
            }
            show_console( tm,              stm, BLACK, GREEN );
            show_console( "         ", 9 - stm, BLACK, GREEN );
            show_console( "|    ",     5,       BLACK, GREEN );

            if( pcb[ entry ].status == CREATED )
                show_console( "created           ", 20, BLACK, GREEN );
            else if( pcb[ entry ].status == READY )
                show_console( "ready             ", 20, BLACK, GREEN );
            else if( pcb[ entry ].status == EXECUTING )
                show_console( "executing         ", 20, BLACK, RED   );
            else if( pcb[ entry ].status == WAITING_READ )
                show_console( "waiting to read   ", 20, BLACK, LBLUE );
            else if( pcb[ entry ].status == WAITING_READ )
                show_console( "waiting to write  ", 20, BLACK, LBLUE );
            else if( pcb[ entry ].status == TERMINATED )
                show_console( "terminated        ", 20, BLACK, GREEN );
            show_console("|\n", 2, BLACK, GREEN );
        }
    }
    show_console( " ___________________________________________________________________ \n", 71 , BLACK, GREEN );
    for( int i = 1; i < 30; i++ )
        if( pos_line < 570 ) {
            show_console( "                                                                              ", 78, BLACK, GREEN );
            show_console("\n", 1, BLACK, GREEN );
        }
}
