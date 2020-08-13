# Assumption that sifs.h will be provided for us in compilation

HEADERS	= ../sifs.h sifs-internal.h md5.h sifs-helper.h
LIBRARY	= libsifs.a


OBJECTS	= mkvolume.o mkdir.o rmdir.o dirinfo.o checkpathname.o validatevolume.o\
		writefile.o readfile.o rmfile.o fileinfo.o\
		perror.o md5.o sifs-helper.o 

CC      = cc
CFLAGS  = -std=c99 -Wall -Werror -pedantic


$(LIBRARY):	$(OBJECTS)
	ar rc $(LIBRARY) $(OBJECTS)
	ranlib $(LIBRARY)
	cp $(LIBRARY) ..


%.o:	%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $<


# ----------------------------------------------------------------

clean:
	rm -f $(LIBRARY) $(OBJECTS)
