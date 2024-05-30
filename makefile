# make release : generate a release build
# make debug   : generate a debug build




# toolchain:
CC := gcc
AS := nasm
LD := gcc


# opt levels:
OPT_DBG := 0
OPT_REL := 2


# output:
BIN := proxy_server.elf64

# output dirs:
BUILDDIR            := build
BUILDDIR_RELEASEDIR := $(BUILDDIR)/release
BUILDDIR_DEBUGDIR   := $(BUILDDIR)/debug


# srcs:
SRCDIR  := ./
SRC_C   := $(shell find $(SRCDIR)/ -type d \( -path ./tests \) -prune -false -o -name '*.c')
SRC_ASM := $(shell find $(SRCDIR)/ -type d \( -path ./tests \) -prune -false -o -name '*.asm')


# objs:
OBJS := ${SRC_C:.c=.o} ${SRC_ASM:.asm=.o}


# toolchain flags:
CCFLAGS_DEBUG   := -O$(OPT_DBG)              \
                    -std=c23                 \
                    -Wall                    \
                    -Wextra                  \
                    -Wpadded                 \
                    -pedantic                \
                    -pedantic-errors         \
                    -pipe                    \
					-pthread                 \
                    -g3                      \
                    -fno-pic                 \
                    -I$(SRCDIR)/inc

CCFLAGS_RELEASE := -O$(OPT_REL)              \
                    -std=c23                 \
                    -Wall                    \
                    -Wextra                  \
                    -pedantic                \
                    -pedantic-errors         \
                    -pipe                    \
					-pthread                 \
                    -fno-pic                 \
                    -I$(SRCDIR)/inc

ASFLAGS          := -O0                      \
                    -f elf64

LDFLAGS_DEBUG    := -O$(OPT_DBG)             \
                    -fno-pie

LDFLAGS_RELEASE  := -O$(OPT_REL)             \
                    -fno-pie




# eval later in setvars_* recipes:
BUILD_OUTPUT_DIR :=
CCFLAGS          :=
LDFLAGS          :=




# START:
all:
	@printf "\nUsage:\n"
	@printf "\nmake release : generate a release build\n"
	@printf "\nmake debug   : generate a debug build\n"


# the main build routines:
debug : dirs_debug setvars_debug compile_and_link
	@printf "\nOUTPUT: ./$(BUILD_OUTPUT_DIR)/$(BIN)\n"
	@printf "\nDEBUG: DONE!\n\n"

release : dirs_release setvars_release compile_and_link
	@printf "\nOUTPUT: ./$(BUILD_OUTPUT_DIR)/$(BIN)\n"
	@printf "\nRELEASE: DONE!\n\n"


# mkdirs:
dirs_debug:
	@mkdir -p $(BUILDDIR)
		@mkdir -p $(BUILDDIR_DEBUGDIR)

dirs_release:
	@mkdir -p $(BUILDDIR)
		@mkdir -p $(BUILDDIR_RELEASEDIR)


# set vars:
setvars_debug:
	$(eval CCFLAGS = $(CCFLAGS_DEBUG))
	$(eval LDFLAGS  = $(LDFLAGS_DEBUG))
	$(eval BUILD_OUTPUT_DIR = $(BUILDDIR_DEBUGDIR))

setvars_release:
	$(eval CCFLAGS = $(CCFLAGS_RELEASE))
	$(eval LDFLAGS  = $(LDFLAGS_RELEASE))
	$(eval BUILD_OUTPUT_DIR = $(BUILDDIR_RELEASEDIR))


# invoke compiling templates and link the bin:
compile_and_link : $(OBJS)
	@printf "LINK: "
	$(LD) $(LDFLAGS) $^ -o $(BUILD_OUTPUT_DIR)/$(BIN)


# compile .c:
%.o : %.c
	@printf "COMPILE: "
	$(CC) $(CCFLAGS) -c $< -o $@

# compile .asm:
%.o : %.asm
	@printf "COMPILE: "
	$(AS) $(ASFLAGS) $<




# clean:
clean:
	rm -rf $(BUILDDIR) $(shell find ./ -type f -name '*.o')
