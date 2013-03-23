// rdslotbox.h
//
// Cart slot label widget for RDCartSlot
//
//   (C) Copyright 2012 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdslotbox.h,v 1.3.2.2 2012/11/16 18:10:40 cvs Exp $
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

#ifndef RDSLOTBOX_H
#define RDSLOTBOX_H

#include <qwidget.h>
#include <qtimer.h>
#include <qdatetime.h>
#include <qpushbutton.h>
#include <qprogressbar.h>
#include <qpalette.h>
#include <qpixmap.h>
#include <qlabel.h>

#include <rdlabel.h>
#include <rdlog_line.h>
#include <rdlog_event.h>
#include <rdplaymeter.h>

//
// Widget Settings
//
#define RDSLOTBOX_FULL_HEIGHT 85
#define RDSLOTBOX_HALF_HEIGHT 50

class RDSlotBox : public QWidget
{
  Q_OBJECT
 public:
  enum BarMode {Transitioning=0,Stopping=1};
  RDSlotBox(QWidget *parent=0);
  QSize sizeHint() const;
  QSizePolicy sizePolicy() const;
  RDLogLine *logLine();
  void setCart(RDLogLine *logline);
  void setService(const QString &svcname);
  void setStatusLine(const QString &str);
  void setTimer(int msecs);
  void clear();
  void setBarMode(RDSlotBox::BarMode);
  void updateMeters(short levels[2]);

 signals:
  void doubleClicked();

 protected:
  void mouseDoubleClickEvent(QMouseEvent *e);
  void paintEvent(QPaintEvent *);

 private:
  void SetColor(QColor);
  QLabel *line_icon_label;
  QLabel *line_cart_label;
  QLabel *line_cut_label;
  QLabel *line_group_label;
  QLabel *line_title_label;
  QLabel *line_description_label;
  QLabel *line_artist_label;
  QLabel *line_outcue_label;
  QLabel *line_length_label;
  QLabel *line_talktime_label;
  QLabel *line_up_label;
  QLabel *line_down_label;
  QProgressBar *line_position_bar;
  QTime line_end_time;
  int log_id;
  RDLogLine *line_logline;
  QFont line_font;
  QFont talk_font;
  QFont line_bold_font;
  QPalette line_unchanged_stop_palette;
  QPalette line_unchanged_play_palette;
  QPalette line_changed_stop_palette;
  QPalette line_changed_play_palette;
  QPalette line_time_palette;
  QPalette line_hard_palette;
  QPalette line_timescale_palette;
  RDLogLine::Type line_type;
  QPixmap *line_playout_map;
  QPixmap *line_macro_map;
  RDPlayMeter *line_meter[2];
};


#endif 
