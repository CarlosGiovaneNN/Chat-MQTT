CC = gcc
CFLAGS = -Wall -I. -I./client -I./message -g
LDFLAGS = -lpaho-mqtt3as

SRCS = main.c $(wildcard client/*.c) $(wildcard message/*.c) $(wildcard shell/*.c) $(wildcard user/*.c) $(wildcard group/*.c)
OBJS = $(SRCS:.c=.o)
TARGET = main

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

client/%.o: client/%.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
