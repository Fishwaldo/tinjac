/* Tinjac - log.cpp
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

/** @file log.cpp 
 *  @brief 
 */ 

#include <stdarg.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "log.hpp"



Log::Log(char* filename) {
  m_stream.open(filename);
}

Log::Log() {
}

void Log::Write(char *file, int LineNo, char* logline){
	cout << "test" << endl;
	if (m_stream.is_open())
		m_stream << file<<":"<<LineNo<< ": " << logline << endl;
	else
		clog << file<<":"<<LineNo<<": " << logline << endl;
}

void Log::Write(char *file, int LineNo, const char* logline, ...){
	va_list argList;
	char cbuffer[1024];
	va_start(argList, logline);
	vsnprintf(cbuffer, 1024, logline, argList);
	va_end(argList);
	if (m_stream.is_open())
		m_stream << file<<":"<<LineNo<< ": " << cbuffer << endl;
	else
		clog << file<<":"<<LineNo<<": " << cbuffer << endl;


}

Log::~Log(){
  m_stream.close();
}


