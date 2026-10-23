CC       ?= cc
CFLAGS   ?= -O2
CFLAGS   += -std=c99 -Wall -Wshadow 
CPPFLAGS += -D_POSIX_C_SOURCE=200809L -D_DARWIN_C_SOURCE
HELPERS := src/a.c
UTILS   := ca cd cm cu da ec en fa he ki ls md od pr so tr tu wc
BINS    := $(addprefix build/,$(UTILS))
a: $(BINS)
build/%: src/%.c $(HELPERS) $(wildcard src/*.h) ; @mkdir -p build ; $(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $< $(HELPERS)
t: a ; @sh tests/a.sh
c: ; rm -rf build
.PHONY: a t c
