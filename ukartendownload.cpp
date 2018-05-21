#include "ukartendownload.h"
#include "region.h"
//#include "ui_regionwahl_qt4.h"
//#include "ui_deineeingabe_qt4.h"
#include "ui_changeUserAgent.h"

//Added by qt3to4:
#include <Q3TextStream>
#include <math.h>
#include <cstdlib>
#include <fstream>
#include "mapservermanager.h"
#include <QtCore>
#include <QtGui>
#include "terminallog.h"

void Tile::setKoords(double lon,double lat, int zoom){
  latitude=lat;
  longitude=lon;
  z=zoom;
  if (lon > 180.0){lon -= 360.0;}
  int scale= (1 << zoom);
  x=(int)(((180.0 + lon) / 360.0) * scale);
  y=(int)((0.5 - log(tan((M_PI / 4.0) + ((M_PI * lat) / 360.0)))  / (2.0 * M_PI)) * scale);
}
void Tile::newZ(int zoom){
  setKoords(longitude,latitude,zoom);
}
UKartendownload::UKartendownload():QDialog(0){
  setupUi(this);
  QSettings settings("UBoss","karten");
  QString mapserverfile=settings.value("mapserverfile").toString();
  if(mapserver.readFromFile(mapserverfile)){
    serverCombo->insertStringList(mapserver.names);
    mapserverDescription->setText(mapserver.descriptions[0]);
    serverCombo->setCurrentIndex(settings.value("mapserverNr",0).toInt());
  }
  connect(serverCombo,SIGNAL(currentIndexChanged(QString)),defaultMapserverLabel,SLOT(setText(QString)));

}
void UKartendownload::selectFolder(QString dir){
  if(dir.isEmpty()) dir=QFileDialog::getExistingDirectory();
  if(dir.isEmpty()) return;
  if(dir.right(1).at(0)!='/') dir+='/';
  downloadFolder->setText(dir);
  /*if(QFile::exists(dir+"tilenumbers.txt")){
    QMessageBox::warning(0,"Achtung",trUtf8("Es gibt in diesem Verzeichnis eine Datei tilenumbers.txt, "
          "die für den Download verwendet wird. Es sei denn, Du erstellst eine neue Kachelliste."));
  }*/
  QString filename=dir+"mapserver_default.txt";
  if(QFile::exists(filename)){
      Mapserver ms;
      if(!ms.readFromFile(filename) || ms.names.isEmpty()){
          QMessageBox::information(0,"Achtung",trUtf8("Die Datei %1 mit dem default-Mapserver kann nicht ordnungsgemäß gelesen werden"
             "Du solltest sie löschen").arg(filename));
          return;
      }
      int index=serverCombo->findText(ms.names.at(0));
      if(index>=0){//default Mapserver existiert in Liste
          serverCombo->setCurrentIndex(index);
      }else{//default Mapserver existiert nicht in Liste
          QMessageBox::information(0,"Achtung",trUtf8("Der in in %s definierte "
              "mapserver ist nicht in der Liste. Ich füge ihn jetzt hinzu. Das ist aber "
              "nicht dauerhaft!").arg(filename));
          mapserver.append(ms.names.at(0),ms.bases.at(0),ms.tileurls.at(0),ms.descriptions.at(0));
          serverCombo->addItem(ms.names.at(0));
          serverCombo->setCurrentItem(serverCombo->count()-1);
      }
  }else//es gibt kein mapserver_default.txt in diesem Verzeichnis
      QMessageBox::information(0,"Information",trUtf8("Kein default-Mapserver für das "
         "Kartenverzeichnis <br/><center>%1</center><br/> definiert. Verwende deswegen %2").arg(dir).arg(serverCombo->currentText()));
}

double koordinate(QString grad,QString minute,short factor){//factor kann +1 oder -1 sein
  return (grad.toDouble()+minute.toDouble()/60)*factor;
}
/*
void UKartendownload::koordsChanged(){
  //bool ok;
  double xvon,yvon,xbis,ybis;
  if( (XvG->text().isEmpty()) || (XbG->text().isEmpty()) ||
      (YvG->text().isEmpty()) ||(YbG->text().isEmpty()) )
    return;
  xvon=koordinate(XvG->text(),XvM->text(),OWCombo_v->currentItem()==0?1:-1);
  xbis=koordinate(XbG->text(),XbM->text(),OWCombo_b->currentItem()==0?1:-1);
  yvon=koordinate(YvG->text(),YvM->text(),NSCombo_v->currentItem()==0?1:-1);
  ybis=koordinate(YbG->text(),YbM->text(),NSCombo_b->currentItem()==0?1:-1);
  von_tile.setKoords(xvon,yvon,Zb->value());
  bis_tile.setKoords(xbis,ybis,Zb->value());
  updateInfo();
}
*/
void UKartendownload::createTileList(){
//  koordsChanged();
  QFile file(downloadFolder->text()+"tilenumbers.txt");
  if (!file.open(QIODevice::WriteOnly)){
    QMessageBox::warning(0,"Mist","Kann Datei "+file.name()+trUtf8(" nicht öffnen!"));
    return;
  }
  QFile exfile(downloadFolder->text()+"existingTiles.txt");
  exfile.open(QIODevice::WriteOnly);
  QTextStream exstream(&exfile);
  QTextStream stream(&file);
  uint neue=0;
  uint alle=0;
  for (int z=Zv->value();z<=Zb->value();z++){
    von_tile.newZ(z);
    bis_tile.newZ(z);
    for (int x=von_tile.x ;x<=bis_tile.x; x++){
      for (int y=bis_tile.y ; y<=von_tile.y;y++){	
        QString fn=downloadFolder->text()+"%1/%2/%3.png.tile";
        fn=fn.arg(z).arg(x).arg(y);
        if (QFile::exists(fn)){
          exstream<<fn<<'\n';
          if (nurNeue->isChecked()){
            alle+=1;
            continue;
          }
        }
        stream<<x<<'\t'<<y<<'\t'<<z<<'\n';
        neue++; alle++;
      }  
    }
  }
  file.close();
  QString info="<table><tr><td><b>erstellte Datei:</b></td><td>%1tilenumbers.txt und existingTiles.txt</td></tr>";
  info+=QString("<tr><td><b>Neue Kacheln</b></td><td>%2 </td></tr>")+
        "<tr><td><b>Kacheln insgesamt</b></td><td>%3</td></tr>";
  info=info.arg(downloadFolder->text()).arg(neue).arg(alle);
  infolabel->setText(info);
  //QMessageBox::information(0,"Hinweis",QString("Kachelliste tilenumbers.txt in %1 erfolgreich erstellt.\n
  //es sind %2 neue Kacheln (%3 insgesamt)").arg(downloadFolder->text()).arg(neue).arg(alle) );
}

void UKartendownload::download(){
  QFile turls(downloadFolder->text()+"tileurls.txt");
  if(!turls.open(QIODevice::WriteOnly)){
    QMessageBox::warning(0,"Mist","Kann Datei "+turls.name()+trUtf8(" nicht öffnen!"));
    return;
  }
  QTextStream outstream(&turls);
  QFile tilenrs(downloadFolder->text()+"tilenumbers.txt");
  if (!tilenrs.open(QIODevice::ReadOnly)){
    QMessageBox::warning(0,"Mist","Kann Datei "+tilenrs.name()+trUtf8(" nicht öffnen!"));
    return;
  }
  infolabel->setText(QString("erstelle die Datei %1").arg(turls.fileName())+trUtf8(" und verwende die dann für wget"));
  QTextStream instream(&tilenrs);
  while(!instream.atEnd()){
    uint x,y,z;
    instream>>x>>y>>z;
    outstream<<mapserver.getTileUrl(serverCombo->currentItem(),x,y,z)<<'\n';
  }
  turls.close();tilenrs.close();
  QString baseUrl=mapserver.bases[serverCombo->currentItem()];
  QString foldername=downloadFolder->text().replace(' ',"\\ ");
  QSettings settings("UBoss","karten");
  QString userAgent=settings.value("useragent","Mozilla/5.0 (X11; Ubuntu; Linux i686; rv:26.0) Gecko/20100101 Firefox/26.0").toString();
  if(userAgent.size()>0)
    userAgent=" --user-agent=\""+userAgent+"\" ";
  //QString hold=holdwget->isChecked()?" -hold ":"";
  QString befehl="cd "+foldername+
                 " && wget -i tileurls.txt -B "+baseUrl+userAgent+" -nH -r --cut-dirs=%1";
  befehl=befehl.arg(baseUrl.count('/')-3);
  //befehl="xterm "+hold+"-e \'"+befehl+"\'";
  //infolabel->setText(infolabel->text()+"<br>Starte mit "+befehl+" den download.");
  //std::system(befehl.utf8()); //hier stattdessen QProcess verwenden.
  Terminallog * oldlog=0;
  foreach(QWidget*widget ,qApp->topLevelWidgets()){
      if (widget->objectName()=="terminallog"){
          oldlog=(Terminallog*)widget;
          break;
      }
  }
  if(oldlog){
      if(QMessageBox::question(0,"Frage","Es existiert bereits ein Downloadfenster. Soll das geschlossen werden?",QMessageBox::Yes|QMessageBox::No)==
              QMessageBox::Yes)
          oldlog->close();
  }
  Terminallog * tl=new Terminallog("sh",QStringList()<<"-c"<<befehl);
  tl->setObjectName("terminallog");
  tl->setCloseOnFinished(!(holdwget->isChecked() ));
  tl->setDeleteOnClose();
  tl->move(-10,-10);
  tl->show();
  tl->raise();
  if(renameCheck->isChecked()){
      QTimer * renameTimer=new QTimer(this);
      connect(renameTimer,SIGNAL(timeout()),this,SLOT(renameTiles()));
      renameTimer->start(2000);
      connect(&tl->proc,SIGNAL(finished(int)),renameTimer,SLOT(deleteLater()));
      connect(&tl->proc,SIGNAL(finished(int)),this,SLOT(renameTiles()));
        //renameTiles();
      //QMessageBox::information(this,"Achtung","den Downloaddialog nicht schließen bevor der Download abgeschlossen ist!");
    }
}

void UKartendownload::renameTiles(){
  QString befehl="";
  if(mapserver.tileurls[serverCombo->currentItem()].contains('&'))
  {//jetzt wirds kompliziert.
    infolabel->setText("Zum Umbenennen: Erzeuge jetzt im Zielverzeichnis die Datei bashbefehle.");
    //Verzeichnisse erzeugen, falls nicht vorhanden
    QDir dir;
    QString baseDir=downloadFolder->text();
    QFile f(baseDir+"tilenumbers.txt");
    QFile g(baseDir+"bashbefehle");
    if(!f.open(QIODevice::ReadOnly)){
      QMessageBox::information(0,"Achtung","Die Datei tilenumbers.txt finde ich nicht.");
      return;
    }
    g.open(QIODevice::WriteOnly);
    Q3TextStream s(&f);
    Q3TextStream bash(&g);
    QString bD=baseDir;
    bash<<"cd "<<bD.replace(' ',"\\ ") <<"\n";
    int xalt=-1,zalt=-1;
    int x,y,z;
    while(!s.atEnd()){
      s>>x>>y>>z;
      QDir dir;
      if(z!=zalt){
	QString neu=baseDir+QString("%1").arg(z) ;
	if(!dir.exists(neu))
	  dir.mkdir(neu);
      }
      if(x!=xalt){
	QString neu=baseDir+QString("%1/%2").arg(z).arg(x);
	if(!dir.exists(neu))
	  dir.mkdir(neu);
      }
      bash<<"mv \'"<<mapserver.getTileUrl(serverCombo->currentItem(),x,y,z)<<"\' "<<z<<"/"<<
	  x<<"/"<<y<<".png.tile\n";
    }//alle nötigen Verzeichnisse erstellt und eine bash-Datei
    f.close();
    g.close();
    QString bef=QString("/bin/bash \"%1\"").arg(baseDir+"bashbefehle");
    infolabel->setText(infolabel->text()+"<br>Setze jetzt "+bef+" ab.");
    system(bef.latin1());
    //jetzt noch die erstellte bash-Datei ausführen.
  }else{//alle pngs befinden sich in den richtigen Verzeichnisssen.
      QFile urls(downloadFolder->text()+"tileurls.txt");
      if (!urls.open(QFile::ReadOnly)){
          QMessageBox::warning(0,"Achtung","Kann die Datei "+urls.fileName()+trUtf8(" nicht öffnen"));
          return;
      }
      QTextStream s(&urls);
      QString datei;
      while(!s.atEnd()){
          datei=s.readLine();
          QFile f(downloadFolder->text()+datei);
          if(!f.exists()) continue;
          int ind;
          QString newName=(ind=datei.indexOf('?'))==-1?
                               datei+".tile":
                               datei.mid(0,ind)+".tile";

          //qDebug()<<"newName ohne downloadFolder: "<<newName;
          QFile fnew(downloadFolder->text()+newName);
          //qDebug()<<"fnew.filename(): "<<fnew.fileName();
          if (fnew.exists()) fnew.remove();
           //qDebug()<<"nenne "<<f.fileName()<<" nun "<<QFileInfo(fnew).fileName();
          f.rename(fnew.fileName());
      }
/*    befehl="cd "+downloadFolder->text().replace(' ',"\\ ")+
    " && { for i in $(cat tileurls.txt) ; do if [ -s $i ] ; then mv $i ${i%\\?*}.tile ; fi  ; done }";
    infolabel->setText("Setze jetzt den Befehl "+befehl+" ab.");
    QProcess * renameProc = new QProcess;
    connect(renameProc,SIGNAL(finished(int)),renameProc,SLOT(deleteLater()));
    connect(renameProc,SIGNAL(finished(int)),this,SIGNAL(downloadReady()));
    renameProc->start("sh",QStringList()<<"-c"<<befehl);
*/
  }
  emit downloadReady();
}
/*
void UKartendownload::saveRegion(){
  infolabel->setText("speichere Region...");
  double vzy,vzx;
  vzy=NSCombo_v->currentItem()==0?1.0:-1.0;
  vzx=OWCombo_v->currentItem()==0?1.0:-1.0;
  Position von(vzy*YvG->text().toDouble(),vzy*YvM->text().toDouble(),
	       vzx*XvG->text().toDouble(),vzx*XvM->text().toDouble());
  vzy=NSCombo_b->currentItem()==0?1.0:-1.0;
  vzx=OWCombo_b->currentItem()==0?1.0:-1.0;
  Position bis(vzy*YbG->text().toDouble(),vzy*YbM->text().toDouble(),
	       vzx*XbG->text().toDouble(),vzx*XbM->text().toDouble());
  Ui::DeineEingabe ui;
  QDialog * eingabe =new QDialog(0);
  ui.setupUi(eingabe);
  //DeineEingabe * eingabe = new DeineEingabe(0);
  ui.editfeld->setText(Regionbox->title());
  if(eingabe->exec()!=QDialog::Accepted){
    delete eingabe;
    return;
  }
  Regionbox->setTitle(ui.editfeld->text());
  URegion region(ui.editfeld->text().utf8(),von,bis);
  delete eingabe;
  ifstream is;
  is.open("regions.txt");
  ofstream os;
  if(!is.good()){
    os.open("regions.txt");
    os<<region;
    os.close();
    return;
  }
  os.open("regions.txt~");
  URegion r;
  bool ersetzt=false;
  while(is.good()){
    is>>r;
    if(r.getName()=="")break;
    if(r.getName()!=region.getName())
      os<<r;
    else{
      os<<region;
      ersetzt=true;
    }
  }
  if(!ersetzt) os<<region;
  is.close();
  os.close();
  system("mv regions.txt~ regions.txt");
}
void UKartendownload::loadRegion(){
  ifstream is;
  is.open("regions.txt");
  if(!is.good()){
    QMessageBox::information(0,"Mist","Keine Regionen vorhanden.");
    return;
  }
  QStringList namen;
  URegion reg;
  while(is.good()){
    is>>reg;
    namen<<QString(reg.getName());
  }
  //is.seekg(0,ios_base::beg);
  is.close();
  Ui::RegionWahl wahl;
  QDialog wahldial(0);
  wahl.setupUi(&wahldial);
  wahl.listBox->insertStringList(namen);
  if(wahldial.exec()!=QDialog::Accepted){
    //is.close();
    return;
  }
  is.open("regions.txt");
  is>>reg;
  while(QString(reg.getName())!=wahl.listBox->currentText() && is.good() ) is>>reg;
  reg.setName(wahl.listBox->currentText());
  setRegion(reg);
}*/

void UKartendownload::setRegion(const Position &von, const Position &bis){
    von_tile.copyPosition(von);
    bis_tile.copyPosition(bis);
}
void UKartendownload::manageServer(){
  MapserverManager man(mapserver);
  if(man.exec()==QDialog::Accepted){
    serverCombo->clear();
    //jetzt Daten fÃ¼r den mapserver aus man-Tabelle holen!
    serverCombo->addItems(mapserver.names);

  }

}
void UKartendownload::on_deleteTilesButton_clicked(){
  QFile f(downloadFolder->text()+"existingTiles.txt");
  if(!f.open(QIODevice::ReadOnly)){
      QMessageBox::warning(0,"Achtung!",trUtf8("Kann die Datei %1existingTiles.txt nicht öffnen").arg(downloadFolder->text()));
      return;
  }
  QTextStream s(&f);
  QString z;int n=0;
  while (!s.atEnd()){
    s>>z;
    if(z.trimmed().length()==0)
      continue;
    //qWarning("lese %s",z.latin1());
    n++;
  }
  s.seek(0);
  s.reset();
  if(QMessageBox::question(0,"Frage",trUtf8("Sollen %1 Kacheln gelöscht werden?").arg(n),QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes)==
    QMessageBox::Yes){
      while(!s.atEnd()){
        s>>z;
        if(z.trimmed().length()==0)
          continue;
        //qWarning("lese %s",z.latin1());
        if(!QFile::remove(z))
          QMessageBox::warning(0,"Hinweis",trUtf8("kann die Datei %1 nicht löschen").arg(z));
     }
  }
  f.close();
  emit(downloadReady());
}

void UKartendownload::on_copyTilesButton_clicked(){
  QString targetDir=QFileDialog::getExistingDirectory(0,trUtf8("Kartenverzeichnis, in das die gewählten Karten kopiert werden sollen"));
  QFile dat(downloadFolder->text()+"existingTiles.txt");
  if(!dat.open(QIODevice::ReadOnly)){
      QMessageBox::warning(0,"Achtung!",trUtf8("Kann die Datei %1existingTiles.txt nicht öffnen").arg(downloadFolder->text()));
      return;
  }
  QString tempname=downloadFolder->text()+"copyFiles.txt";
  if(!dat.copy(tempname)){
      QMessageBox::information(0,"Achtung","Die Datei "+tempname+" kann nicht als Kopie von existingTiles.txt erstellt werden"\
                               "vermutlich existiert sie bereits. Wenn kein anderer Kopierprozess noch aktiv ist, dann"\
                               "kann sie problemlos gelöscht werden" );
      return;
  }
  QFile * f=new QFile(tempname);
  f->open(QIODevice::ReadOnly);
  QTextStream *s=new QTextStream(f);
  QString z;int n=0;
  while (!s->atEnd()){//zähle die Anzahl der kopierfähigen Kacheln
    *s>>z;
    if(z.trimmed().length()==0)
      continue;
    //qWarning("lese %s",z.latin1());
    n++;
  }
  s->seek(0);
  s->reset();
  if(QMessageBox::question(0,"Frage",QString("Sollen bis zu %1 Kacheln nach %2 kopiert werden?").arg(n).arg(targetDir),QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes)==
    QMessageBox::No){
      f->close();
      f->remove();
      delete s; delete f;
      return;
  }
  progressBar->setRange(0,(n/10==0?1:n/10));
  progressBar->setValue(0);
  //hier nun den Kopier-Thread aufrufen
  CopyThread * th=new CopyThread(s,QDir(targetDir));
  connect(th,SIGNAL(fileCopied(int)),progressBar,SLOT(setValue(int)));
  connect(th,SIGNAL(finished()),th,SLOT(deleteLater()));
  th->start();
}
CopyThread::CopyThread(QTextStream*st,QDir d):QThread(){
  stream=st;dir=d;
}
CopyThread::~CopyThread(){
  stream->device()->close();
  QFile * f=(QFile*) stream->device();
  f->remove();
  delete f;
  delete stream;
  //qWarning("copyThread-Destruktor hat stream und file deletet");
}

void CopyThread::run(){
  QString z;
  int copied=0;//Anzahl der neu erstellten Dateien
  int count=0;//Anzahl der gelesenen Dateien.
  QString targetDir=dir.absolutePath();
  qWarning("targetDir: %s",targetDir.toUtf8().data());
  while(!stream->atEnd()){
    *stream>>z;
    if(z.trimmed().length()==0)
      continue;
    QFile source(z);
    int wo=z.indexOf(QRegExp("/[0-9]+/[0-9]+/[0-9]+.png.tile"));
    QString targetPath=z=z.mid(wo+1);
    QString zKoord=z.mid(0,z.indexOf('/'));
    z=z.mid(z.indexOf('/')+1);
    QString yKoord=z.mid(0,z.indexOf('/'));
    QString xTile=z.mid(z.indexOf('/')+1);
    if (!dir.exists(zKoord))
      dir.mkdir(zKoord);
    dir.cd(zKoord);
    if (!dir.exists(yKoord))
      dir.mkdir(yKoord);
    dir.cdUp();
    if(source.copy(targetDir+"/"+targetPath))
      copied++;
    count++;
    if(count % 10==0){
      emit fileCopied(count/10);
        //qWarning("filecopied(%d)-signal",count/10);
    }
  }
  if(count<10) emit fileCopied(1);
  //QMessageBox::information(0,"Info",QString("Es wurden %1 Dateien von %2 kopiert (die andern gabs wohl schon)").arg(copied).arg(count));
  //der Aufruf führt zum Absturz diese Info müsste per SIGNAL an den GUI-Thread geschickt werden.
}

void UKartendownload::on_uaButton_clicked(){
  QSettings settings("UBoss","karten");
  QString userAgent=settings.value("useragent").toString();
  QDialog dial(0);
  Ui::uaDialog ui;
  ui.setupUi(&dial);
  if (userAgent.size()>0)
    ui.ua->setText(userAgent);
  if(dial.exec()==QDialog::Accepted){
      userAgent=ui.ua->text();
  }
  settings.setValue("useragent",userAgent);
}

void UKartendownload::on_defaultMapserverButton_clicked()
{
   int i = serverCombo->currentIndex();
   Mapserver ms;
   ms.append(mapserver.names.at(i),mapserver.bases.at(i),mapserver.tileurls.at(i),mapserver.descriptions.at(i));
   QString filename=downloadFolder->text();
   if(!filename.endsWith('/')) filename+='/';
   filename+="mapserver_default.txt";
   ms.writeToFile(filename);
   QMessageBox::information(0,"Info",trUtf8("<h2>default Mapserver</h2>"
        " für dieses Verzeichnis auf <i>%1</i> festgelegt."
        "<p>Wenn das nicht ok ist, dann wähle einen anderen Mapserver aus, "
        "bzw. lösche die Datei <i>%2</i>.</p>").arg(ms.names.at(0)).arg(filename));
}

void UKartendownload::mapserverChanged(int i){
  mapserverDescription->setText(mapserver.descriptions[i]);
}
/** Ab hier die Methoden der Klasse Mapserver **/
QString Mapserver::getTileUrl(int i,uint x,uint y, uint z){
  if (i>=bases.size()){
    return "";
  }
  QString tu=tileurls[i];
  QString zahl="%1";
  int pos=tu.find("[x]");
  if(pos<0) {
    pos=tu.find("[X]");
    if (pos<0) return "";
  }
  tu=tu.replace(pos,3,zahl.arg(x));
  pos=tu.find("[y]");
  if(pos<0) {
    pos=tu.find("[Y]");
    if (pos<0) return "";
  }
  tu=tu.replace(pos,3,zahl.arg(y));
  pos=tu.find("[z]");
  if(pos<0) {
    pos=tu.find("[Z]");
    if (pos<0) return "";
  }
  tu=tu.replace(pos,3,zahl.arg(z));
  return tu;
}
Position UKartendownload::linksOben(){
  Position p(von_tile.latitude,von_tile.longitude);
  p.setZ(Zv->value());
  return p;
}
void UKartendownload::turboDownload(){
  createTileList();
  renameCheck->setChecked(true);
  download();
  renameTiles();
}

/**  Ab hier Mapserver-Funktionen *******************************************************************/
void Mapserver::append(QString name,QString base,QString tileurl,QString description){
  names.push_back(name);bases.push_back(base);tileurls.push_back(tileurl);descriptions.push_back(description);return;
}
ostream& operator<<(ostream & st,const Mapserver & server){
  for(short i=0;i<server.names.count();i++){
    st<<server.names[i].latin1()
        <<'\t'<<server.bases[i].latin1()
        <<'\t'<<server.tileurls[i].latin1()
        <<server.descriptions[i].latin1()<<'\n';
  }
  return st;
}

istream& operator>>(istream&s,Mapserver & server){
  server.clear();
  char z[300];
  while(s.good()){
    s.getline(z,299);
    QStringList l=QString(z).split('\t');
    server.names<<l[0];server.bases<<l[1];server.tileurls<<l[2];server.descriptions<<l[3];
  }
  return s;
}
void Mapserver::writeToFile(const QString filename){
  QFile f(filename);
  if(!f.open(QIODevice::WriteOnly)){
    QMessageBox::information(0,"Achtung","Kann Datei 'mapserver.txt' nicht öffnen!");
    return;
  }
  for(short i=0;i<names.count();i++){
    QString zeile=names[i]+'\t'+bases[i]+'\t'+tileurls[i]+'\t'+descriptions[i];
    f.write(zeile.toLatin1());
    f.write("\n");
  }
  f.close();
}
bool Mapserver::readFromFile(const QString filename){
  QFile f(filename);
  if(!f.open(QIODevice::ReadOnly)){
    QMessageBox::information(0,"Achtung",QString::fromUtf8("Kann Datei %1 nicht öffnen!").arg(filename));
    return false;
  }
  clear();
  QByteArray z;
  while(!f.atEnd()){
    z=f.readLine(300);
    QStringList l=QString(z).split('\t');
    names<<l[0];bases<<l[1];tileurls<<l[2];descriptions<<l[3].trimmed();
  }
  f.close();
  return true;
}
void Mapserver::clear(){
  names.clear();bases.clear();tileurls.clear();descriptions.clear();
}
