// switcher.h
//
// Abstract base class for Rivendell Switcher/GPIO drivers.
//
//   (C) Copyright 2002-2007,1020 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: switcher.h,v 1.1 2010/08/03 23:39:26 cvs Exp $
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

#ifndef SWITCHER_H
#define SWITCHER_H

#include <qobject.h>

#include <rdmatrix.h>
#include <rdmacro.h>

class Switcher : public QObject
{
 Q_OBJECT
 public:
  Switcher(RDMatrix *matrix,QObject *parent=0,const char *name=0);
  ~Switcher();
  virtual RDMatrix::Type type()=0;
  virtual unsigned gpiQuantity()=0;
  virtual unsigned gpoQuantity()=0;
  virtual bool primaryTtyActive()=0;
  virtual bool secondaryTtyActive()=0;
  virtual void processCommand(RDMacro *cmd)=0;
  virtual void sendGpi();
  virtual void sendGpo();

 signals:
  void rmlEcho(RDMacro *cmd);
  void gpiChanged(int matrix,int line,bool state);
  void gpoChanged(int matrix,int line,bool state);
  void gpiState(int matrix,unsigned line,bool state);
  void gpoState(int matrix,unsigned line,bool state);
};


#endif  // SWITCHER_H
