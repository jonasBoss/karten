#include "koordinateneingabe.h"
#include "region.h"
#include <math.h>
void Koordinateneingabe::setupUi(QWidget*parent){
  ui.setupUi(parent);
  QDoubleValidator * val;
  val=new QDoubleValidator(0.0,88.0,6,ui.YG);
  val->setNotation(QDoubleValidator::StandardNotation);
  ui.YG->setValidator(val);
  val=new QDoubleValidator(0.0,60.0,3,ui.YM);
  val->setNotation(QDoubleValidator::StandardNotation);
  ui.YM->setValidator(val);
  val=new QDoubleValidator(0.0,180.0,6,ui.XG);
  val->setNotation(QDoubleValidator::StandardNotation);
  ui.XG->setValidator(val);
  val=new QDoubleValidator(0.0,60.0,3,ui.XM);
  val->setNotation(QDoubleValidator::StandardNotation);
  ui.XM->setValidator(val);
  //parent->resize(parent->sizeHint());
  //parent->adjustSize();
}

double Koordinateneingabe::getLatitude(){
  double lat;
  lat=ui.YG->text().toDouble()+ui.YM->text().toDouble()/60.0;
  if(ui.NSCombo->currentItem()==1)
    return -lat;
  else
    return lat;
}
double Koordinateneingabe::getLongitude(){
  double lon;
  lon=ui.XG->text().toDouble()+ui.XM->text().toDouble()/60.0;
  if(ui.OWCombo->currentItem()==1)
    return -lon;
  else
    return lon;
}
Position Koordinateneingabe::getPosition(){
  return Position(getLatitude(),getLongitude());
}
void Koordinateneingabe::setLatitude(int latDeg,double latMin){
  ui.YG->setText(QString("%1").arg(abs(latDeg)));
  ui.YM->setText(QString("%1").arg(fabs(latMin),6,'f',3));
  ui.NSCombo->setCurrentItem((1.0*latDeg+latMin/60.0)<0?1:0);
}
void Koordinateneingabe::setLongitude(int lonDeg,double lonMin){
  ui.XG->setText(QString("%1").arg(abs(lonDeg)));
  ui.XM->setText(QString("%1").arg(fabs(lonMin),6,'f',3));
  ui.OWCombo->setCurrentItem((1.0*lonDeg+lonMin/60.0)<0?1:0);
}
void Koordinateneingabe::setLatitude(double latitude){
  setLatitude((int)latitude,(latitude-(int)latitude)*60.0);
}
void Koordinateneingabe::setLongitude(double longitude){
  setLongitude((int)longitude,(longitude-(int)longitude)*60.0);
}
