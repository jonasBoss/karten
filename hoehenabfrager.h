#ifndef HOEHENABFRAGER_H
#define HOEHENABFRAGER_H
#include "track.h"
typedef QList<int> Integerliste;
class QProcess;
class Hoehenabfrager : public QObject
{
    Q_OBJECT
public:
    explicit Hoehenabfrager(Track*t, Integerliste l =QList<int>());//im Falle einer leeren Liste l werden sämtliche Höhen erfragt,
                                    //sonst nur die von den Punkten in der Liste
    bool isPending(){return runningProcesses>0;}
private:
    Track * track;//zeigt auf den Track, für den Höhen zu ermitteln sind.
    Integerliste pointlist;
    QProcess * abfrager;//
    static int runningProcesses;//zählt die noch ausstehenden Antworten.
signals:
  void habeFertig();
public slots:
  void antwortDa(int resultCode);//wenn abfrager seine Arbeit beendet hat.
  void error();

};

#endif // HOEHENABFRAGER_H
