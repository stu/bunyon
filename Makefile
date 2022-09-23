CC = clang
CFLAGS = -O2 -Wall -pedantic -ggdb -g3 $(shell pkg-config garglk --cflags)

bunyon_files = main.o rnd.o msg.o interp.o unix_glkterm.o debug.o

all: bunyon

bunyon: $(bunyon_files)
	$(CC) -o bunyon $(bunyon_files) $(shell pkg-config garglk --libs) -lstdc++

clean:
	rm *.o
	rm bunyon

