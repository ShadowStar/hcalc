_all: hcalc

SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
	else if [ -x /bin/bash ]; then echo /bin/bash; \
	else echo "/bin/sh -c"; fi; fi)

GCC := gcc
CFLAGS := -O2 -W -Wall -Wshadow -Wundef

hcalc: hcalc.c
	@$(GCC) $(CFLAGS) -o $@ $^

clean:
	@-rm -f hcalc

.PHONY: clean

