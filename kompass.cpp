#include "kompass.h"
#include "Atlas.h"
#include <QtGui>
#include <math.h>

Kompass::Kompass():QWidget(){}
Kompass::Kompass(QWidget*parent,short r,short a):QWidget(parent){
  radius=r;angle=a;
  resize(2*radius+40,2*radius+40);
  setAttribute(Qt::WA_DeleteOnClose);
  setMouseTracking(true);
}
void Kompass::moveTo(const QPoint p){//Mittelpunkt soll Koordinaten radius+20,radius+20 haben
  moveTo(p.x(),p.y());
}
void Kompass::moveTo(int x,int y){//bewegt die Mitte des Kompass zur gegebenen Stelle
  move(x-(radius+20),y-(radius+20));
}
void Kompass::setAngle(int a){
  angle=a;repaint();
}
void Kompass::setRadius(short r){
  QPoint mitte=QPoint(x()+radius+20,y()+radius+20);
  radius=r;
  resize(2*r+40,2*r+40);
  moveTo(mitte);
  repaint();
}
void Kompass::paintEvent(QPaintEvent *){
  QPainter paint(this);
  QPen pen(QColor(0,100,255,180));
  pen.setWidth(2);
  paint.setPen(pen);
  paint.drawEllipse(20,20,2*radius,2*radius);
  int R=radius+20;
  paint.drawLine(R,R,R*(1+sin(angle/180.0*M_PI)),R*(1-cos(angle/180.0*M_PI)));

}
void Kompass::mouseMoveEvent(QMouseEvent *e){
   QPoint  p=pos();
   p+=e->pos();
   QMouseEvent e1(QEvent::MouseMove,p,e->globalPos(),e->button(),e->buttons(),e->modifiers());
   ((Atlas*)parentWidget())-> mouseMoveEvent(&e1);
}
