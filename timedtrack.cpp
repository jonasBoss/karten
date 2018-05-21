#include "timedtrack.h"
#include "timedtrackdialog.h"
#include "ui_timedtrackdialog.h"
#include "tracktimer.h"
#include "region.h"
#include "Atlas.h"
#include <QtCore>
#include <QPainter>
#include <cmath>
/** Methoden von TimedTrack() *********************************************/
TrackTimer * TimedTrack::tracktimer=0;
short TimedTrack::count=0;
QPoint TimedTrack::AtlasKoordinaten(const QPoint &p)
{
  return (QPoint(p.x()>>shift,p.y()>>shift)-atlas->getPos().getglobalPixelKoords())*scale;
}

TimedTrack::TimedTrack()
{
  bild=QPixmap("./images/drachen.png");
  count++;
}
TimedTrack::TimedTrack(Atlas *a){
  count++;
  qWarning("habe eben den %d. Track erstellt",count);
  if(tracktimer==0){
      tracktimer=new TrackTimer(a);
      //tracktimer->show();
  }
  atlas=a;
  bild=QPixmap(":/images/drachen.png");
  bildLabel=new QLabel(atlas);
  bildLabel->setPixmap(bild);
  bildLabel->hide();
}

TimedTrack::~TimedTrack()
{
  delete bildLabel;
  count-=1;
  if(count==0){
      qWarning("letzter Track gelöscht. Schließe den Tracktimer.");
      delete tracktimer;
      tracktimer=0;
  }
}

bool TimedTrack::readFromIGCFile(QTextStream &igc){
  QString zeile;
  date=TimedTrackpoint::zeroTime.date();
  pilot="unbekannt";
  do{
     zeile=igc.readLine();
     if(zeile.length()==0 && !igc.atEnd()) continue;
     if(zeile.length()>5 && zeile.mid(0,5)=="HFDTE"){
       date=QDate(2000+zeile.mid(9,2).toInt(),zeile.mid(7,2).toInt(),zeile.mid(5,2).toInt());
     }
     if(zeile.length()>10 && zeile.mid(0,10)=="HFPLTPILOT"){
         int p=zeile.indexOf(':');
         if(p>0) pilot=zeile.mid(p+1);
     }
  }while(!igc.atEnd() && zeile[0]!='B');
  bool r=false;
  if(zeile[0]=='B'){//einen Trackpoint gefunden. Diese Zeilen beginnen mit B
    append(TimedTrackpoint(zeile,date));
    r=true;
  }
  while(!igc.atEnd()){
    zeile=igc.readLine();
    if(zeile[0]=='B'){//einen Trackpoint gefunden. Diese Zeilen beginnen mit B
      append(TimedTrackpoint(zeile,date));
    }
  }
  if(r){//Daten an den TrackTimer übertragen
    if(!tracktimer->isActive()){
      tracktimer->expandRange(TimedTrackpoint::getTimestamp( getFirstDateTime()),
                            TimedTrackpoint::getTimestamp( getLastDateTime()));
      atlas->showCentered(Position(first().latitude,first().longitude));
      showAtTime(getFirstDateTime(),1);
    }else{//Timer ist aktiv. Nun muss Drachen dort losfliegen wo timer steht.
      showAtTime(tracktimer->getTimestamp(),1);
    }
    bildLabel->show();
    shift=20-atlas->getPos().getZ();
    scale=atlas->getScale();
    connect(atlas,SIGNAL(viewportChanged()),this,SLOT(repaint()),Qt::UniqueConnection);
    connect(tracktimer,SIGNAL(timeChanged(int,int)),this,SLOT(showAtTime(int,int)),Qt::UniqueConnection);
    connect(&(tracktimer->timer),SIGNAL(timeout()),this,SLOT(showNext()),Qt::UniqueConnection);
    connect(tracktimer,SIGNAL(intervalChanged(int)),this,SLOT(setInterval(int)), Qt::UniqueConnection);
    if(ttdialog){
        int ind=ttdialog->tabs->indexOf(ttdialog);
        ttdialog->tabs->setTabLabel(ttdialog,pilot);
        ttdialog->tabs->setTabIcon(ind,*(bildLabel->pixmap()));
    }
  }
  return r;
}
void TimedTrack::setDate(const QDate &date){
  for(TimedTrack::Iterator it=begin();it!=end();it++){
    (*it).setDate(date);
  }
}

QDateTime TimedTrack::getFirstDateTime()
{
  if(!isEmpty()){
      return QDateTime(first().getTime());
  }
  else return QDateTime(QDate(2000,1,1),QTime(0,0,0));
}

QDateTime TimedTrack::getLastDateTime()
{
    if(!isEmpty()){
        return QDateTime(last().getTime());
    }
    else return QDateTime(QDate(2000,1,1),QTime(0,0,0));
}

void TimedTrack::transmitRange()
{
   if(!isEmpty()){
       tracktimer->setRange(getFirstDateTime(),getLastDateTime());
       showAtTime(getFirstDateTime(),1);
      // animate();
   }
}

void TimedTrack::adjustHeight(int hoehe)
{
   int oldhoehe;
   if(itnext==begin())
       oldhoehe=(*itnext).height;
   else
       oldhoehe=(*(itnext-1)).height;
   int dazu=hoehe-oldhoehe;
   for(TimedTrack::Iterator it=begin();it!=end();++it){
       (*it).height+=dazu;
   }
}


void TimedTrack::repaint(){
    //bool vis;
    shift=20-atlas->getPos().getZ();
    scale=atlas->getScale();
    tpatlas=AtlasKoordinaten(tp);
    //QPoint dorthin=atlas->getPixelKoords(tp,20,vis) * atlas->getScale();
    //bildLabel->move(dorthin-QPoint(bildLabel->width()/2,bildLabel->height()/2));
    bildLabel->move(tpatlas-QPoint(bildLabel->width()/2,bildLabel->height()/2));
    if(!bildLabel->isVisible()) bildLabel->show();
}
/* kann für Debugging-Zwecke nützlich sein
void TimedTrack::animate()
{
    if(isEmpty()) return;
    showAtTime(first().getTime(),20);
    QTimer * animationTimer=new QTimer;
    animationTimer->setSingleShot(false);
    QObject::connect(animationTimer,SIGNAL(timeout()),this,SLOT(showNext()));
    atlas->showCentered(Position(first().latitude,first().longitude));
    animationTimer->start(20,false);
}
*/
void TimedTrack::showAtTime(const QDateTime & time, int inter)
{
  timestamp=TimedTrackpoint::getTimestamp(time);
  showAtTime(timestamp,inter);
}

void TimedTrack::showAtTime(int stamp, int inter)
{
    timestamp=stamp;
    centis=0;
    interval=inter;//soviele Hundertstelsekunden vergehen zwischen Ticks, kann 0 sein.
    if (isEmpty()) return;
    //bildLabel->setPixmap(bild);
    //bildLabel->resize(bildLabel->sizeHint());
    TimedTrack::const_iterator it=constBegin();
    if((*it).getTimestamp()>stamp){//bereits der erste Punkt ist in der Zukunft
       diff=QPoint(0,0);
       if(interval==0) ticks=(1<<(sizeof(int)*8-1));
       else ticks=((*it).getTimestamp()-stamp)*100/interval;//da interval centiSekunden bis zum nächsten Tick vergehen.
       itnext=it;
       tp=Position::getglobalPixelKoords((*it).latitude,(*it).longitude);
       tpatlas=AtlasKoordinaten(tp);
       repaint();
       return;
    }
    //erster Punkt ist nicht in der Zukunft:
    while(it!=constEnd() && (*it).getTimestamp()<=stamp )
        it++;
    if(it==constEnd()){//der ganze Timetrack liegt in der Vergangenheit
      itnext=it-1;
      const TimedTrackpoint & point=last();
      ticks=1<<(sizeof(int)*8-1);
      diff=QPoint(0,0);
      tp=Position::getglobalPixelKoords(point.latitude,point.longitude);
      bildLabel->setPixmap(bild);
      bildLabel->resize(bildLabel->sizeHint());
      repaint();
    }else{//der stamp liegt zwischen it-1 und it.
       itnext=it;
       QPoint p1=(*(it-1)).getglobalPixelKoords();
       QPoint p2=(*it).getglobalPixelKoords();
       int t1=(*(it-1)).getTimestamp();
       int t2=(*it).getTimestamp();
       tp=p1+(p2-p1)*(stamp-t1)/(t2-t1);
       //tpatlas=AtlasKoordinaten(tp);//wird in repaint berechnet.
       ticks=(interval==0) ? (1<<(sizeof(int)*8-1)): ((t2-stamp)*100/interval);
       diff=(p2-p1)*interval/100/(t2-t1);
       //bool vis;
       //movdiff=(atlas->getPixelKoords(p2,20,vis)-atlas->getPixelKoords(p1,20,vis))
       //           *interval*atlas->getScale()/(t2-t1);
       repaint();
    }
}

void TimedTrack::showNext()
{

  if((centis+=interval)>=100){
      timestamp+=centis/100;
      centis=centis % 100;
  }
  if(ticks>0){
    QPoint schieb=AtlasKoordinaten(tp=tp+diff)-tpatlas;
    bildLabel->move(bildLabel->pos()+schieb);
    tpatlas+=schieb;
    ticks-=1;
  }else{//ticks==0, d.h. der nächste Tick führt in ein nächstes Streckenintervall, das itnext folgt.
    if(itnext+1==constEnd()){//Ende des Tracks erreicht.
      tp=last().getglobalPixelKoords();
      diff=QPoint(0,0);
      tpatlas=AtlasKoordinaten(tp);
      ticks=1<<(sizeof(int)*8-1);
      //movdiff=diff=QPoint(0,0);
      repaint();
      return;
    }//Ende des Tracks noch nicht erreicht Drachen kommt ins nächste Intervall
    short i=1;
    while(itnext+i!=constEnd() && (*(itnext+i)).getTimestamp() <timestamp){
        i++;
    }
    if(itnext+i==constEnd()){
        tp=last().getglobalPixelKoords();
        diff=QPoint(0,0);
        tpatlas=AtlasKoordinaten(tp);
        ticks=1<<(sizeof(int)*8-1);
        //movdiff=diff=QPoint(0,0);
        repaint();
        return;
    }//
    QPoint p1=(*itnext).getglobalPixelKoords();
    QPoint p2=(*(itnext+i)).getglobalPixelKoords();
    int t1=(*itnext).getTimestamp();
    int t2=(*(itnext+i)).getTimestamp();
    itnext=itnext+i;
    tp=p1+(p2-p1)*(timestamp-t1)/(t2-t1);
    //bool vis;
    tpatlas=AtlasKoordinaten(tp);
    ticks=(interval==0) ? (1<<(sizeof(int)*8-1)): ((t2-timestamp)*100/interval);
    diff=(p2-p1)*interval/((t2-t1)*100);
    double angle=atan2(diff.x(),-diff.y())*180.0/M_PI;
 //neue Höhe aufs Bild schreiben
    QPixmap neu=bild.transformed(QTransform().rotate(angle));
    QPainter *p =new QPainter(&neu);
    p->setBackgroundColor(QColor(255,255,255,192));
    p->setBackgroundMode(Qt::OpaqueMode);
    p->drawText(neu.rect(),Qt::AlignHCenter|Qt::AlignVCenter,QString("%1").arg((*itnext).height));
    if(itnext!=begin()){
        int steigrate=100*((*itnext).height-(*(itnext-1)).height)/((*itnext).timestamp-(*(itnext-1)).timestamp);
        QColor c;
        c.setHsv(( (steigrate+200)*4/10+360)%360,0xFF,0xFF,192);
        p->setBrush(c);
        p->drawRect(neu.width()/2-18,neu.height()/2+4,36,5);
    }
    delete p;
    bildLabel->setPixmap(neu);//.transformed(QTransform().rotate(angle)));
    bildLabel->resize(bildLabel->sizeHint());
    //movdiff=(atlas->getPixelKoords(p2,20,vis)-atlas->getPixelKoords(p1,20,vis))
    //           *interval*atlas->getScale()/(t2-t1);
    bildLabel->move(tpatlas-QPoint(bildLabel->width()/2,bildLabel->height()/2));
  }
}

void TimedTrack::hide()
{

}

