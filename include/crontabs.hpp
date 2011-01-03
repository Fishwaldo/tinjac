/* Tinjac - crontabs.hpp
** Copyright (c) 2010 Justin Hammond
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

/** @file crontabs.hpp 
 *  @brief 
 */ 


#ifndef CRONTABS_HPP_
#define CRONTABS_HPP_

#include "config.h"

#include <cassert>
#include <cstdio>    // EOF.
#include <iostream>  // cin, cout.
#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/detail/ios.hpp>  // BOOST_IOS.
#include <boost/iostreams/filter/stdio.hpp>
#include <boost/iostreams/operations.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/filesystem.hpp>

#include <pwd.h>

#include "bitstring.h"
#include "macros.h"


using namespace std;
using namespace boost::filesystem;
using namespace boost::iostreams;



typedef	struct _entry {
	//struct _entry	*next;
	struct passwd	*pwd;
	char		**envp;
	char		*cmd;
	bitstr_t	bit_decl(minute, MINUTE_COUNT);
	bitstr_t	bit_decl(hour,   HOUR_COUNT);
	bitstr_t	bit_decl(dom,    DOM_COUNT);
	bitstr_t	bit_decl(month,  MONTH_COUNT);
	bitstr_t	bit_decl(dow,    DOW_COUNT);
	int		flags;
#define	MIN_STAR	0x01
#define	HR_STAR		0x02
#define	DOM_STAR	0x04
#define	DOW_STAR	0x08
#define	WHEN_REBOOT	0x10
#define	DONT_LOG	0x20
#define MON_STAR	0x40
} entry;




class crontabs {
public:
	crontabs ();
	crontabs (path dbdir, bool system);
	~crontabs();
	bool setPath(path dbdir, bool system);
	bool parseCrontab(string fname, bool system);
	bool printTime(entry *);
private:
	entry *load_entry(FILE * file, void (*error_func) (), struct passwd *pw, char **envp, string fname);
	int get_list(bitstr_t * bits, int low, int high, const char *names[], int ch, FILE * file);
	int get_range(bitstr_t * bits, int low, int high, const char *names[], int ch, FILE * file);
	int get_number(int *numptr, int low, const char *names[], int ch, FILE * file, const char *terms);
	int set_element(bitstr_t * bits, int low, int high, int number);
	void skip_comments(FILE * file);
	int get_string(char *string, int size, FILE * file, char *terms);
	void unget_char(int ch, FILE * file);
	int get_char(FILE * file);
	struct passwd *pw_dup(const struct passwd *pw);
	char **env_set(char **envp, char *envstr);
	char **env_copy(char **envp);
	void env_free(char **envp);
	char **env_init(void);
	char *env_get(char *name, char **envp);
	int glue_strings(char *buffer, size_t buffer_size, const char *a, const char *b, char separator);
	int strcmp_until(const char *left, const char *right, char until);
	path crontabdir;
	bool SystemDir;
	int LineNumber;
};



#endif /* CRONTABS_HPP_ */
