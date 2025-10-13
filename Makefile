CC = gcc
CFLAGS = -Wall -I. -I./client -I./message -g
LIBS = -lpaho-mqtt3as -lssl -lcrypto

SRCS = main.c \
       $(wildcard client/*.c) \
       $(wildcard message/*.c) \
       $(wildcard shell/*.c) \
       $(wildcard user/*.c) \
       $(wildcard group/*.c) \
       $(wildcard threads/*.c) \
       $(wildcard chat/*.c) \
       $(wildcard AES/*.c)

OBJS = $(SRCS:.c=.o)
TARGET = main

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)

client/%.o: client/%.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
