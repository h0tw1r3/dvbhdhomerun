/*
 * log_file.h, logging to file/stdout/syslog/etc
 *
 * Copyright (C) 2010 Villy Thomsen <tfylliv@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef _log_file_h
#define _log_file_h

#include <pthread.h>

#include <fstream>
#include <ostream>
#include <string>

#define LOG() logFile
#define ERR() logFile

class LogFile : public std::streambuf, public std::ostream
{
public:
   enum LogType
   {
      NONE = 0x0,
      COUT = 0x1,
      FILE = 0x2
   };
   
public:
   LogFile();
   ~LogFile();

   void SetLogType(LogFile::LogType _type);
   bool SetAndOpenLogFile(const std::string& _fileName);
   
protected:
   int overflow(int _i);
   int underflow();
   int sync();

private:
   std::string m_buffer;

   LogType m_logTo;
   std::string m_logFileName;
   std::ofstream m_logFile;

   pthread_mutex_t m_mutexLogFile;
};

extern LogFile logFile;

#endif // _log_file_h
