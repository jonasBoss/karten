#include "settingdialog.h"
#include "ui_settingdialog.h"
#include "Atlas.h"
#include <QtGui>
#include <QtCore>

SettingDialog::SettingDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SettingDialog)
{
  ui->setupUi(this);
  QSettings set("UBoss","karten");
  ui->atlasdirectory->setText(set.value("atlasdirectory").toString());
  ui->mapservertxt->setText(set.value("mapserverfile").toString());
  ui->leistungsprofildatei->setText(set.value("leistungsprofildatei").toString());
  ui->saveOnExit->setChecked(set.value("saveOnExit",false).toBool());
  ui->imageSize->setValue(set.value("waypoints/imageSize",120).toInt());
  ui->apiKey->setText(set.value("apiKey","not set").toString());
  connect(this,SIGNAL(accepted()),this,SLOT(on_accept()));
}

SettingDialog::~SettingDialog()
{
  delete ui;
}
void SettingDialog::on_chooseAtlas_clicked(){
  QString dir=QFileDialog::getExistingDirectory(0,"Kartenverzeichnis wÃ¤hlen");
  if(dir.length()>0)
    ui->atlasdirectory->setText(dir);
}
void SettingDialog::on_chooseMapserver_clicked(){
  QString dir=QFileDialog::getOpenFileName(0,"Datei mit Mapserverdefinitionen");
  if(dir.length()>0)
    ui->mapservertxt->setText(dir);
}
void SettingDialog::on_accept(){
  QSettings set("UBoss","karten");
  set.setValue("atlasdirectory",ui->atlasdirectory->text());
  set.setValue("mapserverfile",ui->mapservertxt->text());
  set.setValue("leistungsprofildatei",ui->leistungsprofildatei->text());
  set.setValue("saveOnExit",ui->saveOnExit->isChecked() );
  set.setValue("apiKey",ui->apiKey->text());
  if(set.value("waypoints/imageSize",1).toInt()!=ui->imageSize->value()){
    set.setValue("waypoints/imageSize",ui->imageSize->value());
    emit pixmapSizeChanged(ui->imageSize->value());
  }
}

void SettingDialog::on_chooseLeistungsprofil_clicked()
{
    QString filename=QFileDialog::getOpenFileName(0,"Leistungsprofildatei",".","Textdateien (*.txt)");
    if(!filename.isEmpty()){
        ui->leistungsprofildatei->setText(filename);
    }
}

void SettingDialog::on_buttonBox_helpRequested()
{
    Helpsystem::helpsystem->help("/hilfeseiten/settings.html","Einstellungen");
}
