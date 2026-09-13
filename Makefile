# pickme — build system
#
# Usage:
#   make            debug build      -> bin/pickme          (g -O0)
#   make release    optimized build  -> bin/pickme-release  (-O2)
#   make test       run the ceedling unit tests
#   make clean      remove OUR artifacts (leaves ceedling's build/test/ alone)
#
# Both build types keep their own object tree (build/obj/debug, build/obj/release),
# so switching between them never reuses objects compiled with the wrong flags.
#
# Everything the compiler/linker needs comes from pkg-config, so no include
# paths or library names are hard-coded beyond the package names below.

CC              ?= gcc
WAYLAND_SCANNER ?= wayland-scanner

# pkg-config names of the libraries we use.
PKGS := wayland-client xkbcommon cairo

PKG_CFLAGS := $(shell pkg-config --cflags $(PKGS) 2>/dev/null)
PKG_LIBS   := $(shell pkg-config --libs   $(PKGS) 2>/dev/null)

# Where the xdg-shell protocol description lives (pkg-config knows; fall
# back to the Arch default if the .pc file is missing).
WAYLAND_PROTOCOLS_DIR := $(shell pkg-config --variable=pkgdatadir wayland-protocols 2>/dev/null)
ifeq ($(strip $(WAYLAND_PROTOCOLS_DIR)),)
WAYLAND_PROTOCOLS_DIR := /usr/share/wayland-protocols
endif
XDG_SHELL_XML := $(WAYLAND_PROTOCOLS_DIR)/stable/xdg-shell/xdg-shell.xml

BUILD ?= debug

CFLAGS   += -Wall -Wextra -MMD -MP
CPPFLAGS += -Iinclude -Ibuild/protocols $(PKG_CFLAGS)
LDLIBS   += $(PKG_LIBS)

ifeq ($(BUILD),release)
CFLAGS += -O2
BIN    := bin/pickme-release
else
CFLAGS += -g -O0
BIN    := bin/pickme
endif

OBJDIR := build/obj/$(BUILD)

# Hand-written sources + the wayland-scanner output.
SRC     := $(wildcard src/*.c)
GEN_SRC := build/protocols/xdg-shell-protocol.c

OBJ := $(patsubst src/%.c,$(OBJDIR)/src/%.o,$(SRC)) \
       $(patsubst build/protocols/%.c,$(OBJDIR)/gen/%.o,$(GEN_SRC))
DEP := $(OBJ:.o=.d)

.PHONY: all release test clean

all: $(BIN)

# Convenience: `make release` re-invokes make with BUILD=release.
release:
	$(MAKE) BUILD=release all

$(BIN): $(OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $(OBJ) $(LDLIBS) -o $@

# All sources may rely on the generated xdg-shell header, so make sure it
# exists before compiling anything (order-only: its timestamp alone does
# not force recompiles).
$(OBJDIR)/src/%.o: src/%.c | build/protocols/xdg-shell-client-protocol.h
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(OBJDIR)/gen/%.o: build/protocols/%.c | build/protocols/xdg-shell-client-protocol.h
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Two separate rules: wayland-scanner emits one file per invocation.
# (build/protocols/ — NOT build/gen/, which belongs to ceedling's mocks.)
build/protocols/xdg-shell-client-protocol.h: $(XDG_SHELL_XML)
	@mkdir -p $(dir $@)
	$(WAYLAND_SCANNER) client-header $< $@

build/protocols/xdg-shell-protocol.c: $(XDG_SHELL_XML)
	@mkdir -p $(dir $@)
	$(WAYLAND_SCANNER) private-code $< $@

test:
	ceedling test:all

clean:
	rm -rf build/obj build/protocols bin

# Header dependencies picked up from the compiler (-MMD -MP). The leading
# '-' means "don't complain on the very first build when no .d exists yet".
-include $(DEP)
