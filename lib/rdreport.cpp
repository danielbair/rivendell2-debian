// rdreport.cpp
//
// Abstract a Rivendell Report Descriptor
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdreport.cpp,v 1.27.4.8 2013/12/10 21:25:51 cvs Exp $
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

#include <qobject.h>

#include <rdconf.h>
#include <rdreport.h>
#include <rdcreate_log.h>
#include <rdlog_line.h>
#include <rdescape_string.h>
#include <rddb.h>
#include <rdescape_string.h>
#include <rddatedecode.h>

RDReport::RDReport(const QString &rptname,QObject *parent,const char *name)
{
  report_name=rptname;
  report_error_code=RDReport::ErrorOk;
}


QString RDReport::name() const
{
  return report_name;
}


bool RDReport::exists() const
{
  RDSqlQuery *q=new RDSqlQuery(QString().sprintf("select NAME from REPORTS\
                                                where NAME=\"%s\"",
						 (const char *)
						 RDEscapeString(report_name)));
  if(!q->first()) {
    delete q;
    return false;
  }
  delete q;
  return true;
}


QString RDReport::description() const
{
  return RDGetSqlValue("REPORTS","NAME",report_name,"DESCRIPTION").toString();
}


void RDReport::setDescription(const QString &desc) const
{
  SetRow("DESCRIPTION",desc);
}


RDReport::ExportFilter RDReport::filter() const
{
  return (RDReport::ExportFilter)RDGetSqlValue("REPORTS","NAME",report_name,
					      "EXPORT_FILTER").toInt();
}


void RDReport::setFilter(ExportFilter filter) const
{
  SetRow("EXPORT_FILTER",(int)filter);
}


QString RDReport::exportPath(ExportOs ostype) const
{
  return RDGetSqlValue("REPORTS","NAME",report_name,OsFieldName(ostype)).
    toString();
}


void RDReport::setExportPath(ExportOs ostype,const QString &path) const
{
  SetRow(OsFieldName(ostype),path);
}


bool RDReport::exportTypeEnabled(ExportType type) const
{
  return RDBool(RDGetSqlValue("REPORTS","NAME",report_name,
			    TypeFieldName(type,false)).toString());
}


void RDReport::setExportTypeEnabled(ExportType type,bool state) const
{
  SetRow(TypeFieldName(type,false),RDYesNo(state));
}


bool RDReport::exportTypeForced(ExportType type) const
{
  return RDBool(RDGetSqlValue("REPORTS","NAME",report_name,
			    TypeFieldName(type,true)).toString());
}


void RDReport::setExportTypeForced(ExportType type,bool state) const
{
  SetRow(TypeFieldName(type,true),RDYesNo(state));
}


QString RDReport::stationId() const
{
  return RDGetSqlValue("REPORTS","NAME",report_name,"STATION_ID").toString();
}


void RDReport::setStationId(const QString &id) const
{
  SetRow("STATION_ID",id);
}


unsigned RDReport::cartDigits() const
{
  return RDGetSqlValue("REPORTS","NAME",report_name,"CART_DIGITS").toUInt();
}

  
void RDReport::setCartDigits(unsigned num) const
{
  SetRow("CART_DIGITS",num);
}


bool RDReport::useLeadingZeros() const
{
  return RDBool(RDGetSqlValue("REPORTS","NAME",report_name,"USE_LEADING_ZEROS").
	       toString());
}


void RDReport::setUseLeadingZeros(bool state) const
{
  SetRow("USE_LEADING_ZEROS",state);
}


int RDReport::linesPerPage() const
{
  return RDGetSqlValue("REPORTS","NAME",report_name,"LINES_PER_PAGE").toInt();
}


void RDReport::setLinesPerPage(int lines) const
{
  SetRow("LINES_PER_PAGE",lines);
}


QString RDReport::serviceName() const
{
  return RDGetSqlValue("REPORTS","NAME",report_name,"SERVICE_NAME").toString();
}


void RDReport::setServiceName(const QString &name) const
{
  SetRow("SERVICE_NAME",name);
}


RDReport::StationType RDReport::stationType() const
{
  return (RDReport::StationType)
    RDGetSqlValue("REPORTS","NAME",report_name,"STATION_TYPE").toInt();
}


void RDReport::setStationType(RDReport::StationType type) const
{
  SetRow("STATION_TYPE",(int)type);
}


QString RDReport::stationFormat() const
{
  return RDGetSqlValue("REPORTS","NAME",report_name,"STATION_FORMAT").
    toString();
}


void RDReport::setStationFormat(const QString &fmt) const
{
  SetRow("STATION_FORMAT",fmt);
}


bool RDReport::filterOnairFlag() const
{
  return RDBool(RDGetSqlValue("REPORTS","NAME",report_name,"FILTER_ONAIR_FLAG").
    toString());
}


void RDReport::setFilterOnairFlag(bool state) const
{
  SetRow("FILTER_ONAIR_FLAG",RDYesNo(state));
}


bool RDReport::filterGroups() const
{
  return RDBool(RDGetSqlValue("REPORTS","NAME",report_name,"FILTER_GROUPS").
    toString());
}


void RDReport::setFilterGroups(bool state) const
{
  SetRow("FILTER_GROUPS",RDYesNo(state));
}


RDReport::ErrorCode RDReport::errorCode() const
{
  return report_error_code;
}


bool RDReport::generateReport(const QDate &startdate,const QDate &enddate,
			      RDStation *station,QString *out_path)
{
  QString sql;
  RDSqlQuery *q;
  RDSqlQuery *q1;
  RDSqlQuery *q2;
  RDSvc *svc;
  QString rec_name;
  QString station_sql;
  QString group_sql;
  QString force_sql;
  if(!exists()) {
    return false;
  }

  //
  // Generate the Station List
  //
  sql=QString().sprintf("select STATION_NAME from REPORT_STATIONS \
                         where REPORT_NAME=\"%s\"",
			(const char *)name());
  q=new RDSqlQuery(sql);
  while(q->next()) {
    station_sql+=QString().sprintf("(STATION_NAME=\"%s\")||",
				   (const char *)q->value(0).toString());
  }
  delete q;
  station_sql=station_sql.left(station_sql.length()-2);

  //
  // Next, the group list
  //
  bool where=false;
  if(exportTypeEnabled(RDReport::Generic)) {
    sql="select NAME from GROUPS  ";
  }
  else {
    where=true;
    sql="select NAME from GROUPS where ";
    if(exportTypeEnabled(RDReport::Traffic)) {
      sql+="(REPORT_TFC=\"Y\")||";
    }
    if(exportTypeEnabled(RDReport::Music)) {
      sql+="(REPORT_MUS=\"Y\")||";
    }
  }
  if(filterGroups()) {
    QString sql2=QString().sprintf("select GROUP_NAME from REPORT_GROUPS \
                                    where REPORT_NAME=\"%s\"",
				   (const char *)RDEscapeString(name()));
    q=new RDSqlQuery(sql2);
    while(q->next()) {
      if(!where) {
	sql+="where ";
	where=true;
      }
      sql+=QString().sprintf("(NAME=\"%s\")||",
		     (const char *)RDEscapeString(q->value(0).toString()));
    }
    delete q;
  }
  sql=sql.left(sql.length()-2);
  q=new RDSqlQuery(sql);
  while(q->next()) {
    group_sql+=QString().sprintf("(CART.GROUP_NAME=\"%s\")||",
				 (const char *)q->value(0).toString());
  }
  delete q;
  group_sql=group_sql.left(group_sql.length()-2);
  if(group_sql.length()==2) {
    group_sql="";
  }
  
  //
  // Generate Mixdown Table
  //
  QString mixname=
    QString().sprintf("MIXDOWN%s",(const char *)station->name());
  mixname=RDEscapeStringSQLColumn(mixname);
  sql=QString().sprintf("drop table `%s_SRT`",(const char *)mixname);
  QSqlQuery *p;
  p=new QSqlQuery(sql);
  delete p;
  sql=RDCreateReconciliationTableSql(mixname);
  q=new RDSqlQuery(sql);
  delete q;
  sql=QString().sprintf("select SERVICE_NAME from REPORT_SERVICES \
                         where REPORT_NAME=\"%s\"",
			(const char *)name());
  q=new RDSqlQuery(sql);
  while(q->next()) {
    svc=new RDSvc(q->value(0).toString());
    if(svc->exists()) {
      rec_name=q->value(0).toString();
      rec_name.replace(" ","_");
      force_sql="";
      if(exportTypeForced(RDReport::Traffic)) {
	force_sql+=QString().sprintf("(`%s_SRT`.EVENT_SOURCE=%d)||",
				       (const char *)rec_name,
				       RDLogLine::Traffic);
      }
      if(exportTypeForced(RDReport::Music)) {
	force_sql+=QString().sprintf("(`%s_SRT`.EVENT_SOURCE=%d)||",
				       (const char *)rec_name,
				       RDLogLine::Music);
      }
      force_sql=force_sql.left(force_sql.length()-2);

      sql=QString().sprintf("select LENGTH,LOG_ID,CART_NUMBER,STATION_NAME,\
                           EVENT_DATETIME,EVENT_TYPE,EXT_START_TIME,\
                           EXT_LENGTH,EXT_DATA,EXT_EVENT_ID,EXT_ANNC_TYPE,\
                           PLAY_SOURCE,CUT_NUMBER,EVENT_SOURCE,EXT_CART_NAME,\
                           LOG_NAME,`%s_SRT`.TITLE,`%s_SRT`.ARTIST,\
                           SCHEDULED_TIME,\
                           START_SOURCE,`%s_SRT`.PUBLISHER,`%s_SRT`.COMPOSER,\
                           `%s_SRT`.ALBUM,`%s_SRT`.LABEL,\
                           `%s_SRT`.ISRC,`%s_SRT`.USAGE_CODE,\
                           `%s_SRT`.ONAIR_FLAG,`%s_SRT`.ISCI from `%s_SRT`\
                           left join CART on `%s_SRT`.CART_NUMBER=CART.NUMBER \
                           where (EVENT_DATETIME>=\"%s 00:00:00\")&&\
                           (EVENT_DATETIME<=\"%s 23:59:59\")&&(%s)",
			    (const char *)rec_name,
			    (const char *)rec_name,
			    (const char *)rec_name,
			    (const char *)rec_name,
			    (const char *)rec_name,
			    (const char *)rec_name,
			    (const char *)rec_name,
			    (const char *)rec_name,
			    (const char *)rec_name,
			    (const char *)rec_name,
			    (const char *)rec_name,
			    (const char *)rec_name,
			    (const char *)startdate.toString("yyyy-MM-dd"),
			    (const char *)enddate.toString("yyyy-MM-dd"),
			    (const char *)station_sql);
      if(filterOnairFlag()) {
	sql+="&&(ONAIR_FLAG=\"Y\")";
      }
      sql+="&&(";
      if(!group_sql.isEmpty()) {
	sql+=QString().sprintf("(%s)||",(const char *)group_sql);
      }
      if(!force_sql.isEmpty()) {
	sql+=QString().sprintf("(%s)||",(const char *)force_sql);
      }
      sql=sql.left(sql.length()-2);
      sql+=")";

      q1=new RDSqlQuery(sql);
      while(q1->next()) {
	sql=QString().sprintf("insert into `%s_SRT` set\
                             LENGTH=%d,\
                             LOG_ID=%u,\
                             CART_NUMBER=%d,\
                             STATION_NAME=\"%s\",\
                             EVENT_DATETIME=\"%s\",\
                             EVENT_TYPE=%d,\
                             EXT_START_TIME=\"%s\",\
                             EXT_LENGTH=%d,\
                             EXT_DATA=\"%s\",\
                             EXT_EVENT_ID=\"%s\",\
                             EXT_ANNC_TYPE=\"%s\",\
                             PLAY_SOURCE=%d,\
                             CUT_NUMBER=%d,\
                             EVENT_SOURCE=%d,\
                             EXT_CART_NAME=\"%s\",\
                             LOG_NAME=\"%s\",\
                             TITLE=\"%s\",\
                             ARTIST=\"%s\",\
                             SCHEDULED_TIME=\"%s\",\
                             START_SOURCE=%d,\
                             PUBLISHER=\"%s\",\
                             COMPOSER=\"%s\",\
                             ALBUM=\"%s\",\
                             LABEL=\"%s\",\
                             ISRC=\"%s\",\
                             USAGE_CODE=%d,\
                             ONAIR_FLAG=\"%s\",\
                             ISCI=\"%s\"",
			      (const char *)mixname,
			      q1->value(0).toInt(),
			      q1->value(1).toUInt(),
			      q1->value(2).toInt(),
			      (const char *)
			      RDEscapeString(q1->value(3).toString()),
			      (const char *)
			      RDEscapeString(q1->value(4).toString()),
			      q1->value(5).toInt(),
			      (const char *)
			      RDEscapeString(q1->value(6).toString()),
			      q1->value(7).toInt(),
			      (const char *)
			      RDEscapeString(q1->value(8).toString()),
			      (const char *)
			      RDEscapeString(q1->value(9).toString()),
			      (const char *)
			      RDEscapeString(q1->value(10).toString()),
			      q1->value(11).toInt(),
			      q1->value(12).toInt(),
			      q1->value(13).toInt(),
			      (const char *)
			      RDEscapeString(q1->value(14).toString()),
			      (const char *)
			      RDEscapeString(q1->value(15).toString()),
			      (const char *)
			      RDEscapeString(q1->value(16).toString()),
			      (const char *)
			      RDEscapeString(q1->value(17).toString()),
			      (const char *)q1->value(18).toDate().
			      toString("yyyy-MM-dd hh:mm:ss"),
			      q1->value(19).toInt(),
			      (const char *)
			      RDEscapeString(q1->value(20).toString()),
			      (const char *)
			      RDEscapeString(q1->value(21).toString()),
			      (const char *)
			      RDEscapeString(q1->value(22).toString()),
			      (const char *)
			      RDEscapeString(q1->value(23).toString()),
			      (const char *)
			      RDEscapeString(q1->value(24).toString()),
			      q1->value(25).toInt(),
			      (const char *)
			      RDEscapeString(q1->value(26).toString()),
			      (const char *)
			      RDEscapeString(q1->value(27).toString()));
	q2=new RDSqlQuery(sql);
	delete q2;
      }
      delete q1;
    }
    delete svc;
  }
  delete q;

  bool ret=false;
  switch(filter()) {
      case RDReport::CbsiDeltaFlex:
	ret=ExportDeltaflex(startdate,enddate,mixname);
	break;

      case RDReport::TextLog:
	ret=ExportTextLog(startdate,enddate,mixname);
	break;

      case RDReport::BmiEmr:
	ret=ExportBmiEmr(startdate,enddate,mixname);
	break;

      case RDReport::NaturalLog:
      case RDReport::Technical:
	ret=ExportTechnical(startdate,enddate,mixname);
	break;

      case RDReport::SoundExchange:
	ret=ExportSoundEx(startdate,enddate,mixname);
	break;

      case RDReport::NprSoundExchange:
	ret=ExportNprSoundEx(startdate,enddate,mixname);
	break;

      case RDReport::RadioTraffic:
	ret=ExportRadioTraffic(startdate,enddate,mixname);
	break;

      case RDReport::VisualTraffic:
	ret=ExportDeltaflex(startdate,enddate,mixname);
	break;

      case RDReport::CounterPoint:
      case RDReport::WideOrbit:
	ret=ExportRadioTraffic(startdate,enddate,mixname);
	break;

      case RDReport::Music1:
	ret=ExportRadioTraffic(startdate,enddate,mixname);
	break;

      case RDReport::MusicPlayout:
	ret=ExportMusicPlayout(startdate,enddate,mixname);
	break;

      case RDReport::MusicSummary:
	ret=ExportMusicSummary(startdate,enddate,mixname);
	break;

      default:
	return false;
	break;
  }
#ifdef WIN32
  *out_path=RDDateDecode(exportPath(RDReport::Windows),startdate);
#else
  *out_path=RDDateDecode(exportPath(RDReport::Linux),startdate);
#endif
  //printf("MIXDOWN TABLE: %s_SRT\n",(const char *)mixname);
  sql=QString().sprintf("drop table `%s_SRT`",(const char *)mixname);
  q=new RDSqlQuery(sql);
  delete q;

  return ret;
}


QString RDReport::filterText(RDReport::ExportFilter filter)
{
  switch(filter) {
  case RDReport::CbsiDeltaFlex:
    return QObject::tr("CBSI DeltaFlex Traffic Reconciliation v2.01");

  case RDReport::TextLog:
    return QObject::tr("Text Log");

  case RDReport::BmiEmr:
    return QObject::tr("ASCAP/BMI Electronic Music Report");

  case RDReport::Technical:
    return QObject::tr("Technical Playout Report");

  case RDReport::SoundExchange:
    return QObject::tr("SoundExchange Statutory License Report");

  case RDReport::NprSoundExchange:
    return QObject::tr("NPR/DS SoundExchange Report");

  case RDReport::RadioTraffic:
    return QObject::tr("RadioTraffic.com Traffic Reconciliation");

  case RDReport::VisualTraffic:
    return QObject::tr("VisualTraffic Reconciliation");

  case RDReport::CounterPoint:
    return QObject::tr("CounterPoint Traffic Reconciliation");

  case RDReport::Music1:
    return QObject::tr("Music1 Reconciliation");

  case RDReport::MusicPlayout:
    return QObject::tr("Music Playout");

  case RDReport::MusicSummary:
    return QObject::tr("Music Summary");

  case RDReport::NaturalLog:
    return QObject::tr("NaturalLog Reconciliation");

  case RDReport::WideOrbit:
    return QObject::tr("WideOrbit Traffic Reconciliation");

  default:
    return QObject::tr("Unknown");
  }
  return QObject::tr("Unknown");
}


QString RDReport::stationTypeText(RDReport::StationType type)
{
  switch(type) {
  case RDReport::TypeOther:
    return QObject::tr("Other");

  case RDReport::TypeAm:
    return QObject::tr("AM");

  case RDReport::TypeFm:
    return QObject::tr("FM");

  default:
    break;
  }
  return QObject::tr("Unknown");
}


bool RDReport::multipleDaysAllowed(RDReport::ExportFilter filter)
{
  switch(filter) {
  case RDReport::CbsiDeltaFlex:
  case RDReport::TextLog:
  case RDReport::RadioTraffic:
  case RDReport::VisualTraffic:
  case RDReport::CounterPoint:
  case RDReport::LastFilter:
  case RDReport::Music1:
  case RDReport::MusicPlayout:
  case RDReport::NaturalLog:
  case RDReport::WideOrbit:
    return false;

  case RDReport::BmiEmr:
  case RDReport::SoundExchange:
  case RDReport::NprSoundExchange:
  case RDReport::Technical:
  case RDReport::MusicSummary:
    return true;
  }
  return true;
}


bool RDReport::multipleMonthsAllowed(RDReport::ExportFilter filter)
{
  switch(filter) {
  case RDReport::CbsiDeltaFlex:
  case RDReport::TextLog:
  case RDReport::BmiEmr:
  case RDReport::RadioTraffic:
  case RDReport::VisualTraffic:
  case RDReport::CounterPoint:
  case RDReport::LastFilter:
  case RDReport::Music1:
  case RDReport::MusicPlayout:
  case RDReport::NaturalLog:
  case RDReport::WideOrbit:
    return false;
    
  case RDReport::SoundExchange:
  case RDReport::NprSoundExchange:
  case RDReport::Technical:
  case RDReport::MusicSummary:
    return true;
  }
  return true;
}


QString RDReport::errorText(RDReport::ErrorCode code)
{
  QString ret;
  switch(code) {
      case RDReport::ErrorOk:
	ret=QObject::tr("Report complete!");
	break;

      case RDReport::ErrorCanceled:
	ret=QObject::tr("Report canceled!");
	break;

      case RDReport::ErrorCantOpen:
	ret=QObject::tr("Unable to open report file!");
	break;
  }
  return ret;
}


QString RDReport::StringField(const QString &str) const
{
  QString ret="[unknown]";

  if(!str.isEmpty()) {
    ret=str;
  }

  return ret;
}


void RDReport::SetRow(const QString &param,QString value) const
{
  RDSqlQuery *q;
  QString sql;

  sql=QString().sprintf("UPDATE REPORTS SET %s=\"%s\" WHERE NAME=\"%s\"",
			(const char *)param,
			(const char *)RDEscapeString(value),
			(const char *)report_name);
  q=new RDSqlQuery(sql);
  delete q;
}


void RDReport::SetRow(const QString &param,int value) const
{
  RDSqlQuery *q;
  QString sql;

  sql=QString().sprintf("UPDATE REPORTS SET %s=%d WHERE NAME=\"%s\"",
			(const char *)param,
			value,
			(const char *)report_name);
  q=new RDSqlQuery(sql);
  delete q;
}


void RDReport::SetRow(const QString &param,unsigned value) const
{
  RDSqlQuery *q;
  QString sql;

  sql=QString().sprintf("UPDATE REPORTS SET %s=%u WHERE NAME=\"%s\"",
			(const char *)param,
			value,
			(const char *)report_name);
  q=new RDSqlQuery(sql);
  delete q;
}


void RDReport::SetRow(const QString &param,bool value) const
{
  RDSqlQuery *q;
  QString sql;

  sql=QString().sprintf("UPDATE REPORTS SET %s=\"%s\" WHERE NAME=\"%s\"",
			(const char *)param,
			(const char *)RDYesNo(value),
			(const char *)report_name);
  q=new RDSqlQuery(sql);
  delete q;
}


QString RDReport::OsFieldName(ExportOs os) const
{
  switch(os) {
      case RDReport::Linux:
	return QString("EXPORT_PATH");
	
      case RDReport::Windows:
	return QString("WIN_EXPORT_PATH");
  }
  return QString();
}


QString RDReport::TypeFieldName(ExportType type,bool forced) const
{
  if(forced) {
    switch(type) {
	case RDReport::Traffic:
	  return QString("FORCE_TFC");
	  
	case RDReport::Music:
	  return QString("FORCE_MUS");

	default:
	  return QString();
    }
  }
  else {
    switch(type) {
	case RDReport::Generic:
	  return QString("EXPORT_GEN");
	  
	case RDReport::Traffic:
	  return QString("EXPORT_TFC");
	  
	case RDReport::Music:
	  return QString("EXPORT_MUS");
    }
    return QString();
  }
  return QString();
}
