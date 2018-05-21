#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include <QDialog>

namespace Ui {
  class SettingDialog;
}

class SettingDialog : public QDialog
{
  Q_OBJECT
  
public:
  explicit SettingDialog(QWidget *parent = 0);
  ~SettingDialog();
  
private:
  Ui::SettingDialog *ui;
private slots:
 void on_chooseMapserver_clicked();
 void on_chooseAtlas_clicked();
 void on_accept();
 void on_chooseLeistungsprofil_clicked();

 void on_buttonBox_helpRequested();

signals:
 void pixmapSizeChanged(int); //wird emmitiert, wenn die Größe der Waypointpixmaps sich geändert hat.
};

#endif // SETTINGDIALOG_H
