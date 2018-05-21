#include "region.h"
#include <iostream>
#include <math.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qspinbox.h>
#include <QtCore>
#include <QtAlgorithms>
#include <QtXml/QDomDocument>
Position::Position(double latD, double latM, double lonD, double lonM){
  setLatitude(latD,latM);
  setLongitude(lonD,lonM);
}
Position::Position(const Trackpoint &p,int z){
  setLatitude(p.latitude);setLongitude(p.longitude);setZ(z);
}
Position::Position(const Waypoint & way, int z){
  setLatitude(way.latitude);setLongitude(way.longitude);setZ(z);
}

void Position::setLatitude(double deg,double min){//erwartet beide parameter mit gleichem VZ
  latitude=deg+min/60.0;
  int scale= (1 << z);
  double dy=(0.5 - log(tan((M_PI / 4.0) + ((M_PI * latitude) / 360.0)))  / (2.0 * M_PI)) * scale;
  y=(int)dy;
  yPixel=(short)((dy-y)*256);
}
void Position::setLongitude(double deg,double min){//erwartet beide parameter mit gleichem VZ
  longitude=deg+min/60.0;
  if (longitude > 180.0){longitude -= 360.0;}
  int scale= (1 << z);
  double dx=((180.0 + longitude) / 360.0) * scale;
  x=(int)dx;
  xPixel=(short)((dx-x)*256);
}
void Position::setZ(unsigned int az){
  if(az<=20) z=az;
  setLatitude(latitude);
  setLongitude(longitude);
}
void Position::setZXY(int az,int ax,int ay,short xPix,short yPix){
  z=az;x=ax;y=ay;xPixel=xPix;yPixel=yPix;
  //qWarning("zxy,xpix,ypix: %d,%d,%d,%d,%d",z,x,y,xPixel,yPixel);
  longitude=360.0/(1<<z)*((double)x+(double)xPix/256.0)-180.0;
  //qWarning("longitude zu %g berechnet",longitude);
  double yy=((double)y+(double)yPix/256.0)/(1<<z);
  latitude=(2.0*atan(exp(M_PI*(1-2.0*yy)))-M_PI/2.0)*180.0/M_PI ;
}
double Position::distanceFrom(const Position &p2) const{
  double u=M_PI/180.0;
  return 6371000.2*acos(sin(getLatitude()*u)*sin(p2.getLatitude()*u) +
                         cos(getLatitude()*u)*cos(p2.getLatitude()*u)*cos(getLongitude()*u-p2.getLongitude()*u) );
}
double Position::meterPerPixel()const{
  return 4e7*cos(getLatitude()/180.0*M_PI)/(double)(1<<(z+8));
}
QPoint Position::getglobalPixelKoords(double lat, double lon, unsigned int z){
    Position p(lat,lon);
    p.setZ(z);
    return p.getglobalPixelKoords();
}

/*ostream& operator <<(ostream&s,const URegion reg){
  s.precision(8);
  s<<reg.name.latin1()<<'\t'<<reg.von.getLatitude()<<'\t'<<reg.von.getLongitude()<<'\t'<<
     reg.bis.getLatitude()<<'\t'<<reg.bis.getLongitude()<<'\n';
  return s;
}
istream& operator >>(istream & s,URegion & reg){
  char zeile[80];
  s.get(zeile,79,'\t');
  std::cerr<<"habe gelesen die Zeile: \'"<<zeile<<"\'\n";
  reg.name=zeile;
  double vlon,vlat,blon,blat;
  s>>vlat>>vlon>>blat>>blon>>ws;
  reg.von.setLatitude(vlat);reg.von.setLongitude(vlon);
  reg.bis.setLatitude(blat);reg.bis.setLongitude(blon);
  return s;
}*/
/*
QTextStream & operator >>(QTextStream& st,Waypoint & w){//Achtung. Als Zeileendezeichen ist nur 0x0A erlaubt.
  QString s,t;
  s=st.readLine();
  //qWarning("zuerst gelesen:");
  //cout<<s.latin1()<<endl;
  int pos;
  while(!s.contains("<Placemark>") && !st.atEnd()){
    s=st.readLine();
  }
  if(st.atEnd()){
    w.latitude=89.0;
    return st;
  }
  //qWarning("erste Zeile: %s",s.latin1());
  //jetzt so lange lesen bis </Placemark gefunden
  while(!s.contains("</Placemark>") && !st.atEnd()){
    s+=st.readLine();
    //qWarning("ergänzt: %s",s.latin1());
  }
  if(st.atEnd()){
    w.latitude=89.0;
    return st;
  }
  qDebug()<<"gefunden: "<<s;
  if((pos=s.indexOf("<name>"))==-1){//name nicht gefunden
    w.title="noName";
  }else
    w.title=s.mid(pos+6,s.indexOf("</name",pos+6,Qt::CaseInsensitive)-pos-6);
  if((pos=s.indexOf("<description>",0,Qt::CaseInsensitive))==-1){//description nicht gefunden
    w.description="";
  }else
    w.description=s.mid(pos+13,s.indexOf("</descri",pos+13,Qt::CaseInsensitive)-pos-13);
  if((pos=s.indexOf("<coordinates>",0,Qt::CaseInsensitive))==-1)
    w.latitude=w.longitude=0.0;
  else{
    QString coords=s.mid(pos+13,s.indexOf("</coordi",0,Qt::CaseInsensitive)-pos-13);
    QStringList l=coords.split(',');
    if(l.count()>=2){
      w.latitude=l[1].toDouble();
      w.longitude=l[0].toDouble();
    }else
      w.latitude=w.longitude=0.0;
  }
  if(w.isImage()){
      QPixmap p(w.imageFilename());
      if(p.isNull()){
          qDebug()<<"Konnte Bild "<<w.imageFilename()<<" nicht laden!";
          return st;
      }else
          qDebug()<<"Waypoint mit Image "<<w.imageFilename()<<" gefunden.";
      QSettings set("UBoss","karten");
      int size=set.value("waypoints/imageSize",120).toInt();
      w.setPixmap(p.scaled(size,size,Qt::KeepAspectRatio,Qt::SmoothTransformation));
  }
  return st;
}
*/
QString Waypoint::kmlvorspann="<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
   <kml xmlns=\"http://earth.google.com/kml/2.1\"\n\
           xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n\
     <Document>\n\
       <name>Uwes Wegmarken</name>\n\
       <Folder>\n\
           <name>Wegmarken</name>\n";
QString Waypoint::kmlnachspann="    </Folder>\n  </Document>\n</kml>";
QTextStream & operator <<(QTextStream & st,const Waypoint & w){
  st<<"<Placemark>\n  <name>"<<w.title<<"</name>\n";
  st<<"  <Snippet/>\n  <description>"<<w.description<<"</description>\n";
  st<<"  <Point>\n    <coordinates>"<<w.longitude<<","<<w.latitude<<","<<"0.0"<<"</coordinates>\n";
  st<<"   </Point>\n </Placemark>\n";
  return st;
}

void Waypoints::append(Waypoint p){
  *this<<p;
    isSaved=false;
}

void Waypoints::appendFromIODevice(QIODevice *device)
{
    QSettings set("UBoss","karten");
    int imageSize=set.value("waypoints/imageSize",120).toInt();
    QDomDocument doc;
    QString errorMsg; int errorLine,errorColumn;
    if(!doc.setContent(device,&errorMsg,&errorLine,&errorColumn)){
        QMessageBox::warning(0,"Doof",QString("Fehler beim Lesen der Waypoints, Zeile %1, Spalte %2: %3").arg(errorLine).
                    arg(errorColumn).arg(errorMsg));
        return;
    }
    QDomNodeList placemarks=doc.elementsByTagName("Placemark");
    for(int i=0;i<placemarks.count();i++){
        const QDomElement & pl=placemarks.at(i).toElement();
        Waypoint w;
        w.title=pl.firstChildElement("name").text();
        w.description=pl.firstChildElement("description").text();
        QString coords=pl.firstChildElement("Point").
                toElement().firstChildElement("coordinates").text();
        QStringList coordinates=coords.split(',');
        if(coordinates.count()<2){
            qDebug()<<"Koordinatenangabe defekt.";
            continue;
        }
        w.longitude=coordinates[0].toDouble();
        w.latitude=coordinates[1].toDouble();
        if(w.isImage()){
            QPixmap pm(w.imageFilename());
            if(pm.isNull()){
                qDebug()<<"Konnte Bild "<<w.imageFilename()<<" nicht laden!";continue;
            }else
                qDebug()<<"Waypoint mit Image "<<w.imageFilename()<<" gefunden.";
            w.setPixmap(pm.scaled(imageSize,imageSize,Qt::KeepAspectRatio,Qt::SmoothTransformation));
        }
        Waypointliste::append(w);
    }
}

QComboBox *Waypoints::getComboBox(QWidget*parent)
{
    QStringList liste;
    //qSort(*this);//Achtung, das scheint keine gute Idee zu sein.
    QComboBox * combo=new QComboBox(parent);
    for (int i=0;i<size();i++)  liste.append(at(i).title);
    qSort(liste);//Dadurch geraten die indizes durcheinander!
    combo->addItems(liste);
    return combo;
}

int Waypoints::indexOf(QString wpTitle, int searchFrom) const
{
    if(searchFrom>=size()) return -1;
    for (int i=searchFrom;i<size();i++)
        if(at(i).title==wpTitle) return i;
    return -1;
}


bool Waypoint::isImage() const
{
    return description.mid(0,6)=="Image:";
}

QString Waypoint::imageFilename() const
{
    if(!isImage()) return "";
    return description.mid(6);
}
