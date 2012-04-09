/*
 * main.c, skeleton driver for the HDHomeRun devices
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

#include "hdhomerun_control.h"
#include "hdhomerun_controller.h"
#include "log_file.h"
#include "../kernel/dvb_hdhomerun_control_messages.h"

#include <fstream>
#include <iostream>

#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>

using namespace std;

static void usage(const char* argv0)
{
   printf("usage: %s [options]\n", argv0);
   printf("\n");
   printf(" -f run as a daemon\n");
   printf(" -u <username>   Run as user <username>. Requires -f\n");
   printf(" -g <groupname>  Run as group <groupname>. Requires -f\n");
   printf(" -l <log file>   Default is /var/log/dvbhdhomerun.log. Requires -f\n");
   printf("\n");
   exit(0);
}

bool g_stop = false;

static void sigexit(int)
{
   LOG() << "Exiting" << endl;
   g_stop = true;
}

static void handle_sigpipe(int x)
{
   return;
}


int main(int argc, char** argv)
{
   //
   // Parse cmd line options
   //
   const char* userName = NULL;
   const char* groupName = NULL;
   std::string logFileName;
   bool forkChild = false;
   int c;
   while((c = getopt(argc, argv, "u:g:fl:")) != -1)
   {
      switch(c)
      {
      case 'u':
         userName = optarg;
         break;
      case 'g':
         groupName = optarg;
         break;
      case 'f':
         forkChild = true;
         break;
      case 'l':
         logFileName = optarg;
         break;
      default:
         usage(argv[0]);
         break;
      }
   }

   if(forkChild) {
      // Setup logging
      if(logFileName.empty()) {
         logFileName = "/var/log/dvbhdhomerun.log";
      }
      if(!logFile.SetAndOpenLogFile(logFileName)) {
         cerr << "Couldn't open log file: " << logFileName << endl;
         exit(2);
      }
      logFile.SetLogType(LogFile::FILE);

      // "Daemonize" the app and run.
      if(daemon(0,1)) {
        exit(2);
      }

      umask(0);

      // /* Close the standard file descriptors */
      close(STDIN_FILENO);
      close(STDOUT_FILENO);
      close(STDERR_FILENO);
        
      struct group* group = getgrnam(groupName ?: "video");
      struct passwd* passwd = userName ? getpwnam(userName) : NULL;
      
      if(group != NULL) {
         setgid(group->gr_gid);
         LOG() << "Running as group: " << group->gr_name << endl;
      }
      // else {
      //    setgid(1);
      // }
      
      if(passwd != NULL) {
         setuid(passwd->pw_uid);
         LOG() << "Running as user: " << passwd->pw_name << endl;
      }
      // else {
      //    setuid(1);
      // }
      
      // if(passwd != NULL) {
      //    const char* homedir = passwd->pw_dir;
      //    setenv("HOME", homedir, 1);
      // }
   }
   else {
      if(logFileName.empty()) {
         logFile.SetLogType(LogFile::COUT);
      }
      else {
         if(!logFile.SetAndOpenLogFile(logFileName)) {
            cerr << "Couldn't open log file: " << logFileName << endl;
            exit(2);
         }
         logFile.SetLogType(LogFile::FILE);
      }
   }

   // 
   // We are good to go - connect to HDHomeRun's and kernel driver.
   //
   HdhomerunController hdhomerun(4);

   //
   // Wait for signals to stop
   //
   signal(SIGPIPE, handle_sigpipe);
   signal(SIGINT, sigexit);
   signal(SIGTERM, sigexit);
   while(!g_stop) {
      usleep(10000000);
   }

   return 0;
}



