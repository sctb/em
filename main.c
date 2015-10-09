/* This file is in the public domain. */

/*
 *	Mainline.
 */

#include "def.h"
#include "kbd.h"
#include "funmap.h"
#include "macro.h"

#include <err.h>
#include <limits.h>
#include <locale.h>

int		 thisflag;			/* flags, this command	*/
int		 lastflag;			/* flags, last command	*/
int		 curgoal;			/* goal column		*/
int		 startrow;			/* row to start		*/
int		 doaudiblebell;			/* audible bell toggle	*/
int		 dovisiblebell;			/* visible bell toggle	*/
struct buffer	*curbp;				/* current buffer	*/
struct buffer	*bheadp;			/* BUFFER list head	*/
struct mgwin	*curwp;				/* current window	*/
struct mgwin	*wheadp;			/* MGWIN listhead	*/
char		 pat[NPAT];			/* pattern		*/

static void	 edinit(struct buffer *);
static __dead void usage(void);

extern char	*__progname;

static __dead void
usage()
{
	fprintf(stderr, "usage: %s [-n] [-f mode] [+number] [file ...]\n",
	    __progname);
	exit(1);
}

int
main(int argc, char **argv)
{
	char		*cp, *init_fcn_name = NULL;
	PF		 init_fcn = NULL;
	int	 	 o, i, nfiles;
	struct buffer	*bp = NULL;

	while ((o = getopt(argc, argv, "f:")) != -1)
		switch (o) {
		case 'f':
			if (init_fcn_name != NULL)
				errx(1, "cannot specify more than one "
				    "initial function");
			init_fcn_name = optarg;
			break;
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	setlocale(LC_CTYPE, "");

	maps_init();		/* Keymaps and modes.		*/
	funmap_init();		/* Functions.			*/

	if (init_fcn_name &&
	    (init_fcn = name_function(init_fcn_name)) == NULL)
		errx(1, "Unknown function `%s'", init_fcn_name);

	vtinit();		/* Virtual terminal.		*/
	dirinit();		/* Get current directory.	*/
	edinit(bp);		/* Buffers, windows.		*/
	bellinit();		/* Audible and visible bell.	*/

	/*
	 * doing update() before reading files causes the error messages from
	 * the file I/O show up on the screen.	(and also an extra display of
	 * the mode line if there are files specified on the command line.)
	 */
	update(CMODE);

	/* Force FFOTHARG=1 so that this mode is enabled, not simply toggled */
	if (init_fcn)
		init_fcn(FFOTHARG, 1);

	for (nfiles = 0, i = 0; i < argc; i++) {
		if (argv[i][0] == '+' && strlen(argv[i]) >= 2) {
			long lval;
			char *end;

			lval = strtol(&argv[i][1], &end, 0);
			if (argv[i][1] == '\0' || *end != '\0')
				goto notnum;
			startrow = lval;
		} else {
notnum:
			cp = adjustname(argv[i], FALSE);
			if (cp != NULL) {
				if (nfiles == 1)
					splitwind(0, 1);

				if ((curbp = findbuffer(cp)) == NULL) {
					vttidy();
					errx(1, "Can't find current buffer!");
				}
				(void)showbuffer(curbp, curwp, 0);
				if (readin(cp) != TRUE)
					killbuffer(curbp);
				else {
					/* Ensure enabled, not just toggled */
					if (init_fcn_name)
						init_fcn(FFOTHARG, 1);
					nfiles++;
				}
			}
		}
	}

	if (nfiles > 2)
		listbuffers(0, 1);

	/* fake last flags */
	thisflag = 0;
	for (;;) {
		if (epresf == KCLEAR)
			eerase();
		if (epresf == TRUE)
			epresf = KCLEAR;
		if (winch_flag) {
			do_redraw(0, 0, TRUE);
			winch_flag = 0;
		}
		update(CMODE);
		lastflag = thisflag;
		thisflag = 0;

		switch (doin()) {
		case TRUE:
			break;
		case ABORT:
			ewprintf("Quit");
			/* FALLTHRU */
		case FALSE:
		default:
			macrodef = FALSE;
		}
	}
}

/*
 * Initialize default buffer and window. Default buffer is called *scratch*.
 */
static void
edinit(struct buffer *bp)
{
	struct mgwin	*wp;

	bheadp = NULL;
	bp = bfind("*scratch*", TRUE);		/* Text buffer.          */
	if (bp == NULL)
		panic("edinit");

	wp = new_window(bp);
	if (wp == NULL)
		panic("edinit: Out of memory");

	curbp = bp;				/* Current buffer.	 */
	wheadp = wp;
	curwp = wp;
	wp->w_wndp = NULL;			/* Initialize window.	 */
	wp->w_linep = wp->w_dotp = bp->b_headp;
	wp->w_ntrows = nrow - 2;		/* 2 = mode, echo.	 */
	wp->w_rflag = WFMODE | WFFULL;		/* Full.		 */
}

/*
 * Quit command.  If an argument, always quit.  Otherwise confirm if a buffer
 * has been changed and not written out.  Normally bound to "C-X C-C".
 */
/* ARGSUSED */
int
quit(int f, int n)
{
	int	 s;

	if ((s = anycb(FALSE)) == ABORT)
		return (ABORT);
	if (s == FIOERR || s == UERROR)
		return (FALSE);
	if (s == FALSE
	    || eyesno("Modified buffers exist; really exit") == TRUE) {
		vttidy();
		exit(0);
	}
	return (TRUE);
}

/*
 * User abort.  Should be called by any input routine that sees a C-g to abort
 * whatever C-g is aborting these days. Currently does nothing.
 */
/* ARGSUSED */
int
ctrlg(int f, int n)
{
	return (ABORT);
}
