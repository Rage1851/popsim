CC = gcc-11
CFLAGS = -I include/ -lpthread -lm
CFILES = src/popsimio.c lib/arrurn.c lib/linurn.c lib/bsturn.c lib/aliurn.c lib/coll.c lib/hgeom.c lib/intpmap.c lib/popsim.c

popsim: $(CFILES)
	$(CC) $(CFLAGS) -o popsimio $(CFILES)
