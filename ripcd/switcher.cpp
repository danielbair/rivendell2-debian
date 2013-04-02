// switcher.cpp
//
// Abstract base class for Rivendell Switcher/GPIO drivers.
//
//   (C) Copyright 2002-2007,2010 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: switcher.cpp,v 1.1.8.2 2013/03/03 22:58:22 cvs Exp $
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License version 2 as
//   published by the Free Software Foundation.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public
//   License along with this program; if not, write to the Free Software
//   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#include <syslog.h>

#include <switcher.h>

#include <globals.h>

Switcher::Switcher(RDMatrix *matrix,QObject *parent,const char *name)
  : QObject(parent,name)
{
}


Switcher::~Switcher()
{
}


void Switcher::sendGpi()
{
}


void Switcher::sendGpo()
{
}


void Switcher::executeMacroCart(unsigned cartnum)
{
  RDMacro rml;
  rml.setRole(RDMacro::Cmd);
  rml.setCommand(RDMacro::EX);
  rml.setAddress(rdstation->address());
  rml.setEchoRequested(false);
  rml.setArgQuantity(1);
  rml.setArg(0,cartnum);
  emit rmlEcho(&rml);
}


void Switcher::logBytes(uint8_t *data,int len)
{
  QString str;

  for(int i=0;i<len;i++) {
    str+=QString().sprintf("%02X ",0xff&data[i]);
  }
  syslog(LOG_NOTICE,"bytes: %s",(const char *)str);
}
