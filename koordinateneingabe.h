#ifndef KOORDINATENEINGABE_H
#define KOORDINATENEINGABE_H
#include "ui_koordinateneingabe.h"
#include "region.h"
class Koordinateneingabe{
public:
  Ui::KoordinateneingabeUi ui;
  //Koordinateneingabe();
  double getLatitude();
  double getLongitude();
  void setLatitude(int latDeg,double latMin);
  void setLatitude(double latitude);
  void setLongitude(int lonDeg,double lonMin);
  void setLongitude(double longitude);
  Position getPosition();
  void setupUi(QWidget*parent);
};

#endif // KOORDINATENEINGABE_H
