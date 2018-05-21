#include "kompassinterface.h"
#include "ui_kompass.h"
#include "region.h"
#include "Atlas.h"
Kompassinterface::Kompassinterface(Kompass*k, Atlas*a, Waypoint * wp, QWidget*parent):QDialog(parent)
{
  setupUi(this);
  kompass=k;
  atlas=a;
  waypoint=wp;
  connect(this,SIGNAL(destroyed()),k,SLOT(close()));
  connect(direction,SIGNAL(valueChanged(int)),k,SLOT(setAngle(int)));
  connect(a,SIGNAL(viewportChanged()),this,SLOT(repaintKompass()));
  setAttribute(Qt::WA_DeleteOnClose);
}
void Kompassinterface::updateRadius(){
  int meter=radkm->value()*1000+radm->value();
  double meterPerPixel=atlas->meterPerPixel();
  short r=meter/meterPerPixel>600?0:meter/meterPerPixel;
  kompass->setRadius(r);
}
void Kompassinterface::updateAngle(){
  kompass->setAngle(direction->value());
}
void Kompassinterface::repaintKompass(){
    //qWarning("repaintKompass aufgerufen. ");
    bool vis;
    QPoint point=atlas->getPixelKoords(waypoint->latitude,waypoint->longitude,vis);
    point.setX(point.x()*atlas->getScale());point.setY(point.y()*atlas->getScale());
    //qWarning("Kompass nach %d,%d bewegt",point.x(),point.y());
    kompass->moveTo(point);
    updateRadius();
}

void Kompassinterface::on_closeButton_clicked(){
 /* kompass->close(); Diese Zeilen sind nicht nötig, da der Kompass das Signal zum Schließen automatisch erhält
            und sich dann auch wegen seines Attributs selbst löscht.
  delete kompass; */
  close();
  //delete this; nicht nötig, wegen WA_DeleteOnClose;
}
