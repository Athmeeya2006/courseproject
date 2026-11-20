# Usage:
#   make / make all       build all three
#   make scheduler / sch  build only the scheduler
#   make client           build only the client
#   make worker           build only the worker
#   make clean            remove build output

CC ?= cc

# C23. GCC >= 14 / Clang >= 18 call it -std=c23; GCC 9-13 / Clang 9-17 only know the
# older name -std=c2x (same standard, draft name). Use whichever this compiler accepts.
CSTD := $(shell echo 'int x;' | $(CC) -std=c23 -x c -c - -o /dev/null >/dev/null 2>&1 && echo -std=c23 || echo -std=c2x)
POSIX := -D_POSIX_C_SOURCE=200809L

# strict POSIX hides _SC_NPROCESSORS_ONLN and _SC_PHYS_PAGES on macOS
ifeq ($(shell uname -s),Darwin)
  POSIX += -D_DARWIN_C_SOURCE
else
  POSIX += -D_DEFAULT_SOURCE
endif

# Probe each warning flag so old/new GCC and Clang never fail on an unknown option.
cc-option = $(shell echo 'int x;' | $(CC) $(1) -Werror -x c -c - -o /dev/null >/dev/null 2>&1 && echo $(1))

WANT_WARNINGS := -Wall -Wextra -Wpedantic \
            -Wshadow -Wconversion -Wsign-conversion -Wdouble-promotion \
            -Wcast-qual -Wcast-align -Wpointer-arith -Wbad-function-cast \
            -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations \
            -Wold-style-definition -Wredundant-decls -Wnested-externs \
            -Wwrite-strings -Wformat=2 -Wundef -Wfloat-equal -Wvla \
            -Wswitch-enum -Wimplicit-fallthrough -Wnull-dereference \
            -Wshorten-64-to-32 -Wcomma -Wassign-enum \
            -Wconditional-uninitialized -Wloop-analysis \
            -Wunreachable-code-aggressive \
            -Wlogical-op -Wduplicated-cond -Wduplicated-branches \
            -Wjump-misses-init -Wformat-signedness \
            -Wformat-overflow=2 -Wformat-truncation=2 \
            -Wstringop-overflow=4 -Wcast-align=strict \
            -Wtrampolines -Wmissing-include-dirs

WARNINGS := $(foreach w,$(WANT_WARNINGS),$(call cc-option,$(w)))

CFLAGS   ?= -O2
CPPFLAGS += -Iinclude -MMD -MP $(POSIX)
LDFLAGS  ?=
LDLIBS   ?=

BUILD := build
# The spec runs ./scheduler, ./worker and ./client from the project root, so the
# binaries are built here (the sources live in src/ to avoid a name clash).
PROGS := scheduler client worker

COMMON_SRCS := $(wildcard include/*.c)
COMMON_OBJS := $(COMMON_SRCS:%.c=$(BUILD)/%.o)

SCHED_SRCS  := $(wildcard src/scheduler/*.c)
CLIENT_SRCS := $(wildcard src/client/*.c)
WORKER_SRCS := $(wildcard src/worker/*.c)

SCHED_OBJS  := $(SCHED_SRCS:%.c=$(BUILD)/%.o)  $(COMMON_OBJS)
CLIENT_OBJS := $(CLIENT_SRCS:%.c=$(BUILD)/%.o) $(COMMON_OBJS)
WORKER_OBJS := $(WORKER_SRCS:%.c=$(BUILD)/%.o) $(COMMON_OBJS)

.PHONY: all sch clean

all: $(PROGS)

sch: scheduler

scheduler: $(SCHED_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

client: $(CLIENT_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

worker: $(WORKER_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CSTD) $(CPPFLAGS) $(WARNINGS) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD) bin
	rm -f $(PROGS)

-include $(SCHED_OBJS:.o=.d) $(CLIENT_OBJS:.o=.d) $(WORKER_OBJS:.o=.d)
