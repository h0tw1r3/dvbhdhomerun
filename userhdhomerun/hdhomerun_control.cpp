/*
 * hdhomerun_control.cpp, skeleton driver for the HDHomeRun devices
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
#include "hdhomerun_tuner.h"
#include "log_file.h"

#include "../kernel/dvb_hdhomerun_control_messages.h"

#include <sys/ioctl.h>

#include <iostream>

using namespace std;

Control::Control(HdhomerunController* _hdhomerun) 
: m_device_name("/dev/hdhomerun_control"), 
  m_hdhomerun(_hdhomerun), m_fdIoctl(0)
{
  m_write.open(m_device_name.c_str(), ios::binary);
  if(!m_write) {
     ERR() << "Couldn't open: " << m_device_name << endl;
    _exit(-1);
  }
}

Control::~Control()
{
  m_write.close();
}

void Control::run()
{
   m_read.open(m_device_name.c_str(), ios::binary);
   if(!m_read) {
      ERR() << "Couldn't open: " << m_device_name << endl;
      _exit(-1);
   }
      
   while(!m_stop) {
      while(!m_read.eof()) {
         struct dvbhdhomerun_control_mesg mesg;
      
         m_read.read( (char*)&mesg, sizeof(dvbhdhomerun_control_mesg));
         if(m_read.fail() && !m_read.eof()) {
            ERR() << "Error FAIL reading data from " << m_device_name << endl;
         }
         if(m_read.bad()) {
            ERR() << "Error BAD reading data from " << m_device_name << endl;
         }
      
         if(m_read.gcount() > 0) {
            //LOG() << "Read " << m_read.gcount() << " bytes, expected " << sizeof(dvbhdhomerun_control_mesg) << endl;
            if(m_read.gcount() < sizeof(dvbhdhomerun_control_mesg)) {
               ERR() << "FAIL! We didn't receive enough bytes from the device driver!" << endl;
               _exit(-1);
            }
	
            m_messages.push(mesg);
         }
      }      
    
      if(m_messages.size() > 0) {
         this->ProcessMessages();
      }
    
      m_read.clear();

      //this->sleep(1);
      usleep(10000);
   }

   m_read.close();
}


void Control::WriteToDevice(const struct dvbhdhomerun_control_mesg& _mesg)
{
  m_write.write( (char*)&_mesg, sizeof(dvbhdhomerun_control_mesg));
  if(m_write.fail() && !m_write.eof()) {
    ERR() << "Error FAIL writing data to " << m_device_name << endl;
  }
  if(m_write.bad()) {
    ERR() << "Error BAD writing data to " << m_device_name << endl;
  }
  
  m_write.flush();
}


bool Control::Ioctl(int _numOfTuners, const std::string& _name, int& _id, int _type) 
{
  m_fdIoctl = open(m_device_name.c_str(), O_RDONLY);
  if(!m_fdIoctl) {
    ERR() << "Couldn't open: " << m_device_name << " for IOCTL" << endl;
    _exit(-1);
  }

  struct hdhomerun_register_tuner_data tuner_data;
  tuner_data.num_of_devices = _numOfTuners;
  strncpy(tuner_data.name, _name.c_str(), 11);
  tuner_data.name[10] = 0;
  tuner_data.type = _type;
  int ret = ioctl(m_fdIoctl, HDHOMERUN_REGISTER_TUNER, &tuner_data);
  if(ret != 0) {
    ERR() << "Couldn't create tuner! ioctl failed. This means the kernel module is either not loaded or has malfunctioned. Check dmesg." << endl;
    return false;
  }
  else {
    LOG() << "Registered tuner, id from kernel: " << tuner_data.id << " name: " << tuner_data.name << endl;
  }  
  _id = tuner_data.id;

  return true;
}


void Control::ProcessMessages()
{
  //LOG() << "Processing " << m_messages.size() << " messages." << endl;

  while(!m_messages.empty()) {
    struct dvbhdhomerun_control_mesg mesg = m_messages.front();

    switch (mesg.type) {
    case DVB_HDHOMERUN_FE_SET_FRONTEND: {
      FE_SET_Frontend(mesg);
      break;
    }

    case DVB_HDHOMERUN_FE_READ_STATUS: {
      FE_READ_Status(mesg);
      break;
    }

    case DVB_HDHOMERUN_FE_READ_SIGNAL_STRENGTH: {
      FE_READ_SIGNAL_Strength(mesg);
      break;
    }

    case DVB_HDHOMERUN_START_FEED: {
      StartFeed(mesg);
      break;
    }

    case DVB_HDHOMERUN_STOP_FEED: {
      StopFeed(mesg);
      break;
    }

    default:
      ERR() << "Unknown message from device driver! " << mesg.type << endl;
      break;
    }

    m_messages.pop();
  }
}

void Control::FE_SET_Frontend(const struct dvbhdhomerun_control_mesg& _mesg)
{
  dvb_frontend_parameters frontend_param = _mesg.u.frontend_parameters;
  printf("FE_SET_FRONTEND, freq: %d, inv: %d, symb rate: %d, fec: %d, mod: %d\n",
	 frontend_param.frequency,
	 frontend_param.inversion,
	 frontend_param.u.qam.symbol_rate,
	 frontend_param.u.qam.fec_inner,    
	 frontend_param.u.qam.modulation);    
  
  // Need to send stuff to HDHOMERUN
  HdhomerunTuner* tuner = m_hdhomerun->GetTuner(_mesg.id);
  int ret = tuner->Tune(frontend_param.frequency);

  this->WriteToDevice(_mesg);
}


void Control::FE_READ_Status(struct dvbhdhomerun_control_mesg& _mesg)
{
  LOG() << "FE_READ_STATUS" << endl;

  HdhomerunTuner* tuner = m_hdhomerun->GetTuner(_mesg.id);
  fe_status_t status = (fe_status_t)tuner->ReadStatus();
  
  _mesg.u.fe_status = status;

  this->WriteToDevice(_mesg);
}

void Control::FE_READ_SIGNAL_Strength(struct dvbhdhomerun_control_mesg& _mesg)
{
  LOG() << "FE_READ_SIGNAL_STRENGTH" << endl;

  HdhomerunTuner* tuner = m_hdhomerun->GetTuner(_mesg.id);
  uint16_t strength = tuner->ReadSignalStrength();
  
  _mesg.u.signal_strength = strength;

  this->WriteToDevice(_mesg);
}


void Control::DMX_SET_PES_Filter(const struct dvbhdhomerun_control_mesg& _mesg)
{
  struct dmx_pes_filter_params pes_filter = _mesg.u.dmx_pes_filter;

  printf("DMX_SET_PES_FILTER: pid %x, input %d, output %d, type %d, flags %x\n",
	 pes_filter.pid,
	 pes_filter.input,
	 pes_filter.output,
	 pes_filter.pes_type,
	 pes_filter.flags);

  // flags 1 = DMX_CHECK_CRC, 2 = DMX_ONESHOT, 4 = DMX_IMMEDIATE_START

  HdhomerunTuner* tuner = m_hdhomerun->GetTuner(_mesg.id);
  tuner->SetPesFilter(pes_filter.pid, pes_filter.output, pes_filter.pes_type);
  
  this->WriteToDevice(_mesg);
}

void Control::StartFeed(const struct dvbhdhomerun_control_mesg& _mesg)
{
  struct hdhomerun_dvb_demux_feed feed = _mesg.u.demux_feed;

  LOG() << "START FEED: Pid = " << hex << feed.pid <<  endl;

  HdhomerunTuner* tuner = m_hdhomerun->GetTuner(_mesg.id);
  tuner->StartStreaming(feed.pid);

  this->WriteToDevice(_mesg);
}

void Control::StopFeed(const struct dvbhdhomerun_control_mesg& _mesg)
{
  struct hdhomerun_dvb_demux_feed feed = _mesg.u.demux_feed;

  LOG() << "STOP FEED: Pid = " << hex << feed.pid <<  endl;

  HdhomerunTuner* tuner = m_hdhomerun->GetTuner(_mesg.id);
  tuner->StopStreaming(feed.pid);

  this->WriteToDevice(_mesg);
}
