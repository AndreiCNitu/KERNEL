#include "interface.h"

extern uint16_t      fb[ 600 ][ 800 ];
extern uint16_t prev_fb[ 600 ][ 800 ];
extern char     input_buffer[ 100 ];
extern char     shell_line[ 100 ];
extern int      buffer_pointer;
extern int      line_size;
extern int      pos_line;
extern int      pos_col;
extern int      prev_pos_line;
extern int      prev_pos_col;
extern int      caps_lock;
extern int      menu;

extern char command_hist[ 10 ][ 100 ];
extern int  command_size_hist[ 10 ];
extern int  hist_pos;

void set_background( int colour )
{
    for( int i = 0; i < 600; i++ ) {
        for( int j = 0; j< 800; j++ ) {
            fb[ i ][ j ] = colour;
        }
    }
}

void handle_press( uint8_t code )
{
    switch( code ) {
        case 0x48: // up arrow
            if( hist_pos < 9 ) {
                hist_pos++;
                pos_col = 7*10;
                show_console( command_hist[ hist_pos ], command_size_hist[ hist_pos ], BLACK, WHITE );
                show_console( "                              ", 30, BLACK, WHITE ); pos_col -= 30*10;

                buffer_pointer = 0;
                for( int i = 0; i < command_size_hist[ hist_pos ]; i++ ) {
                    input_buffer[ i ] = command_hist[ hist_pos ][ i ];
                    buffer_pointer++;
                }
            }
            break;
        case 0x50: // down arrow
            if( hist_pos > 0 ) {
                hist_pos--;
                pos_col = 7*10;
                show_console( command_hist[ hist_pos ], command_size_hist[ hist_pos ], BLACK, WHITE );
                show_console( "                              ", 30, BLACK, WHITE ); pos_col -= 30*10;

                buffer_pointer = 0;
                for( int i = 0; i < command_size_hist[ hist_pos ]; i++ ) {
                    input_buffer[ i ] = command_hist[ hist_pos ][ i ];
                    buffer_pointer++;
                }
            }
            break;
        case 0x4B: // left arrow

            break;
        case 0x4D: // right arrow

            break;
        case 0x3A: // caps lock
            if( caps_lock == CAPS_ON )
                caps_lock = CAPS_OFF;
            else
                caps_lock = CAPS_ON;
            break;
        case 0x2A: // press left shift
        if( caps_lock == CAPS_ON )
            caps_lock = CAPS_OFF;
        else
            caps_lock = CAPS_ON;
            break;
        case 0xAA: // lift left shift
        if( caps_lock == CAPS_ON )
            caps_lock = CAPS_OFF;
        else
            caps_lock = CAPS_ON;
            break;
        case 0x36: // press right shift
        if( caps_lock == CAPS_ON )
            caps_lock = CAPS_OFF;
        else
            caps_lock = CAPS_ON;
            break;
        case 0xB6: // lift right shift
        if( caps_lock == CAPS_ON )
            caps_lock = CAPS_OFF;
        else
            caps_lock = CAPS_ON;
            break;
        case 0x1C: // enter
            input_buffer[ buffer_pointer ] = '\0'; // end of line
            buffer_pointer++;
            for( int i = 0; i < buffer_pointer; i++ ) {
                shell_line[ i ] = input_buffer[ i ];
            }
            line_size = buffer_pointer;
            buffer_pointer = 0;
            show_console( "\n", 1, BLACK, GREEN );

            // shift commands up
            for( int i = 9; i > 0; i-- ) {
                for( int j = 0; j < command_size_hist[ i-1 ]; j++ )
                    command_hist[ i ][ j ] = command_hist[ i-1 ][ j ];
                command_size_hist[ i ] = command_size_hist[ i-1 ];
            }
            // copy last one
            for( int j = 0; j < line_size-1; j++ )
                command_hist[ 0 ][ j ] = shell_line[ j ];
            command_size_hist[ 0 ] = line_size-1;

            hist_pos = -1;

            break;
        case 0x39: // space
            input_buffer[ buffer_pointer ] = ' ';
            buffer_pointer++;
            show_console( " ", 1, BLACK, WHITE );
            break;
        case 0x0E: // backspace
            if( buffer_pointer > 0 ) {
                buffer_pointer--;
                for( int i = pos_line; i < pos_line+22; i++ ) {
                    for( int j = pos_col; j < pos_col+10; j++ ) {
                        fb[ i ][ j ] = BLACK;
                    }
                }
                if( pos_col > 0 )
                    pos_col -= 10;
                else {
                    pos_line -= 22;
                    pos_col = 780;
                }
            }
            break;
        case 0x10:
            if( menu == IN_TOP ) {
                set_background( BLACK );
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        fb[ i ][ j ] = prev_fb[ i ][ j ];
                    }
                }
                pos_col = prev_pos_col;
                pos_line = prev_pos_line;
                menu = IN_CONSOLE;
                show_console( "shell$ ", 7, BLACK, WHITE );
            }
            else {
                input_buffer[ buffer_pointer ] = 'q';
                buffer_pointer++;
                show_console( "q", 1, BLACK, WHITE );
            }
            break;
        case 0x11:
            input_buffer[ buffer_pointer ] = 'w';
            buffer_pointer++;
            show_console( "w", 1, BLACK, WHITE );
            break;
        case 0x12:
            input_buffer[ buffer_pointer ] = 'e';
            buffer_pointer++;
            show_console( "e", 1, BLACK, WHITE );
            break;
        case 0x13:
            input_buffer[ buffer_pointer ] = 'r';
            buffer_pointer++;
            show_console( "r", 1, BLACK, WHITE );
            break;
        case 0x14:
            input_buffer[ buffer_pointer ] = 't';
            buffer_pointer++;
            show_console( "t", 1, BLACK, WHITE );
            break;
        case 0x15:
            input_buffer[ buffer_pointer ] = 'y';
            buffer_pointer++;
            show_console( "y", 1, BLACK, WHITE );
            break;
        case 0x16:
            input_buffer[ buffer_pointer ] = 'u';
            buffer_pointer++;
            show_console( "u", 1, BLACK, WHITE );
            break;
        case 0x17:
            input_buffer[ buffer_pointer ] = 'i';
            buffer_pointer++;
            show_console( "i", 1, BLACK, WHITE );
            break;
        case 0x18:
            input_buffer[ buffer_pointer ] = 'o';
            buffer_pointer++;
            show_console( "o", 1, BLACK, WHITE );
            break;
        case 0x19:
            if( caps_lock == CAPS_OFF ) {
                input_buffer[ buffer_pointer ] = 'p';
                show_console( "p", 1, BLACK, WHITE );
            }
            else {
                input_buffer[ buffer_pointer ] = 'P';
                show_console( "P", 1, BLACK, WHITE );
            }
            buffer_pointer++;
            break;
        case 0x1E:
            input_buffer[ buffer_pointer ] = 'a';
            buffer_pointer++;
            show_console( "a", 1, BLACK, WHITE );
            break;
        case 0x1F:
            input_buffer[ buffer_pointer ] = 's';
            buffer_pointer++;
            show_console( "s", 1, BLACK, WHITE );
            break;
        case 0x20:
            input_buffer[ buffer_pointer ] = 'd';
            buffer_pointer++;
            show_console( "d", 1, BLACK, WHITE );
            break;
        case 0x21:
            input_buffer[ buffer_pointer ] = 'f';
            buffer_pointer++;
            show_console( "f", 1, BLACK, WHITE );
            break;
        case 0x22:
            input_buffer[ buffer_pointer ] = 'g';
            buffer_pointer++;
            show_console( "g", 1, BLACK, WHITE );
            break;
        case 0x23:
            input_buffer[ buffer_pointer ] = 'h';
            buffer_pointer++;
            show_console( "h", 1, BLACK, WHITE );
            break;
        case 0x24:
            input_buffer[ buffer_pointer ] = 'j';
            buffer_pointer++;
            show_console( "j", 1, BLACK, WHITE );
            break;
        case 0x25:
            input_buffer[ buffer_pointer ] = 'k';
            buffer_pointer++;
            show_console( "k", 1, BLACK, WHITE );
            break;
        case 0x26:
            input_buffer[ buffer_pointer ] = 'l';
            buffer_pointer++;
            show_console( "l", 1, BLACK, WHITE );
            break;
        case 0x2C:
            input_buffer[ buffer_pointer ] = 'z';
            buffer_pointer++;
            show_console( "z", 1, BLACK, WHITE );
            break;
        case 0x2D:
            input_buffer[ buffer_pointer ] = 'x';
            buffer_pointer++;
            show_console( "x", 1, BLACK, WHITE );
            break;
        case 0x2E:
            input_buffer[ buffer_pointer ] = 'c';
            buffer_pointer++;
            show_console( "c", 1, BLACK, WHITE );
            break;
        case 0x2F:
            input_buffer[ buffer_pointer ] = 'v';
            buffer_pointer++;
            show_console( "v", 1, BLACK, WHITE );
            break;
        case 0x30:
            input_buffer[ buffer_pointer ] = 'b';
            buffer_pointer++;
            show_console( "b", 1, BLACK, WHITE );
            break;
        case 0x31:
            input_buffer[ buffer_pointer ] = 'n';
            buffer_pointer++;
            show_console( "n", 1, BLACK, WHITE );
            break;
        case 0x32:
            input_buffer[ buffer_pointer ] = 'm';
            buffer_pointer++;
            show_console( "m", 1, BLACK, WHITE );
            break;
        case 0x02:
            input_buffer[ buffer_pointer ] = '1';
            buffer_pointer++;
            show_console( "1", 1, BLACK, WHITE );
            break;
        case 0x03:
            input_buffer[ buffer_pointer ] = '2';
            buffer_pointer++;
            show_console( "2", 1, BLACK, WHITE );
            break;
        case 0x04:
            input_buffer[ buffer_pointer ] = '3';
            buffer_pointer++;
            show_console( "3", 1, BLACK, WHITE );
            break;
        case 0x05:
            input_buffer[ buffer_pointer ] = '4';
            buffer_pointer++;
            show_console( "4", 1, BLACK, WHITE );
            break;
        case 0x06:
            input_buffer[ buffer_pointer ] = '5';
            buffer_pointer++;
            show_console( "5", 1, BLACK, WHITE );
            break;
        case 0x07:
            input_buffer[ buffer_pointer ] = '6';
            buffer_pointer++;
            show_console( "6", 1, BLACK, WHITE );
            break;
        case 0x08:
            input_buffer[ buffer_pointer ] = '7';
            buffer_pointer++;
            show_console( "7", 1, BLACK, WHITE );
            break;
        case 0x09:
            input_buffer[ buffer_pointer ] = '8';
            buffer_pointer++;
            show_console( "8", 1, BLACK, WHITE );
            break;
        case 0x0A:
            input_buffer[ buffer_pointer ] = '9';
            buffer_pointer++;
            show_console( "9", 1, BLACK, WHITE );
            break;
        case 0x0B:
            input_buffer[ buffer_pointer ] = '0';
            buffer_pointer++;
            show_console( "0", 1, BLACK, WHITE );
            break;
        case 0x34:
            input_buffer[ buffer_pointer ] = '.';
            buffer_pointer++;
            show_console( ".", 1, BLACK, WHITE );
            break;
        case 0x0C:
            input_buffer[ buffer_pointer ] = '-';
            buffer_pointer++;
            show_console( "-", 1, BLACK, WHITE );
            break;
    }
}


void show_console( char* x, int n, int background, int foreground )
{
  if( x[0] == 's' && x[1] == 'h' && x[2] == 'e' && x[3] == 'l' && x[4] == 'l' && x[5] == '$' && n == 7 )
    foreground = GREEN;
  for( int i = 0; i < n; i++ ) {
    switch( x[ i ] )
    {
        case '\n': {
            if( pos_line < 570 ) {
                pos_col  = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            break;
        }
        case ' ': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                        fb[ i+pos_line ][ j+pos_col ] = background;
                }
            }
            break;
        }
        case 'q': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( small_q[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( small_q[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 'w': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( small_w[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( small_w[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 'e': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( small_e[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( small_e[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 'r': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( small_r[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( small_r[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 't': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( small_t[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( small_t[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 'y': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( small_y[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( small_y[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 'u': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( small_u[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( small_u[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 'i': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( small_i[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( small_i[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 'o': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( small_o[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( small_o[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 'p': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( small_p[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( small_p[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 'a': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( small_a[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( small_a[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 's': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( small_s[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( small_s[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 'd': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( small_d[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( small_d[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 'f': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( small_f[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( small_f[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 'g': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( small_g[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( small_g[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 'h': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( small_h[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( small_h[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 'j': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( small_j[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( small_j[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 'k': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( small_k[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( small_k[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 'l': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( small_l[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( small_l[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 'z': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( small_z[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( small_z[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 'x': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( small_x[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( small_x[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 'c': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( small_c[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( small_c[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 'v': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( small_v[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( small_v[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 'b': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( small_b[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( small_b[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 'n': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( small_n[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( small_n[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 'm': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( small_m[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( small_m[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case '1': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( digit1[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( digit1[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case '2': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( digit2[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( digit2[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case '3': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( digit3[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( digit3[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case '4': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( digit4[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( digit4[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case '5': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( digit5[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( digit5[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case '6': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( digit6[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( digit6[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case '7': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( digit7[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( digit7[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case '8': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( digit8[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( digit8[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case '9': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( digit9[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( digit9[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case '0': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( digit0[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( digit0[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case '$': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( dollar[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( dollar[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case '.': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( dot[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( dot[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 'P': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( big_p[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( big_p[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 'I': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( big_i[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( big_i[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 'D': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( big_d[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( big_d[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 'R': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( big_r[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( big_r[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 'N': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( big_n[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( big_n[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 'T': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( big_t[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( big_t[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 'M': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( big_m[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( big_m[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 'E': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( big_e[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( big_e[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 'S': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( big_s[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( big_s[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 'A': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( big_a[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( big_a[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case 'U': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( big_u[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( big_u[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case '-': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( minus[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( minus[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case '|': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( vertical[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( vertical[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
        case '_': {
            if( pos_col < 780 )
                pos_col += 10;
            else if( pos_line < 570 ) {
                pos_col = 0;
                pos_line += 22;
            }
            else {
                pos_col = 0;
                // shift UP
                for( int i = 0; i < 600; i++ ) {
                    for( int j = 0; j < 800; j++ ) {
                        if( i+22 < 600 )
                            fb[ i ][ j ] = fb[ i + 22 ][ j ];
                        else
                            fb[ i ][ j ] = BLACK;
                    }
                }
            }
            for( int i = 0; i < 21; i++ ) {
                for( int j = 0; j < 10; j++ ) {
                    if( horizontal[i][j] == BACKGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = background;
                    if( horizontal[i][j] == FOREGROUND )
                        fb[ i+pos_line ][ j+pos_col ] = foreground;
                }
            }
            break;
        }
    }
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
