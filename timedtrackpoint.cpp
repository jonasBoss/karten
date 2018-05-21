#include "timedtrackpoint.h"
#include "region.h"
#include <QtCore>
QDateTime TimedTrackpoint::zeroTime=QDateTime(QDate(2000,1,1),QTime(0,0));

void TimedTrackpoint::actualizeTimestamp(){
  timestamp=zeroTime.secsTo(time);
}
int TimedTrackpoint::getTimestamp(const QDateTime &t){
   return TimedTrackpoint::zeroTime.secsTo(t);
}
QDateTime TimedTrackpoint::getDateTime(int timestamp){
   return zeroTime.addSecs(timestamp);
}

TimedTrackpoint::TimedTrackpoint(){
  latitude=longitude=0.0;
  height=0;
  timestamp=0;
  time=QDateTime(zeroTime);
}
TimedTrackpoint::TimedTrackpoint(double lat,double lon,int h,int time){
  latitude=lat;longitude=lon;height=h;timestamp=time;
  actualizeTimestamp();
}

TimedTrackpoint::TimedTrackpoint(QString igcstring,QDate date){
  QString & zeile=igcstring;
  if (zeile[0]!='B'){
      latitude=longitude=0.0;
      height=0;
      time=QDateTime(date,QTime(0,0));
      actualizeTimestamp();
      return;
  }
  time=QDateTime(date,QTime(0,0));
  //qWarning("Tag hat Timestamp: %d",zeroTime.secsTo(time));
  time=time.addSecs(zeile.mid(1,2).toInt()*3600+zeile.mid(3,2).toInt()*60+zeile.mid(5,2).toInt());
  actualizeTimestamp();
  //qWarning("nach Aktualisieren des Zeitstemples: %d",timestamp);
  int deg=zeile.mid(7,2).toInt();//Grad Breite
  int mmin=zeile.mid(9,5).toInt();//Minuten Breite
  latitude=deg*1.0+mmin/60000.0;
  if (zeile[14]!='N') latitude*=-1;//dann ists im Süden.
  deg=zeile.mid(15,3).toInt();
  mmin=zeile.mid(18,5).toInt();
  longitude=deg*1.0+mmin/60000.0;
  if (zeile[23]!='E') longitude*=-1;//dann ists im Westen.
  if(zeile[24]!='A')
      height=0;
  else
     height=zeile.mid(25,5).toInt();
  return;
}
QString TimedTrackpoint::print() const{
   return QString::fromUtf8("Breite: %1, Länge: %2, Höhe: %3 \n Zeitstempel: %4, Zeit: ").arg(latitude).arg(longitude).arg(height).arg(timestamp)+
           time.toString("dd. MMM yy, hh:mm:ss");

}
void TimedTrackpoint::setDate(const QDate &date){
   time.setDate(date);
   actualizeTimestamp();
}

void TimedTrackpoint::addSeconds(int secs)
{
   time=time.addSecs(secs);
   timestamp+=secs;
}


