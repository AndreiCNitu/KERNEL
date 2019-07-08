#ifndef __INTERFACE_H
#define __INTERFACE_H

#include <string.h>
#include "fonts.h"

// This file includes functionality relating to
// manipulation of the LCD display and
// interaction with the keyboard

// caps lock flag
#define CAPS_OFF 0
#define CAPS_ON  1

// console and top menus
#define IN_CONSOLE 0
#define IN_TOP     1


void set_background( int colour );

void handle_press( uint8_t code );

void show_console( char* x, int n, int background, int foreground );

// integer to string, returns size of string
int itoas( char* r, int x );

// float to string, returns size of string
int ftoas( char *res, float fVal );

#endif
