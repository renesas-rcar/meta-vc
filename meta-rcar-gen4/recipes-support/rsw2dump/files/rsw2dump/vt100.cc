// Helper functions for VT100 terminal control
// The class supresses control sequences when not action on a real tty
//
// Christian Mardmöller (christian@mardmoeller.de)
// 6/2004  Initial version
//-----------------------------------------------------------------------------
#include "vt100.hh"
#include <termios.h>


int c_tty::get_pos(int *y, int *x)    //return 1 on error
{
	struct termios term, restore;

	char buf[30];
	int i;
	char ch;

	if (!tty || !realtty || !x || !y)
		return 1;

	tcgetattr(0, &term);
	tcgetattr(0, &restore);
	term.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(0, TCSANOW, &term);

	if (write(1, "\033[6n", 4) != 4) {
		tcsetattr(0, TCSANOW, &restore);
		return 1;
	}
	for(i = 0; ch != 'R'; i++) {
		if (!read(0, &ch, 1)) {
			tcsetattr(0, TCSANOW, &restore);
			return 1;
		}
		buf[i] = ch;
	}
	buf[i] = 0;
	tcsetattr(0, TCSANOW, &restore);
	//printf("\ni=%d  buf='%s'\n", i, buf+1);

	if (buf[0] != 27 || buf[1] != '[')
		return 1;

	*y = 0;
	*x = 0;
	if (sscanf(buf + 2, "%d;%d", x, y) != 2)
		return 1;

	return 0;
}
