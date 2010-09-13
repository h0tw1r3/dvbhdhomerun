/*
 * hdhomerun_control.h, skeleton driver for the HDHomeRun devices
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

#ifndef _hdhomerun_control_h_
#define _hdhomerun_control_h_

#include <QThread>

#include <fstream>
#include <queue>
#include <string>

class HdhomerunController;

class Control : public QThread
{
 public:
  Control(HdhomerunController* _hdhomerun);
  ~Control();
  
  void run();

  bool Ioctl(int _numOfTuners, const std::string& _name, int& _id, int type);

 private:
  void ProcessMessages();

  // Forwarded IOCTL's from the device driver.
  void FE_SET_Frontend(const struct dvbhdhomerun_control_mesg& _mesg);

  void FE_READ_Status(struct dvbhdhomerun_control_mesg& _mesg);

  void FE_READ_SIGNAL_Strength(struct dvbhdhomerun_control_mesg& _mesg);

  void DMX_SET_PES_Filter(const struct dvbhdhomerun_control_mesg& _mesg);

  void StartFeed(const struct dvbhdhomerun_control_mesg& _mesg);
  void StopFeed(const struct dvbhdhomerun_control_mesg& _mesg);

  void WriteToDevice(const struct dvbhdhomerun_control_mesg& mesg);

 private:
  std::ifstream m_read;
  std::ofstream m_write;
  int m_fdIoctl;

  std::queue<dvbhdhomerun_control_mesg> m_messages;

  bool m_stop;

  std::string m_device_name;

  HdhomerunController* m_hdhomerun;
};

#endif // _hdhomerun_control_h_
