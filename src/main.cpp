/* Tinjac - main.cpp
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

/** @file main.cpp
 *  @brief Main Entrypoint into tinjac.
 */

#include <stdio.h>
#include <iostream>
#include <fstream>
#include "log.hpp"
#include "crontabs.hpp"

using namespace std;

Log *logFacility;

int main(int argc, char *argv[]) {
	logFacility = new Log();
	try {
    	crontabs *ct = new crontabs("/etc/cron.d", true);
    } catch(std::exception &e) {
    	cerr << e.what() << "\n";
    }
    return 1;
}
