#ifndef TRACK_H
#define TRACK_H
#include <QtGui>
#include "leistungsprofil.h"
//#include "region.h"
typedef QList<int> Integerliste;
class Atlas;
class QRadioButton;
class Position;
class Trackpoint {
public:
  Trackpoint(){latitude=longitude=altitude=0.0;}
  Trackpoint(double lat,double lon, double alt=0.0){
    latitude=lat;longitude=lon;altitude=alt;
  }
  Trackpoint(const Position & p);
  double latitude,longitude,altitude;
  bool operator ==(const Trackpoint&p2){return latitude==p2.latitude && longitude==p2.longitude;}
};
typedef QList<Trackpoint> Pointlist;
typedef QSet<QString> Tileset;

class Track: public QObject
{
      Q_OBJECT
public:
    Track(Atlas*at=0);
    Track & operator=(const Track & t);
    QString name;
    QString filename;//Dateiname, in der der Track gespeichert werden soll.
    QString description;
    bool  stored; //gibt an, ob aktuelle Version gespeichert ist
    QColor farbe; //Farbe, in der der Pfad gezeichnet wird.
    bool showPoints;//gibt an, ob die Punkte gezeichnet werden sollen.
    Pointlist points;
    int currentIndex; //index des Punktes, hinter den eingefügt werden soll.
    Tileset getTileset(short z,short environ);//erstellt eine Kachelliste wie in tilenumbers.txt mit
     //Kacheln aus der Zoomstufe z und um jeden Punkt in alle vier Richtungen environ kacheln.
    int indexBefore(double percentage, double *length1=0, double *length2=0)const;//liefert den letzte Punkt bei dem gerade noch nicht percentage
                    //der Gesamtstrecke sind. in length1 wird die Länge bis zu diesem Punkt zurückgegeben, in length2 die bis zum nächsten.
    void insertPoint(double lat,double lon,double alt=0.0,int after=-1);
    void setCurrentIndex(int ind){currentIndex=ind;if (showPoints) emit(paintMe());}
    double calcLength();//berechnet Gesamtlänge und passt this->length an;Emittiert das Signal mit neuer Länge.
    double calcLength(int toIndex)const;//berechnet Länge des Pfades in m bis zum Trackpoint mit Index toIndex.
    double getAltitudeOf(int index);//gibt die Höhe des i-ten Punktes aus, falls sie gespeichert ist, sonst 0.
    bool storesAltitudes()const{return hatHoehe;}
    void setAtlas(Atlas*at){atlas=at;}
    void setPopupMenu(QMenu*m){popupMenu=m;}
    Trackpoint getTrackpoint(double rel); //rel=0 die Position des ersten Trackpoints rel=1: des letzten Trackpoints. Sonst dazwischen.
    double getLength()const {return length;}
    void setHatHoehe(bool on=true){hatHoehe=on;}
    Atlas*getAtlas()const{return atlas;}
    void setProfilButton(QRadioButton*but){profilButton=but;}
    int fahrzeit(const Leistungsprofil & profil, int von=0, int bis=0)const;//Liefert die Fahrzeit bei gegebenem Leistungsprofil in Sekunden.
    void actualizePopupMenu();//aktualisiert das PopupMenu nach den Datein in settings.
    static QString printLength(double l);//wenn l>10000 dann wird l in km ausgegeben sonst in m. mit Einheit.
    static int round125(double x);//rundet x auf 1,2,5,10,20,50  u.s.w.
    static Pointlist join(Trackpoint from,Trackpoint to,ushort n);//verbindet from mit to durch n Punkte (n>=2)
private:
    bool hatHoehe;
    bool interpolated;//hat interpolierte Höhen, die noch abgefragt werden müssen.
    Atlas * atlas;//zeigt ggf. auf den Atlas, zu dem der Track gehört.
    double length;//speichert Länge des Pfades. Wird von calcLength() aktualisiert.
    QRadioButton * profilButton;
    QMenu*popupMenu;//zeigt auf Menu mit wählbaren Tracks (s. Atlas::trackMenu())
    void interpolateAltitudes(const Integerliste & liste);
public slots:
    void changeColor(QColor acolor);
    void setShowPoints(int state);//{showPoints=(state==Qt::Checked);emit(paintMe());}
    void storeToGPX();//speichert den Track in eine Datei.
    void storeToGPXAs(){filename=QString();storeToGPX();}
    void loadAction(QAction * action);//reagiert auf das Signal des popupMenus.
    void loadFromGPX(QString afilename="");//lädt aus einer Datei die Trackpoints.
    void loadFromJSON(QString filename);//lädt aus einer Datei die Daten wie sie Google nach der Höhenabfrage zurückgibt. Die Punkte
        //werden ggf. an bestehende angefügt.
    void clear();
    void deletePoint(int i=-1);
    void revert();//kehrt die Reihenfolge der Punkte im Pfad um. (damit man den Pfad in beide Richtungen ergänzen kann.
    void changeName(QString aname);
    void changeDescription(QString adesc);
    void showProfile(bool show);//Höhenprofil anzeigen
    void getAltitudes();//erfragt alle Höhen, die 0.0 sind bei Google und trägt sie ein.
    void hoehenErmittelt();//wird aufgerufen, wenn der Höhendownload fertig ist.
    void help();
signals:
    void nameChanged(QString);
    void filenameChanged(QString);
    void descriptionChanged(QString);
    void toSave(bool);
    void paintMe();
    void newLength(QString);//im String ist die Länge mit Einheit.
    void hideProfile();//wird gesendet, wenn das Höhenprofil geschlossen werden soll.
    void profileChanged();//wird gesendet, wenn das Höhenprofil sich geändert hat.
};

#endif // TRACK_H
