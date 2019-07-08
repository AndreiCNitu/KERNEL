## ARM Kernel

Operating system kernel, capable of executing and managing resources on an ARM-based platform.

***
### Building and executing:
#### MacOS
1. Open 3 new terminal windows
1. `make build; make launch-qemu`
2. `make launch-console`
3. `make launch-gdb`, `continue`

#### Linux
1. Open 2 new terminal windows
2. `make build; make launch-qemu`
3. `make launch-gdb`, `continue`

***
### Features

- pre-emptive multi-tasking
- priority-based scheduler
- fork, exec, exit, kill, nice system calls
- Inter-Process Communication (IPC) using pipes
- read and write operations have both blocking and non-blocking versions
- shell GUI interface using the LCD screen and PS/2 device drivers
- bit-mapped font for rendering letters and other characters
- top, clear and history commands
