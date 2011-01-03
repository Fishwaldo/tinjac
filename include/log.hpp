/* Tinjac - log.hpp
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

/** @file log.hpp 
 *  @brief 
 */ 




#ifndef LOG_HPP_
#define LOG_HPP_

#include <fstream>

using namespace std;

#define DLOG(z, ...) logFacility->Write(__FILE__, __LINE__, z, __VA_ARGS__)
#define ELOG(z, ...) logFacility->Write(__FILE__, __LINE__, z, __VA_ARGS__)

class Log {
  public:
    Log(char* filename);
    Log();
    ~Log();
    void Write(char *file, int LineNo, char* logline);
    void Write(char *file, int LineNo, const char* logline, ...);
  private:
    ofstream m_stream;
};

extern Log *logFacility;

#endif /* LOG_HPP_ */
