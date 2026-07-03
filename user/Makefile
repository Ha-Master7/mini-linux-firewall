# user/Makefile

CC ?= gcc

CFLAGS ?= -Wall -Wextra -g
CPPFLAGS += -Iinclude
CPPFLAGS += -I../include

TARGET := mfwctl

SRCS := src/main.c \
        src/mfw_client.c

OBJS := $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)