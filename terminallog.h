#ifndef TERMINALLOG_H
#define TERMINALLOG_H

#include <QWidget>
#include <QtCore>
#include <QtGui>

namespace Ui {
  class Terminallog;
}

class Terminallog : public QWidget
{
  Q_OBJECT

public:
  explicit Terminallog(QWidget *parent = 0);
  Terminallog(QString command,QStringList args=QStringList());
  ~Terminallog();
  void setCloseOnFinished(bool on=true){closeOnFinished=on;}
  void setDeleteOnClose(bool on=true){setAttribute(Qt::WA_DeleteOnClose,on);}
  void showCerr(bool show=true){showcerr=show;}//sorgt dafür,dass cerr besonders hervorgehoben wird.
  QProcess proc;
private:
  Ui::Terminallog *ui;
  bool closeOnFinished;
  bool showcerr;
  void closeEvent(QCloseEvent *event);
private slots:
  void readStandardOutput();
  void readStandardError();
  void displayMessage(QProcess::ProcessError e);
  void bye();
  void on_closeButton_clicked();
  void on_killButton_clicked();
  void processStateChanged(QProcess::ProcessState newState);
};

#endif // TERMINALLOG_H
