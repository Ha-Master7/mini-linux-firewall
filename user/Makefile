CC ?= gcc
CFLAGS ?= -Wall -Wextra -Werror -g -O2
CPPFLAGS += -I../include

all: mfwctl

mfwctl: mfwctl.c ../include/mfw_uapi.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ mfwctl.c

clean:
	rm -f mfwctl
