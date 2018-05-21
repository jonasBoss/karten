#include "tracktimer.h"
#include "ui_tracktimer.h"
#include "timedtrackpoint.h"
#include <QtCore>
#include <QtGui>

TrackTimer::TrackTimer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TrackTimer)
{
    ui->setupUi(this);
    connect(&timer,SIGNAL(timeout()),this,SLOT(tick()));
    setRange(0,1000);
    delay=20;//50 Hz Ticker
    interval=20;// 1/5 Sekunde Zeitfortschritt pro Tick.
    setBackgroundColor(QColor(255,255,255,164));
    setBackgroundRole(QPalette::Window);
    centis=100;
}

TrackTimer::~TrackTimer()
{
    delete ui;
}

void TrackTimer::expandRange(int von, int bis)
{
  if(minTime==0)
      setRange(von,bis);
  else{
      if(minTime>von)
          minTime=von;
      if(maxTime<bis)
          maxTime=bis;
      adjustMinMax();
  }
}

void TrackTimer::setRange(QDateTime von, QDateTime bis)
{
   setRange(TimedTrackpoint::getTimestamp(von),TimedTrackpoint::getTimestamp(bis));
}

void TrackTimer::setRange(int von, int bis)
{
  minTime=von;maxTime=bis;
  adjustMinMax();
}

void TrackTimer::adjustMinMax()
{
    ui->timeSlider->setMinimum(minTime);
    ui->timeSlider->setMaximum(maxTime);
    ui->timeSlider->setValue(minTime);
    ui->dateTime->setDateTime(TimedTrackpoint::getDateTime(minTime));
}

void TrackTimer::tick()
{
  int tim=ui->timeSlider->value();
  if(tim>=maxTime){
      timer.stop();
      ui->startButton->setIcon(QPixmap(":/images/media-playback-start"));
      return;
  }
  if((centis-=interval)<=0){
    short step=(-centis)/100+1;//soviele Sekunden ist der Slider weiterzustellen.
    ui->timeSlider->setValue(tim+step>maxTime?maxTime:(tim+step));
    ui->dateTime->setTime(ui->dateTime->time().addSecs(step));
    centis+=100*step;
  }
}

void TrackTimer::on_trackDialogButton_clicked()
{
    emit showDialogs();
}

void TrackTimer::on_startButton_clicked()
{
    if(timer.isActive()){
      ui->startButton->setIcon(QPixmap(":/images/media-playback-start"));
      timer.stop();
    }else{
      emit timeChanged(ui->timeSlider->value(),interval);
      centis=100;
      ui->startButton->setIcon(QPixmap(":/images/media-playback-pause"));
      timer.start(delay,false);
    }
}

void TrackTimer::on_speedIncButton_clicked()
{
    if ((delay*8+3)/10 >= 16){
      delay=(delay*8+3)/10;
      timer.setInterval(delay);
    }else{
      if((125*interval+51)/100>interval)
          interval=(125*interval+51)/100;
      else interval++;
      centis=100;
      //emit intervalChanged(interval);
      emit timeChanged(ui->timeSlider->value(),interval);
    }
    ui->factorLabel->setText(QString(" x %1 ").arg((10.0*interval)/delay,0,'f',2));
}

void TrackTimer::on_speedDecButton_clicked()
{
  if((delay*125)/100 < 30){
    delay=(delay*125)/100;
    timer.setInterval(delay);
  }else{
    interval=interval*8/10;
    centis=100;
    //emit intervalChanged(interval);
    emit timeChanged(ui->timeSlider->value(),interval);
  }
  ui->factorLabel->setText(QString(" x %1 ").arg(10.0*interval/delay,0,'f',2));
}

void TrackTimer::on_timeSlider_sliderMoved(int position)
{
/*  qWarning("Slider moved to %d",position);
  QDateTime dt=TimedTrackpoint::zeroTime.addSecs(position);
  dt=QDateTime(QDate(2000,1,1),QTime(0,0));
  qWarning(dt.toString("Zeit: dd-MM-yyyy hh-mm-ss").toUtf8().constData());*/
  ui->dateTime->setDateTime(TimedTrackpoint::getDateTime(position));
  emit timeChanged(position,interval);
}

void TrackTimer::on_closeButton_clicked()
{
   hide();
}

