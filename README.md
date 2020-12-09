# Rock

This is a simple multicore operating system that is made for fun

# Features

- x86_64 kernel
  - PMM
  - VMM
  - SMP
  - APIC
  - AHCI
  - EXT2
  - PCI
  - VFS
  
# Demo

![](demo.jpg)

# Build Instructions

Here are some programs you will need to build rock:
  - `nasm`
  - `make`
  - `qemu`
  - `tar`
  - `sed`
  - `wget`
  - `git`
  
Now that you have those just run the build script `tools/build_tools.sh`

The build script compiles a cross compiler along with cloning into limine

To compile and run rock, there are three default build targets available:
  - `make qemu`
    - run with regular qemu (live serial debugger)
  - `make info`
    -  run with the qemu console 
  - `make debug`
    - run with the qemu interrupt monitor

# Contributing

Contributors are very welcome, just make sure to use the same code style as me :^)
