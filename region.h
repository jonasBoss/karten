#ifndef REGION_H
#define REGION_H
//#include "waypointdialog.h"
#include <iostream>
#include <QtGui>
#include "track.h"
typedef enum {N,S} nsRichtung;
typedef enum {O,W} owRichtung;
using namespace std;
class waypointDialog;
class Waypoint{
private:
  waypointDialog * wpdialog; //zeigt auf den zugehörigen Dialog, falls er existiert, sonst 0.
  QPixmap  pixmap; //speichert eine pixmap des Bildes, falls der Waypoint ein Bild zeigt.
public:
  Waypoint(){wpdialog=0;pixmap=0;}
  Waypoint(double lat,double lon){latitude=lat;longitude=lon;wpdialog=0;pixmap=0;}
  //~Waypoint(){qDebug()<<"Waypoint "<<title<<" futsch.";}
  void setWpDialog(waypointDialog*dial){wpdialog=dial;}
  waypointDialog * getWpDialog()const{return wpdialog;}
  void setPixmap(const QPixmap & p){pixmap=p;}
  const QPixmap & getPixmap()const{return pixmap;}
    QString title;
    QString description;
    double latitude;
    double longitude;
    static QString kmlvorspann;
    static QString kmlnachspann;
public:
    bool operator==(const Waypoint&wp){return (latitude==wp.latitude && longitude==wp.longitude && title==wp.title);}
    bool operator<(const Waypoint & wp)const{return title<wp.title;}
    bool isImage()const;//wenn der Waypoint auf ein Bild verweist.
    QString imageFilename()const; //liefert den Dateinamen incl. Pfad des Bildes.
};
/** Die Klasse Position berechnet die Pixelkoordinaten aus latitude und longitude in dem Augenblick,
 * wo über setZ die Methoden setLatitude und setLongitude aufgerufen werden **/

class Position{
private:
  double latitude,longitude;//breite und länge. Beides muss negativ sein, wenn Süd, bzw. West.
  int x,y,z;//Kacheldaten
  short xPixel,yPixel;//Pixel auf Kachel.
public:
  Position(double lat,double lon){latitude=lat;longitude=lon;}
  Position(double latD, double latM, double lonD, double lonM);
  Position(){latitude=longitude=0.0;x=y=128;z=0;}
  Position(const Trackpoint &p, int z=0);
  Position(const Waypoint &way, int z=0);
  double getLatitude() const{return latitude;}
  double getLongitude() const{return longitude;}
  int getLatDeg()const{return (int)latitude;}
  int getLonDeg()const{return (int)longitude;}
  double getLatMin()const{return 60*(latitude -(int) latitude);}//ist negativ, wenn südliche Hemisphäre
  double getLonMin()const{return 60*(longitude-(int)longitude);}//ist negativ, wenn westlich von 0°
  QPoint getglobalPixelKoords(){return QPoint(x*256+xPixel,y*256+yPixel);}//liefert globale Pixelkoordinaten beim Zoomfaktor z.
  static QPoint getglobalPixelKoords(double lat,double lon,unsigned int z=20);//liefert globale Pixelkoordinaten des Punktes in der Zoomstufe z.
  void setGlobalPixelKoords(const QPoint & p){setZXY(z,p.x()>>8,p.y()>>8,p.x()&255,p.y()&255);}
  void setLatitude(double deg,double min=0.0);
  void setLongitude(double deg,double min=0.0);
  void setZXY(int az,int ax,int ay,short xPix=0,short yPix=0);
  void setZ(unsigned int az);
  void getTileData(int&ax,int&ay,short&axPixel,short&ayPixel){
    ax=x;ay=y;axPixel=xPixel;ayPixel=yPixel;
  }
  int getX()const{return x;}
  int getY()const{return y;}
  int getZ()const{return z;}
  short getXPix()const{return xPixel;}
  short getYPix()const{return yPixel;}
  double distanceFrom(const Position & p2)const;//misst die Entfernung von p2 in Metern. r_Erde=6371 km
  double meterPerPixel()const;//wie viele Meter ist ein Pixel?
};
/*
class URegion{
  public:
  URegion(const QString n,const Position v,const Position b){name=n;von=v;bis=b;}
  //URegion(const char*n,const Position v,const Position b){name=QString(n);von=v;bis=b;}
  URegion(){name=QString("Region");}
  QString getName()const{return name;}
  Position getVon()const{return von;}
  Position getBis()const{return bis;}
  void setName(const char*aname){name=aname;}
  void setName(const QString aname){name=aname;}
  private:
  QString name;
  Position von,bis;
  friend ostream& operator <<(ostream& s,const URegion reg);
  friend istream& operator >>(istream & s,URegion & reg);
};
*/
//QTextStream & operator >>(QTextStream&s,Waypoint & w);
QTextStream & operator <<(QTextStream&s,const Waypoint & w);
typedef QList<Waypoint> Waypointliste;
class Waypoints : public Waypointliste{
  public:
  Waypoints():Waypointliste(){filename="";isSaved=true;}
  QString filename;
  void append(Waypoint p);
  void appendFromIODevice(QIODevice * device);//hängt alle WPs, die aus dem device zu lesen sind, hinten an.
  bool isSaved;
  QComboBox*getComboBox(QWidget *parent=0);//liefert eine Combobox mit
   //den nach Namen sortierten Waypoint-Titeln.
  int indexOf(QString wpTitle,int searchFrom=0)const;//liefert den ersten Index ab searchFrom mit dem Titel wpTitle
};

#endif
