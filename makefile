TARGET = prog
LIBS =
CC = gcc
EXTPATH = ./lib
CFLAGS = -g -Wall -I $(EXTPATH)
.PHONY: default all clean
default: $(TARGET)
all: default
OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@
.PRECIOUS: $(TARGET) $(OBJECTS)
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
