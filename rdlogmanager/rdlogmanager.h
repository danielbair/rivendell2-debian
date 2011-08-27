// rdlogmanager.h
//
// The Log Manager Utility for Rivendell.
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdlogmanager.h,v 1.14 2011/05/03 19:46:30 cvs Exp $
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


#ifndef RDLOGMANAGER_H
#define RDLOGMANAGER_H

#include <qwidget.h>
#include <qsize.h>
#include <qsizepolicy.h>
#include <qsqldatabase.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qpixmap.h>

#include <rduser.h>
#include <rdripc.h>
#include <rdlibrary_conf.h>
#include <rdlog_line.h>
#include <rdconfig.h>

#define RDLOGMANAGER_USAGE "[-g] [-m] [-t] [-d <date>] -s <svc-name>\n\n-g\n     Generate a new log for the specified service.\n\n-m\n     Merge the Music log for the specified service.\n\n-t\n     Merge the Traffic log for the specified service.\n\n-d <date-offset>\n     Perform the specified operation(s) for the date <date-offset> days\n     after the current date (default '0').\n\n-s <service-name>\n     Perform the specified operation(s) on service <service-name>.\n\n"

class MainWidget : public QWidget
{
 Q_OBJECT
 public:
  MainWidget(QWidget *parent=0,const char *name=0);
  QSize sizeHint() const;
  QSizePolicy sizePolicy() const;
  
 private slots:
  void userData();
  void eventsData();
  void clocksData();
  void gridsData();
  void generateData();
  void reportsData();
  void quitMainWidget();

 private:
  void LoadConfig();
  RDConfig *log_config;
  QSqlDatabase *log_db;
  QString log_filename;
  QLabel *log_user_label;
  QPushButton *log_events_button;
  QPushButton *log_clocks_button;
  QPushButton *log_grids_button;
  QPushButton *log_logs_button;
  QPushButton *log_reports_button;
  QPushButton *log_close_button;
  QPixmap *log_rivendell_map;
};


#endif 
