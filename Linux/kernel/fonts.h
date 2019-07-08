#ifndef __FONTS_H
#define __FONTS_H

#include <stdint.h>

// This file includes renders (most) characters on
// the LCD display using a bit-mapped font

#define BACKGROUND 0x1234
#define FOREGROUND 0x4321

#define YELLOW 0x03FF
#define WHITE  0x7FFF
#define BLACK  0x0000
#define DBLUE  0x6C00
#define LBLUE  0x6FA0
#define GREEN  0x03E0
#define RED    0x001F


const uint16_t small_q[21][10];
const uint16_t small_w[21][10];
const uint16_t small_e[21][10];
const uint16_t small_r[21][10];
const uint16_t small_t[21][10];
const uint16_t small_y[21][10];
const uint16_t small_u[21][10];
const uint16_t small_i[21][10];
const uint16_t small_o[21][10];
const uint16_t small_p[21][10];
const uint16_t small_a[21][10];
const uint16_t small_s[21][10];
const uint16_t small_d[21][10];
const uint16_t small_f[21][10];
const uint16_t small_g[21][10];
const uint16_t small_h[21][10];
const uint16_t small_j[21][10];
const uint16_t small_k[21][10];
const uint16_t small_l[21][10];
const uint16_t small_z[21][10];
const uint16_t small_x[21][10];
const uint16_t small_c[21][10];
const uint16_t small_v[21][10];
const uint16_t small_b[21][10];
const uint16_t small_n[21][10];
const uint16_t small_m[21][10];
const uint16_t dollar[21][10];
const uint16_t digit1[21][10];
const uint16_t digit2[21][10];
const uint16_t digit3[21][10];
const uint16_t digit4[21][10];
const uint16_t digit5[21][10];
const uint16_t digit6[21][10];
const uint16_t digit7[21][10];
const uint16_t digit8[21][10];
const uint16_t digit9[21][10];
const uint16_t digit0[21][10];
const uint16_t dot[21][10];
const uint16_t big_p[21][10];
const uint16_t big_i[21][10];
const uint16_t big_d[21][10];
const uint16_t big_r[21][10];
const uint16_t big_n[21][10];
const uint16_t big_t[21][10];
const uint16_t big_m[21][10];
const uint16_t big_e[21][10];
const uint16_t big_s[21][10];
const uint16_t big_a[21][10];
const uint16_t big_u[21][10];
const uint16_t minus[21][10];
const uint16_t vertical[21][10];
const uint16_t horizontal[21][10];

#endif
