all:
	gcc *.c -g -D_FILE_OFFSET_BITS=64 `pkg-config fuse python3 --cflags --libs` -o thfs

clean:
	rm thfs
	rm -rf /tmp/thfs
	rm -rf __pycache__