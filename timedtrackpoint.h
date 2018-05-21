#ifndef TIMEDTRACKPOINT_H
#define TIMEDTRACKPOINT_H
#include "region.h"
#include <QtCore>
class TimedTrackpoint{
    friend class TimedTrack;
public:
  double latitude;
  double longitude;
  int height; //Höhe über NN
private:
  QDateTime time;
  int timestamp;//Zeitstempel=Sekunden nach 1.1.2000, 0.00 Uhr
  void actualizeTimestamp();
public:
  TimedTrackpoint();
  TimedTrackpoint(double lat, double lon, int h, int time);
  TimedTrackpoint(QString igcstring,QDate date=QDate(2000,0,0));//erzeugt einen Trackpoint aus einem igc-String, der mit "B" beginnen muss.
  void setDate(const QDate &date);//ändert das Datum, nicht die Zeit.
  void addSeconds(int secs);
  QString print() const;
  static QDateTime zeroTime;
  static int getTimestamp(const QDateTime &t);
  static QDateTime getDateTime(int timestamp);
  QDateTime getTime() const{return time;}
  int getTimestamp() const{return timestamp;}
  QPoint getglobalPixelKoords(unsigned int z=20) const{return Position::getglobalPixelKoords(latitude,longitude,z);}
};
typedef QList<TimedTrackpoint> TimedPointList;


#endif // TIMEDTRACKPOINT_H
