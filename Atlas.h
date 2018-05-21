#ifndef ATLAS_H
#define ATLAS_H
#include <QtGui>
#include "region.h"
#include "track.h"
#include "ukartendownload.h"
typedef enum{links,rechts,oben,unten,nix,rectangle} Border; //rect zeigt an, dass die Maus gezogen wird.
class Helpsystem :public QObject {
    Q_OBJECT
public:
    static void showPage(QUrl url, QString title="Hilfe");
    static Helpsystem * helpsystem;
public slots:
    void help(QString helpPage=QString("/hilfeseiten/atlas.html"), QString title="Hilfe");//page als Pfad im qrc-System mit / als Trenner
};

class Atlas : public QWidget
{
    Q_OBJECT
  public:
  Atlas();
  ~Atlas();
    Waypoints waypoints;
    Track track;
    QDialog * trackDialog;
   // QList<TimedTrack*> timedTrackList;//verwaltet die timedTracks.
  private:
    //Border border; //welcher Rand muss aus den gespeicherten Kacheln gezeichnet werden?
    QMenu * popup;
    QPixmap  bild;//speichert die ganze Karte als Pixmap.
    bool useBild;//zeigt an, ob das Bild verwendet werden kann.
    Position pos;//Position der linken oberen Kachelkoordinaten    
    QRect bildxy; //Kacheln, die im bild gespeichert sind.
    int bildz; //z-Koordinate, auf die sich die Kacheln in bildxy beziehen.
    QRect mouseRect;//gibt das derzeit gezeichnete Rechteck an, ist keines gezeichnet, so ist topLeft==bottomRight
    QPoint newMousePoint;//gibt an, wohin die Maus gezogen wurde.
    Position mousePosition;//gibt die Position an, die auch in der Statusbar angezeigt wird.
    double scale;
    QString baseDir; //Basisverzeichnis für die Kacheln;
   QStatusBar * bar;
   QLabel * posLabel;//wird in der StatusBar angezeigt.
   QLabel * hintLabel;//Hinweislabel für die Waypoint-Description.
   UKartendownload * downloadDialog;
   bool showWaypoints;
   bool markArea;
   Position getMousePosition(const QPoint mousepos);//rechnet die Koordinaten im Widget in eine Position um   
   void showMousePosition(QPoint mousepos);//gibt die Position an der Mausposition in Statusbar aus.
   virtual void paintEvent(QPaintEvent *);
   void paintMore(QPainter&);//zeichnet Waypoints und Tracks
   void paintMassstab(QPainter&p);//zeichnet den Maßstab.
   //virtual void mouseMoveEvent(QMouseEvent * e);
   virtual void mousePressEvent(QMouseEvent *e);
   virtual void mouseReleaseEvent(QMouseEvent *e);
   virtual void mouseDoubleClickEvent(QMouseEvent*e);
   virtual void wheelEvent(QWheelEvent *e);
   void showDownloadDialog(bool force=false);//force=true erzwingt das Zeigen in jedem Fall.
   void dragEnterEvent(QDragEnterEvent * e);
   void dropEvent(QDropEvent *e);
public:
   virtual void keyPressEvent(QKeyEvent * e);
   void movePositionTo(Position  p,const QPoint & pixelkoord);
   void mouseMoveEvent(QMouseEvent * e);
   Position getPos(){return pos;}
   double getScale(){return scale;}
   double meterPerPixel(){return pos.meterPerPixel()/scale;}//meter pro Bildschirmpixel an der Position pos;
   QPoint getPixelKoords(double lat,double lon,bool&visible);//gibt das Pixel aus, in dem die Koordinaten liegen.
          //visible zeigt an, ob der Punkt überhaupt sichtbar ist. Die Koordinaten sind nicht Bildschirmkoordinaten, sondern
          //Pixel in den Karten, die ggf. noch scaliert werden müssen, um die Bildschirmkoordinaten zu liefern.
   QPoint getPixelKoords(Position pos,bool&visbible);//wie oben.
   QPoint getPixelKoords(const QPoint &globalPixelKoords, int z, bool &vis);//wie oben. QPoint enthält die globalen Pixelkoordinaten in Zoomstufe z.
   void showCentered(const Position & apos);//zeigt die Position in der gegenwärtigen Zoomstufe zentriert
   const Position & getMousePosition(){return mousePosition;}
   //void deleteWaypoint(Waypoint*wp){waypoints.removeAll(*waypoint)};//löscht den Waypoint aus der Waypointliste.
  private:
   QStringList getImagesAt(QPoint p);//liefert eine ggf. leere Liste von Dateinamen der Bilder, die bei p angezeigt werden. p in Bildschirmpixel.
  private slots:
    void deleteTileAtMousePoint();
    void changeBaseDir();
    void clip();//kopiert die Koordinaten des Punktes unter der Mous (newMousePosition) ins Clipboard
    int getWaypointIndexAt(Position mouse);//liefert den Index des waypoints, der in der Nähe von mouse ist. -1, wenn dies keiner ist.
    int getTrackpointIndexAt(Position mouse);//wie getWaypointIndexAt.
    void addWaypoint();//fügt die Position unter der Maus als Waypoint hinzu, fragt Name und Beschreibung ab.
    void loadWaypoints(QString filename="");//lädt Waypoints aus einer Datei. Die wird erfragt, wenn useFilename=false ist, sonst waypoints.filename benutzen
    void saveWaypoints();//speichert die Waypoints in eine Datei.
    void closeWaypointFile();//schließt die geöffnete Waypoint-Datei.
    void deleteWaypoint();//löscht den Waypoint unter der Maus.
    void showWaypointCombo();//zeigt ComboBox mit allen Waypoints, die dadurch ausgewählt und zentriert werden können.
//    void changeRegion();//holt die Daten aus dem Downloaddialog und wechselt auf die dort angegebene Region.
    void trackMenu();//zeigt das Track-Menu an.
    void saveTrack();//speichert den Track als gpx-Datei, falls er erweitert wurde
    void expandTrack();//hängt an den Track die aktuelle Cursorposition dran.
    void loadTrackEnvironment();//veranlasst das Herunterladen der Umgebung des Tracks.  
    void hideAndShowWaypoints();
    void help();
    void settings();
    void resizeWaypointPixmaps(int size);
    virtual void resizeEvent(QResizeEvent*e){emit resized(); QWidget::resizeEvent(e); }
  public slots:
    //bool close();
    void addTimedTrack();//erweitert die TimedTrackList um einen Track, öffnet das Dialogfenster.
    void showTrackTimer();//Zeigt, wenn möglich, denn Tracktimer an.
    void newPaint();//zeichnet alles nochmal, ohne verwendung der gespeicherten Karte.
    void repaint();
    void closed();
    void markTrack(double rel);//markiert den Trackpunkt an der relativen Position pos mit dem Hintlabel. pos=0 ganz am Anfang pos=1 ganz am Ende
    void showWaypoint(QString wpTitle,bool withDialog=false);//zentriert den Waypoint mit dem gegebenen Titel (den ersten, der gefunden wird) und öffnet dessen Dialog.
signals:
    void viewportChanged(); //wird emittiert, wenn die Anzeige sich ändert
    void resized(); //wird vom resizeEvent aufgerufen
    void mouseMoved(); //wird bei jeder Mausbewegung emittiert. (weiß nicht, ob das gut ist)
};
#endif
