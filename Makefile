CFLAGS+=-std=c89 -Wall
LDFLAGS+=-lcurses

SRCS= basic.c bell.c buffer.c cinfo.c dir.c display.c echo.c file.c	\
	fileio.c funmap.c help.c kbd.c keymap.c line.c macro.c		\
	main.c match.c modes.c paragraph.c util.c reallocarray.c	\
	regex.c region.c search.c spawn.c tty.c ttyio.c undo.c 		\
	window.c word.c yank.c

%.o : %.c
	cc ${CFLAGS} -c $<

em : $(SRCS:.c=.o)
	cc ${LDFLAGS} $^ -o $@
