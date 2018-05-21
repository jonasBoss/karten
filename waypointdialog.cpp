#include "ui_waypointdialog.h"
#include "waypointdialog.h"
#include "Atlas.h"
#include "region.h"
#include "kompassinterface.h"
#include "kompass.h"
#include <QtXml/QDomDocument>

waypointDialog::waypointDialog(Atlas*atl,Waypoint *p):QDialog(0)
{
  setupUi(this);
  kE.setupUi(koordEing);
  nomatimButton->setIcon(style()->standardIcon(QStyle::SP_MessageBoxQuestion));
  connect(nomatimButton,SIGNAL(clicked()),this,SLOT(nomatimAbfrage()));
  //koordEing->resize(koordEing->sizeHint());
  atlas=atl; waypoint=p;
  Position po;
  tileCalcButton->hide();
  if(p!=0){//es wurde ein existierender Waypoint übergeben.
    title->setText(p->title);
    description->setPlainText(p->description);
    po=Position(p->latitude,p->longitude);
    po.setZ(atl->getPos().getZ());//Z-Koordinate der gegenwÃ¤rtigen Karte Ã¼bernehmen.
    setPosition(po);
    if(p->isImage()){
        tileCalcButton->setText("Bild zeigen");
        tileCalcButton->show();
    }
  }
  kachelKoordinatenBox->hide();
  toggleButton->setText("v-----v-----v  Kachelkoordinaten anzeigen  v-----v-----v");
  //resize(QSize(335,300));
  //resize(sizeHint());
  setAttribute(Qt::WA_QuitOnClose,false);//Programm kann sich beenden, auch wenn dies Fenster offen ist.
  connect(Zoom,SIGNAL(valueChanged(int)),  this, SLOT(on_tileCalcButton_clicked()));//damit die Kacheldaten bei ZoomfaktorÃ¤nderung stimmen.
  connect(title, SIGNAL(editingFinished()),this, SLOT(on_okButton_clicked()));//Waypointdaten werden aktualisiert und angezeigt.
  connect(kE.ui.XG,SIGNAL(editingFinished()),this, SLOT(on_okButton_clicked()));
  connect(kE.ui.XM,SIGNAL(editingFinished()),this, SLOT(on_okButton_clicked()));
  connect(kE.ui.YG,SIGNAL(editingFinished()),this, SLOT(on_okButton_clicked()));
  connect(kE.ui.YM,SIGNAL(editingFinished()),this, SLOT(on_okButton_clicked()));
  connect(tile_x,SIGNAL(editingFinished()),this,SLOT(on_posCalcButton_clicked()));
  connect(tile_y,SIGNAL(editingFinished()),this,SLOT(on_posCalcButton_clicked()));
  connect(pix_x,SIGNAL(editingFinished()),this,SLOT(on_posCalcButton_clicked()));
  connect(pix_y,SIGNAL(editingFinished()),this,SLOT(on_posCalcButton_clicked()));
  connect(atlas,SIGNAL(mouseMoved()),this,SLOT(updateMouseDistance()));
  new QShortcut(QKeySequence(Qt::Key_F1),this,SLOT(help()));
}
void waypointDialog::setPosition(Position pos){
  kE.setLatitude(pos.getLatitude());
  kE.setLongitude(pos.getLongitude());
  Zoom->setValue(pos.getZ());
  int x,y; short xPixel,yPixel;
  pos.getTileData(x,y,xPixel,yPixel);
  tile_x->setText(QString("%1").arg(x));
  tile_y->setText(QString("%1").arg(y));
  pix_x->setText(QString("%1").arg(xPixel));
  pix_y->setText(QString("%1").arg(yPixel));
}

void waypointDialog::on_toggleButton_clicked(){
  if(tileCalcButton->isVisible()){
    kachelKoordinatenBox->hide();
    tileCalcButton->hide();
    toggleButton->setText("v-----v-----v  Kachelkoordinaten anzeigen  v-----v-----v");
    //updateGeometry();
    resize(sizeHint());
  }else{
    kachelKoordinatenBox->show();
    tileCalcButton->show();
    toggleButton->setText("^-----^-----^  Kachelkoordinaten ausblenden  ^-----^-----^");
    //updateGeometry();
    resize(sizeHint());
  }
}
void waypointDialog::on_posCalcButton_clicked(){
  int z=Zoom->value();
  int tx=tile_x->text().toInt();
  int ty=tile_y->text().toInt();
  short pixx=pix_x->text().toInt();
  short pixy=pix_y->text().toInt();
  Position pos;
  pos.setZXY(z,tx,ty,pixx,pixy);
  kE.setLatitude(pos.getLatDeg(),pos.getLatMin());
  kE.setLongitude(pos.getLonDeg(),pos.getLonMin());
  on_okButton_clicked();//damit die Daten auch zum Waypoint Ã¼bertragen und angezeigt werden.
}
void waypointDialog::on_tileCalcButton_clicked(){
  if(waypoint!=0 && waypoint->isImage()){
      QProcess* bildzeigen=new QProcess;
      connect(bildzeigen,SIGNAL(finished(int)),bildzeigen,SLOT(deleteLater()));
      bildzeigen->start("gwenview",QStringList()<<waypoint->imageFilename(),QIODevice::Unbuffered);
      return;
  }
  Position pos;
  pos.setLatitude(kE.getLatitude());
  pos.setLongitude(kE.getLongitude());
  pos.setZ(Zoom->value());
  int x,y; short xPixel,yPixel;
  pos.getTileData(x,y,xPixel,yPixel);
  tile_x->setText(QString("%1").arg(x));
  tile_y->setText(QString("%1").arg(y));
  pix_x->setText(QString("%1").arg(xPixel));
  pix_y->setText(QString("%1").arg(yPixel));
}
void waypointDialog::on_okButton_clicked(){
  if(atlas==0){
    QDialog::accept();
    return;
  }
  bool del=false;
  if (waypoint==0){//einen neuen Waypoint anlegen.Kann eigentlich garnicht sein, da vor Ã¶ffnen des Dialogs der WP schon erstellt ist.
    waypoint=new Waypoint;
    del=true;
  }
  waypoint->description=description->toPlainText();
  waypoint->title=title->text();
  waypoint->latitude=getLatitude();
  waypoint->longitude=getLongitude();
  if(del){//sollte nie der Fall sein.
    atlas->waypoints.append(*waypoint);
    delete waypoint;
  }
  atlas->waypoints.isSaved=false;
  atlas->repaint();
  //QDialog::accept();
  //delete this;
}
void waypointDialog::on_closeButton_clicked(){
  reject();
}
void waypointDialog::reject(){
    waypoint->setWpDialog(0);
    QDialog::reject();
    deleteLater();
}

void waypointDialog::updateMouseDistance()
{
    kE.ui.mouseDistanceLabel->setText(Track::printLength(atlas->getMousePosition().distanceFrom(Position(getLatitude(),getLongitude()))));
}

void waypointDialog::on_kompassButton_clicked(){
  kompassButton->setEnabled(false);
  Kompass * kompass=new Kompass(atlas,100,0);
  bool vis;
  QPoint point=atlas->getPixelKoords(getLatitude(),getLongitude(),vis);
  point.setX(point.x()*atlas->getScale());point.setY(point.y()*atlas->getScale());
  qWarning("Kompass nach %d,%d bewegt",point.x(),point.y());
  kompass->moveTo(point);
  kompass->releaseMouse();//Kompass soll keine MousePressEvents bekommen.
  kompass->show();
  Kompassinterface* interface=new Kompassinterface(kompass,atlas,waypoint,0);
  double mpP=atlas->meterPerPixel();
  interface->radkm->setValue((int)(0.1*mpP));
  interface->radm ->setValue((int)(100*mpP)-1000*interface->radkm->value());
  connect(interface,SIGNAL(destroyed()),this,SLOT(enableKompassButton()));
  interface->setParent(this);
  interface->move(10,110);
  interface->show();
}
void waypointDialog::on_centerButton_clicked(){
  atlas->showCentered(Position(*waypoint));
}
void waypointDialog::on_deleteButton_clicked(){
  atlas->waypoints.removeAll(*waypoint);
  on_closeButton_clicked();
  //das geht wohl mit waypoints.removeAll(*waypoint), allerdings nur, wenn die Waypoints den Operator
  //== verstehen.
}

void waypointDialog::enableKompassButton(){
  kompassButton->setEnabled(true);
}

void waypointDialog::on_luftlinieButton_clicked()
{
    QComboBox * combo=atlas->waypoints.getComboBox();
    QDialog dial;
    dial.setWindowTitle("Waypointliste");
    QVBoxLayout * lay=new QVBoxLayout(&dial);
    lay->addWidget(new QLabel(trUtf8("Zielwaypoint wählen:")));
    lay->addWidget(combo);
    QPushButton * butt=new QPushButton("&Ok");
    lay->addWidget(butt,0,Qt::AlignCenter);
    connect(butt,SIGNAL(clicked(bool)),&dial,SLOT(accept()));
    QString title;
    dial.move(QCursor::pos()-combo->pos());
    if(dial.exec()==QDialog::Accepted){
        title=combo->currentText();
    }else return;
    Track track(atlas);
    Waypoint*ziel=0;
    for(Waypoint &w : atlas->waypoints){
        if(w.title==title) { ziel=&w; break;}
    }
    if(ziel==0){
        qDebug()<<"Verdammt kann das ziel nicht finden.";
        ziel=&(atlas->waypoints[0]);
    }
    track.points=Track::join(Trackpoint(waypoint->latitude,waypoint->longitude),
                              Trackpoint(ziel->latitude,ziel->longitude),20);
    track.description=trUtf8("Luftlinie %1 nach %2").arg(waypoint->title).arg(ziel->title);
    if(QMessageBox::question(0,"Frage","Soll die Luftlinie zum aktuellen Track gemacht werden?",
                          QMessageBox::Yes|QMessageBox::No)==QMessageBox::Yes){
        if(!atlas->track.stored){
            atlas->track.storeToGPX();
        }
        track.farbe=atlas->track.farbe;
        atlas->track.clear();
        if(atlas->trackDialog!=0) {
            atlas->trackDialog->close();
            atlas->trackDialog->deleteLater();
            atlas->trackDialog=0;
        }
        atlas->track=track;
        atlas->repaint();
    }else{
       track.storeToGPXAs();
    }
}

void waypointDialog::nomatimAbfrage()
{
    if(title->text().trimmed().isEmpty()){
        QMessageBox::warning(0,"Achtung","kein Abfragetext definiert");
        return;
    }
    QProcess  abfrager;
    QString url="https://nominatim.openstreetmap.org/search?q=";
    url+=title->text().replace(' ','+')+"&format=xml";
    //qDebug()<<"frage nominatim-Server ab mit "<<url;
    abfrager.start("wget",QStringList()<<"-O"<<"-"<<"-t"<<"5"<<"--"<<url,QIODevice::ReadOnly);
    if(!abfrager.waitForStarted(500)){
        QMessageBox::warning(0,"Mist",QString("Prozess wurde nicht gestartet: ")+abfrager.errorString());
        return;
    }
    if(!abfrager.waitForFinished(10000)||abfrager.exitCode()!=0){
        QString err;
        if(abfrager.state()==QProcess::NotRunning){
             err=trUtf8(abfrager.readAllStandardError().data());
        }else
            err="timeout nach 10 s";
        QMessageBox::warning(0,"Mist",trUtf8("Abfrage war nicht möglich: ")+err);
        if(abfrager.state()==QProcess::Running) abfrager.terminate();
        return;
    }
    QDomDocument doc;
    abfrager.setReadChannel(QProcess::StandardOutput);
    /*QFile file("/home/uwe/nominatim2.xml");
    file.open(QIODevice::ReadOnly);*/
    QString errorMsg;
    if(!doc.setContent(&abfrager,&errorMsg)){
        QMessageBox::warning(0,"Schade!",trUtf8("Fehler beim Lesen der xml-Antwort: %1").arg(errorMsg));
        return;
    }
    abfrager.close();
    QDomNodeList places=doc.elementsByTagName("place");
    if(places.count()==0){
        QMessageBox::warning(0,"Schade!","keine Places gefunden.");
        return;
    }
    QComboBox*combo=new QComboBox;
    for(int i=0;i<places.count();i++){
        QString lat=places.at(i).toElement().attribute("lat");
        QString lon=places.at(i).toElement().attribute("lon");
        QString desc=places.at(i).toElement().attribute("display_name");
        switch (i) {
            case 0 : kE.ui.XG->setText(lon);kE.ui.XM->setText("");
                    kE.ui.YG->setText(lat); kE.ui.YM->setText("");
                    description->setPlainText(desc);
                    combo->addItem(title->text());
            break;
            default :
              Waypoint newwp(lat.toDouble(),lon.toDouble());
              newwp.title=title->text()+QString("-%1").arg(i+1);
              newwp.description=desc;
              atlas->waypoints.append(newwp);
              combo->addItem(newwp.title);
            break;
        }
    }
    if(places.count()<2){
        combo->clear();delete combo;
        atlas->showWaypoint(title->text());
    }
    else{
        connect(combo,SIGNAL(activated(QString)),atlas,SLOT(showWaypoint(QString)));
        combo->setWindowTitle("Suchergebnisse");
        combo->setAttribute(Qt::WA_DeleteOnClose);
        combo->setAttribute(Qt::WA_QuitOnClose,false);
        combo->setWindowFlags(Qt::WindowStaysOnTopHint);
        combo->resize(combo->sizeHint());
        combo->move(atlas->mapToGlobal(QPoint(atlas->width()/2+5,atlas->height()/2+5)));
        combo->show();
    }
    QMessageBox::information(0,"Erledigt",
                             trUtf8("Abfrage bei openstreetmap.org lieferte %1 Suchergebnisse. Diese wurden als Waypoints hinzugefügt").arg(places.count()));
}
