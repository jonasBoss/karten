#ifndef TIMEDTRACK_H
#define TIMEDTRACK_H
#include "timedtrackpoint.h"
#include "track.h"
#include "tracktimer.h"
// #include "ui_tracktimer.h"
#include <QtCore>
class Atlas;
class TimedTrackDialog;
class TimedTrack : public QObject,public TimedPointList
{
    friend class TimedTrackDialog;
Q_OBJECT
private:
  static short count;//zählt die Tracks.
  QPixmap bild;
  Atlas * atlas;
  QLabel * bildLabel;//Label, das im Atlas gezeigt wird, und zu dem das bild gehört.
  QPoint diff;//Verschiebung in globalen Pixelkoordinaten Zoomstufe 20.
  QPoint tp; //Globale Pixelkoordinaten der Zoomstufe 20 des derzeitigen Trackpoints.
  QPoint tpatlas;//Atlaskoordinaten von tp mit Berücksichtigung von scale.
  int ticks;//soviele ticks linear um diff verschieben, dann neu berechnen.
  int timestamp;//timestamp + centis/100 zeigt Zeit in Sekunden an des Punktes tp.
  short centis;//s.o.
  int interval;//gibt an, wieviele Hundertstelsekunden zwischen zwei Ticks der Uhr vergehen. Diese Daten werden durch showAtTime übergeben.
  TimedTrack::const_iterator itnext;//iterator auf den nächsten Punkt der durch die Ticks erreicht werden wird.
  TimedTrackDialog * ttdialog;
  short shift;//20-atlas->getPos().getZ(): um soviele Bits müssen die globalen Pixelkoordinaten verschoben werden, damit es Atlaskoordinaten werden.
  double scale;//atlas->getScale();
  QPoint AtlasKoordinaten(const QPoint & p);//berechnet die Atlaskoordinaten eines Punktes p unter Berücksichtigung von Zoomstufe und scale:
    //Soll schneller sein als die entsprechende Funktion von Position
public:
  static TrackTimer * tracktimer;
  QDate date;   //Tag des Fluges
  QString pilot;//Name des Piloten
  TimedTrack();
  TimedTrack(Atlas*a);
  ~TimedTrack();
  bool readFromIGCFile(QTextStream &igc);//liest alle Einträge aus dem Stream;
  void setDate(const QDate&date);
  const QPixmap & getBild()const{return bild;}
  void setBild(QPixmap & b){bild=b;bildLabel->setPixmap(b);bildLabel->repaint();}
  QDateTime getFirstDateTime();//ermittelt ersten Zeitstempel
  QDateTime getLastDateTime();//ermittelt letzten Zeitstempel
  void transmitRange();//überträgt das Zeitfenster des Fluges an den Timer.
  void adjustHeight(int hoehe);
public slots:
  void showAtTime(const QDateTime &time, int inter=0);/*Zeigt das Bild im Atlas an der Position, die durch time gegeben ist.
                                            jeder kommende Aufruf von showNext() zeigt die Position inter/100 sekunden später.*/
  void showAtTime(int stamp, int inter=0);//
  void setInterval(int newinterval){interval=newinterval;}
  void showNext();
  void hide();//versteckt das Bild
  void repaint();//muss aufgerufen werden, wenn der Atlas sich neu zeichnet, verschiebt das Bild an die richtige Stelle.
  //void animate();//speed: soviele Millisekunden zwischen den Sekunden-Ticks.
};
#endif
