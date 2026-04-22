CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -O2 \
           $(shell sdl2-config --cflags)
LIBS    = $(shell sdl2-config --libs) \
           -lSDL2_image -lSDL2_ttf

SRC     = main.c background.c player.c
OBJ     = $(SRC:.c=.o)
TARGET  = game

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
