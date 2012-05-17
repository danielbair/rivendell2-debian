// list_clocks.cpp
//
// List Rivendell Log Clocks
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: list_clocks.cpp,v 1.28.8.1 2012/04/23 17:22:47 cvs Exp $
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

#include <qdialog.h>
#include <qstring.h>
#include <qtextedit.h>
#include <qpainter.h>
#include <qmessagebox.h>
#include <qpixmap.h>
#include <qpainter.h>

#include <rddb.h>
#include <rd.h>
#include <rdevent.h>
#include <rdcreate_log.h>

#include <list_clocks.h>
#include <add_clock.h>
#include <edit_clock.h>
#include <globals.h>
#include <rename_item.h>


ListClocks::ListClocks(QString *clockname,QWidget *parent,const char *name)
  : QDialog(parent,name,true)
{
  QStringList services_list;
  QString str1=tr("Log Clocks - User: ");
  setCaption(QString().sprintf("%s%s",(const char *)str1,
			       (const char *)rdripc->user()));
  edit_clockname=clockname;

  //
  // Fix the Window Size
  //
  setMinimumWidth(sizeHint().width());
  setMaximumWidth(sizeHint().width());
  setMinimumHeight(sizeHint().height());
  setMaximumHeight(sizeHint().height());

  //
  // Create Fonts
  //
  QFont bold_font=QFont("Helvetica",12,QFont::Bold);
  bold_font.setPixelSize(12);
  QFont font=QFont("Helvetica",12,QFont::Normal);
  font.setPixelSize(12);

  //
  // Event Filter
  //
  edit_filter_box=new QComboBox(this,"edit_filter_box");
  edit_filter_box->setGeometry(65,10,sizeHint().width()-75,20);
  QLabel *label=new QLabel(edit_filter_box,tr("Filter:"),
			   this,"edit_filter_label");
  label->setGeometry(10,10,50,20);
  label->setFont(bold_font);
  label->setAlignment(AlignRight|AlignVCenter);
  connect(edit_filter_box,SIGNAL(activated(int)),
	  this,SLOT(filterActivatedData(int)));

  //
  // Clocks List
  //
  edit_clocks_list=new QListView(this,"edit_clocks_list");
  edit_clocks_list->setGeometry(10,45,
				sizeHint().width()-20,sizeHint().height()-115);
  edit_clocks_list->setAllColumnsShowFocus(true);
  edit_clocks_list->setItemMargin(5);
  edit_clocks_list->addColumn(tr("Name"));
  edit_clocks_list->addColumn(tr("Code"));
  edit_clocks_list->addColumn(tr("Color"));
  edit_clocks_list->setColumnAlignment(2,AlignCenter);
  connect(edit_clocks_list,
	  SIGNAL(doubleClicked(QListViewItem *,const QPoint &,int)),
	  this,SLOT(doubleClickedData(QListViewItem *,const QPoint &,int)));

  if(edit_clockname==NULL) {
    //
    //  Add Button
    //
    QPushButton *button=new QPushButton(this,"add_button");
    button->setGeometry(10,sizeHint().height()-60,80,50);
    button->setFont(bold_font);
    button->setText(tr("&Add"));
    connect(button,SIGNAL(clicked()),this,SLOT(addData()));
    
    //
    //  Edit Button
    //
    button=new QPushButton(this,"edit_button");
    button->setGeometry(100,sizeHint().height()-60,80,50);
    button->setFont(bold_font);
    button->setText(tr("&Edit"));
    connect(button,SIGNAL(clicked()),this,SLOT(editData()));
    
    //
    //  Delete Button
    //
    button=new QPushButton(this,"delete_button");
    button->setGeometry(190,sizeHint().height()-60,80,50);
    button->setFont(bold_font);
    button->setText(tr("&Delete"));
    connect(button,SIGNAL(clicked()),this,SLOT(deleteData()));
    
    //
    //  Rename Button
    //
    button=new QPushButton(this,"rename_button");
    button->setGeometry(310,sizeHint().height()-60,80,50);
    button->setFont(bold_font);
    button->setText(tr("&Rename"));
    connect(button,SIGNAL(clicked()),this,SLOT(renameData()));
    
    //
    //  Close Button
    //
    button=new QPushButton(this,"close_button");
    button->setGeometry(sizeHint().width()-90,sizeHint().height()-60,80,50);
    button->setDefault(true);
    button->setFont(bold_font);
    button->setText(tr("C&lose"));
    connect(button,SIGNAL(clicked()),this,SLOT(closeData()));
  }
  else {
    //
    //  Clear Button
    //
    QPushButton *button=new QPushButton(this,"clear_button");
    button->setGeometry(10,sizeHint().height()-60,80,50);
    button->setFont(bold_font);
    button->setText(tr("C&lear"));
    connect(button,SIGNAL(clicked()),this,SLOT(clearData()));

    //
    //  OK Button
    //
    button=new QPushButton(this,"ok_button");
    button->setGeometry(sizeHint().width()-180,sizeHint().height()-60,80,50);
    button->setDefault(true);
    button->setFont(bold_font);
    button->setText(tr("&OK"));
    connect(button,SIGNAL(clicked()),this,SLOT(okData()));

    //
    //  Cancel Button
    //
    button=new QPushButton(this,"cancel_button");
    button->setGeometry(sizeHint().width()-90,sizeHint().height()-60,80,50);
    button->setFont(bold_font);
    button->setText(tr("&Cancel"));
    connect(button,SIGNAL(clicked()),this,SLOT(cancelData()));
  }

  //
  // Populate Data
  //
  edit_filter_box->insertItem(tr("ALL"));
  edit_filter_box->insertItem(tr("NONE"));

  if (rdstation_conf->broadcastSecurity() == RDStation::UserSec) {
    services_list = rduser->services();
  } else { // RDStation::HostSec
    QString sql="select NAME from SERVICES";
    RDSqlQuery *q=new RDSqlQuery(sql);
    while(q->next()) {
      services_list.append( q->value(0).toString() );
    }
    delete q;
  } 
  services_list.sort();
  for ( QStringList::Iterator it = services_list.begin(); 
        it != services_list.end();
        ++it ) {
    edit_filter_box->insertItem(*it);
    if(*clock_filter==*it) {
      edit_filter_box->setCurrentItem(edit_filter_box->count()-1);
    }
  }
  RefreshList();

  if(edit_clockname!=NULL) {
    QListViewItem *item=edit_clocks_list->firstChild();
    while(item!=NULL) {
      if(item->text(0)==*edit_clockname) {
	edit_clocks_list->setSelected(item,true);
      }
      item=item->nextSibling();
    }
  }
}


QSize ListClocks::sizeHint() const
{
  return QSize(600,400);
} 


QSizePolicy ListClocks::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


void ListClocks::addData()
{
  QString clockname;
  QString clockname_esc;
  QString sql;
  RDSqlQuery *q;
  RDSqlQuery *q1;
  std::vector<QString> new_clocks;

  AddClock *add_dialog=new AddClock(&clockname,this,"add_dialog");
  if(add_dialog->exec()<0) {
    delete add_dialog;
    return;
  }
  delete add_dialog;
  sql=QString().sprintf("select NAME from CLOCKS where NAME=\"%s\"",
			(const char *)clockname);
  q=new RDSqlQuery(sql);
  if(q->first()) {
    QMessageBox::
      information(this,tr("Clock Exists"),
		  tr("An clock with that name already exists!"));
    delete q;
    return;
  }
  delete q;
  sql=QString().sprintf("insert into CLOCKS set NAME=\"%s\",ARTISTSEP=15",
			(const char *)clockname);
  q=new RDSqlQuery(sql);
  delete q;
  sql=RDCreateClockTableSql(clockname);
  q=new RDSqlQuery(sql);
  delete q;
  EditClock *clock_dialog=new EditClock(clockname,true,&new_clocks,
					this,"clock_dialog");
  if(clock_dialog->exec()<0) {
    clockname_esc=clockname;
    clockname_esc.replace(" ","_");
    clockname_esc+="_CLK";
    sql=QString().sprintf("drop table %s",(const char *)clockname_esc);
    q=new RDSqlQuery(sql);
    delete q;
    sql=QString().sprintf("delete from CLOCKS where NAME=\"%s\"",
			  (const char *)clockname);
    q=new RDSqlQuery(sql);
    delete q;
  }
  else {
    if(edit_filter_box->currentItem()==0) {
      sql="select NAME from SERVICES";
      q=new RDSqlQuery(sql);
      while(q->next()) {
        // FIXME: not sure if the usersec service filter should be applied
        // here, or if all services should be brought over and later filtered
        // by edit_perms.cpp dialog.
	sql=QString().sprintf("insert into CLOCK_PERMS set\
                               CLOCK_NAME=\"%s\",SERVICE_NAME=\"%s\"",
			      (const char *)clockname,
			      (const char *)q->value(0).toString());
	q1=new RDSqlQuery(sql);
	delete q1;
      }
      delete q;
    }
    else {
      sql=QString().sprintf("insert into CLOCK_PERMS set\
                             CLOCK_NAME=\"%s\",SERVICE_NAME=\"%s\"",
			    (const char *)clockname,
			    (const char *)edit_filter_box->currentText());
      q=new RDSqlQuery(sql);
      delete q;
    }
    QListViewItem *item=new QListViewItem(edit_clocks_list);
    item->setText(0,clockname);
    RefreshItem(item,&new_clocks);
    edit_clocks_list->setSelected(item,true);
  }
  delete clock_dialog;
}


void ListClocks::editData()
{
  std::vector<QString> new_clocks;
  QListViewItem *item=edit_clocks_list->selectedItem();
  if(item==NULL) {
    return;
  }
  EditClock *clock_dialog=
    new EditClock(item->text(0),false,&new_clocks,this,"clock_dialog");
  if(clock_dialog->exec()<0) {
    delete clock_dialog;
    return;
  }
  delete clock_dialog;
  RefreshItem(item,&new_clocks);
}


void ListClocks::deleteData()
{
  QString str1;
  QString str2;
  int n;
  QString svc_list;
  QListViewItem *item=edit_clocks_list->selectedItem();
  if(item==NULL) {
    return;
  }
  str1=QString(tr("Are you sure you want to\ndelete"));
  if(QMessageBox::question(this,tr("Delete Clock"),
			   QString().sprintf("%s \'%s\'?",(const char *)str1,
			 (const char *)item->text(0)),
			  QMessageBox::Yes,QMessageBox::No)
     !=QMessageBox::Yes) {
    return;
  }
  if((n=ActiveClocks(item->text(0),&svc_list))>0) {
    str1=QString(tr("is in use in the following grid(s):"));
    str2=QString(tr("Do you still want to delete it?"));
    if(QMessageBox::warning(this,tr("Clock In Use"),
			 QString().sprintf("\'%s\' %s:\n\n%s\n%s",
					   (const char *)item->text(0),
					   (const char *)str1,
					   (const char *)svc_list,
					   (const char *)str2),
			    QMessageBox::Yes,
			    QMessageBox::No)!=QMessageBox::Yes) {
      return;
    }
  }
  DeleteClock(item->text(0));
  RefreshList();
}


void ListClocks::renameData()
{
  QString sql;
  RDSqlQuery *q;
  RDSqlQuery *q1;
  QListViewItem *item=edit_clocks_list->selectedItem();
  if(item==NULL) {
    return;
  }
  QString new_name=item->text(0);
  RenameItem *rename_dialog=
    new RenameItem(&new_name,"CLOCKS",this,"event_dialog");
  if(rename_dialog->exec()<-1) {
    delete rename_dialog;
    return;
  }
  delete rename_dialog;
  
  //
  // Rename Grid References
  //
  sql="select NAME from SERVICES";
  // FIXME: not sure if the usersec service filter should be applied here, or
  // if all services should be brought over
  q=new RDSqlQuery(sql);
  while(q->next()) {
    for(int i=0;i<168;i++) {
      sql=QString().sprintf("update SERVICES set CLOCK%d=\"%s\"\
                             where CLOCK%d=\"%s\"",
			    i,(const char *)new_name,
			    i,(const char *)item->text(0));
      q1=new RDSqlQuery(sql);
      delete q1;
    }
  }
  delete q;

  //
  // Rename Meta Table
  //
  QString old_name_esc=item->text(0);
  old_name_esc.replace(" ","_");
  QString new_name_esc=new_name;
  new_name_esc.replace(" ","_");
  sql=QString().sprintf("alter table %s_CLK rename to %s_CLK",
			(const char *)old_name_esc,
			(const char *)new_name_esc);
  q=new RDSqlQuery(sql);
  delete q;
  sql=QString().sprintf("alter table %s_RULES rename to %s_RULES",
			(const char *)old_name_esc,
			(const char *)new_name_esc);
  q=new RDSqlQuery(sql);
  delete q;

  //
  // Rename Service Permissions
  //
  sql=QString().sprintf("update CLOCK_PERMS set CLOCK_NAME=\"%s\"\
                         where CLOCK_NAME=\"%s\"",
			(const char *)new_name,
			(const char *)item->text(0));
  q=new RDSqlQuery(sql);
  delete q;

  //
  // Rename Primary Key
  //
  sql=QString().sprintf("update CLOCKS set NAME=\"%s\" where NAME=\"%s\"",
			(const char *)new_name,
			(const char *)item->text(0));
  q=new RDSqlQuery(sql);
  delete q;

  item->setText(0,new_name);
  RefreshItem(item);
}


void ListClocks::filterActivatedData(int id)
{
  RefreshList();
}


void ListClocks::doubleClickedData(QListViewItem *item,const QPoint &,int)
{
  if(edit_clockname==NULL) {
    editData();
  }
  else {
    okData();
  }
}


void ListClocks::closeData()
{
  *clock_filter=edit_filter_box->currentText();
  done(0);
}


void ListClocks::clearData()
{
  QListViewItem *item=edit_clocks_list->selectedItem();
  if(item!=NULL) {
    edit_clocks_list->setSelected(item,false);
  }
}


void ListClocks::okData()
{
  QListViewItem *item=edit_clocks_list->selectedItem();
  *clock_filter=edit_filter_box->currentText();
  if(item==NULL) {
    *edit_clockname="";
  }
  else {
    *edit_clockname=item->text(0);
  }
  done(0);
}


void ListClocks::cancelData()
{
  done(-1);
}


void ListClocks::closeEvent(QCloseEvent *e)
{
  cancelData();
}


void ListClocks::RefreshList()
{
  QString filter;

  if(edit_filter_box->currentItem()==1) {
    filter=GetNoneFilter();
  }
  else {
    if(edit_filter_box->currentItem()>1) {
      filter=GetClockFilter(edit_filter_box->currentText());
    }
  }

  edit_clocks_list->clear();
  QString sql=QString().sprintf("select NAME,SHORT_NAME,COLOR from CLOCKS %s",
				(const char *)filter);
  RDSqlQuery *q=new RDSqlQuery(sql);
  QListViewItem *item=NULL;
  while(q->next()) {
    item=new QListViewItem(edit_clocks_list);
    WriteItem(item,q);
  }
  delete q;
}


void ListClocks::RefreshItem(QListViewItem *item,
			     std::vector<QString> *new_clocks)
{
  QListViewItem *new_item;
  UpdateItem(item,item->text(0));

  if(new_clocks!=NULL) {
    for(unsigned i=0;i<new_clocks->size();i++) {
      if((new_item=edit_clocks_list->findItem(new_clocks->at(i),0))==NULL) {
	new_item=new QListViewItem(edit_clocks_list);
      }
      UpdateItem(new_item,new_clocks->at(i));
    }
  }
}


void ListClocks::UpdateItem(QListViewItem *item,QString name)
{
  QString sql=QString().sprintf("select NAME,SHORT_NAME,COLOR from CLOCKS\
                                 where NAME=\"%s\"",(const char *)name);
  RDSqlQuery *q=new RDSqlQuery(sql);
  if(q->next()) {
    item->setText(0,name);
    WriteItem(item,q);
  }
  delete q;
}


void ListClocks::WriteItem(QListViewItem *item,RDSqlQuery *q)
{
  QPixmap *pix;
  QPainter *p=new QPainter();

  item->setText(0,q->value(0).toString());
  item->setText(1,q->value(1).toString());
  pix=new QPixmap(QSize(15,15));
  p->begin(pix);
  p->fillRect(0,0,15,15,QColor(q->value(2).toString()));
  p->end();
  item->setPixmap(2,*pix);

  delete p;
}


int ListClocks::ActiveClocks(QString clockname,QString *svc_list)
{
  int n=0;
  QString sql;
  RDSqlQuery *q;
  QString svcname;

  sql="select NAME from SERVICES where ";
  for(int i=0;i<167;i++) {
    sql+=QString().sprintf("(CLOCK%d=\"%s\")||",i,(const char *)clockname);
  }
  sql+=QString().sprintf("(CLOCK167=\"%s\")",(const char *)clockname);
  q=new RDSqlQuery(sql);
  while(q->next()) {
    n++;
    *svc_list+=
      QString().sprintf("    %s\n",(const char *)q->value(0).toString());
  }
  delete q;

  return n;
}


void ListClocks::DeleteClock(QString clockname)
{
  QString sql;
  RDSqlQuery *q;
  QString base_name=clockname;
  base_name.replace(" ","_");

  //
  // Delete Active Clocks
  //
  for(int i=0;i<168;i++) {
    sql=QString().sprintf("update SERVICES set CLOCK%d=NULL\
                             where CLOCK%d=\"%s\"",
			  i,i,(const char *)clockname);
    q=new RDSqlQuery(sql);
    delete q;
  }

  //
  // Delete Service Associations
  //
  sql=QString().sprintf("delete from CLOCK_PERMS where CLOCK_NAME=\"%s\"",
			(const char *)clockname);
  q=new RDSqlQuery(sql);
  delete q;

  //
  // Delete Clock Definition
  //
  sql=QString().sprintf("delete from CLOCKS where NAME=\"%s\"",
				(const char *)clockname);
  q=new RDSqlQuery(sql);
  delete q;
  sql=QString().sprintf("drop table %s_CLK",(const char *)base_name);
  q=new RDSqlQuery(sql);
  delete q;
  sql=QString().sprintf("drop table %s_RULES",(const char *)base_name);
  q=new RDSqlQuery(sql);
  delete q;
}


QString ListClocks::GetClockFilter(QString svc_name)
{
  QString filter="where ";
  QString sql=QString().sprintf("select CLOCK_NAME from CLOCK_PERMS\
                                 where SERVICE_NAME=\"%s\"",
				(const char *)svc_name);
  RDSqlQuery *q=new RDSqlQuery(sql);
  if(q->size()>0) {
    while(q->next()) {
      filter+=QString().sprintf("(NAME=\"%s\")||",
				(const char *)q->value(0).toString());
    }
    filter=filter.left(filter.length()-2);
  }
  else {
    filter="(SERVICE_NAME=\"\")";
  }
  delete q;

  return filter;
}


QString ListClocks::GetNoneFilter()
{
  QString sql;
  RDSqlQuery *q;
  QString filter;

  sql="select CLOCK_NAME from CLOCK_PERMS";
  q=new RDSqlQuery(sql);
  if(q->size()>0) {
    filter="where ";
  }
  while(q->next()) {
    filter+=QString().sprintf("(NAME!=\"%s\")&&",
			      (const char *)q->value(0).toString());
  }
  if(q->size()>0) {
    filter=filter.left(filter.length()-2);
  }
  delete q;

  return filter;
}

