#ifndef UKARTENDOWNLOAD_H
#define UKARTENDOWNLOAD_H
//#include "kartendownload.h"
#include "ui_kartendownload_qt4.h"
#include "region.h"
#include <qstringlist.h>
#include <fstream>
#include <QtCore>
#include <QtGui>
class QString;
class Tile {//diese Klasse unterscheidet sich kaum von der Klasse Position, die nur viel mehr Methoden hat.
  public:
  double latitude,longitude;
  int x,y,z;
  void setKoords(double lat,double lon, int zoom);
  void copyPosition(const Position&posi ){x=posi.getX();y=posi.getY();z=posi.getZ();
                                         latitude=posi.getLatitude();longitude=posi.getLongitude();}
  void newZ(int zoom);
};
class Mapserver{
  public:
  QStringList names;//bezeichner, der in Auswahlbox auftritt
  QStringList bases;//Basisadresse fÃ¼r das -B argument von wget. alles, was danach kommt, muss
                   //in das mit -i eingebundene file tiles.url
  QStringList tileurls;//url nach der base mit Platzhalter [x] [y] und [z]
  QStringList descriptions;//Beschreibungen der Mapserver
  friend ostream& operator<<(ostream&s,const Mapserver & server);
  friend istream& operator>>(istream&s,Mapserver & server);
  bool readFromFile(const QString filename);
  void writeToFile(const QString filename);
  void clear();//lÃ¶scht alle Daten.
  QString getTileUrl(int i,uint x,uint y, uint z);//liefert den Teil nach base;
  void append(QString name,QString base,QString tileurl,QString description="keine Beschreibung");
};
class UKartendownload : public QDialog, public Ui::Kartendownload{
    Q_OBJECT
  public:
  UKartendownload();
  void setRegion(const Position &von,const Position &bis);//setzt von_tile und bis_tile.
  Position linksOben();//liefert die obere linke Ecke der Region.
private:
  Tile von_tile,bis_tile;//speichert linke ober und rechte untere Ecke des zu downloadenden Bereiches.
  //QStringList serverListe;
  Mapserver mapserver;
//  void updateInfo();
public slots:
  void download();
  void turboDownload();
  void selectFolder(QString dir=QString());
//  void regionChanged();//setzt von_tile und bis_tile nach den Daten, die als Eingabezeilen in der Ui-Region gespeichert sind.
  void createTileList();
  void renameTiles();
//  void loadRegion();
//  void saveRegion();
  void manageServer();
  void on_deleteTilesButton_clicked();
  void on_copyTilesButton_clicked();
  void on_uaButton_clicked();
  void on_defaultMapserverButton_clicked();
  void mapserverChanged(int);
signals:
  void downloadReady();
};
/** QThread-Klasse für das Kopieren der Karten auf ein anderes Verzeichnis **/
class CopyThread : public QThread{
   Q_OBJECT
private:
  QTextStream * stream;//stream aus dem die Daten zu lesen sind.
  QDir dir;
  ~CopyThread();
public:
  CopyThread(QTextStream*st,QDir d);
  void run();
signals:
  void fileCopied(int);//zeigt an, dass die 10*n-te Datei kopiert wurde.
};

#endif
