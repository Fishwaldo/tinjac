/* Tinjac - crontabs.cpp
** Copyright (c) 2010 Justin Hammond
**
** Portions Copyright:
** Copyright 1988,1990,1993,1994 by Paul Vixie
**
** Copyright (c) 2004 by Internet Systems Consortium, Inc. ("ISC")
** Copyright (c) 1997,2000 by Internet Software Consortium, Inc.
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
**  USA
**
** Tinjac SVN Identification:
** $Rev$
*/

/** @file crontabs.cpp 
 *  @brief Read system (cronnie formated) crontabs
 */ 


#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "log.hpp"
#include "crontabs.hpp"



using namespace std;
using namespace boost::filesystem;
using namespace boost::posix_time;
namespace io = boost::iostreams;



typedef enum ecode {
	e_none, e_minute, e_hour, e_dom, e_month, e_dow,
	e_cmd, e_timespec, e_username, e_option, e_memory
} ecode_e;

static const char *ecodes[] = {
	"no error",
	"bad minute",
	"bad hour",
	"bad day-of-month",
	"bad month",
	"bad day-of-week",
	"bad command",
	"bad time specifier",
	"bad username",
	"bad option",
	"out of memory"
};

const char *MonthNames[]
	= {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
		NULL
	};

const char *DowNames[]
	= {
		"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun",
		NULL
	};


crontabs::crontabs () {

}
crontabs::crontabs (path dbdir, bool system) {
	this->setPath(dbdir, system);
}
crontabs::~crontabs() {

}

bool crontabs::setPath(path dbdir, bool system) {
	this->SystemDir = system;
	if (!exists(dbdir)) {
		DLOG("%s does not exist", dbdir.string().c_str());
		return false;
	}
	this->crontabdir = dbdir;
	directory_iterator end_itr; // default construction yields past-the-end
	for ( directory_iterator itr( this->crontabdir ); itr != end_itr; ++itr ) {
	    if ( is_regular_file(itr->status()) )
	    {
	    	DLOG("Checking %s for Valid Crontab", itr->path().string().c_str());
	    	this->parseCrontab(itr->path().string() , this->SystemDir);
	    }
	    else
	    	DLOG("Not a valid Crontab: %s", itr->path().string().c_str());
	  }
	return true;
}


bool crontabs::parseCrontab(string fname, bool system) {
	int crontab_fd;
	FILE *file;

	if ((crontab_fd = open(fname.c_str(), O_RDONLY | O_NONBLOCK, 0)) == -1) {
		ELOG("Can't open %s: %s", fname.c_str(), strerror(errno));
		return (false);
	}
	if (!(file = fdopen(crontab_fd, "r")))	{
		int save_errno = errno;
		ELOG("Failed to Fdopen on %s: %s", fname.c_str(), strerror(errno));
		return (false);
	}
	struct passwd *pw = NULL;
	char **envp = env_init();
	entry *e = this->load_entry(file, NULL, pw, envp, fname);
	if (e) {
		cout << " Command: " << e->cmd << "\n";
		this->printTime(e);
		if ((e) && (e->pwd != NULL) && (e->pwd->pw_name != NULL))
			cout << " User: " << e->pwd->pw_name << "\n";
	}
}

bool crontabs::printTime(entry *e) {
	int i;
	cout << "Minute:";
	for (i = FIRST_MINUTE; i < LAST_MINUTE; i++)
		cout << (bit_test(e->minute, i) ? '1' : '0');
	cout << endl;
	cout << "Hour:";
	for (i = FIRST_HOUR; i < LAST_HOUR; i++)
		cout << (bit_test(e->hour, i) ? '1': '0');
	cout << endl;
	cout << "Day:";
	for (i = FIRST_DOM; i < LAST_DOM; i++)
		cout << (bit_test(e->dom, i) ? '1': '0');
	cout << endl;
	cout << "Month:";
	for (i = FIRST_MONTH; i < LAST_MONTH; i++)
		cout << (bit_test(e->month, i) ? '1': '0');
	cout << endl;
	cout << "DOW:";
	for (i = FIRST_DOW; i < LAST_DOW; i++)
		cout << (bit_test(e->dow, i) ? '1': '0');
	cout << endl;
	ptime now(second_clock::local_time());
	tm tm_now = to_tm(now);
	tm_now.tm_mday;
	tm tm_then = to_tm(now);
	tm tm_start = to_tm(now);
	cout << now << endl;
	bool ok = false;
	//int i = 0;
	if ((e->flags & MON_STAR)) {
		tm_then.tm_mon = tm_now.tm_mon;
		ok = true;
	} else {
restartm:
		for (i = tm_now.tm_mon; i <= LAST_MONTH; i++) {
			if (bit_test(e->month, i)) {
				/* its ok */
				tm_then.tm_mon = tm_now.tm_mon;
				ok = true;
				break;
			}
		}
		if (ok == false) {
			tm_then.tm_year += 1;
			tm_now.tm_mon = FIRST_MONTH;
			tm_now.tm_mday = FIRST_DOM;
			tm_now.tm_hour = FIRST_HOUR;
			tm_now.tm_min = FIRST_MINUTE;
			if (tm_start.tm_year +1 < tm_now.tm_year) {
				ELOG("Didnt get a valid Month", NULL);
				return false;
			}
			goto restartm;
		}
	}
	ok = false;
	if ((e->flags & DOM_STAR)) {
		tm_then.tm_mday = tm_now.tm_mday;
		ok = true;
	} else {
		for (i = tm_now.tm_mday; i <= LAST_DOM; i++) {
			if (bit_test(e->dom, i)) {
				/* its ok */
				tm_then.tm_mday = i+1;
				ok = true;
				break;
			}
		}
		if (ok == false) {
			tm_now.tm_mon += 1;
			tm_now.tm_mday = FIRST_DOM;
			tm_now.tm_hour = FIRST_HOUR;
			tm_now.tm_min = FIRST_MINUTE;
			if (tm_start.tm_mon+12 < tm_now.tm_mon) {
				ELOG("Didn't get a valid Day of Month", NULL);
				return false;
			}
			goto restartm;
		}
	}
	ok = false;
	if ((e->flags &  HR_STAR)) {
		tm_then.tm_hour = tm_now.tm_hour;
		ok = true;
	} else {
		for (i = tm_now.tm_hour; i <= LAST_HOUR; i++) {
			if (bit_test(e->hour, i)) {
				/* its ok */
				tm_then.tm_hour = i;
				ok = true;
				break;
			}
		}
		if (ok == false) {
			tm_now.tm_mday += 1;
			tm_now.tm_hour = FIRST_HOUR;
			tm_now.tm_min = FIRST_MINUTE;
			if (tm_start.tm_hour + 24 < tm_now.tm_hour) {
				ELOG("Didn't get a valid Hour", NULL);
				return false;
			}
			goto restartm;
		}
	}
	ok = false;
	if ((e->flags & MIN_STAR)) {
		tm_then.tm_min = tm_now.tm_min;
		ok = true;
	} else {
		for (i = tm_now.tm_min; i <= LAST_MINUTE; i++) {
			if (bit_test(e->minute, i)) {
				/* its ok */
				tm_then.tm_min = i;
				ok = true;
				break;
			}
		}
		if (ok == false) {
			tm_now.tm_hour += 1;
			tm_now.tm_min = FIRST_MINUTE;
			if (tm_start.tm_min+ 60 < tm_now.tm_min) {
				ELOG("Didn't get a valid Minute", NULL);
				return false;
			}
			goto restartm;
		}
	}
	cout << ptime_from_tm(tm_then) << endl;
}


/* return NULL if eof or syntax error occurs;
 * otherwise return a pointer to a new entry.
 */
entry *crontabs::load_entry(FILE * file, void (*error_func) (), struct passwd *pw, char **envp, string fname) {
	/* this function reads one crontab entry -- the next -- from a file.
	 * it skips any leading blank lines, ignores comments, and returns
	 * NULL if for any reason the entry can't be read and parsed.
	 *
	 * the entry is also parsed here.
	 *
	 * syntax:
	 *   user crontab:
	 *  minutes hours doms months dows cmd\n
	 *   system crontab (/etc/crontab):
	 *  minutes hours doms months dows USERNAME cmd\n
	 */

	ecode_e ecode = e_none;
	entry *e;
	int ch;
	char cmd[MAX_COMMAND];
	char envstr[MAX_ENVSTR];
	char **tenvp;

	DLOG("load_entry()...about to eat comments in %s", fname.c_str());

	skip_comments(file);

	ch = get_char(file);
	if (ch == EOF)
		return (NULL);

	/* ch is now the first useful character of a useful line.
	 * it may be an @special or it may be the first character
	 * of a list of minutes.
	 */

	e = (entry *) calloc(sizeof (entry), sizeof (char));

	/* check for '-' as a first character, this option will disable
	* writing a syslog message about command getting executed
	*/
	if (ch == '-') {
	/* if we are editing system crontab or user uid is 0 (root)
	* we are allowed to disable logging
	*/
		if (pw == NULL || pw->pw_uid == 0)
			e->flags |= DONT_LOG;
		else {
			ELOG("Only Privileged Users can disable Logging in %s", fname.c_str());
			ecode = e_option;
			goto eof;
		}
		ch = get_char(file);
		if (ch == EOF)
			return NULL;
	}

	if (ch == '@') {
		/* all of these should be flagged and load-limited; i.e.,
		 * instead of @hourly meaning "0 * * * *" it should mean
		 * "close to the front of every hour but not 'til the
		 * system load is low".  Problems are: how do you know
		 * what "low" means? (save me from /etc/cron.conf!) and:
		 * how to guarantee low variance (how low is low?), which
		 * means how to we run roughly every hour -- seems like
		 * we need to keep a history or let the first hour set
		 * the schedule, which means we aren't load-limited
		 * anymore.  too much for my overloaded brain. (vix, jan90)
		 * HINT
		 */
		ch = get_string(cmd, MAX_COMMAND, file, " \t\n");
		if (!strcmp("reboot", cmd)) {
			e->flags |= WHEN_REBOOT;
		}
		else if (!strcmp("yearly", cmd) || !strcmp("annually", cmd)) {
			bit_set(e->minute, 0);
			bit_set(e->hour, 0);
			bit_set(e->dom, 0);
			bit_set(e->month, 0);
			bit_nset(e->dow, 0, (LAST_DOW - FIRST_DOW + 1));
			e->flags |= DOW_STAR;
		}
		else if (!strcmp("monthly", cmd)) {
			bit_set(e->minute, 0);
			bit_set(e->hour, 0);
			bit_set(e->dom, 0);
			bit_nset(e->month, 0, (LAST_MONTH - FIRST_MONTH + 1));
			bit_nset(e->dow, 0, (LAST_DOW - FIRST_DOW + 1));
			e->flags |= DOW_STAR;
		}
		else if (!strcmp("weekly", cmd)) {
			bit_set(e->minute, 0);
			bit_set(e->hour, 0);
			bit_nset(e->dom, 0, (LAST_DOM - FIRST_DOM + 1));
			bit_nset(e->month, 0, (LAST_MONTH - FIRST_MONTH + 1));
			bit_set(e->dow, 0);
			e->flags |= DOW_STAR;
		}
		else if (!strcmp("daily", cmd) || !strcmp("midnight", cmd)) {
			bit_set(e->minute, 0);
			bit_set(e->hour, 0);
			bit_nset(e->dom, 0, (LAST_DOM - FIRST_DOM + 1));
			bit_nset(e->month, 0, (LAST_MONTH - FIRST_MONTH + 1));
			bit_nset(e->dow, 0, (LAST_DOW - FIRST_DOW + 1));
		}
		else if (!strcmp("hourly", cmd)) {
			bit_set(e->minute, 0);
			bit_nset(e->hour, 0, (LAST_HOUR - FIRST_HOUR + 1));
			bit_nset(e->dom, 0, (LAST_DOM - FIRST_DOM + 1));
			bit_nset(e->month, 0, (LAST_MONTH - FIRST_MONTH + 1));
			bit_nset(e->dow, 0, (LAST_DOW - FIRST_DOW + 1));
			e->flags |= HR_STAR;
		}
		else {
			ecode = e_timespec;
			goto eof;
		}
		/* Advance past whitespace between shortcut and
		 * username/command.
		 */
		Skip_Blanks(ch, file);
		if (ch == EOF || ch == '\n') {
			ecode = e_cmd;
			goto eof;
		}
	}
	else {
		DLOG("load_entry()...about to parse numerics in %s", fname.c_str());

		if (ch == '*')
			e->flags |= MIN_STAR;
		ch = get_list(e->minute, FIRST_MINUTE, LAST_MINUTE, PPC_NULL, ch, file);
		if (ch == EOF) {
			ecode = e_minute;
			goto eof;
		}

		/* hours
		 */

		if (ch == '*')
			e->flags |= HR_STAR;
		ch = get_list(e->hour, FIRST_HOUR, LAST_HOUR, PPC_NULL, ch, file);
		if (ch == EOF) {
			ecode = e_hour;
			goto eof;
		}

		/* DOM (days of month)
		 */

		if (ch == '*')
			e->flags |= DOM_STAR;
		ch = get_list(e->dom, FIRST_DOM, LAST_DOM, PPC_NULL, ch, file);
		if (ch == EOF) {
			ecode = e_dom;
			goto eof;
		}

		/* month
		 */
		if (ch == '*')
			e->flags |= MON_STAR;
		ch = get_list(e->month, FIRST_MONTH, LAST_MONTH, MonthNames, ch, file);
		if (ch == EOF) {
			ecode = e_month;
			goto eof;
		}

		/* DOW (days of week)
		 */

		if (ch == '*')
			e->flags |= DOW_STAR;
		ch = get_list(e->dow, FIRST_DOW, LAST_DOW, DowNames, ch, file);
		if (ch == EOF) {
			ecode = e_dow;
			goto eof;
		}
	}

	/* make sundays equivalent */
	if (bit_test(e->dow, 0) || bit_test(e->dow, 7)) {
		bit_set(e->dow, 0);
		bit_set(e->dow, 7);
	}

	/* check for permature EOL and catch a common typo */
	if (ch == '\n' || ch == '*') {
		ecode = e_cmd;
		goto eof;
	}

	/* ch is the first character of a command, or a username */
	unget_char(ch, file);

	if (!pw) {
		char *username = cmd;	/* temp buffer */

		DLOG("load_entry()...about to parse username in %s", fname.c_str());
		ch = get_string(username, MAX_COMMAND, file, " \t\n");

		DLOG("load_entry()...got %s in %s", username, fname.c_str());
		if (ch == EOF || ch == '\n' || ch == '*') {
			ecode = e_cmd;
			goto eof;
		}

		pw = getpwnam(username);
		if (pw == NULL) {
			ecode = e_username;
			goto eof;
		}
		DLOG("load_entry()...uid %ld, gid %ld",(long) pw->pw_uid, (long) pw->pw_gid);
	}

	if ((e->pwd = pw_dup(pw)) == NULL) {
		ecode = e_memory;
		goto eof;
	}
	bzero(e->pwd->pw_passwd, strlen(e->pwd->pw_passwd));

	/* copy and fix up environment.  some variables are just defaults and
	 * others are overrides.
	 */
	if ((e->envp = env_copy(envp)) == NULL) {
		ecode = e_memory;
		goto eof;
	}
	if (!env_get("SHELL", e->envp)) {
		if (glue_strings(envstr, sizeof envstr, "SHELL", _PATH_BSHELL, '=')) {
			if ((tenvp = env_set(e->envp, envstr)) == NULL) {
				ecode = e_memory;
				goto eof;
			}
			e->envp = tenvp;
		}
		else
			ELOG("ERROR: can't set SHELL in %s", fname.c_str());
	}
	if (!env_get("HOME", e->envp)) {
		if (glue_strings(envstr, sizeof envstr, "HOME", pw->pw_dir, '=')) {
			if ((tenvp = env_set(e->envp, envstr)) == NULL) {
				ecode = e_memory;
				goto eof;
			}
			e->envp = tenvp;
		}
		else
			ELOG("ERROR: can't set HOME in %s", fname.c_str());
	}
#ifndef LOGIN_CAP
	/* If login.conf is in used we will get the default PATH later. */
	if (!env_get("PATH", e->envp)) {
		if (glue_strings(envstr, sizeof envstr, "PATH", _PATH_DEFPATH, '=')) {
			if ((tenvp = env_set(e->envp, envstr)) == NULL) {
				ecode = e_memory;
				goto eof;
			}
			e->envp = tenvp;
		}
		else
			ELOG("ERROR can't set PATH in %s", fname.c_str());
	}
#endif /* LOGIN_CAP */
	if (glue_strings(envstr, sizeof envstr, "LOGNAME", pw->pw_name, '=')) {
		if ((tenvp = env_set(e->envp, envstr)) == NULL) {
			ecode = e_memory;
			goto eof;
		}
		e->envp = tenvp;
	}
	else
		ELOG("ERROR can't set LOGNAME in %s", fname.c_str());
#if defined(BSD) || defined(__linux)
	if (glue_strings(envstr, sizeof envstr, "USER", pw->pw_name, '=')) {
		if ((tenvp = env_set(e->envp, envstr)) == NULL) {
			ecode = e_memory;
			goto eof;
		}
		e->envp = tenvp;
	}
	else
		ELOG("ERROR can't set USER in %s", fname.c_str());
#endif

	DLOG("load_entry()...about to parse command in %s", fname.c_str());

	/* Everything up to the next \n or EOF is part of the command...
	 * too bad we don't know in advance how long it will be, since we
	 * need to malloc a string for it... so, we limit it to MAX_COMMAND.
	 */
	ch = get_string(cmd, MAX_COMMAND, file, "\n");

	/* a file without a \n before the EOF is rude, so we'll complain...
	 */
	if (ch == EOF) {
		ecode = e_cmd;
		goto eof;
	}

	/* got the command in the 'cmd' string; save it in *e.
	 */
	if ((e->cmd = strdup(cmd)) == NULL) {
		ecode = e_memory;
		goto eof;
	}

	DLOG("load_entry()...returning successfully from parsing %s", fname.c_str());

		/* success, fini, return pointer to the entry we just created...
		 */
		return (e);

  eof:
	if (e->envp)
		env_free(e->envp);
	if (e->pwd)
		free(e->pwd);
	if (e->cmd)
		free(e->cmd);
	free(e);
	while (ch != '\n' && !feof(file))
		ch = get_char(file);
	if (ecode != e_none && error_func)
		//(*error_func) (ecodes[(int) ecode]);
		ELOG("Error: %s in %s", ecodes[(int) ecode], fname.c_str());
	return (NULL);
}

int crontabs::get_list(bitstr_t * bits, int low, int high, const char *names[], int ch, FILE * file) {
	int done;

	/* we know that we point to a non-blank character here;
	 * must do a Skip_Blanks before we exit, so that the
	 * next call (or the code that picks up the cmd) can
	 * assume the same thing.
	 */

	DLOG("get_list()...entered", NULL);

	/* list = range {"," range}
	 */
	/* clear the bit string, since the default is 'off'.
	 */
	bit_nclear(bits, 0, (high - low + 1));

	/* process all ranges
	 */
	done = FALSE;
	while (!done) {
		if (EOF == (ch = get_range(bits, low, high, names, ch, file)))
			return (EOF);
		if (ch == ',')
			ch = get_char(file);
		else
			done = TRUE;
	}

	/* exiting.  skip to some blanks, then skip over the blanks.
	 */
	Skip_Nonblanks(ch, file)
	Skip_Blanks(ch, file)

	DLOG("get_list()...exiting w/ %02x", ch);

	return (ch);
}

int crontabs::get_range(bitstr_t * bits, int low, int high, const char *names[], int ch, FILE * file) {
	/* range = number | number "-" number [ "/" number ]
	 */

	int i, num1, num2, num3;

	DLOG("get_range()...entering, exit won't show", NULL);

	if (ch == '*') {
		/* '*' means "first-last" but can still be modified by /step
		 */
		num1 = low;
		num2 = high;
		ch = get_char(file);
		if (ch == EOF)
			return (EOF);
	}
	else {
		ch = get_number(&num1, low, names, ch, file, ",- \t\n");
		if (ch == EOF)
			return (EOF);

		if (ch != '-') {
			/* not a range, it's a single number.
			 */
			if (EOF == set_element(bits, low, high, num1)) {
				unget_char(ch, file);
				return (EOF);
			}
			return (ch);
		}
		else {
			/* eat the dash
			 */
			ch = get_char(file);
			if (ch == EOF)
				return (EOF);

			/* get the number following the dash
			 */
			ch = get_number(&num2, low, names, ch, file, "/, \t\n");
			if (ch == EOF || num1 > num2)
				return (EOF);
		}
	}

	/* check for step size
	 */
	if (ch == '/') {
		/* eat the slash
		 */
		ch = get_char(file);
		if (ch == EOF)
			return (EOF);

		/* get the step size -- note: we don't pass the
		 * names here, because the number is not an
		 * element id, it's a step size.  'low' is
		 * sent as a 0 since there is no offset either.
		 */
		ch = get_number(&num3, 0, PPC_NULL, ch, file, ", \t\n");
		if (ch == EOF || num3 == 0)
			return (EOF);
	}
	else {
		/* no step.  default==1.
		 */
		num3 = 1;
	}

	/* range. set all elements from num1 to num2, stepping
	 * by num3.  (the step is a downward-compatible extension
	 * proposed conceptually by bob@acornrc, syntactically
	 * designed then implemented by paul vixie).
	 */
	for (i = num1; i <= num2; i += num3)
		if (EOF == set_element(bits, low, high, i)) {
			unget_char(ch, file);
			return (EOF);
		}

	return (ch);
}

int crontabs::get_number(int *numptr, int low, const char *names[], int ch, FILE * file, const char *terms) {
	char temp[MAX_TEMPSTR], *pc;
	int len, i;

	pc = temp;
	len = 0;

	/* first look for a number */
	while (isdigit((unsigned char) ch)) {
		if (++len >= MAX_TEMPSTR)
			goto bad;
		*pc++ = ch;
		ch = get_char(file);
	}
	*pc = '\0';
	if (len != 0) {
		/* got a number, check for valid terminator */
		if (!strchr(terms, ch))
			goto bad;
		*numptr = atoi(temp);
		return (ch);
	}

	/* no numbers, look for a string if we have any */
	if (names) {
		while (isalpha((unsigned char) ch)) {
			if (++len >= MAX_TEMPSTR)
				goto bad;
			*pc++ = ch;
			ch = get_char(file);
		}
		*pc = '\0';
		if (len != 0 && strchr(terms, ch)) {
			for (i = 0; names[i] != NULL; i++) {
				DLOG("get_num, compare(%s,%s)", names[i], temp);
				if (!strcasecmp(names[i], temp)) {
					*numptr = i + low;
					return (ch);
				}
			}
		}
	}

  bad:
	unget_char(ch, file);
	return (EOF);
}

int crontabs::set_element(bitstr_t * bits, int low, int high, int number) {
	DLOG("set_element(?,%d,%d,%d)", low, high, number);

	if (number < low || number > high)
		return (EOF);

	bit_set(bits, (number - low));
	return (OK);
}


/* get_char(file) : like getc() but increment LineNumber on newlines
 */
int crontabs::get_char(FILE * file) {
	int ch;

	ch = getc(file);
	if (ch == '\n')
		Set_LineNum(LineNumber + 1)
	return (ch);
}

/* unget_char(ch, file) : like ungetc but do LineNumber processing
 */
void crontabs::unget_char(int ch, FILE * file) {
	ungetc(ch, file);
	if (ch == '\n')
		Set_LineNum(LineNumber - 1)
}

/* get_string(str, max, file, termstr) : like fgets() but
 *      (1) has terminator string which should include \n
 *      (2) will always leave room for the null
 *      (3) uses get_char() so LineNumber will be accurate
 *      (4) returns EOF or terminating character, whichever
 */
int crontabs::get_string(char *string, int size, FILE * file, char *terms) {
	int ch;

	while (EOF != (ch = get_char(file)) && !strchr(terms, ch)) {
		if (size > 1) {
			*string++ = (char) ch;
			size--;
		}
	}

	if (size > 0)
		*string = '\0';

	return (ch);
}

/* skip_comments(file) : read past comment (if any)
 */
void crontabs::skip_comments(FILE * file) {
	int ch;

	while (EOF != (ch = get_char(file))) {
		/* ch is now the first character of a line.
		 */
		while (ch == ' ' || ch == '\t')
			ch = get_char(file);

		if (ch == EOF)
			break;

		/* ch is now the first non-blank character of a line.
		 */

		if (ch != '\n' && ch != '#')
			break;

		/* ch must be a newline or comment as first non-blank
		 * character on a line.
		 */

		while (ch != '\n' && ch != EOF)
			ch = get_char(file);

		/* ch is now the newline of a line which we're going to
		 * ignore.
		 */
	}
	if (ch != EOF)
		unget_char(ch, file);
}

struct passwd *crontabs::pw_dup(const struct passwd *pw) {
	char		*cp;
	size_t		 nsize=0, psize=0, gsize=0, dsize=0, ssize=0, total=0;
	struct passwd	*newpw;

	/* Allocate in one big chunk for easy freeing */
	total = sizeof(struct passwd);
	if (pw->pw_name) {
		nsize = strlen(pw->pw_name) + 1;
		total += nsize;
	}
	if (pw->pw_passwd) {
		psize = strlen(pw->pw_passwd) + 1;
		total += psize;
	}
#ifdef LOGIN_CAP
	if (pw->pw_class) {
		csize = strlen(pw->pw_class) + 1;
		total += csize;
	}
#endif /* LOGIN_CAP */
	if (pw->pw_gecos) {
		gsize = strlen(pw->pw_gecos) + 1;
		total += gsize;
	}
	if (pw->pw_dir) {
		dsize = strlen(pw->pw_dir) + 1;
		total += dsize;
	}
	if (pw->pw_shell) {
		ssize = strlen(pw->pw_shell) + 1;
		total += ssize;
	}
	if ((cp = (char *)malloc(total)) == NULL)
		return (NULL);
	newpw = (struct passwd *)cp;

	/*
	 * Copy in passwd contents and make strings relative to space
	 * at the end of the buffer.
	 */
	(void)memcpy(newpw, pw, sizeof(struct passwd));
	cp += sizeof(struct passwd);
	if (pw->pw_name) {
		(void)memcpy(cp, pw->pw_name, nsize);
		newpw->pw_name = cp;
		cp += nsize;
	}
	if (pw->pw_passwd) {
		(void)memcpy(cp, pw->pw_passwd, psize);
		newpw->pw_passwd = cp;
		cp += psize;
	}
#ifdef LOGIN_CAP
	if (pw->pw_class) {
		(void)memcpy(cp, pw->pw_class, csize);
		newpw->pw_class = cp;
		cp += csize;
	}
#endif /* LOGIN_CAP */
	if (pw->pw_gecos) {
		(void)memcpy(cp, pw->pw_gecos, gsize);
		newpw->pw_gecos = cp;
		cp += gsize;
	}
	if (pw->pw_dir) {
		(void)memcpy(cp, pw->pw_dir, dsize);
		newpw->pw_dir = cp;
		cp += dsize;
	}
	if (pw->pw_shell) {
		(void)memcpy(cp, pw->pw_shell, ssize);
		newpw->pw_shell = cp;
		cp += ssize;
	}

	return (newpw);
}

char **crontabs::env_init(void) {
	char **p = (char **) malloc(sizeof (char **));

	if (p != NULL)
		p[0] = NULL;
	return (p);
}

void crontabs::env_free(char **envp) {
	char **p;

	for (p = envp; *p != NULL; p++)
		free(*p);
	free(envp);
}

char **crontabs::env_copy(char **envp) {
	int count, i, save_errno;
	char **p;

	for (count = 0; envp[count] != NULL; count++) ;

	p = (char **) malloc((count + 1) * sizeof (char *));	/* 1 for the NULL */
	if (p != NULL) {
		for (i = 0; i < count; i++)
			if ((p[i] = strdup(envp[i])) == NULL) {
				save_errno = errno;
				while (--i >= 0)
					free(p[i]);
				free(p);
				errno = save_errno;
				return (NULL);
			}
		p[count] = NULL;
	}
	return (p);
}

char **crontabs::env_set(char **envp, char *envstr) {
	int count, found;
	char **p, *envtmp;

	/*
	 * count the number of elements, including the null pointer;
	 * also set 'found' to -1 or index of entry if already in here.
	 */
	found = -1;
	for (count = 0; envp[count] != NULL; count++) {
		if (!strcmp_until(envp[count], envstr, '='))
			found = count;
	}
	count++;	/* for the NULL */

	if (found != -1) {
		/*
		 * it exists already, so just free the existing setting,
		 * save our new one there, and return the existing array.
		 */
		if ((envtmp = strdup(envstr)) == NULL)
			return (NULL);
		free(envp[found]);
		envp[found] = envtmp;
		return (envp);
	}

	/*
	 * it doesn't exist yet, so resize the array, move null pointer over
	 * one, save our string over the old null pointer, and return resized
	 * array.
	 */
	if ((envtmp = strdup(envstr)) == NULL)
		return (NULL);
	p = (char **) realloc((void *) envp,
		(size_t) ((count + 1) * sizeof (char **)));
	if (p == NULL) {
		free(envtmp);
		return (NULL);
	}
	p[count] = p[count - 1];
	p[count - 1] = envtmp;
	return (p);
}
char *crontabs::env_get(char *name, char **envp) {
	int len = strlen(name);
	char *p, *q;

	while ((p = *envp++) != NULL) {
		if (!(q = strchr(p, '=')))
			continue;
		if ((q - p) == len && !strncmp(p, name, len))
			return (q + 1);
	}
	return (NULL);
}

/*
 * glue_strings is the overflow-safe equivalent of
 *		sprintf(buffer, "%s%c%s", a, separator, b);
 *
 * returns 1 on success, 0 on failure.  'buffer' MUST NOT be used if
 * glue_strings fails.
 */
int crontabs::glue_strings(char *buffer, size_t buffer_size, const char *a, const char *b, char separator) {
	char *buf;
	char *buf_end;

	if (buffer_size <= 0)
		return (0);
	buf_end = buffer + buffer_size;
	buf = buffer;

	for ( /* nothing */ ; buf < buf_end && *a != '\0'; buf++, a++)
		*buf = *a;
	if (buf == buf_end)
		return (0);
	if (separator != '/' || buf == buffer || buf[-1] != '/')
		*buf++ = separator;
	if (buf == buf_end)
		return (0);
	for ( /* nothing */ ; buf < buf_end && *b != '\0'; buf++, b++)
		*buf = *b;
	if (buf == buf_end)
		return (0);
	*buf = '\0';
	return (1);
}

int crontabs::strcmp_until(const char *left, const char *right, char until) {
	while (*left && *left != until && *left == *right) {
		left++;
		right++;
	}

	if ((*left == '\0' || *left == until) && (*right == '\0' ||
			*right == until)) {
		return (0);
	}
	return (*left - *right);
}
