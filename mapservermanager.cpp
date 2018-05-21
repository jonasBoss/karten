#include "mapservermanager.h"
#include "ui_mapserverdialog.h"
#include "ukartendownload.h"
#include <QtCore>
#include <QtGui>
#include <fstream>
MapserverManager::MapserverManager():QDialog(0){
  setupUi(this);
  table->setHorizontalHeaderLabels(QStringList()<<"Titel"<<"baseUrl"<<"query"<<"Beschreibung");
  mapserver=0;
}

MapserverManager::MapserverManager(Mapserver & maps):QDialog(0)
{
    setupUi(this);
    mapserver=&maps;
    table->setHorizontalHeaderLabels(QStringList()<<"Titel"<<"baseUrl"<<"query"<<"Beschreibung");
    //maps.readFromFile(MAPSERVERTXT);
    table->setRowCount(maps.names.count());
    for(short r=0;r<maps.names.count();r++){
      table->setItem(r,0,new QTableWidgetItem(maps.names[r]));
      table->setItem(r,1,new QTableWidgetItem(maps.bases[r]));
      table->setItem(r,2,new QTableWidgetItem(maps.tileurls[r]));
      table->setItem(r,3,new QTableWidgetItem(maps.descriptions[r]));
    }
    table->resizeColumnsToContents();
    table->resize(table->sizeHint());
}
void MapserverManager::accept(){
  if(mapserver){//Daten aus dem Tablewidget in mapserver schreiben
    mapserver->clear();
    for (short i=0; i<table->rowCount();i++){
      mapserver->names<<table->item(i,0)->text();
      mapserver->bases<<table->item(i,1)->text();
      mapserver->tileurls<<table->item(i,2)->text();
      mapserver->descriptions<<table->item(i,3)->text();
    }
    QSettings settings("UBoss","karten");
    QString mapserverfile=settings.value("mapserverfile").toString();
    mapserver->writeToFile(mapserverfile);
  }
  QDialog::accept();
}
void MapserverManager::addServer(){
  table->setRowCount(table->rowCount()+1);
  table->setItem(table->rowCount()-1,0,new QTableWidgetItem("New"));
  QTableWidgetItem*it=table->item(table->rowCount()-1,0);
  table->scrollToItem(it);
  //table->setCurrentItem(it);
  table->editItem(it);
  //table->openPersistentEditor(it);
}
void MapserverManager::removeServer(){
  table->removeRow(table->currentRow());
}
