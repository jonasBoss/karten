#include "Atlas.h"
#include "ukartendownload.h"
#include "region.h"
#include "koordinateneingabe.h"
#include "waypointdialog.h"
#include "ui_trackdialog.h"
#include "upopupmenu.h"
#include "settingdialog.h"
#include "timedtrack.h"
#include "timedtrackdialog.h"
#include <QtGui>
#include <Q3VBoxLayout>
Atlas::Atlas():QWidget(){
  setAcceptDrops(true);
  bild=QPixmap();
  useBild=false;
  showWaypoints=true;
  QWidget * envelop=new QWidget(0,"envelop");
  QSettings settings("UBoss","karten");
  baseDir=settings.value("atlasdirectory").toString();
  if(baseDir.right(1)!="/") baseDir+="/";
  if(settings.contains("position/z") && settings.value("saveOnExit").toBool()){
    pos.setZXY(settings.value("position/z").toInt(),settings.value("position/x").toInt(),
               settings.value("position/y").toInt());
    envelop->setGeometry(10,10,settings.value("width").toInt(),settings.value("height").toInt()+20);
    setGeometry(0,0,settings.value("width").toInt(),settings.value("height").toInt());
  }else{
    pos.setZXY(0,0,0);
    envelop->setGeometry(10,10,800,620);
    setGeometry(0,0,800,600);
  }
  waypoints.filename=settings.value("waypoints/filename").toString();
  track.filename=settings.value("track/filename").toString();
  track.farbe=settings.value("track/farbe").value<QColor>();
  track.setAtlas(this);
  if(!waypoints.filename.isEmpty()) loadWaypoints(waypoints.filename);
  if(!track.filename.isEmpty())     track.loadFromGPX(track.filename);
  QVBoxLayout * lay =new QVBoxLayout(envelop);
  reparent(envelop,QPoint(0,0),true);
  lay->addWidget(this);
  setFocusPolicy(Qt::StrongFocus);
  bar=new QStatusBar(envelop,"statusBar");
  bar->setGeometry(0,height(),width(),20);
  bar->setMaximumHeight(20);
  posLabel = new QLabel(bar,"posLabel");
  posLabel->setText(trUtf8("Position nicht verfügbar"));
  hintLabel = new QLabel(this);//für die Waypoint-descriptions
  hintLabel->setAutoFillBackground(true);
  hintLabel->setFrameShape(QFrame::Box);
  bar->addWidget(posLabel,0,true);  
  lay->addWidget(bar);
  envelop->show();
  scale=1.0;

  setMouseTracking(true); //damit wird das MouseMoveEvent immer ausgelöst!
  mouseRect.setRect(0,0,0,0);
  downloadDialog = 0;
  trackDialog=0;
  showWaypoints=true; markArea=false;
  QAction * act;
  popup=new QMenu(this);
  act=popup->addAction(trUtf8("Kartenverzeichnis ändern"),this,SLOT(changeBaseDir()),Qt::Key_K);
  addAction(act);
  act=popup->addAction("Neu zeichnen",this,SLOT(newPaint()),Qt::Key_R);
  addAction(act);
  /**  Hier kommt jetzt das Waypointmenu ********************************************************/
  QMenu * waypointMenu = new QMenu("Wegpunkte und Pfade...",popup);
  act=waypointMenu->addAction("Wegmarken und Tracks verstecken",this,SLOT(hideAndShowWaypoints()),Qt::Key_H);
  act->setCheckable(true);
  addAction(act);
  act=waypointMenu->addAction("Wegmarke erstellen/editieren",this,SLOT(addWaypoint()),Qt::Key_W);
  addAction(act);
  act=waypointMenu->addAction("Wegmarken aus Datei laden",this,SLOT(loadWaypoints()),Qt::Key_L);
  addAction(act);
  waypointMenu->addAction("Wegmarken in Datei Speichern",this,SLOT(saveWaypoints()));
  waypointMenu->addAction(trUtf8("Wegmarkendatei schließen"),this,SLOT(closeWaypointFile()));
  act=waypointMenu->addAction(trUtf8("Wegmarkenliste anzeigen"),this,SLOT(showWaypointCombo()),Qt::Key_I);
  addAction(act);
  act=waypointMenu->addAction(trUtf8("Wegmarke/Pfadpunkt löschen"),this,SLOT(deleteWaypoint()),Qt::Key_Delete);
  addAction(act);
  act=waypointMenu->addAction("Track-menu...",this,SLOT(trackMenu()),Qt::Key_T);
  addAction(act);
  //act=waypointMenu->addAction("Track speichern in GPX-Datei",this,SLOT(saveTrack()));
  act=waypointMenu->addAction("Track erweitern",this,SLOT(expandTrack()),Qt::Key_X);
  addAction(act);
  act=waypointMenu->addAction("Kacheln um Track herum laden...",this,SLOT(loadTrackEnvironment()));
  popup->addMenu(waypointMenu);
  /** Ende Waypointmenu ***********************************************************************/
  /** Beginn TimedTracksMenu ******************************************************************/
  QMenu * timedTrackMenu = new QMenu("Timed Tracks...",popup);
  timedTrackMenu->addAction("Timed Track laden",this,SLOT(addTimedTrack()));
  timedTrackMenu->addAction("TrackTimer-Uhr anzeigen",this,SLOT(showTrackTimer()));
  popup->addMenu(timedTrackMenu);
  /** Ende TimedTrackMenu *********************************************************************/
  popup->addAction(trUtf8("diese Kachel löschen"),this,SLOT(deleteTileAtMousePoint()));
  popup->addAction(trUtf8("Koordinaten in Zwischenablage übertragen"),this,SLOT(clip()),0);
  popup->addSeparator();
  act=(popup-> addAction("Beenden",qApp,SLOT(quit()),Qt::CTRL+Qt::Key_Q));
  act->setShortcutContext(Qt::ApplicationShortcut);
  addAction(act);
  popup->addSeparator();
  act=popup->addAction("Hilfe",this,SLOT(help()),Qt::Key_F1);
  act->setShortcutContext(Qt::WidgetShortcut);
  addAction(act);
  popup->addAction("Einstellungen",this,SLOT(settings()));
  connect(&track,SIGNAL(paintMe()),this,SLOT(repaint()));

}

void Atlas::paintEvent(QPaintEvent*){
  if(markArea){//wenn Mausrechteck zu zeichnen ist
    QPainter paint(this);
    paint.drawPixmap(-pos.getXPix()*scale,-pos.getYPix()*scale,bild);//hier neg. Koordinaten.
    paint.setPen(Qt::black);
    paint.setCompositionMode(QPainter::CompositionMode_Xor);
    //paint.drawRect(mouseRect.normalize());
    mouseRect.setBottomRight(newMousePoint);
    paint.drawRect(mouseRect.normalize());
    //border=nix;
    paintMore(paint);//Waypoints und tracks.
    return;
  }
  bool allesNeu=( !useBild || bildxy.isEmpty() || bildz!=pos.getZ());
  // wenn allesNeu=true ist, dann muss komplett alles neu geladen werden. ansonsten nur der neue Teil des Bildes.
  //benötigtes Kachelqrect berechnen:
  QPoint origo=pos.getglobalPixelKoords();
  //QSize s=size();
  QPoint ende=origo +(QPoint(width(),height())/scale); //QPoint((int)(width()/scale),(int)(height()/scale));
  QSize s((ende.x()>>8)-(origo.x()>>8)+1, (ende.y()>>8)-(origo.y()>>8)+1 );//so viele Kacheln breit und hoch ist das bild
  QRect noetig(QPoint(pos.getX(),pos.getY()),s );//diese Kacheln werden benötigt zur Darstellung
  QRect intersect=noetig & bildxy;//diese Kacheln können verwendet werden
  allesNeu=intersect.isEmpty() || allesNeu;
  if(!allesNeu && noetig==bildxy){
      QPainter paint(this);
      paint.drawPixmap(-pos.getXPix()*scale,-pos.getYPix()*scale,bild);//hier neg. Koordinaten.
      paintMore(paint);
      return;
  }
  QPainter paint;
  QPixmap buff(noetig.width()*256*scale,noetig.height()*256*scale);//muss größer sein.
  paint.begin(&buff,this);
  paint.setRenderHint(QPainter::SmoothPixmapTransform);
  /* in diesen Puffer muss nun der brauchbare Teil intersect hineinkopiert werden, und dann alle weiteren Gebiete
    mit neuen Kacheln gefüllt werden. Die weiteren Gebiete teilen sich in zwei Rechtecke auf. neu1 und neu2.
    so dass neu1, neu2 und intersect das ganze noetige Rechteck ausfüllen*/
  if(allesNeu){
      //qWarning("alles neu...");
      paint.scale(scale,scale);//im Puffer wird bereits skaliert gezeichnet.
      for (int x=noetig.x();x<=noetig.right();x++){
        for (int y=noetig.y();y<=noetig.bottom();y++){
          QPixmap tile(QString(baseDir+"%1/%2/%3.png.tile").arg(pos.getZ()).arg(x).arg(y));
          if(tile.isNull()){
            paint.setPen(Qt::red);
            paint.setBrush(Qt::lightGray);
            paint.drawRect((x-pos.getX())*256,(y-pos.getY())*256,256,256);
          }
          else paint.drawPixmap((x-pos.getX())*256,(y-pos.getY())*256,tile);
        }
      }
  }else{//nicht alles neu
      QRect neu1=noetig; QRect neu2=noetig;//neu1 geht über die volle x-Breite.
      if(intersect.y()==noetig.y()){//unten ist volle breite dranzuladen
          neu1.setTop(intersect.bottom()+1);
          neu2.setBottom(intersect.bottom());
      }else{//oben fehlt volle breite,
          neu1.setHeight(intersect.y()-noetig.y());
          neu2.setTop(intersect.top());
      }
      if(intersect.x()==noetig.x()){//neuer Bereich rechts
          neu2.setLeft(intersect.right()+1);
      }else{//neuer Bereich links
          neu2.setWidth(intersect.x()-noetig.x());//kann 0 sein
      }
      //jetzt intersect mit altem Bild füllen und neu1 und neu2 aus Datei laden. Damit buff beglücken.
      //intersect nun in die Koordinaten innerhalb von bild umrechnen.
      QPoint sourceOffset=intersect.topLeft()-bildxy.topLeft();//Offset des zu kopierenden Bereiches in bild in Kachelzahlen.
      QPoint targetOffset=intersect.topLeft()-noetig.topLeft();//Offset in buff.
      paint.drawPixmap(targetOffset*256*scale, bild, QRect(sourceOffset*256*scale,intersect.size()*256*scale));
      //jetzt noch die neuen Bereiche füllen mit Kacheln aus Datei.
      paint.scale(scale,scale);
      for (int x=neu1.x();x<=neu1.right();x++){
        for (int y=neu1.y();y<=neu1.bottom();y++){
          QPixmap tile(QString(baseDir+"%1/%2/%3.png.tile").arg(pos.getZ()).arg(x).arg(y));
          if(tile.isNull()){
            paint.setPen(Qt::red);
            paint.setBrush(Qt::lightGray);
            paint.drawRect((x-pos.getX())*256,(y-pos.getY())*256,256,256);
          }
          else paint.drawPixmap((x-pos.getX())*256,(y-pos.getY())*256,tile);
        }
      }
      for (int x=neu2.x();x<=neu2.right();x++){
        for (int y=neu2.y();y<=neu2.bottom();y++){
          QPixmap tile(QString(baseDir+"%1/%2/%3.png.tile").arg(pos.getZ()).arg(x).arg(y));
          if(tile.isNull()){
            paint.setPen(Qt::red);
            paint.setBrush(Qt::lightGray);
            paint.drawRect((x-pos.getX())*256,(y-pos.getY())*256,256,256);
          }
          else paint.drawPixmap((x-pos.getX())*256,(y-pos.getY())*256,tile);
        }
      }
  }
  paint.end();
  paint.begin(this);
  paint.drawPixmap(-pos.getXPix()*scale,-pos.getYPix()*scale,buff);
  paintMore(paint);//Wegpunkte und Pfade
  paint.end();
  bild=buff;
  bildxy=noetig;bildz=pos.getZ();
  bar->clearMessage();
  bar->showMessage(QString("Z:%1, X:%2, Y:%3, scale:%4").arg(pos.getZ()).arg(pos.getX()).
          arg(pos.getY()).arg(scale));
  useBild=true;
  //QWidget::paintEvent(e);
}//ende paintEvent

void Atlas::paintMore(QPainter&paint){
    paintMassstab(paint);
  if(!showWaypoints){
    if(waypoints.count()+track.points.count() > 0){
      QRectF bR=paint.boundingRect(10,10,0,0,Qt::AlignLeft,"Wegpunkte unsichtbar!");
      paint.fillRect(bR,QColor(243,229,130));
      paint.setPen(Qt::red);
      paint.drawText(bR,Qt::AlignLeft,"Wegpunkte unsichtbar!");
    }
    return;
  }
  paint.scale(scale,scale);
  /** Jetzt Die Waypoints zeichnen ****************************************************/
  if(waypoints.count()>0){
    paint.setBrush(Qt::red);
    paint.setPen(Qt::darkRed);
    for(Waypoints::const_iterator lit=waypoints.begin();lit!=waypoints.end();lit++){
      bool vis;
      QPoint p=getPixelKoords(lit->latitude,lit->longitude,vis);
      if(vis){
        if(! lit->isImage()){
            paint.drawEllipse(p,(int)(5/scale),(int)(5/scale));
            //paint.setBackgroundMode(Qt::OpaqueMode);
            //paint.setBrush(QColor(243,229,130));
            paint.setPen(Qt::black);
            QRectF bR=paint.boundingRect(QRectF(p.x()+7/scale,p.y(),0,0),Qt::AlignLeft,lit->title);
            paint.fillRect(bR,QColor(243,229,130));
            paint.drawText(bR,Qt::AlignLeft,lit->title);
        }else{
            paint.drawPixmap(p,lit->getPixmap());
        }
      }
    }
  }
  /** Jetzt den Track zeichnen ****************************************************/
  if(track.points.count()>1){
      paint.setRenderHint(QPainter::Antialiasing);
    QPen pen=paint.pen();
    pen.setWidth((int)(2.0/scale+0.5));
    pen.setColor(track.farbe);
    paint.setPen(pen);
    bool vis1,vis2;
    QPoint p1=getPixelKoords(track.points[0].latitude,track.points[0].longitude,vis1);
    for(int i=1;i<=track.points.count();i++){
      if(vis1 && track.showPoints){//Punkte sind zu zeichnen
        //qWarning("Soll Trackpunkte zeichen!\n");
        pen.setStyle(Qt::SolidLine);
        paint.setPen(pen);
        if((i-1)==track.currentIndex){//Punkt ausgehöhlt zeichnen
            QPointF p[5]={ QPointF(0,4/scale),QPointF(4/scale,0),QPointF(0,-4/scale),QPointF(-4/scale,0),QPointF(0,4/scale) };
            for (int i=0;i<5;i++) p[i]+=p1;
            //pen.setWidth(1/scale);
            paint.drawPolyline(p,5);
          //paint.setBrush(Qt::NoBrush);
        }else{
            paint.setBrush(QBrush(track.farbe,Qt::SolidPattern));
            paint.drawRect(QRectF(p1.x()-3/scale,p1.y()-3/scale,6/scale,6/scale));
        }
      }//Ende Punkte Zeichnen.
      if(i==track.points.count()) break;
      QPoint p2=getPixelKoords(track.points[i].latitude,track.points[i].longitude,vis2);
      if(vis1||vis2){
        if(track.currentIndex==i-1){
            pen.setStyle(Qt::DotLine);
            paint.setPen(pen);
            paint.drawLine(p1,p2);
            pen.setStyle(Qt::SolidLine);paint.setPen(pen);
        }
        else paint.drawLine(p1,p2);
      }
      p1=p2;
      vis1=vis2;
    }
  }
}
void Atlas::paintMassstab(QPainter &p){
    //qDebug()<<"paintMassstab...";
   p.setPen(Qt::black);
   p.setBrush(QColor(192,192,255,192));
   p.setWorldMatrix(QMatrix());
   QPoint start=QPoint(20,height()-20);
   QPoint ende=start+QPoint(256,0);
   Position posi=getMousePosition(start-QPoint(0,25));
   double mpp=posi.meterPerPixel()/scale;
   //qDebug()<<"posi hat z "<<posi.getZ()<<"mpp "<<mpp;
   p.drawLine(start,ende);
   p.drawLine(start-QPoint(0,25),start);
   p.drawLine(ende-QPoint(0,25),ende);
   QRect textRect=QRect(start-QPoint(0,20),QSize(256,20));
   p.drawRect(textRect);
   p.drawText(textRect,Qt::AlignCenter,Track::printLength(mpp*256));
   /*** jetzt die gerundete Länge ***/
   int rund=Track::round125(mpp*256);
   int pixel=1.0*rund/mpp;//so viele Pixel für den gerundeten Wert
   start+=QPoint(286,0);
   ende=start+QPoint(pixel,0);
   p.drawLine(start,ende);
   p.drawLine(start-QPoint(0,25),start);
   p.drawLine(ende-QPoint(0,25),ende);
   textRect=QRect(start-QPoint(0,20),QSize(pixel,20));
   p.drawRect(textRect);
   p.drawText(textRect,Qt::AlignCenter,Track::printLength(rund*1.0));

}

void Atlas::repaint(){
    QWidget::repaint();
    emit viewportChanged();
}

Position Atlas::getMousePosition(QPoint mouse){
  Position mousePos;
  mousePos.setZ(pos.getZ());
  mousePos.setGlobalPixelKoords(pos.getglobalPixelKoords()+mouse/scale);
  return mousePos;
}
void Atlas::showMousePosition(QPoint mouse){
  mousePosition=getMousePosition(mouse);
  QString labeltext=trUtf8("Kachel: (%1,%2),Pix (%3,%4):N%5° %6' E %7° %8'");
  posLabel->setText(labeltext.arg(mousePosition.getX()).arg(mousePosition.getY()).
    arg(mousePosition.getXPix(),3).arg(mousePosition.getYPix(),3).
    arg(mousePosition.getLatDeg()).arg(mousePosition.getLatMin(),6,'f',3).
    arg(mousePosition.getLonDeg()).arg(mousePosition.getLonMin(),6,'f',3) );
}
void Atlas::mouseMoveEvent(QMouseEvent * e){
  showMousePosition(e->pos());  
  //qWarning("%d",e->state());
  if(e->buttons() & Qt::LeftButton) {
      if(!(e->modifiers()&Qt::ShiftModifier)){//Shift-Taste nicht gedrückt
        if(e->pos()!=newMousePoint){//Maus hat sich bewegt
          QPoint delta=(newMousePoint-e->pos());
          QPoint p=pos.getglobalPixelKoords()+(delta/scale);
          pos.setGlobalPixelKoords(p);
        }
      }
      newMousePoint=e->pos();
      repaint();
   }else{//linke Taste nicht gedrückt.
      int i=getWaypointIndexAt(mousePosition);
      if (i>=0){//Maus in der Nähe eines Waypoints
        if( !hintLabel->isVisible()){
          hintLabel->setText(waypoints[i].isImage()?waypoints[i].title:waypoints[i].description);
          hintLabel->resize(hintLabel->sizeHint());
          hintLabel->move(e->pos().x()-hintLabel->width()-4,e->pos().y());
          hintLabel->show();
        }
      }else/*Maus nicht in der Nähe eines Waypoints*/
      {
          if(hintLabel->isVisible())
              hintLabel->hide();
      }
      i=getTrackpointIndexAt(mousePosition);
      if(i>=0){//Maus in der Nähe eines Trackpoints.
          double l=track.calcLength(i);
          double alt=track.points.at(i).altitude;
          if(hintLabel->isVisible()){//Label zeigt schon info zu Waypoint
              if(!(hintLabel->text().contains("Pfad"))){
                  hintLabel->setText(hintLabel->text()+trUtf8("\n Pfad bis hier: %1 ").arg(Track::printLength(l)));
                  hintLabel->resize(hintLabel->sizeHint());
              }
          }else{//Label wird noch nicht angezeigt.
              hintLabel->setText(Track::printLength(l));
              if(alt!=0.0){
                  QString hoch=trUtf8("%1 m ü NN").arg(alt);
                  hintLabel->setText(hintLabel->text()+"\n"+hoch);
               }
              hintLabel->resize(hintLabel->sizeHint());
              hintLabel->move(e->pos().x()-hintLabel->width()-4,e->pos().y());
              hintLabel->show();
          }
      }//ende Maus in der Nähe eines Trackpoints
      emit mouseMoved();
  }
  e->accept();
}
void Atlas::mousePressEvent(QMouseEvent * e){
  newMousePoint=e->pos();
  int i=-1;
  if(e->button()==Qt::LeftButton){
    short was=0;
    was=(showWaypoints && getWaypointIndexAt(mousePosition)!=-1)?1:0;
    was+=(track.showPoints&&(i=getTrackpointIndexAt(mousePosition))!=-1)?2:0;
    if(was==3){
        UPopupMenu pop("Klick nicht eindeutig!");
        pop.addButton("Wegmarke gemeint",1);
        pop.addButton("Pfadpunkt gemeint",2);
        was = pop.popup(mapToGlobal(e->pos()));
    }
    if(was==1){
          addWaypoint();
          e->accept(); return;
    }
    if(was==2){
        if (track.currentIndex==i){
          trackMenu();
        }else
          track.setCurrentIndex(i);
        e->accept(); return;
    }
    if(e->modifiers()&Qt::ShiftModifier){
      markArea=true;
      mouseRect.setTopLeft(e->pos());
      mouseRect.setBottomRight(e->pos());
    }
    e->accept();
  }else if(e->button()==Qt::RightButton){
      popup->popup(e->globalPos());
  }
  e->accept();
}
void Atlas::mouseReleaseEvent(QMouseEvent * e){
  if(markArea&&e->button()==Qt::LeftButton && ! mouseRect.normalize().isEmpty()){
    showDownloadDialog();
    markArea=false;
  }
  e->accept();
}

void Atlas::mouseDoubleClickEvent(QMouseEvent *e)
{
    QStringList bilder=getImagesAt(e->pos());
    if(bilder.isEmpty()) return;
    for(const QString & s : bilder){
       QDesktopServices::openUrl(QUrl(QString("file://"+s)));
    }

}
void Atlas::wheelEvent(QWheelEvent *e){
  int key=(e->delta()>0)?Qt::Key_Plus:Qt::Key_Minus;
  /*Position p; p.setZ(pos.getZ());
  p.setGlobalPixelKoords(pos.getglobalPixelKoords()+(e->pos()/scale));*/
  QKeyEvent ev(QEvent::KeyPress,key,e->modifiers());
  keyPressEvent(&ev);
  //movePositionTo(p,e->pos());
}

void Atlas::keyPressEvent(QKeyEvent *e){
/** die meisten keyboard-Rekationen werden durch die Actions im Konstruktor von Atlas definiert**/
  switch(e->key()){
    case Qt::Key_Up :   
      pos.setGlobalPixelKoords(pos.getglobalPixelKoords()-QPoint(0,10)/scale);  break;
    case Qt::Key_Down : 
      pos.setGlobalPixelKoords(pos.getglobalPixelKoords()+QPoint(0,10)/scale); break;
    case Qt::Key_Left : 
      pos.setGlobalPixelKoords(pos.getglobalPixelKoords()-QPoint(10,0)/scale); break;
    case Qt::Key_Right :
      pos.setGlobalPixelKoords(pos.getglobalPixelKoords()+QPoint(10,0)/scale); break;
    case Qt::Key_Minus :
      useBild=false;
      if(e->state() & (Qt::ShiftModifier | Qt::ControlModifier)) {//scale ändern
          Position p=pos;
          p.setGlobalPixelKoords(pos.getglobalPixelKoords()+mapFromGlobal(QCursor::pos())/scale);
          scale*=0.8;
          movePositionTo(p,mapFromGlobal(QCursor::pos()));
      }
      else
        if(pos.getZ()>0){
          Position p=pos;
          p.setGlobalPixelKoords(pos.getglobalPixelKoords()+mapFromGlobal(QCursor::pos())/scale);
          pos.setZ(pos.getZ()-1);
          movePositionTo(p,mapFromGlobal(QCursor::pos()));
      }
      break;
    case Qt::Key_Plus : 
      //border=nix;
      useBild=false;
      if(e->state() & (Qt::ShiftModifier | Qt::ControlModifier)){//scale ändern
        Position p=pos;
        p.setGlobalPixelKoords(pos.getglobalPixelKoords()+mapFromGlobal(QCursor::pos())/scale);
        scale*=1.25;
        movePositionTo(p,mapFromGlobal(QCursor::pos()));
      }
      else//Zoomfaktor ändern
        if(pos.getZ()<18){
            Position p=pos;
            p.setGlobalPixelKoords(pos.getglobalPixelKoords()+mapFromGlobal(QCursor::pos())/scale);
            pos.setZ(pos.getZ()+1);
            movePositionTo(p,mapFromGlobal(QCursor::pos()));
        }
      break;
    default : e->ignore();  return;
  }
  repaint();
}
void Atlas::changeBaseDir(){
  QString newdir=QFileDialog::getExistingDirectory(this,trUtf8("Atlas wählen"),baseDir);
  if(newdir.isNull()) return;
  useBild=false;
  //border=nix;
  baseDir=newdir+"/";
  QString befehl="cd "+baseDir +
  "  && { for i in */*/*.png ; do mv $i $i.tile ; done }";
  system(befehl.latin1());
  /* auskommentiertes sorgt dafür, dass die Anzeige auf alle Fälle mit Karten beginnt, die es gibt.
  short z;
  bool found=false;
  for (z=0; z<19; z++){
    if (QFile::exists(baseDir+QString::number(z))){
      found=true;
      break;
    }
  }
  if(!found){
    QMessageBox::warning(this,"Achtung","In diesem Verzeichnis gibt es keine Karten");
    return;
  }
  //qWarning("Habe z-Wert %d gefunden",z);
  QDir dir(baseDir+QString::number(z));
  int x=dir[2].toInt();
  //qWarning("habe x-Wert %d gefunden",x);
  dir.cd(dir[2]);
  QString tile=dir[2];
  //qWarning("habe Datei %s gefunden",tile.latin1());
  int y=tile.left(tile.find('.')).toInt();
  //qWarning("habe y gleich %d gesetzt",y);
  pos.setZXY(z,x,y);
*/
  newPaint();
}
void Atlas::showDownloadDialog(bool force){
  if(!force&&(mouseRect.normalize().isEmpty())) return;
  if(!downloadDialog){
    downloadDialog=new UKartendownload();
    downloadDialog->setAttribute(Qt::WA_QuitOnClose,false);//das Programm kann sich beenden auch wenn dieses Fenster noch offen ist.
//    connect(downloadDialog->anzeigeButton,SIGNAL(clicked()),this,SLOT(changeRegion()));
    connect(downloadDialog,SIGNAL(downloadReady()),this,SLOT(newPaint()));
  }
  mouseRect=mouseRect.normalize();
  downloadDialog->setRegion(getMousePosition(mouseRect.bottomLeft()),getMousePosition(mouseRect.topRight()));
  downloadDialog->selectFolder(baseDir);
  downloadDialog->Zv->setValue(pos.getZ());
  downloadDialog->Zb->setValue(pos.getZ());
  downloadDialog->show();
  downloadDialog->raise();
}

void Atlas::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls())
             e->acceptProposedAction();
}

void Atlas::dropEvent(QDropEvent *e)
{
  QStringList filenames;
  for (auto url : e->mimeData()->urls()){
      filenames<<url.toLocalFile();
  }
  QProcess WPmachen;
  bar->showMessage("Starte geotagExtrahieren zum Ermitteln der Geodaten...");
  QString tempdir=QDesktopServices::displayName(QDesktopServices::TempLocation);
  if(tempdir=="") tempdir="/tmp";
  QFile f(tempdir+"/kml.fmt");
  f.open(QIODevice::WriteOnly);
  QTextStream s(&f);
  s<<"#[HEAD]<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
     "#[HEAD]<kml xmlns=\"http://earth.google.com/kml/2.0\">\n"
     "#[HEAD]  <Document>\n"
   "#[HEAD]    <name>Fotos</name>\n"
   "#[HEAD]    <Folder>\n"
   "#[HEAD]      <name>Fotowaypoints</name>\n"
   "#[BODY]      <Placemark>\n"
   "#[BODY]        <description>Image:$filepath</description>\n"
   "#[BODY]        <Snippet/>\n"
   "#[BODY]        <name>$datetimeoriginal</name>\n"
   "#[BODY]        <Point>\n"
   "#[BODY]          <coordinates>$gpslongitude#,$gpslatitude#</coordinates>\n"
   "#[BODY]        </Point>\n"
   "#[BODY]      </Placemark>\n"
   "#[TAIL]    </Folder>\n"
   "#[TAIL]  </Document>\n"
   "#[TAIL]</kml>\n";
  f.close();
  WPmachen.start("exiftool",QStringList()<<"-p"<<f.fileName()<<
      "-d"<<"%d.%m %H:%M"<<"-if"<<"defined $GPSPosition"<<filenames);
  if(!WPmachen.waitForStarted(500)){
      QMessageBox::warning(0,"Achtung","Der Programm exiftool konnte nicht gestartet werden.");
      return;
  }
  WPmachen.waitForFinished();
  if(WPmachen.exitCode()!=0){
      QMessageBox::warning(0,"Achtung","Konnte keinen Geotag extrahieren:"+
                           WPmachen.readAllStandardError());
      e->accept();
      return;
  }else{
      WPmachen.setReadChannel(QProcess::StandardOutput);
      waypoints.appendFromIODevice(&WPmachen);
      WPmachen.close();
      showCentered(Position(waypoints.last()));
  }
  e->accept();
}
void Atlas::movePositionTo(Position  p,const QPoint & pixelkoord){
  p.setZ(pos.getZ());
  pos.setGlobalPixelKoords(p.getglobalPixelKoords()-(pixelkoord/scale));
  newPaint();
  QWidget*child=childAt(width()-10,height()-10);
  if(child!=0) child->repaint();
}

void Atlas::newPaint(){
  useBild=false;
  repaint();
}
void Atlas::deleteTileAtMousePoint(){
  Position p=getMousePosition(newMousePoint);
  QString tilename=baseDir+QString("%1/%2/%3.png.tile").arg(p.getZ()).arg(p.getX()).arg(p.getY());
  QFile tilefile(tilename);
  if(!tilefile.remove())
      QMessageBox::information(0,"Hallo",tilename+trUtf8(" konnte nicht gelöscht werden."));
  else{
      useBild=false;
      repaint();
  }
}
void Atlas::clip(){
    Position p=getMousePosition(newMousePoint);
    QString laengenangabe;
    QString breitenangabe;
    QMenu box;
    QAction*act1=box.addAction(trUtf8("Weiterverwendung für Google-Earth"));
    box.addAction(trUtf8("... für geotag"));
    bool googleearth=(box.exec(QCursor::pos(),act1)==act1);
    if(googleearth){
        if(p.getLongitude()<0){
            laengenangabe=trUtf8("W %1°").arg(-p.getLongitude(),8,'f',6);
        }
        else{
            laengenangabe=trUtf8("E %1°").arg(p.getLongitude(),8,'f',6);
        }
        if(p.getLatitude()<0){
            breitenangabe=trUtf8("S %1°").arg(-p.getLatitude(),8,'f',6);
        }
        else{
            breitenangabe=trUtf8("N %1°").arg(p.getLatitude(),8,'f',6);
        }
    }else{
        breitenangabe=trUtf8("%L1°").arg(p.getLatitude(),8,'f',6);
        laengenangabe=trUtf8("%L1°").arg(p.getLongitude(),8,'f',6);
    }
    QString t=breitenangabe+" "+laengenangabe;
    qApp->clipboard()->setText(t);
}
QPoint Atlas::getPixelKoords(double lat,double lon,bool&visible){
  Position p(lat,lon);
  p.setZ(pos.getZ());
  visible=true;
  QPoint offset=p.getglobalPixelKoords()-pos.getglobalPixelKoords();
  if ( offset.x()<0 || offset.x()*scale>width() ||  offset.y()<0 || offset.y()*scale>height() ){
    visible=false;
  }
  return offset;
}

QPoint Atlas::getPixelKoords(Position p,bool&visible){
  return(getPixelKoords(p.getLatitude(),p.getLongitude(),visible));
}
QPoint Atlas::getPixelKoords(const QPoint & globalPixelKoords, int z,bool &vis){
  QPoint offset;
  if(z>=pos.getZ()){
    short d=z-pos.getZ();//differenz der Zoomfaktoren
    offset=QPoint(globalPixelKoords.x()>>d, globalPixelKoords.y()>>d) - pos.getglobalPixelKoords() ;
  }else{
      short d=pos.getZ()-z;//differenz der Zoomfaktoren
      offset=QPoint(globalPixelKoords.x()<<d, globalPixelKoords.y()<<d) - pos.getglobalPixelKoords() ;
  }
  vis=true;
  if ( offset.x()<0 || offset.x()*scale>width() ||  offset.y()<0 || offset.y()*scale>height() ){
    vis=false;
  }
  return offset;
}

void Atlas::showCentered(const Position &apos){
  movePositionTo(apos,QPoint(width(),height())/2);
  //newPaint();
}

QStringList Atlas::getImagesAt(QPoint p)
{
    QStringList liste;
    for(const Waypoint  & wp : waypoints){
        bool visible;
        QPoint wo=getPixelKoords(wp.latitude,wp.longitude,visible);
        if(!visible) continue;
        if(!wp.isImage()) continue;
        if(QRect(scale*wo,scale*wp.getPixmap().size()).contains(p)){
            liste<<wp.imageFilename();
        }
    }
    return liste;
}
int Atlas::getWaypointIndexAt(Position mouse){
  for(int i=0;i<waypoints.count();i++){
    bool vis;
    QPoint p1=getPixelKoords(Position(waypoints[i].latitude,waypoints[i].longitude),vis);
    if(!vis) continue;
    QPoint p2=getPixelKoords(mouse,vis);
    if((p1.y()-p2.y())*(p1.y()-p2.y()) + (p1.x()-p2.x())*(p1.x()-p2.x()) > (int)(25/scale) )
      continue;
    else{
      return i;
    }
  }
  return -1;
}
int Atlas::getTrackpointIndexAt(Position mouse){
  for(int i=0;i<track.points.count();i++){
      bool vis;
      QPoint p1=getPixelKoords(Position(track.points[i].latitude,track.points[i].longitude),vis);
      if(!vis) continue;
      QPoint p2=getPixelKoords(mouse,vis);
      if((p1.y()-p2.y())*(p1.y()-p2.y()) + (p1.x()-p2.x())*(p1.x()-p2.x()) > (int)(25/scale) )
        continue;
      else{
        return i;
      }
    }
    return -1;
}

void Atlas::addWaypoint(){//wird auch aufgerufen, wenn Waypoint nur editiert wird.
  int i=getWaypointIndexAt(mousePosition);//Maus in der Nähe eines Waypoints?
  if(i<0){
      Waypoint wp(mousePosition.getLatitude(),mousePosition.getLongitude());
      /*i=0;//waypoint alphabetisch korrekt einsortieren. Hat keinen Sinn
      while(i<waypoints.count() && waypoints.at(i)<wp) i++;
      waypoints.insert(i,wp);*/
      waypoints.append(wp);
      i=waypoints.size()-1;
      waypoints.isSaved=false;
  }
  Waypoint *p=&waypoints[i];
  if(p->getWpDialog()==0){
    waypointDialog *dial = new waypointDialog(this,p);//der Dialog setzt seine Felder gemäß p, wenn möglich
    p->setWpDialog(dial);
    dial->setWindowTitle(i<0?"neue Wegmarke":trUtf8("Wegmarke %1").arg(p->title));
    dial->resize(dial->sizeHint());
  //if(p==0) dial->setPosition(mousePosition);//sonst bekommt er hier die Position.
    dial->show();//ein nicht modaler Aufruf!
  }else
    p->getWpDialog()->raise();
}
void Atlas::loadWaypoints(QString filename){
  bool dateinameAngegeben=(filename!="" && filename!=waypoints.filename);
  QSettings settings("UBoss","karten");
  QString dir=settings.value("waypoints/directory",QString("")).toString();
  if(filename==""){
      QFileDialog fd(0,"Waypoint-Datei",dir,"kml-Dateien (*.kml)");
      fd.setFileMode(QFileDialog::ExistingFile);
      QList<QUrl> sbu;
      for(QString & dir:fd.history())
          sbu<<QUrl(QString("file://")+dir);
      fd.setSidebarUrls(sbu);//so wird die Liste der zuletzt geöffneten Verzeichnisse in die Sidebar übertragen.
      fd.setLabelText(QFileDialog::FileName,"Waypoint-Datei:");
      if(fd.exec()!=QDialog::Accepted){
          return;
      }
      waypoints.filename=fd.selectedFile();
              //QFileDialog::getOpenFileName(0,"kml-Datei",dir,"kml-Dateien (*.kml)");
      filename=waypoints.filename;
  }
  if(filename.isEmpty()) return;
  QFile f(filename);
  if(!f.open(QIODevice::ReadOnly)){
    QMessageBox::warning(this,"Achtung",trUtf8("Konnte die Waypointdatein nicht öffnen"));
    return;
  }
  dir=QFileInfo(f).path();
  if(!dateinameAngegeben) settings.setValue("waypoints/directory",dir);
  if(waypoints.count()>0){
    if(QMessageBox::question(0,"Frage",trUtf8("Sollen die Wegpunkte aus der Datei zu den bestehenden hinzugefügt werden?"),
                             QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes)==QMessageBox::No){
      waypoints.clear();
      waypoints.isSaved=true;
    }else{
      waypoints.isSaved=false;
    }
  }
  waypoints.appendFromIODevice(&f);
  if(f.isOpen())
      f.close();
  newPaint();
  if(!waypoints.isEmpty()) showWaypointCombo();
}
void Atlas::saveWaypoints(){
  waypoints.filename=QFileDialog::getSaveFileName(0,"kml-Datei",waypoints.filename,"kml-Dateien (*.kml)");
  if(waypoints.filename.isEmpty()) return;
  QFile f(waypoints.filename);
  f.open(QIODevice::WriteOnly);
  QTextStream st(&f);
  st<<Waypoint::kmlvorspann;
  for(int i=0;i<waypoints.count();i++){
    st<<waypoints[i];
  }
  st<<Waypoint::kmlnachspann;
  waypoints.isSaved=true;
  f.close();
}

void Atlas::closeWaypointFile()
{
   if(!waypoints.isSaved){
       QMessageBox::information(0,"Achtung","Wegmarken wurden noch nicht gespeichert!");
       saveWaypoints();
   }
   waypoints.isSaved=true;
   waypoints.filename="";
   waypoints.clear();
   repaint();
}
void Atlas::deleteWaypoint(){
  //mousePosition: hier wurde geklickt.
  int i,j;
  short was=0;
  was=(showWaypoints && (i=getWaypointIndexAt(mousePosition))>=0)?1:0;
  was+=(showWaypoints&&track.showPoints&&(j=getTrackpointIndexAt(mousePosition))>=0)?2:0;
  if(was==3){
      UPopupMenu * pop=new UPopupMenu("Klick nicht eindeutig!");
      pop->addButton(trUtf8("Wegmarke löschen"),1);
      pop->addButton(trUtf8("Pfadpunkt löschen"),2);
      pop->addButton("weder noch",0);
      was=pop->popup(QCursor::pos());
      delete pop;
  }
  if(was==1){
      waypointDialog*dial;
      if((dial=waypoints[i].getWpDialog()) != 0){
          dial->reject();
          //delete dial;
      }
      waypoints.removeAt(i);
      waypoints.isSaved=false;
      repaint();
      return;
  }else
    if(was==2){
      track.deletePoint(j);
    }
}

void Atlas::showWaypointCombo()
{
    if (waypoints.isEmpty()) return;
    QComboBox * combo=waypoints.getComboBox();
    connect(combo,SIGNAL(activated(QString)),this,SLOT(showWaypoint(QString)));
    combo->setWindowTitle("Waypoints");
    combo->setAttribute(Qt::WA_DeleteOnClose);
    combo->setAttribute(Qt::WA_QuitOnClose,false);
    combo->setWindowFlags(Qt::WindowStaysOnTopHint);
    combo->resize(combo->sizeHint());
    combo->move(mapToGlobal(QPoint(width()/2-combo->width()-10,height()/2+55)));
    combo->show();
}
void Atlas::closed() { //wird im Destruktor von Atlas aufgerufen.
    qDebug()<<"Atlas::closed aufgerufen";
    if(waypoints.count()>0 && (waypoints.isSaved==false) &&
        QMessageBox::question(0,"Sicherheitsabfrage",QString::fromUtf8("Sollen die Wegmarken gespeichert werden?\n"
  "Die anzugebende Datei wird dann überschrieben"),QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes)
      ==QMessageBox::Yes){
        saveWaypoints();
    }
    if(!track.stored &&
       QMessageBox::question(0,"Sicherheitsabfrage",
                "Soll der Track gespeichert werden?",QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes)==QMessageBox::Yes) {
       trackMenu();
       trackDialog->setAttribute(Qt::WA_QuitOnClose,true);
       trackDialog->exec();
    }
    //track.showProfile(false);
    QSettings settings("UBoss","karten");
    if(settings.value("SaveOnExit").toBool()){
       settings.setValue("position/x",pos.getX());
       settings.setValue("position/y",pos.getY());
       settings.setValue("position/z",pos.getZ());
       settings.setValue("width",width());
       settings.setValue("height",parentWidget()->height()-8);
       settings.setValue("waypoints/filename",waypoints.filename);
       settings.setValue("track/filename",track.filename);
       settings.setValue("track/farbe",track.farbe);
       settings.setValue("atlasdirectory",baseDir);
       if(downloadDialog){
         settings.setValue("mapserverNr",downloadDialog->serverCombo->currentIndex());
       }
    }

  //qApp->quit();
}

void Atlas::markTrack(double rel)
{
    bool vis;
    Trackpoint ort=track.getTrackpoint(rel);
    //qDebug()<<"ort: "<<ort.latitude<<ort.longitude<<ort.altitude;
    Position port=Position(ort,pos.getZ());
    QPoint wo=getPixelKoords(ort.latitude,ort.longitude,vis);
    if(!vis){//gegebener Punkt nicht sichtbar, wird jetzt in Mitte geholt
        showCentered(port);
        wo=getPixelKoords(ort.latitude,ort.longitude,vis);
    }
    hintLabel->move(wo.x()*scale,wo.y()*scale);
    hintLabel->setText(trUtf8("%1 m ü NN").arg(round(ort.altitude)));
    hintLabel->resize(hintLabel->sizeHint());
    hintLabel->show();
}

void Atlas::showWaypoint(QString wpTitle, bool withDialog)
{
    int i=waypoints.indexOf(wpTitle);
    if(i<0) return;
    Waypoint & wp=waypoints[i];
    showCentered(Position(wp));
    if(withDialog && wp.getWpDialog()!=0){
        wp.getWpDialog()->raise();
    }else if(withDialog){
        waypointDialog *dial = new waypointDialog(this,&wp) ;//der Dialog setzt seine Felder gemäß p, wenn möglich
        wp.setWpDialog(dial);
        dial->setWindowTitle(wp.title);
        dial->move(10,10);
        dial->resize(dial->sizeHint());
      //if(p==0) dial->setPosition(mousePosition);//sonst bekommt er hier die Position.
        dial->show();//ein nicht modaler Aufruf!
    }
}
Atlas::~Atlas(){
  closed();
  delete hintLabel;
}
/*
void Atlas::changeRegion(){
  pos=downloadDialog->linksOben();
  qWarning("Habe N %g,E %g erhalten",pos.getLatitude(),pos.getLongitude() );
  newPaint();
}*/
void Atlas::trackMenu(){
  if(trackDialog==0){
     trackDialog=new QDialog;
     QDialog & dial=*trackDialog;
     Ui::TrackDialog ui;
     ui.setupUi(&dial);
     dial.setCaption("Track-Dialog");
     ui.filename->setText(track.filename);
     ui.colorButton->setColor(track.farbe);
     ui.description->setText(track.description);
     ui.name->setText(track.name);
     ui.colorButton->setColor(track.farbe);
     ui.saveTrack->setEnabled(!track.stored);
     ui.showPoints->setChecked(track.showPoints);
     track.setProfilButton(ui.profilButton);
     QMenu * trackpopup=new QMenu(trackDialog);
     track.setPopupMenu(trackpopup);
     track.actualizePopupMenu();
     ui.loadTrack->setPopup(trackpopup);
     connect(trackpopup,SIGNAL(triggered(QAction*)),&track,SLOT(loadAction(QAction*)));
     connect(ui.colorButton,SIGNAL(changed(QColor)),&track,SLOT(changeColor(QColor)));
     connect(ui.saveTrack,SIGNAL(clicked()),&track,SLOT(storeToGPX()));
     connect(ui.saveTrackAs,SIGNAL(clicked()),&track,SLOT(storeToGPXAs()));
     //connect(ui.loadTrack,SIGNAL(clicked()),&track,SLOT(loadFromGPX()));
     connect(ui.deleteTrack,SIGNAL(clicked()),&track,SLOT(clear()));
     connect(ui.deletePoint,SIGNAL(clicked()),&track,SLOT(deletePoint()));
     connect(ui.name,SIGNAL(textEdited(QString)),&track,SLOT(changeName(QString)));
     connect(ui.description,SIGNAL(textEdited(QString)),&track,SLOT(changeDescription(QString)));
     connect(ui.revertButton,SIGNAL(clicked()),&track,SLOT(revert()));
     connect(ui.showPoints,SIGNAL(stateChanged(int)),&track,SLOT(setShowPoints(int)));
     connect(ui.tileLoadButton,SIGNAL(clicked()),this,SLOT(loadTrackEnvironment()));
     connect(ui.profilButton,SIGNAL(clicked(bool)),&track,SLOT(showProfile(bool)));
     connect(ui.getAltitudeButton,SIGNAL(clicked(bool)),&track,SLOT(getAltitudes()));
     connect(ui.buttonBox,SIGNAL(helpRequested()),&track,SLOT(help()));
     connect(&track,SIGNAL(nameChanged(QString)),ui.name,SLOT(setText(QString)));
     connect(&track,SIGNAL(filenameChanged(QString)),ui.filename,SLOT(setText(QString)));
     connect(&track,SIGNAL(descriptionChanged(QString)),ui.description,SLOT(setText(QString)));
     connect(&track,SIGNAL(toSave(bool)),ui.saveTrack,SLOT(setEnabled(bool)));
     connect(&track,SIGNAL(newLength(QString)),ui.laenge,SLOT(setText(QString)));
     dial.move(mapToGlobal(QWidget::pos()));
     trackDialog->setAttribute(Qt::WA_QuitOnClose,false);//das Programm kann sich beenden auch wenn dieses Fenster noch offen ist.
  }
  trackDialog->show();//nicht modal öffnen.
  trackDialog->raise();
  track.calcLength();//damit die Länge richtig angezeigt wird.
}
void Atlas::expandTrack(){
  track.insertPoint(mousePosition.getLatitude(),mousePosition.getLongitude());
  //track.stored=false;
}
void Atlas::saveTrack(){
  if (track.name.length()==0){
    track.name="Spaziergang";//hier noch Dialog einblenden, der nach Name fragt
  }
  track.storeToGPX();
}

void Atlas::loadTrackEnvironment(){
  qWarning("kacheln um Track");
  QList<QString> list=track.getTileset(pos.getZ(),1).toList();
  QFile file(baseDir+"/tilenumbers.txt");
  if (!file.open(QIODevice::WriteOnly)){
    QMessageBox::warning(0,"Mist","Kann Datei "+file.name()+trUtf8(" nicht öffnen!"));
    return;
  }
  QTextStream stream(&file);
  uint neue=0;
  for(int n=0 ;n<list.count();n++){
    QStringList l=list[n].split('\t');
    QString fn=baseDir+"%1/%2/%3.png.tile";
    fn=fn.arg(l[2].trimmed()).arg(l[0]).arg(l[1]);
    if (QFile::exists(fn)){
            continue;
     }
     stream<<list[n];
     neue++;
  }
  file.close();
  showDownloadDialog(true);
  downloadDialog->infolabel->setText(trUtf8("<font color=\"red\"><b>Hinweis: </b> die %1 Kacheln der Trackumgebung sind in tilenumbers.txt.").
                                     arg(neue)+trUtf8("Sie können jetzt mit \"download\" geladen werden."));
}

void Atlas::addTimedTrack()
{
  TimedTrack * tt=new TimedTrack(this);
  //timedTrackList.append(tt);
  TimedTrackDialog * ttDial=new TimedTrackDialog(tt,this);
  if(!ttDial->init())
      qWarning("...diese Timed Track ist leer");
  else
      showTrackTimer();
}

void Atlas::showTrackTimer()
{
    if(TimedTrack::tracktimer==0){
        QMessageBox::information(0,"Achtung",trUtf8("erst einen Track hinzufügen, vorher gibts den Timer nicht."));
        addTimedTrack();
        return;
    }
    TimedTrack::tracktimer->show();
}

void Atlas::hideAndShowWaypoints(){
  showWaypoints=!showWaypoints;
  repaint();
}
void Atlas::help(){
   Helpsystem::showPage(QUrl("qrc:///hilfeseiten/atlas.html"));
     /*QLabel * h=new QLabel(0);
     h->setAttribute(Qt::WA_DeleteOnClose);
     h->move(10,10);
     h->setText(trUtf8("<h2>Hilfe zum Atlas-Programm</h2> ...erhält man in erster Linie über die Tool-Tips, die angezeigt werden, sobald man"
          "mit der Maus über die verschiedenen Elemente, z.B. Schaltflächen fährt. Manchmal kann man durch Rechtsklick auf die"
          "Schaltflächen \ausführlichere \"What's this\"-Hilfen erhalten."
          "<h3>Maus-Bedienung</h3>"
          "<b>rechte Maustaste</b> öffnet ein Popupmenu, aus dem alles erreichbar ist. Hier werden die Tastaturkürzel auch angezeigt,"
          "die die Bedienung deutlich schneller machen."
           "<p><b>linke Maustaste:</b> Hiermit kann man die Karten verschieben oder durch einfachen Klick ohne Schieben Wegpunkte und"
          "Pfadpunkte anklicken.</p>"
          "<p><b>Shift-Linke Maustaste:</b> Hiermit zieht man einen Bereich auf, für den man Kacheln herunter laden kann. Es öffnet"
          "sich ein entsprechendes Dialogfenster</p>"));
     h->setWordWrap(true);
   h->resize(h->sizeHint());
   h->show();*/
}
void Atlas::settings(){
     SettingDialog * dial=new SettingDialog(this);
     connect(dial,SIGNAL(pixmapSizeChanged(int)),this,SLOT(resizeWaypointPixmaps(int)));
     dial->setAttribute(Qt::WA_DeleteOnClose);
     dial->show();
     //delete dial;
}

void Atlas::resizeWaypointPixmaps(int size)
{
    for(Waypoints::Iterator it=waypoints.begin();it!=waypoints.end();it++){
        if(it->isImage()){
            Waypoint &w=*it;
            QPixmap p(w.imageFilename());
            if(p.isNull()){
                qDebug()<<"Konnte Bild "<<w.imageFilename()<<" nicht laden!";
                continue;
            }
            w.setPixmap(p.scaled(size,size,Qt::KeepAspectRatio));
        }
    }
    repaint();
}

Helpsystem * Helpsystem::helpsystem=new Helpsystem;

void Helpsystem::showPage(QUrl url,QString title)
{
    QTextBrowser *brows=new QTextBrowser(0);
    brows->setSource(url);
    brows->setOpenExternalLinks(true);
    if(!brows->source().isValid()){
        QMessageBox::warning(0,"Achtung!",trUtf8("Das Dokument %1 konnte nicht gefunden werden.").arg(url.toString()));
        delete brows;
        return;
    }
    brows->setFocusPolicy(Qt::StrongFocus);
    QWidget * hilfefenster=new QWidget(0);
    QVBoxLayout * lay=new QVBoxLayout(hilfefenster);
    lay->addWidget(brows);
    QHBoxLayout * bg=new QHBoxLayout;
    QToolButton* back=new QToolButton(Qt::LeftArrow,0,"tb");
    //QPushButton * back=new QPushButton(trUtf8(" ← "));
    connect(back,SIGNAL(clicked()),brows,SLOT(backward()));
    bg->addWidget(back,0,Qt::AlignRight);
    bg->addWidget(new QToolButton(QIcon(":/images/window-close.png"),"","",hilfefenster,SLOT(close()),0),Qt::AlignCenter);
    //QPushButton*close=new QPushButton(QIcon(":/images/window-close.png"),"");
    //connect(close,SIGNAL(clicked()),hilfefenster,SLOT(close()));
    //bg->addWidget(close,Qt::AlignCenter);
    QToolButton* forward=new QToolButton(Qt::RightArrow,0,"rb");
    connect(forward,SIGNAL(clicked()),brows,SLOT(forward()));
    bg->addWidget(forward,0,Qt::AlignLeft);
    lay->addLayout(bg);
    hilfefenster->setWindowTitle(title);
    hilfefenster->setAttribute(Qt::WA_DeleteOnClose);
    hilfefenster->setAttribute(Qt::WA_QuitOnClose,false);
    hilfefenster->resize(800,600);
    hilfefenster->move(80,80);
    hilfefenster->show();
}

void Helpsystem::help(QString helpPage, QString title)
{
    QUrl url(QString("qrc://"+helpPage));
    showPage(url,title);
}
