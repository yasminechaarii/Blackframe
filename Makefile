all:
	gcc main.c settings.c -o settings_menu `sdl2-config --cflags --libs` -lSDL2_image -lSDL2_mixer -lSDL2_ttf

run: all
	./settings_menu

clean:
	rm -f settings_menu
