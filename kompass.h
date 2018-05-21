#ifndef KOMPASS_H
#define KOMPASS_H

#include <QWidget>

class Kompass : public QWidget
{
  Q_OBJECT
private:
  short radius;//Radius in Pixeln des Kreises
  short angle;//Winkel in die ein Strahl gezeichnet wird.
public:
    Kompass();
    Kompass(QWidget*parent,short r,short a);
    void moveTo(const QPoint p);
    void moveTo(int x,int y);//bewegt die Mitte des Kompass zur gegebenen Stelle
  public slots:
    void setAngle(int a);
    void setRadius(short r);
    void setValues(short r,short a){setAngle(a);setRadius(r);}
  private:
    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent * e);
};

#endif // KOMPASS_H
/*** Idee zur Benutzung des Kompass:
  Im WaypointDialog gibt es einen Knopf zum Ãffnen des Kompass-Dialoges.
  ***/
