PROJECT		= sifs

HEADER		= $(PROJECT).h
LIBRARY		= lib$(PROJECT).a

APPLICATIONS	= sifs_mkvolume sifs_dirinfo sifs_fileinfo sifs_mkdir sifs_checkpathname sifs_getfile sifs_putfile sifs_rmfile sifs_rmdir sifs_validatevolume

# ----------------------------------------------------------------

CC      = cc
CFLAGS  = -std=c99 -Wall -Werror -pedantic
LIBS	= -L. -lsifs -lm


all:	$(APPLICATIONS)

$(LIBRARY):
	make -C library

%:	%.c $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)


clean:
	rm -f $(LIBRARY) $(APPLICATIONS)
	make -C library clean

