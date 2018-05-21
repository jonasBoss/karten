#ifndef KOMPASSINTERFACE_H
#define KOMPASSINTERFACE_H
#include "ui_kompass.h"
#include "kompass.h"
#include <QDialog>
class Atlas;
class Waypoint;
class Kompassinterface : public QDialog,public Ui::kompassdaten
{
  Q_OBJECT
public:
    Kompassinterface(Kompass* k,Atlas*a,Waypoint*wp,QWidget *parent=0);
private:
    Kompass * kompass; //der fernzusteuernde Kompass
    Atlas* atlas; //Atlas zu dem der Kompass gehört
    Waypoint*waypoint;//Waypoint um den der Kompass gezeichnet wird.
  private slots:
    void updateRadius();
    void updateAngle();
    void on_closeButton_clicked();
    void repaintKompass();


};

#endif // KOMPASSINTERFACE_H
