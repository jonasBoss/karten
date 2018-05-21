#ifndef TRACKTIMER_H
#define TRACKTIMER_H

#include <QWidget>
#include <QtCore>
#include "ui_tracktimer.h"
namespace Ui {
class TrackTimer;
}

class TrackTimer : public QWidget
{
    Q_OBJECT

public:
    QTimer timer;
    explicit TrackTimer(QWidget *parent = 0);
    ~TrackTimer();
    void expandRange(int von, int bis);//erweitert den Zeitraum so, dass minTime und maxTime auf alle Fälle im Zeitraum sind.
    void setRange(QDateTime von,QDateTime bis);
    void setRange(int von, int bis);//gleiche Funktion mit Zeitstempeln (Sekunden nach 1.1.2000 O Uhr);
    bool isActive(){return timer.isActive();}
    int getTimestamp(){return ui->timeSlider->value();}
private:
    Ui::TrackTimer *ui;
    int minTime,maxTime;//minimale Timestamps und maximale Timestamps für den Slider
    void adjustMinMax();//stellt Slider und Zeitanzeige auf die durch minTime und maxTime gegebenen Mindestwerte
    int delay;//verzögerung zwischen zwei Ticks in Millisekunden. (reale Zeit)
    int interval;//Zeitfortschritt zwischen zwei Ticks in Zentisekunden. (Beim Simulierten Flug) kann auch >100 sein
    short centis;//zählt Zentisekunden rückwärts bis zum nächsten Weiterstellen des Timesliders. (der Sekunden zählt)<=100
signals:
  void timeChanged(int stamp,int interval);
  void showDialogs();//können die TrackDialoge empfangen um sich zu zeigen.
  void intervalChanged(int interval);
private slots:
  void tick();
  void on_trackDialogButton_clicked();
  void on_startButton_clicked();
  void on_speedIncButton_clicked();
  void on_speedDecButton_clicked();
  void on_timeSlider_sliderMoved(int position);
  void on_closeButton_clicked();
};

#endif // TRACKTIMER_H
