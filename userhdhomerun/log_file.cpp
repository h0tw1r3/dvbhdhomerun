/*
 * log_file.cpp, logging to file/stdout/syslog/etc
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

#include "log_file.h"

#include <assert.h>
#include <iostream>

using namespace std;

LogFile logFile;

LogFile::LogFile()
   : ostream(this), m_logTo(LogFile::NONE)
{
}

LogFile::~LogFile()
{
   m_logFile.close();
}

void LogFile::SetLogType(LogFile::LogType _type)
{
   m_logTo = static_cast<LogFile::LogType>(m_logTo | _type);
}

bool LogFile::SetAndOpenLogFile(const std::string& _fileName)
{
   bool ret = false;
   m_logFile.open(_fileName.c_str(), ios::out | ios::app);
   if(m_logFile.is_open())
   {
      ret = true;
   }
   return ret;
}

int LogFile::overflow(int _i)
{
   m_buffer += _i;
   
   if(_i == '\n')  {
      assert(m_logTo != LogFile::NONE);
   
      if (m_logTo & LogFile::COUT) {
         cout << m_buffer;
      }

      if (m_logTo & LogFile::FILE) {
         if(m_logFile.is_open()) {
            m_logFile.write(m_buffer.c_str(), m_buffer.size());
            m_logFile.flush();
         }
      }
      
      m_buffer.clear();
   }

   return _i;
}

int LogFile::underflow()
{
   cout << "underflow" << endl;
   assert(false); // Do we ever end up here?
   return -1;
}

int LogFile::sync()
{
   return 0;
}
