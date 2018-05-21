#ifndef WAYPOINTDIALOG_H
#define WAYPOINTDIALOG_H
#include "ui_waypointdialog.h"
#include "koordinateneingabe.h"
#include "Atlas.h"
#include "region.h"
/** Der Dialog wird nichtmodal geÃ¶ffnet. Aus diesem Grunde bekommt er den Atlas und den Waypoint
  mitgeteilt. Ein Nullzeiger als Waypoint zeigt an, dass es sich um einen neuen handelt.
  Der Atlas wird gebraucht fÃ¼r den Kompass.
  Beim Beenden Ã¤ndert der Dialog die Waypointliste entsprechend ab.
  **/
class waypointDialog : public QDialog,public Ui::waypointDialog
{
  Q_OBJECT
public:
    waypointDialog(Atlas*atl=0,Waypoint *way=0);
    double getLatitude(){return kE.getLatitude();}
    double getLongitude(){return kE.getLongitude();}
    void setPosition(Position pos);//setzt die Position korrekt in alle Eingabefelder ein.
private:
    Koordinateneingabe kE;
    Atlas*atlas;//dient um den Atlas zu markieren, fÃ¼r den dieser Dialog zustÃ¤ndig ist
    Waypoint * waypoint;//markiert den Waypoint. Kann 0 sein, wenn es ein neuer ist.
  private slots:
    void on_okButton_clicked();
    void on_closeButton_clicked();
   void on_toggleButton_clicked();
   void on_tileCalcButton_clicked();
   void on_posCalcButton_clicked();
   void on_kompassButton_clicked();
   void on_centerButton_clicked();
   void on_deleteButton_clicked();
   void enableKompassButton();
   void on_luftlinieButton_clicked();
private slots:
   void nomatimAbfrage();
   void help()const{Helpsystem::showPage(QUrl("qrc:///hilfeseiten/track_waypoint.html"),"Hilfe zu Waypoints");}
public slots:
   void reject();
   void updateMouseDistance();
private:
   QSize si;
};

#endif // WAYPOINTDIALOG_H
