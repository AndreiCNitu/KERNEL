#ifndef __DEVICE_H
#define __DEVICE_H

#define RSVD(x,y,z) uint8_t reserved_##x[ z - y + 1 ]

#define RO volatile const /* read  only */
#define WO volatile       /* write only */
#define RW volatile       /* read/write */
#define WR volatile       /* write/read */

#endif
