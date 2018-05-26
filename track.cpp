#include "track.h"
#include "hoehenprofil.h"
#include "Atlas.h"
#include "hoehenabfrager.h"
#include <QtGui>
#include <QtXml/QDomDocument>
//#include <qcolor.h>
#include "region.h"
#include "cartesian.h"
Track::Track(Atlas *at)
{
  name="Track";
  description="";
  stored=true;
  currentIndex=-1;showPoints=false;
  hatHoehe=false;
  interpolated=false;
  atlas=at;
  popupMenu=0;profilButton=0;colorButton=0;
}

Track &Track::operator=(const Track &t)
{
    currentIndex=t.currentIndex;description=t.description;
    farbe=t.farbe;filename=t.filename;name=t.name;points=t.points;
    showPoints=t.showPoints;stored=t.stored;atlas=t.atlas;hatHoehe=t.hatHoehe;
    interpolated=t.interpolated;length=t.length;popupMenu=t.popupMenu;profilButton=t.profilButton;
    colorButton=t.colorButton;
    return *this;
}
void Track::loadFromGPX(QString afilename){
  if(afilename.isEmpty()) return;
  showProfile(false);
  filename=afilename;
  if(points.count()>0 &&
          QMessageBox::question(0,"Frage",trUtf8("Sollen die Trackpunkte verworfen werden? Bei Nein: neue Punkte werden hinter aktuellen eingefügt"),
                                QMessageBox::Yes,QMessageBox::No)==QMessageBox::Yes){
    points.clear();
  }
  //QSettings settings("UBoss","karten");
  QFile f(filename);
  //dir=QFileInfo(f).path();
  if(!f.open(QIODevice::ReadOnly)){
    filename="";
    emit filenameChanged(filename);
    QMessageBox::warning(0,"Mist!",trUtf8("kann datei %1 mit Wegpunkten nicht öffnen!").arg(filename));
    return;
  }
  QDomDocument doc;
  QString errString;
  if(!doc.setContent(&f,&errString)){
      QMessageBox::warning(0,"Mist",trUtf8("Probleme beim Lesen von %1: %2").arg(filename).arg(errString));
      filename=""; emit filenameChanged(filename);
      return;
  }
  //qDebug()<<"Track gelesen: "<<doc.toString();
  QDomNodeList tracks=doc.elementsByTagName("trk");
  if(tracks.isEmpty()){
      QMessageBox::warning(0,"Schade",trUtf8("Leider kein Tag <trk> gefunden"));
      return;
  }
  const QDomElement & trk = tracks.at(0).toElement();
  name=trk.firstChildElement("name").text();//könnte null sein.
  if(name.isNull()) name="not found";
  emit(nameChanged(name));
  description=trk.firstChildElement("description").text();
  emit(descriptionChanged(description));
  //qDebug()<<"Track gefunden: "<<name<<description;
  const QDomNodeList & trkptliste=trk.elementsByTagName("trkpt");
  for(int i=0; i<trkptliste.count();i++){
      const QDomElement & trkpt = trkptliste.at(i).toElement();
      double lat=trkpt.attribute("lat","0").toDouble();
      double lon=trkpt.attribute("lon","0").toDouble();
      double alt=trkpt.firstChildElement("ele").text().isEmpty()?0:
                 trkpt.firstChildElement("ele").text().toDouble();
      if(alt!=0) hatHoehe=true;
      points.append(Trackpoint(lat,lon,alt));
      //qDebug()<<"punkt "<<latitude<<longitude<<altitude;
  }

  stored=true;
  currentIndex=points.count()-1;
  calcLength();
  emit(toSave(false));emit(paintMe());
}

void Track::loadFromJSON(QString filename)
{
   QFile f(filename);
   f.open(QIODevice::ReadOnly);
   QTextStream str(&f);
   QString zeile;
   while(!str.atEnd()){
       int pos;
       zeile=str.readLine();
       if((pos=zeile.indexOf("elevation"))!=-1){
           pos=zeile.indexOf(':',pos);
           double altitude=zeile.mid(pos+1,zeile.indexOf(',',pos+1)-pos-1).toDouble();
           do{
               zeile=str.readLine();
           }while((pos=zeile.indexOf("lat"))==-1);
           pos=zeile.indexOf(':',pos);
           double latitude=zeile.mid(pos+1,zeile.indexOf(',',pos+1)-pos-1).toDouble();
           do{
               zeile=str.readLine();
           }while((pos=zeile.indexOf("lng"))==-1);
           pos=zeile.indexOf(':',pos);
           double longitude=zeile.mid(pos+1).toDouble();
           //qDebug()<<"Punkt gelesen: "<<latitude<<longitude<<altitude;
           insertPoint(latitude,longitude,altitude);
        }
       hatHoehe=true;
   }
   f.close();
}
void Track::storeToGPX(){
    if (filename.length()==0){
        filename=QFileDialog::getSaveFileName(0,"GPX-Pfad speichern",".", "gpx-Dateien (*.gpx)" );
        if(filename.isEmpty()){
            QMessageBox::warning(0,"Achtung","Track wurde nicht gespeichert.");
            return;
        }
        emit(filenameChanged(filename));
    }
    QFile f(filename);
    if(!f.open(QIODevice::WriteOnly)){
      QMessageBox::warning(0,"Mist!",trUtf8("kann datei nicht öffnen!"));
      return;
    }
    QTextStream out(&f);
    out<<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"<<
                 "<gpx version=\"1.0\">\n"<<
                 "<trk>\n"<<
         "  <name>"<<name<<"</name>\n  <description>"<<description<<"</description>\n<trkseg>\n";
    for (int i=0; i<points.count();i++){
        out<<"<trkpt lat=\""<<points[i].latitude<<"\" lon=\""<<points[i].longitude<<"\">\n";
        if(points[i].altitude!=0.0)
           out<<"  <ele>"<<points[i].altitude<<"</ele>\n";
        out<<"</trkpt>\n";
    }
    out<<"</trkseg>\n</trk>\n</gpx>";
    stored=true;
    emit(toSave(false));
    f.close();
}

void Track::loadAction(QAction *action)
{
    if(action==0) return;
    QSettings settings("UBoss","karten");
    QString defaultdir=settings.value("track/directory",QString("")).toString();
    filename=action->text();
    if(action->shortcut()==QKeySequence(Qt::Key_0)){//neue Datei laden
      QFileDialog fd(0,"Waypoint-Datei",defaultdir,"gpx-Dateien (*.gpx)");
      fd.setFileMode(QFileDialog::ExistingFile);
      QList<QUrl> sbu;
      for(QString & dir:fd.history())
          sbu<<QUrl(QString("file://")+dir);
      fd.setSidebarUrls(sbu);//so wird die Liste der zuletzt geöffneten Verzeichnisse in die Sidebar übertragen.
      fd.setLabelText(QFileDialog::FileName,"Track-Datei:");
      if(fd.exec()!=QDialog::Accepted){
          return;
      }
      filename=fd.selectedFile();
      if(filename.isNull())
              return;
    }
    QStringList history=settings.value("track/history",QStringList()).toStringList();
    int historySize=settings.value("track/historySize",-1).toInt();
    if(historySize==-1){ settings.setValue("track/historySize",6); historySize=6;}
    int i;
     if((i=history.indexOf(QRegExp(QString("*")+filename,Qt::CaseSensitive,
                                   QRegExp::WildcardUnix)))==-1){//neue Datei
        history.prepend(filename);
        while(history.count()>historySize) history.removeLast();
     }else{
        filename=history.takeAt(i);
        history.prepend(filename);
     }//filename enthält vollen Pfad zur zu ladenden Datei.
     settings.setValue("track/history",history);
     QString dir=QFileInfo(QFile(filename)).path();
     settings.setValue("track/directory",dir);
     actualizePopupMenu();
     emit(filenameChanged(filename));
     loadFromGPX(filename);
}
void Track::clear(){
  points.clear();stored=false;currentIndex=-1;
  filename="";

  emit(toSave(false));emit(paintMe());
  emit filenameChanged(filename);
  calcLength();
}
void Track::deletePoint(int i){
  if(points.count()==0) return;
  if(i==-1){
      points.removeAt(currentIndex);
      if(currentIndex>0) currentIndex--;
    }else{
      points.removeAt(i);
      if(currentIndex>=i) currentIndex--;
    }
  stored=false;
  calcLength();
  emit(toSave(true));emit(paintMe());
}
void Track::revert(){
  if (points.count()<2)
    return;
  int last=points.count()-1;
  for (int i=0;i<points.count()/2;i++){
      points.swap(i,last-i);
    }
  if(showPoints) emit(paintMe());
  emit profileChanged();
}

void Track::changeName(QString aname){
  name=aname;
  stored=false;emit(toSave(true));
}
void Track::changeDescription(QString adesc){
    description=adesc;stored=false;emit(toSave(true));
}

void Track::showProfile(bool show)
{
    if(profilButton==0) return;
    if(show){
       if (points.size()==0){
           QMessageBox::warning(0,"Achtung",trUtf8("kein Pfad definiert, dessen Höhenprofil hier gezeigt werden könnte."));
           profilButton->setChecked(false);
           return;
       }
       QLayout * lay=((QWidget*)(atlas->parent()))->layout();
       Hoehenprofil*profil=new Hoehenprofil(profilButton,0,this);
       //profil->resize(atlas->width(),50);
       lay->addWidget(profil);
    //   QPoint point(atlas->x(),atlas->y()-30);
    //   profil->move((atlas->mapToGlobal(point)));
       connect(profil,SIGNAL(mouseMoved(double)),atlas,SLOT(markTrack(double)));
       connect(this,SIGNAL(paintMe()),profil,SLOT(repaint()));
       profil->show();profil->repaint();
       connect(this,SIGNAL(profileChanged()),profil,SLOT(repaint()));
       connect(this,SIGNAL(hideProfile()),profil,SLOT(close()));
       //connect(profil,SIGNAL(destroyed(QObject*)),profilButton,SLOT(toggle()));
    }else{
        //profilButton->toggle();
        //profilButton->setChecked(false);
        emit(hideProfile());
    }
}

void Track::getAltitudes()
{
    if(points.size()==0){
        QMessageBox::warning(0,"Achtung",trUtf8("Keine Höhen abzufragen, alle bekannt"));
        return;
    }
    Hoehenabfrager * frager;
    if(!storesAltitudes()){
      frager=new Hoehenabfrager(this);
      connect(frager,SIGNAL(habeFertig()),this,SLOT(hoehenErmittelt()));
    }else
    {
        Integerliste list;
        for (int i=0;i<points.size();i++){
            if (points.at(i).altitude==0.0)
                list<<i;
        }
        if(list.isEmpty()){
            QMessageBox::warning(0,"Achtung",trUtf8("Keine Höhen abzufragen, alle bekannt"));
            return;
        }
        QMenu menu(trUtf8("Höhen ermitteln durch..."));
        QAction *act1=menu.addAction("Abfrage bei Google");
        QAction *act2=menu.addAction("Interpolation");
        QAction*choice=menu.exec(QCursor::pos(),act1);
        if(choice==act1){
            frager=new Hoehenabfrager(this,list);
            connect(frager,SIGNAL(habeFertig()),this,SLOT(hoehenErmittelt()));
        }else if (choice==act2){
            interpolateAltitudes(list);
            emit profileChanged();
        }else
            QMessageBox::warning(0,"Achtung",trUtf8("Es werden keine Höhendaten ermittelt"));
    }

}

void Track::hoehenErmittelt()
{
    emit profileChanged();
    emit toSave(true);
    hatHoehe=true;
    QMessageBox * mb=new QMessageBox(QMessageBox::Information,"Super!",trUtf8("Die Höhendaten wurden ermittelt!"));
    mb->resize(mb->sizeHint());
    mb->show();
    QTimer::singleShot(2000,mb,SLOT(deleteLater()));

}

void Track::help()
{
    Helpsystem::helpsystem->help("/hilfeseiten/track_waypoint.html","Tracks und Waypoints");
}


void Track::insertPoint(double lat, double lon, double alt, int after){
  /*if(alt==0.0 && hatHoehe){//Höhe wird durch Interpolation ermittelt
      int j=(after==-1)? currentIndex:after;
       if(j==points.length()-1)
             alt=points.at(currentIndex).altitude;
       else{
             const Trackpoint & p1=points.at(j);
             const Trackpoint & p2=points.at(j+1);
             double rel=(p1.latitude==p2.latitude)?
                         (lat-p1.latitude)/(p2.latitude-p1.latitude):
                         (lon-p1.longitude)/(p2.longitude-p1.longitude);
             alt=rel*p2.altitude+(1-rel)*p1.altitude;
      }
      interpolated=true;
  }*/
  if(after==-1)
    points.insert(++currentIndex,Trackpoint(lat,lon,alt));
  else
    points.insert(after+1,Trackpoint(lat,lon,alt));
  stored=false;
  calcLength();//berechnet Länge und schickt Signal newLength.
  emit(paintMe());
  emit(toSave(true));
}
QString Track::printLength(double l){
  if(l>5000.0)
    return QString("%1 km").arg(l/1000);
  else
      return QString("%1 m").arg(l);
}
double lg(double x){return log(x)/log(10);}
int Track::round125(double x)
{
    if (x<=0.7) return 0;
    x*=1.3;
    double loga=lg(x);
    int ganz=loga;
    int delta=5;
    if (loga-ganz<=0.7)
        delta=2;
    if(loga-ganz<=0.3)
        delta=1;
    for(int i=0;i<ganz;i++) delta*=10;
    return delta;
}

Pointlist Track::join(Trackpoint from, Trackpoint to, ushort n)
{
    if (from.longitude==to.longitude){
        QMessageBox::information(0,"Schade",trUtf8("bei gleichen Längenkreisen ist das noch nicht implementiert."));
        return Pointlist()<<from<<to;
    }
    if(to.longitude<from.longitude){
        Trackpoint hilf=from;
        from=to;
        to=hilf;
    }
    if(n<=2) return Pointlist()<<from<<to;
    CartesianPoint p1(from);
    CartesianPoint p2(to);
    CartesianPoint normal=CartesianPoint::cross(p1,p2);
    double phiG=atan2(-normal.x,normal.y);//Längenwinkel des Schnittpunktes Großkreis mit Äquator.
    if(CartesianPoint::degree(phiG)>from.longitude) phiG-=M_PI;
    double cosgamma=normal.z/normal.length();//gamma:Winkel Großkreis mit Äquator.
    double tangamma=sqrt(1-cosgamma*cosgamma)/cosgamma;
    double phi;
    double beta;
    Pointlist liste;
    for (short i=0;i<n;i++){
        phi=CartesianPoint::rad(from.longitude)+i*CartesianPoint::rad((to.longitude-from.longitude))/(n-1);
        beta= atan(tangamma*sin(phi-phiG));
        liste.append( Trackpoint(CartesianPoint::degree(beta),CartesianPoint::degree(phi)));
    }
    return liste;
}

int Track::fahrzeit(const Leistungsprofil &profil,int von,int bis) const
{
    double zeit=0;
    if(von==0 && bis==0) bis=points.size()-1;
    Trackpoint  p1=points.at(von);
    for(int i=von+1 ; i<=bis;i++){
        const Trackpoint & p2=points.at(i);
        double dist=Position(p2).distanceFrom(Position(p1));
        double steigung=(p2.altitude-p1.altitude)/dist;
        double speed=profil.getSpeed(steigung);
        zeit+=dist/speed;
        p1=p2;
    }
    return (int)zeit;
}

void Track::actualizePopupMenu()
{
    popupMenu->clear();
    QSettings settings("UBoss","karten");
    QStringList history=settings.value("track/history",QStringList()).toStringList();
    int i=1;
    for(QString & hist:history){
        if(hist!=filename)
          // popupMenu->addAction(hist,this,SLOT(loadFromGPX()),0x30+i++);
          popupMenu->addAction(hist,0,0,0x30+i++);
    }
    popupMenu->addAction("andere Datei...",this,SLOT(loadFromGPX()),0x30);
}

void Track::interpolateAltitudes(const Integerliste &liste)
{
    if(liste.isEmpty()) return;
    for(int i : liste){
       int j=i;
       while(j>0 && points.at(j).altitude==0.0) j--;
       Trackpoint & vorig=points[j];
       j=i;
       while(j<points.size()-1 && points[j].altitude==0.0) j++;
       Trackpoint & danach=points[j];
       if(vorig.altitude==0.0) points[i].altitude=danach.altitude;
       else if(danach.altitude==0.0) points[i].altitude=vorig.altitude;
       else{
           Position p(points.at(i));
           double e=Position(vorig).distanceFrom(p)+p.distanceFrom(Position(danach));
           double rel=Position(p).distanceFrom(Position(vorig))/e;
           points[i].altitude=rel*danach.altitude+(1-rel)*vorig.altitude;
       }

    }
}
double Track::calcLength(){
    if(points.isEmpty()) return 0;
    length=calcLength(points.count()-1);
    emit(newLength(printLength(length)));
    return length;
}

double Track::calcLength(int toIndex)const{
    if(toIndex==-1) toIndex=points.count()-1;
    if(toIndex==0) return 0;
    if(toIndex>=points.count()) return length;
    double l=0;
    Position  p=Position(points.at(0));
    Position q;
    for(int j=1;j<=toIndex;j++){
        q=Position(points.at(j));
        l+=p.distanceFrom(q);
        p=q;
    }
    return l;
}

double Track::getAltitudeOf(int index)
{
    if(hatHoehe && index<points.length()){
       return points.at(index).altitude;
    }else return 0.0;
}

Trackpoint Track::getTrackpoint(double rel)
{
   if(rel>=1) rel=0.999999;
   if(rel<0) rel=0;
   double lookFor=rel*length;//die gesuchte Position
   double sum=0;
   double inc=0;
   int i=0;
   for(i=0;i<points.length()-1;i++){
       inc=Position(points.at(i)).distanceFrom(Position(points.at(i+1)));
       if(lookFor>= sum && lookFor<sum+inc) break;
       sum+=inc;
   }
   if(i>=points.length()-1) i=points.length()-2;
   double r=(lookFor-sum)/inc;//wo auf der Teilstrecke i nach i+1 liegt der Punkt
   double lat=(1.0-r)*points.at(i).latitude +r*(points.at(i+1).latitude);
   double lon=(1.0-r)*points.at(i).longitude+r*(points.at(i+1).longitude);
   double alt=(1.0-r)*points.at(i).altitude +r*(points.at(i+1).altitude);
   return Trackpoint(lat,lon,alt);
}


void Track::changeColor(QColor acolor){
  farbe=acolor;
  emit(paintMe());
}

void Track::changeQColor()
{
    farbe=QColorDialog::getColor(farbe,0,trUtf8("Farbe für den Track"),QColorDialog::ShowAlphaChannel);
    colorButton->setStyleSheet(QString("background-color:%1").arg(farbe.name()));
    emit(paintMe());
}
Tileset Track::getTileset(short z, short environ){
  Tileset set;
  if (points.count()<2) return set;
  Position pos1(points[0].latitude,points[0].longitude);pos1.setZ(z);
  QPoint p1=pos1.getglobalPixelKoords();
  for (int i=1;i<points.count();i++){//von Punkt i-1 bis zum Punkt i die Strecke abwandern.
    Position pos2(points[i].latitude,points[i].longitude);
    pos2.setZ(z);
    QPoint p2=pos2.getglobalPixelKoords();
    int x2=p2.x(); int y2=p2.y();
    int x1=p1.x(); int y1=p1.y();
    int deltax=x2-x1>0?x2-x1:x1-x2;
    int deltay=y2-y1>0?y2-y1:y1-y2;
    int xalt=0; int yalt=0;//speichert die zuletzt gefundenen Kachelwerte.
    if(deltax>deltay){//iteration über die x-Werte, y-Werte jeweils berechnen
        if (x2<x1){//p1 und p2 vertauschen
            int klo=x1;x1=x2;x2=klo;klo=y1;y1=y2;y2=klo;
        }
        for(int x=x1;x<x2;x++){//in der Iteration nun die globalen Pixelkoordinaten berechnen.
            int y=(y2-y1)*(x-x1)/(x2-x1)+y1;
            if((x>>8) != xalt || (y>>8) != yalt){
              for(short d=-environ;d<=environ;d++){//zur Kachelmenge hinzu
                set<<QString("%1\t%2\t%3\n").arg((x>>8)+d).arg(y>>8).arg(z);
                set<<QString("%1\t%2\t%3\n").arg(x>>8).arg((y>>8)+d).arg(z);
              }
              xalt=x>>8;yalt=y>>8;
            }
        }
    }else{//deltay > deltax: iteration über y-Werte, x-Werte werden berechnet.
        if (y2<y1){//p1 und p2 vertauschen
            int klo=x1;x1=x2;x2=klo;klo=y1;y1=y2;y2=klo;
        }
        for(int y=y1;y<y2;y++){//in der Iteration nun die globalen Pixelkoordinaten berechnen.
            int x=(x2-x1)*(y-y1)/(y2-y1)+x1;
            if((x>>8) != xalt || (y>>8) != yalt){
                qWarning("Kacheln um Pankt gewünscht");
              for(short d=-environ;d<=environ;d++){//zur Kachelmenge hinzu
                set<<QString("%1\t%2\t%3\n").arg((x>>8)+d).arg(y>>8).arg(z);
                set<<QString("%1\t%2\t%3\n").arg(x>>8).arg((y>>8)+d).arg(z);
              }
              xalt=x>>8;yalt=y>>8;
            }
        }
    }
    p1=p2;
  }
  return set;
}

int Track::indexBefore(double percentage, double*length1, double *length2) const
{
    if(percentage<=0) {
        if(length1!=0) *length1=0;
        return 0;
    }
    if(percentage>=1){
        if(length2!=0) *length2=length;
        return points.size()-1;
    }
    int i=1;
    double l=0;
    Position p1=Position(points.at(0));
    Position p2=p1;
    if(length1!=0)*length1=l;
    while(i<points.size() && l/length <= percentage){
        p2=Position(points.at(i));
        if(length1!=0)*length1=l;
        l+=p1.distanceFrom(p2);
        p1=p2;
        i++;
    }
    if(length2!=0) *length2=l;
    return i-2;
}

void Track::setShowPoints(int state){
  showPoints=(state==Qt::Checked);
  emit(paintMe());
}

Trackpoint::Trackpoint(const Position &p)
{
    latitude=p.getLongitude();longitude=p.getLongitude();altitude=0;
}
