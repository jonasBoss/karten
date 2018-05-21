#include "leistungsprofil.h"
#include <QList>
#include <QtAlgorithms>
#include <QString>
#include <QStringList>


Leistungsprofil::Leistungsprofil()
{
    name="";
    data=Leistungsdaten();
}

Leistungsprofil::Leistungsprofil(QString aname, Leistungsdaten daten)
{
  name=aname;
  data=daten;
  qSort(data);
}

bool Leistungsprofil::good()
{
    return !name.isEmpty() && data.size()>0;
}

double Leistungsprofil::getSpeed(double steigung) const
{
    if(size()==0) return 1;
    int i=0;
    while(i<size()&&steigung >data.at(i).steigung){
        i++;
    }//i==0 heißt steigung <= kleinste steigung
     //0<i<size() heißt:steigung(i-1) < steigung <= steigung(i)
     //i=size() heißt: steigung>größte Steigung.
    if(i==0) return data.at(0).v;
    if(i<size()){
        const double & s0=data.at(i-1).steigung;
        const double & s1=data.at(i).steigung;
        double r=(steigung-s0)/(s1-s0);//zwischen 0 und 1
        return r*data.at(i).v+(1-r)*data.at(i-1).v;
    }
    const Leistungsdatum&letzter=data.last();
    return letzter.steigung/steigung * letzter.v;
}
QTextStream & operator >> (QTextStream & st, Leistungsprofil & leist)
{
    if(!leist.data.isEmpty()) leist.data.clear();
    leist.name="";
    QString s;
    s=st.readLine();
    while(!st.atEnd() &&(s.isEmpty()||s.at(0)=='#')) s=st.readLine();
    leist.name=s;
    s="#";
    while(!st.atEnd() && !s.isEmpty()){//Datenbereich beendet durch Leerzeile oder Streamende
        while(!st.atEnd() && !s.isEmpty() && s.at(0)=='#')
            s=st.readLine();
        if(s.trimmed().isEmpty()) break;//falls Leerzeichen in der Leerzeile sind.
        QStringList l=s.split(';'); //';' separiert Steigung von Gescwindigkeit
        if(l.size()<2) continue;
        bool ok;
        double slope=l.at(0).toDouble(&ok);
        if(!ok) continue;
        double speed=l.at(1).toDouble(&ok);
        if(!ok) continue;
        leist.data.append(Leistungsdatum(slope,speed));
        s="#";
    }
    if(!leist.data.isEmpty()) qSort(leist.data);
    return st;
}

QString Leistungsprofil::getDate(int i)
{
    if(i<0 || i>=size()) return QString("i out of range");
    return QString("Steigung: %1 -> Geschwindigkeit. %2").arg(data.at(i).steigung).arg(data.at(i).v);
}
