 PROJECT_PATH     = $(shell find . -mindepth 1 -maxdepth 1 -type d)
 PROJECT_SOURCES  = $(shell find ${PROJECT_PATH} -name *.c -o -name *.s)
 PROJECT_HEADERS  = $(shell find ${PROJECT_PATH} -name *.h             )
 PROJECT_OBJECTS  = $(addsuffix .o, $(basename ${PROJECT_SOURCES}))
 PROJECT_TARGETS  = image.elf image.bin

 QEMU_PATH        = /usr/local
 QEMU_GDB         = 127.0.0.1:1234
 QEMU_UART        = stdio
 QEMU_UART       += telnet:127.0.0.1:1235,server
 QEMU_DISPLAY     = -display none

 LINARO_PATH      = "/usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc"
 LINARO_PREFIX    = arm-none-eabi


%.o   : %.s
	@${LINARO_PATH}/bin/${LINARO_PREFIX}-as  $(addprefix -I , ${PROJECT_PATH} ${LINARO_PATH}/${LINARO_PREFIX}/libc/usr/include) -mcpu=cortex-a8                                       -g                            -o ${@} ${<}
%.o   : %.c
	@${LINARO_PATH}/bin/${LINARO_PREFIX}-gcc $(addprefix -I , ${PROJECT_PATH} ${LINARO_PATH}/${LINARO_PREFIX}/libc/usr/include) -mcpu=cortex-a8 -mabi=aapcs -ffreestanding -std=gnu99 -g -c -fomit-frame-pointer -O -o ${@} ${<}

%.elf : ${PROJECT_OBJECTS}
	@${LINARO_PATH}/bin/${LINARO_PREFIX}-ld  -L ${LINARO_PATH}/lib/gcc/arm-none-eabi/8.2.1 -L ${LINARO_PATH}/arm-none-eabi/lib -T ${*}.ld -o ${@} ${^} -lc -lgcc
%.bin : %.elf
	@${LINARO_PATH}/bin/${LINARO_PREFIX}-objcopy -O binary ${<} ${@}


.PRECIOUS   : ${PROJECT_OBJECTS} ${PROJECT_TARGETS}

build       : ${PROJECT_TARGETS}

launch-qemu : ${PROJECT_TARGETS}
	@${QEMU_PATH}/bin/qemu-system-arm -M realview-pb-a8 -m 512M ${QEMU_DISPLAY} -gdb tcp:${QEMU_GDB} $(addprefix -serial , ${QEMU_UART}) -S -kernel $(filter %.bin, ${PROJECT_TARGETS})

launch-gdb  : ${PROJECT_TARGETS}
	@${LINARO_PATH}/bin/${LINARO_PREFIX}-gdb -ex "file $(filter %.elf, ${PROJECT_TARGETS})" -ex "target remote ${QEMU_GDB}"

kill-qemu   :
	@-killall --quiet --user ${USER} qemu-system-arm

kill-gdb    :
	@-killall --quiet --user ${USER} ${LINARO_PREFIX}-gdb

clean       :
	@rm -f core ${PROJECT_OBJECTS} ${PROJECT_TARGETS}

include Makefile.console
