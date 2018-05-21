#ifndef MAPSERVERMANAGER_H
#define MAPSERVERMANAGER_H
#include "ui_mapserverdialog.h"
#include "ukartendownload.h"
#endif // MAPSERVERMANAGER_H
//#define MAPSERVERTXT ":/mapserver.txt" 
//#define MAPSERVERTXT "/home/uwe/Cpp/qt4/Karten/mapserver.txt"
//hier mÃ¼sste man QSettings verwenden, um da den Pfad zu speichern.
class MapserverManager:public QDialog, public Ui::MapserverDialog{
  Q_OBJECT
public:
    MapserverManager();
    MapserverManager(Mapserver & maps);
private:
    Mapserver * mapserver;
 public slots:
    void accept();
    void addServer();
    void removeServer();
};
