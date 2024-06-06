# make release : generate a release build
# make debug   : generate a debug build




# toolchain:
CC := gcc
LD := gcc


# opt levels:
OPT_DBG := 0
OPT_REL := 2


# output:
BIN := proxy_server

# output dirs:
BUILDDIR            := build
BUILDDIR_RELEASEDIR := $(BUILDDIR)/release
BUILDDIR_DEBUGDIR   := $(BUILDDIR)/debug


# srcs:
SRCDIR := ./
SRC_C  := $(shell find $(SRCDIR)/ -type d \( -path ./tests \) -prune -false -o -name '*.c')


# objs:
OBJS := ${SRC_C:.c=.o}


# toolchain flags:
CCFLAGS_DEBUG   := -O$(OPT_DBG)             \
                   -std=gnu23               \
                   -Wall                    \
                   -Wextra                  \
                   -Wpadded                 \
                   -pedantic                \
                   -pedantic-errors         \
                   -pipe                    \
				   -pthread                 \
				   -lrt                     \
                   -g3

CCFLAGS_RELEASE := -O$(OPT_REL)             \
                   -std=gnu23               \
                   -Wall                    \
                   -Wextra                  \
                   -Wpadded                 \
                   -pedantic                \
                   -pedantic-errors         \
                   -pipe                    \
				   -pthread                 \
				   -lrt

LDFLAGS_DEBUG   := -O$(OPT_DBG)

LDFLAGS_RELEASE := -O$(OPT_REL)




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


# make build dirs:
dirs_debug:
	@mkdir -p $(BUILDDIR)
		@mkdir -p $(BUILDDIR_DEBUGDIR)

dirs_release:
	@mkdir -p $(BUILDDIR)
		@mkdir -p $(BUILDDIR_RELEASEDIR)


# set build vars:
setvars_debug:
	$(eval CCFLAGS = $(CCFLAGS_DEBUG))
	$(eval LDFLAGS  = $(LDFLAGS_DEBUG))
	$(eval BUILD_OUTPUT_DIR = $(BUILDDIR_DEBUGDIR))

setvars_release:
	$(eval CCFLAGS = $(CCFLAGS_RELEASE))
	$(eval LDFLAGS  = $(LDFLAGS_RELEASE))
	$(eval BUILD_OUTPUT_DIR = $(BUILDDIR_RELEASEDIR))


# invoke the .c compiling template and link the bin:
compile_and_link : $(OBJS)
	@printf "LINK: "
	$(LD) $(LDFLAGS) $^ -o $(BUILD_OUTPUT_DIR)/$(BIN)


# compile .c:
%.o : %.c
	@printf "COMPILE: "
	$(CC) $(CCFLAGS) -c $< -o $@




# clean:
clean:
	rm -rf $(BUILDDIR) $(shell find ./ -type f -name '*.o')
