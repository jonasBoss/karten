#include "hoehenabfrager.h"
#include "region.h"
#include "polylineencoder.h"
#include <QtAlgorithms>
#include <QtCore>
#include <QtGui>
int Hoehenabfrager::runningProcesses=0;
Hoehenabfrager::Hoehenabfrager(Track*t, Integerliste l) : QObject(0)
{
   track=t;
   pointlist=l;
   if(!pointlist.isEmpty()) qSort(pointlist.begin(),pointlist.end());
   PolylineEncoder encoder;
   if(pointlist.isEmpty()){//Achtung es sollten nicht zu viele Abfragen gleichzeitig passieren. Da muss ich nochmal ran.
       for(int i=0;i<t->points.length();i++){
           encoder.addPoint(t->points.at(i).latitude,t->points.at(i).longitude);
       }
   }else
       for(int j=0;j<pointlist.size();j++){
           int i=pointlist.at(j);
           encoder.addPoint(t->points.at(i).latitude,t->points.at(i).longitude);
       }
   std::string encoded=encoder.encode();
   qDebug()<<"Abzufragenden Punkte: "<<encoder.size();
   QString server="https://maps.googleapis.com/maps/api/elevation/json?";
   QSettings set("UBoss","karten");
   QString apiKey=set.value("apiKey","").toString();
   QString url=server+"key="+apiKey+"&locations=enc:"+QString(encoded.c_str());
   abfrager=new QProcess(this);
   connect(abfrager,SIGNAL(finished(int)),this,SLOT(antwortDa(int)));
   connect(abfrager,SIGNAL(error(QProcess::ProcessError)),this,SLOT(error()));
   runningProcesses++;
   abfrager->setReadChannel(QProcess::StandardOutput);
   abfrager->start("wget",QStringList()<<"-O"<<"-"<<"-t"<<"5"<<"--"<<url,QIODevice::ReadOnly);
   //abfrager->start("cat",QStringList()<<"/home/uwe/testantwort",QIODevice::ReadOnly);
   qDebug()<<url;

}
void Hoehenabfrager::antwortDa(int resultCode){
  if(resultCode!=0){
      QMessageBox::warning(0,"Achtung",trUtf8("Höhendaten konnten nicht heruntergeladen werden:\n")+
                           QString(abfrager->readAllStandardError()));
      runningProcesses--;
      deleteLater();
  }
  QTextStream antwort(abfrager);
  QString zeile;
  int lastFound=-1;//index des letzten gefundenen Punktes, für den eine Höhe verfügbar ist.
  while(!antwort.atEnd()){
      int pos;
      zeile=antwort.readLine();
      //qDebug()<<"gelesen: "<<zeile;
      if((pos=zeile.indexOf("elevation"))!=-1){
          pos=zeile.indexOf(':',pos);
          double altitude=zeile.mid(pos+1,zeile.indexOf(',',pos+1)-pos-1).toDouble();
          do{
              zeile=antwort.readLine();
          }while((pos=zeile.indexOf("lat"))==-1);
          pos=zeile.indexOf(':',pos);
          double latitude=zeile.mid(pos+1,zeile.indexOf(',',pos+1)-pos-1).toDouble();
          do{
              zeile=antwort.readLine();
          }while((pos=zeile.indexOf("lng"))==-1);
          pos=zeile.indexOf(':',pos);
          double longitude=zeile.mid(pos+1).toDouble();
          //qDebug()<<"Punkt gelesen: "<<latitude<<longitude<<altitude;
          for(int i=lastFound+1;i<track->points.size();i++){
              Trackpoint &p=track->points[i];
              if(Position(latitude,longitude).distanceFrom(Position(p)) < 10.0){
                  p.altitude=altitude;
                  lastFound=i;
                  qDebug()<<"Punkt "<<i<<"aktualisiert: "<<altitude;
                  break;
              }
          }
          //insertPoint(latitude,longitude,altitude);
       }
  }
  qDebug()<<"StandardError: "<<abfrager->readAllStandardError();
  abfrager->closeReadChannel(QProcess::StandardOutput);
  emit habeFertig();
  abfrager->deleteLater();
  deleteLater();
}

void Hoehenabfrager::error()
{
    QMessageBox::warning(0,"Achtung!",trUtf8("Die Höhenabfrage konnte garnicht erst gestartet werden.")+abfrager->readAllStandardError());
    deleteLater();
}


