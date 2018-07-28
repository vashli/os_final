all: ssfs

ssfs: ssfs.c
	gcc ssfs.c -o ssfs `pkg-config fuse --cflags --libs`

clean:
	rm -f ssfs
