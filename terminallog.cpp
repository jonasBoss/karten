#include "terminallog.h"
#include "ui_terminallog.h"
#include "signal.h"

Terminallog::Terminallog(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::Terminallog)
{
  ui->setupUi(this);
  closeOnFinished=showcerr=false;
  //setAttribute(Qt::WA_DeleteOnClose);
}
Terminallog::Terminallog(QString command, QStringList args):QWidget(0),ui(new Ui::Terminallog){
  ui->setupUi(this);
  ui->progName->setText(command);
  closeOnFinished=showcerr=false;
  // setAttribute(Qt::WA_DeleteOnClose);
  proc.start(command,args,QIODevice::ReadOnly);
  proc.waitForStarted(10000);
  connect(&proc,SIGNAL(readyReadStandardOutput()),this,SLOT(readStandardOutput()));
  connect(&proc,SIGNAL(readyReadStandardError()),this,SLOT(readStandardError()));
  connect(&proc,SIGNAL(error(QProcess::ProcessError)),this,SLOT(displayMessage(QProcess::ProcessError)));
  connect(&proc,SIGNAL(finished(int)),this,SLOT(bye()));
  connect(&proc,SIGNAL(stateChanged(QProcess::ProcessState)),this,SLOT(processStateChanged(QProcess::ProcessState)));
  processStateChanged(proc.state());
}
void Terminallog::readStandardOutput(){
  proc.setReadChannel(QProcess::StandardOutput);
  if(proc.canReadLine()){
    ui->logView->appendPlainText(QString::fromUtf8(proc.readAllStandardOutput()));
  }
}
void Terminallog::readStandardError(){
  proc.setReadChannel(QProcess::StandardError);
  if(proc.canReadLine()){
    if(showcerr) ui->logView->appendHtml("<font color=\"blue\">cerr:</font>");
    ui->logView->appendPlainText(QString::fromUtf8(proc.readAllStandardError()));
  }
}

void Terminallog::displayMessage(QProcess::ProcessError e){
  QString message="<font color=\"red\">";
  switch (e){
    case  QProcess::FailedToStart : message+="process Failed to start"; break;
    case QProcess::Crashed: message+="process crashed"; break;
    case QProcess::Timedout: message+="process Timeout"; break;
      case QProcess::WriteError: message+="process WriteError"; break;
      case QProcess::ReadError: message+="process ReadError"; break;
      case QProcess::UnknownError: message+="process Unknown Error"; break;
  }
  message+="</font>";
  ui->logView->appendHtml(message);
}
void Terminallog::bye(){
  if(closeOnFinished)
    close();
}
void Terminallog::on_closeButton_clicked(){
  if(proc.state()==QProcess::NotRunning){
      close();
    }
}
void Terminallog::on_killButton_clicked(){
  if(proc.state()==QProcess::Running)
      ::kill(proc.pid(),SIGTERM);
}

void Terminallog::processStateChanged(QProcess::ProcessState newState)
{
 QString status;
 switch (newState) {
   case    QProcess::NotRunning : status=QString::fromUtf8("Prozess läuft nicht"); break;
   case QProcess::Starting : status="Prozess startet..."; break;
   case QProcess::Running : status=QString::fromUtf8("Prozess läuft...");break;
   default : status="unklar";
 }
 ui->state->setText(status);
 ui->killButton->setEnabled(newState != QProcess::NotRunning);
 ui->closeButton->setEnabled(newState == QProcess::NotRunning);
}
void Terminallog::closeEvent(QCloseEvent *event){
  if (proc.state()==QProcess::Running){
    QMessageBox::warning(0,"Achtung",QString::fromUtf8("Der Prozess läuft noch"));
    event->ignore();
  }else
    QWidget::closeEvent(event);
}

Terminallog::~Terminallog()
{
  delete ui;
}
