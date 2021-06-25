CXX = ../tools/bin/x86_64-elf-g++
LD = ../tools/bin/x86_64-elf-ld

CXX_FLAGS = -I. \
			-Ilib \
			-I../tools/cxxshim/stage2/include \
			-Wall \
			-Wextra \
			-ffreestanding \
			-fno-stack-protector \
			-fno-exceptions \
			-mno-sse \
			-mno-sse2 \
			-mno-80387 \
			-mno-red-zone \
			-gdwarf \
			-fno-omit-frame-pointer \
			-fno-pic \
			-std=c++20 \
			-fno-use-cxa-atexit \
			-fno-rtti \
			-mcmodel=kernel \
			-fno-builtin \
			-flto 

LINK_FLAGS = -nostartfiles \
			 -nodefaultlibs \
			 -nostdlib \
			 -no-pie \
			 -lgcc 

CXX_SRC = $(shell find . -type f -name '*.cpp')
ASM_SRC = $(shell find . -type f \( -iname "*.asm" ! -iname "crt*" \))
REAL_SRC = $(shell find . -type f -name '*.real')

OBJ = $(CXX_SRC:.cpp=.o) $(ASM_SRC:.asm=.o)
BINS = $(REAL_SRC:.real=.bin)

CRTBEGIN_OBJ = $(shell $(CXX) $(CXX_FLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ = $(shell $(CXX) $(CXX_FLAGS) -print-file-name=crtend.o)

KERNEL = $(shell pwd)/rock.elf

all: $(KERNEL)

$(KERNEL): $(BINS) $(OBJ)
	nasm -felf64 crti.asm -o crti.o
	nasm -felf64 crtn.asm -o crtn.o
	$(CXX) $(LINK_FLAGS) crti.o $(CRTBEGIN_OBJ) $(OBJ) $(CRTEND_OBJ) crtn.o -n -T linker.ld -o $@

%.bin: %.real
	nasm -fbin $< -o $@

%.o: %.cpp
	$(CXX) $(CXX_FLAGS) -c $< -o $@

%.o: %.asm
	nasm -felf64 $< -o $@

clean:
	rm -rf $(OBJ) $(KERNEL) $(BINS) crti.o crtn.o
