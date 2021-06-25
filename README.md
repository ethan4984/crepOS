# Rock

rockOS is a simple kernel that strives to be unix-like 

# Features

- PMM/VMM (4/5 level paging)
- XAPIC/2XAPIC
- IDT/GDT/TSS
- EXT2
- VFS
- RAMFS
- DEVFS
- SMP
- PCI
- AHCI
- NVME
- XHCI (Work in progress)
- HPET
- Preemptive multicore scheduler
- Slab allocator

# Goals
  - Port various pieces of software like bash, gcc, python, xwayland, etc
  - Create a network stack with TCP, ARP, UDP, DHCP, ICMP, etc

# Build Instructions

Here are some programs you will need to build rock:
  - `nasm`
  - `make`
  - `qemu`
  - `tar`
  - `sed`
  - `wget`
  - `git`
  - `xbstrap`
  
Once you have those run the script `build_tools.sh` in `/tools`

To compile and run rock, there are three default build targets available:
  - `make qemu`
    - run with regular qemu (live serial debugger)
  - `make info`
    -  run with the qemu console 
  - `make debug`
    - run with the qemu interrupt monitor

# Contributing

Contributors are very welcome, just make sure to use the same code style as me :^)
