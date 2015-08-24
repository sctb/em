# (Common) compile-time options:
#
#	FKEYS		-- add support for function key sequences.
#	XKEYS		-- use termcap function key definitions.
#				note: XKEYS and bsmap mode do _not_ get along.
#
CFLAGS+=-std=c89 -Wall -DFKEYS -DXKEYS
LDFLAGS+=-lcurses

SRCS= basic.c bell.c buffer.c cinfo.c dir.c display.c echo.c extend.c	\
	file.c fileio.c funmap.c help.c kbd.c keymap.c line.c macro.c	\
	main.c match.c modes.c paragraph.c random.c reallocarray.c	\
	regex.c region.c search.c spawn.c tty.c ttyio.c ttykbd.c	\
	undo.c version.c window.c word.c yank.c

%.o : %.c
	cc ${CFLAGS} -c $<

em : $(SRCS:.c=.o)
	cc ${LDFLAGS} $^ -o $@
