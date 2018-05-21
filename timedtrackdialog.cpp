#include "timedtrackdialog.h"
#include "ui_timedtrackdialog.h"
#include "ui_hoehendialog.h"
#include "timedtrack.h"
#include "Atlas.h"
#include <QList>
#include <QTabWidget>
QTabWidget* TimedTrackDialog::tabs = 0;
TimedTrackDialog::TimedTrackDialog(TimedTrack *timet, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TimedTrackDialog)
{
    ui->setupUi(this);
    if(tabs==0){
        tabs=new QTabWidget(0);
        tabs->setAttribute(Qt::WA_QuitOnClose,false);
        tabs->setWindowTitle(QString::fromUtf8("Flüge"));
        connect(tt->tracktimer,SIGNAL(showDialogs()),tabs,SLOT(show()));
        tabs->setIcon(QPixmap(":/images/drachen.png"));
    }
    tabs->addTab(this,QPixmap(":/images/drachen.png"),QString("NN"));
    tabs->adjustSize();
    tabs->show();
    tt=timet;
    adjustSize();
}

TimedTrackDialog::~TimedTrackDialog()
{
    delete ui;
}

bool TimedTrackDialog::init()
{
  tt->ttdialog=this;
  if(!on_ladenButton_clicked()){
      QMessageBox::information(0,"Achtung","Laden eines Tracks fehlgeschlagen.");
  }
  show();
  if(tt->isEmpty()){
      return false;
  }else{//Initialisierung geglückt
    int ind=tabs->indexOf(this);
    tabs->setTabLabel(this,tt->pilot);
    tabs->setTabIcon(ind,ui->bildButton->icon());
    return true;
  }
  adjustSize();
  repaint();
}

bool TimedTrackDialog::on_ladenButton_clicked()
{
  if(!tt->isEmpty() &&
      QMessageBox::question(0,"Frage",
                     trUtf8("Es ist bereits ein Track geladen, soll der wirklich überschrieben werden?"),
                     QMessageBox::Yes|QMessageBox::No,QMessageBox::No)==QMessageBox::No){
      return false;
  }
  QSettings settings("UBoss","karten");
  QString dir=settings.value(trUtf8("Flügeverzeichnis"),".").toString();
  QString fname=QFileDialog::getOpenFileName(dir,"*.igc",0,"igcopendialog","IGC-Datei wählen");
  if(fname.isNull()) return false;
  QFile f(fname);
  dir= QFileInfo(f).absolutePath();
  settings.setValue( trUtf8("Flügeverzeichnis"),dir);
  if(!f.open(QIODevice::ReadOnly)){
      QMessageBox::information(0,"Achtung",trUtf8("Konnte die Datei %1 nicht öffnen").arg(fname));
      return false;
  }
  if(!tt->isEmpty()) tt->clear();
  QTextStream s(&f);
  if((!tt->readFromIGCFile(s))||(tt->isEmpty())){
      QMessageBox::information(0,"Achtung",QString::fromUtf8("laden der Datei %1 fehlgeschlagen").arg(fname));
      return false;
  }
  ui->startZeit->setDateTime(tt->getFirstDateTime());
  ui->landungZeit->setText(tt->getLastDateTime().toString("dd.MM.yyyy  hh.mm"));
  ui->filename->setText(fname);ui->filename->resize(ui->filename->sizeHint());
  ui->pilot->setText(tt->pilot);
  ui->pilot->adjustSize();
  return true;
}

void TimedTrackDialog::on_bildButton_clicked()
{
    QString fname=QFileDialog::getOpenFileName(".","Bilder (*.png *.xpm *.jpg)");
    //QString fname=QFileDialog::getOpenFileName(":","*");
    if(fname.isNull()) return;
    QPixmap pix(fname);
    if(!pix.isNull()){
      tt->setBild(pix);
      ui->bildButton->setIcon(pix);
      ui->bildButton->repaint();
    }
}

void TimedTrackDialog::on_startZeit_dateTimeChanged(const QDateTime &tnew)
{
   QDateTime told=tt->first().getTime();
   int diff=told.secsTo(tnew);//diff muss nun auf alle Zeiten draufgeschlagen werden.
   for(TimedPointList::Iterator it=tt->begin();it!=tt->end();++it){
       (*it).addSeconds(diff);
   }
   ui->landungZeit->setText(tt->last().getTime().toString("dd.MM yyyy  hh:mm"));
}

void TimedTrackDialog::on_cancelButton_clicked()
{
   delete tt;
   qWarning("timed Track deleted");
   //tabs->removeTab(tabs->indexOf(this));
   if(tt->count==0){
       delete tabs;
       tabs=0;
       qWarning("tabs geloescht");
   }else
     deleteLater();
}

void TimedTrackDialog::on_openTimerButton_clicked()
{
  TrackTimer & ti=*TimedTrack::tracktimer;
  ti.show();
  ti.resize(ti.sizeHint());
}

void TimedTrackDialog::on_transmitRangeButton_clicked()
{
  tt->transmitRange();
}

void TimedTrackDialog::on_newButton_clicked()
{
    tt->atlas->addTimedTrack();
}

void TimedTrackDialog::on_bluebird_clicked()
{
    QPixmap pix(":/images/drachen_blau.png");
    tt->setBild(pix);
    ui->bildButton->setIcon(pix);
    ui->bildButton->repaint();
    tabs->setTabIcon(tabs->indexOf(this),pix);
}

void TimedTrackDialog::on_redbird_clicked()
{
    QPixmap pix(":/images/drachen.png");
    tt->setBild(pix);
    ui->bildButton->setIcon(pix);
    ui->bildButton->repaint();
    tabs->setTabIcon(tabs->indexOf(this),pix);
}

void TimedTrackDialog::on_pilot_textChanged(const QString &arg1)
{
    tt->pilot=arg1;
    tabs->setTabLabel(this,arg1);
}


void TimedTrackDialog::on_expandRangeButton_clicked()
{
    tt->tracktimer->expandRange(TimedTrackpoint::getTimestamp( tt->getFirstDateTime()),
                          TimedTrackpoint::getTimestamp( tt->getLastDateTime()));
}

void TimedTrackDialog::on_heightButton_clicked()
{
  QDialog * dial=new QDialog(this);
  Ui::hoehenDialog ui;
  ui.setupUi(dial);
  if(dial->exec()==QDialog::Accepted){
    int newhoehe=ui.hoehe->text().toInt();
    if(newhoehe<=0 || newhoehe>=4000){
      if(QMessageBox::question(0,"Frage",trUtf8("Soll wirklich die Höhe %1 m übernommen werden?").arg(newhoehe),
         QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes)==QMessageBox::No){//Nein
          delete dial;
          return;
      }
    }
    delete dial;
    tt->adjustHeight(newhoehe);
  }else{
      delete dial;
  }
}

void TimedTrackDialog::on_greenbird_clicked()
{
    QPixmap pix(":/images/drachen_gruen.png");
    tt->setBild(pix);
    ui->bildButton->setIcon(pix);
    ui->bildButton->repaint();
    tabs->setTabIcon(tabs->indexOf(this),pix);
}


void TimedTrackDialog::on_yellowbird_clicked()
{
    QPixmap pix(":/images/drachen_gelb.png");
    tt->setBild(pix);
    ui->bildButton->setIcon(pix);
    ui->bildButton->repaint();
    tabs->setTabIcon(tabs->indexOf(this),pix);
}
