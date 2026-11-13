# Usage:
#   make / make all       build all three
#   make scheduler / sch  build only the scheduler
#   make client           build only the client
#   make worker           build only the worker
#   make clean            remove build output

CC ?= cc

CSTD := -std=c23
POSIX := -D_POSIX_C_SOURCE=200809L

# strict POSIX hides _SC_NPROCESSORS_ONLN and _SC_PHYS_PAGES on macOS
ifeq ($(shell uname -s),Darwin)
  POSIX += -D_DARWIN_C_SOURCE
else
  POSIX += -D_DEFAULT_SOURCE
endif

WARNINGS := -Wall -Wextra -Wpedantic \
            -Wshadow -Wconversion -Wsign-conversion -Wdouble-promotion \
            -Wcast-qual -Wcast-align -Wpointer-arith -Wbad-function-cast \
            -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations \
            -Wold-style-definition -Wredundant-decls -Wnested-externs \
            -Wwrite-strings -Wformat=2 -Wundef -Wfloat-equal -Wvla \
            -Wswitch-enum -Wimplicit-fallthrough -Wnull-dereference

ifneq ($(shell $(CC) --version 2>/dev/null | grep -i clang),)
  WARNINGS += -Wshorten-64-to-32 -Wcomma -Wassign-enum \
              -Wconditional-uninitialized -Wloop-analysis \
              -Wunreachable-code-aggressive
else
  WARNINGS += -Wlogical-op -Wduplicated-cond -Wduplicated-branches \
              -Wjump-misses-init -Wformat-signedness \
              -Wformat-overflow=2 -Wformat-truncation=2 \
              -Wstringop-overflow=4 -Wcast-align=strict \
              -Wtrampolines -Wmissing-include-dirs
endif

CFLAGS   ?= -O2
CPPFLAGS += -Iinclude -MMD -MP $(POSIX)
LDFLAGS  ?=
LDLIBS   ?=

BUILD := build
BIN   := bin

COMMON_SRCS := $(wildcard include/*.c)
COMMON_OBJS := $(COMMON_SRCS:%.c=$(BUILD)/%.o)

SCHED_SRCS  := $(wildcard scheduler/*.c)
CLIENT_SRCS := $(wildcard client/*.c)
WORKER_SRCS := $(wildcard worker/*.c)

SCHED_OBJS  := $(SCHED_SRCS:%.c=$(BUILD)/%.o)  $(COMMON_OBJS)
CLIENT_OBJS := $(CLIENT_SRCS:%.c=$(BUILD)/%.o) $(COMMON_OBJS)
WORKER_OBJS := $(WORKER_SRCS:%.c=$(BUILD)/%.o) $(COMMON_OBJS)

.PHONY: all sch scheduler client worker clean

all: scheduler client worker

sch:       scheduler
scheduler: $(BIN)/scheduler
client:    $(BIN)/client
worker:    $(BIN)/worker

$(BIN)/scheduler: $(SCHED_OBJS)
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BIN)/client: $(CLIENT_OBJS)
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BIN)/worker: $(WORKER_OBJS)
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CSTD) $(CPPFLAGS) $(WARNINGS) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD) $(BIN)

-include $(SCHED_OBJS:.o=.d) $(CLIENT_OBJS:.o=.d) $(WORKER_OBJS:.o=.d)
