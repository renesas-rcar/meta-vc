// Helper functions for VT100 terminal control
// The class supresses control sequences when not action on a real tty
//
// Christian Mardmöller (christian@mardmoeller.de)
// 6/2004  Initial version
// 3/2006  Add control to overrule the tty detection
// 10/2014 Add some more colour codes
//
// Usage
// Define an instance of class c_tty (typically as global variable)
//   class c_tty tty;
// Use the colour codes in printf
//   printf("Something in %sred%s...\n", tty.red(), tty.norm());
//-----------------------------------------------------------------------------
#ifndef __VT100_HH__
#define __VT100_HH__

#include <stdio.h>          //stdin
#include <unistd.h>         //isatty
#include <string.h>         //strcat
#include <sys/ioctl.h>

//-----------------------------------------------------------------------------
class c_tty {
private:
	int realtty;
public:
	int tty;

	c_tty() : realtty(isatty(fileno(stdin))), tty(isatty(fileno(stdin))) {}
	c_tty(int  force) : realtty(force), tty(force) {}
	~c_tty() {}
	int getTty() { return realtty; }
	void clearRealTty() { realtty = 0; }

	const char *norm()    { return (tty) ? "\033[m" : ""; }

	const char *black()   { return (tty) ? "\033[30;2m" : ""; }
	const char *red()     { return (tty) ? "\033[31;2m" : ""; }
	const char *green()   { return (tty) ? "\033[32;2m" : ""; }
	const char *yellow()  { return (tty) ? "\033[33;2m" : ""; }
	const char *blue()    { return (tty) ? "\033[34;2m" : ""; }
	const char *magenta() { return (tty) ? "\033[35;2m" : ""; }
	const char *cyan()    { return (tty) ? "\033[36;2m" : ""; }
	const char *gray()    { return (tty) ? "\033[37;2m" : ""; }

	const char *L_black()   { return (tty) ? "\033[30;1m" : ""; }
	const char *L_red()     { return (tty) ? "\033[31;1m" : ""; }
	const char *L_green()   { return (tty) ? "\033[32;1m" : ""; }
	const char *L_yellow()  { return (tty) ? "\033[33;1m" : ""; }
	const char *L_blue()    { return (tty) ? "\033[34;1m" : ""; }
	const char *L_magenta() { return (tty) ? "\033[35;1m" : ""; }
	const char *L_cyan()    { return (tty) ? "\033[36;1m" : ""; }
	const char *white()     { return (tty) ? "\033[37;1m" : ""; }

	const char *B_black()   { return (tty) ? "\033[40m" : ""; }
	const char *B_red()     { return (tty) ? "\033[41m" : ""; }
	const char *B_green()   { return (tty) ? "\033[42m" : ""; }
	const char *B_yellow()  { return (tty) ? "\033[43m" : ""; }
	const char *B_blue()    { return (tty) ? "\033[44m" : ""; }
	const char *B_magenta() { return (tty) ? "\033[45m" : ""; }
	const char *B_cyan()    { return (tty) ? "\033[46m" : ""; }
	const char *B_white()   { return (tty) ? "\033[47m" : ""; }

	const char *B_white_red()     { return (tty) ? "\033[47;31;1m" : ""; }
	const char *B_yellow_black()  { return (tty) ? "\033[43;30;2m" : ""; }
	const char *B_green_black()  { return (tty) ? "\033[42;30;2m" : ""; }
	const char *B_blue_yellow()  { return (tty) ? "\033[44;33;1m" : ""; }

	const char* indent(int c)
	{
		static char s[20];
		if (!tty || !realtty)
			sprintf(s, "\033[%dC", c);
		else {
			s[0] = 0;
			while (c > 8) {
				strcat(s, "\t");
				c -= 8;
			}
		}
		return s;
	}

	int rows()
	{
		struct winsize w;
		if (!tty || !realtty)
			return 1;
		ioctl(0, TIOCGWINSZ, &w);
		return w.ws_row;
	}

	int cols()
	{
		struct winsize w;
		if (!tty || !realtty)
			return 1;
		ioctl(0, TIOCGWINSZ, &w);
		return w.ws_col;
	}

	int get_pos(int *y, int *x);    //return 1 on error
};


extern class c_tty tty;

#endif
