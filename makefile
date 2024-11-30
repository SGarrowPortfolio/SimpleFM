CFLAGS = -Os -std=c99 -Wall
LDFLAGS = -lraylib -lopengl32 -lgdi32 -lwinmm
FILES = 

ifeq ($(OS),Windows_NT)
	FILES += $(shell dir /b src\*.c)
else 
	FILES += $(shell find . -type f -name '*.c')
endif

FILES_FULL = $(patsubst %.c,src/%.c,$(FILES))

all:
	gcc $(FILES_FULL) -o game -Os -Wall -Iinclude -Llib -lraylib -lopengl32 -lgdi32 -lwinmm

web: 
	emcc -o game.html $(FILES_FULL) lib/web/libraylib.a -Os -Wall -Iinclude -Llib/web -s USE_GLFW=3  -DPLATFORM_WEB --shell-file lib/web/template.html
