#ifndef HOEHENPROFIL_H
#define HOEHENPROFIL_H
#include "track.h"
#include "leistungsprofil.h"
#include <QtGui>
class QLabel;
class Hoehenprofil : public QWidget
{
    Q_OBJECT
public:
    explicit Hoehenprofil(QRadioButton*shPrBu,QWidget *parent = 0,const Track * t=0);
//    void setTrack(Track*t){track=t;setWindowTitle(t->name);
//              from=0;to=track->points.count()-1;fromL=0;to=track->getLength();}
    ~Hoehenprofil();
    static QString help_leistungsprofil;
private:
    bool murks=false;
    QMenu * popup;
    const Track * track;
    int from,to;//indizes in track->points. Dieser Bereich wird dargestellt.
    double fromL,toL;//Längen des Pfades bis from bzw. bis to.
    QLabel* hintLabel;
    QAction*closeAction;//zeigt auf die Aktion, die das Evaluationwidget schließt.
    virtual void keyPressEvent(QKeyEvent *e);
    bool resizing;//zeigt an, dass Fenster mit Mouse resized wird.
    int mousey;//wo war die Maus beim letzten MouseMoveEvent oder MouseClickEvent
    bool marking;//zeigt an, dass gerade ein tracksegment markiert wird.
    int segmentStart,segmentStop;//indizes in track.points, die das markierte Segment anzeigen.
    double segmentStartL,segmentStopL;//Längen des Pfades bis zu den entsprechenden Punkten
    Leistungsprofil leistungsprofil;
    QFrame*evaluationWidget;
    QRadioButton*showProfileButton;//Button im Track-Menu, über den die Anzeige des Menus gesteuert wird.
   bool inResizeArea(const QPoint&pos)const;//entscheidet, ob pos innerhalb des Resize-Bereiches ist.
   virtual void mousePressEvent(QMouseEvent*e);
   virtual void mouseReleaseEvent(QMouseEvent*e);
   virtual void mouseMoveEvent(QMouseEvent*e);
   void paintEvent(QPaintEvent*);
   void zoomTo(int afrom, int ato);
private slots:
   void evaluate(int von=0,int bis=0);//Auswertung anzeigen von Punkt von bis Punkt exklusive bis (Index in track.points)
   void adjustEvaluationWidget();//wird aufgerufen wenn das Widget in der Position im Atlas korrigiert werden muss.
   void     closeEvaluationWidget();//schließt das Auswertungsfenster, falls offen.
   void chooseLeistungsprofil();
   void zoom();//zoomt in den selektierten Bereich, bzw ganz heraus, wenn keiner selektiert ist.
   void help();
signals:
    void mouseMoved(double);//pos beschreibt den Prozentsatz 0=ganz links 1=ganz rechts
};

#endif // HOEHENPROFIL_H
