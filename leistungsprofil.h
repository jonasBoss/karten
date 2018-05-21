#ifndef LEISTUNGSPROFIL_H
#define LEISTUNGSPROFIL_H
#include <QList>
#include <QString>
#include <QTextStream>
class Leistungsdatum{
public:
    Leistungsdatum(){v=steigung=0;}
    Leistungsdatum(double slope,double speed){steigung=slope;v=speed;}
    double steigung;//Steigung der Strecke
    double v;//Geschwindigkeit in m/s
    bool operator<(const Leistungsdatum&l)const{return steigung<l.steigung;}
};
typedef QList<Leistungsdatum> Leistungsdaten;
class Leistungsprofil
{
public:
    Leistungsprofil();
    Leistungsprofil(QString aname,Leistungsdaten daten);
    int size()const{return data.size();}
    QString name;
    void setName(QString aname){name=aname;}
    void setData(Leistungsdaten dat){data=dat;}
    bool good();//zeigt ein gültiges Profil an.
    double getSpeed(double steigung) const;//liefert die Geschwindigkeit, die zur Steigung gehört durch Interpolation
     //zwischen den Datenpunkten. Bei zu kleinen Steigungen liefert er die Geschwindigkeit des ersten Datenpunktes
     //bei zu großen die, die zur gleichen Steiggeschwindigkeit führt wie die des letzten Datenpunktes.
    friend QTextStream & operator >> (QTextStream & st, Leistungsprofil & leist);
 /** Formatvorgaben für den TextStream:
  * Zeilen, die mit einer # beginnen, werden ignoriert.
  * erste Zeile wird als Name genommen. Dann müssen Zeilen der Form folgen:
  * steigung ; Geschwindigkeit
  * eine Leerzeile markiert das Ende des Datenbereiches, danach kann eine weitere Profildefinition folgen.
  * **/
    QString getDate(int i);
private:
    Leistungsdaten data;//enthält die Daten mit aufsteigenden Steigungen.
};


#endif // LEISTUNGSPROFIL_H
