#ifndef TIMEDTRACKDIALOG_H
#define TIMEDTRACKDIALOG_H

#include <QDialog>
#include <QDateTime>
class QTabWidget;
class TimedTrack;
namespace Ui {
class TimedTrackDialog;
}

class TimedTrackDialog : public QDialog
{
friend class TimedTrack;
    Q_OBJECT
TimedTrack * tt;
public:
  static QTabWidget * tabs;
    explicit TimedTrackDialog(TimedTrack * timet = 0,QWidget *parent = 0);
    ~TimedTrackDialog();
    bool init();//wie exec(), ruft aber als erstes den Laden-Dialog auf und gibt false zurück, wenn das Initialisieren des Timedtracks fehlschlägt.

private slots:
    bool on_ladenButton_clicked();

    void on_bildButton_clicked();

    void on_startZeit_dateTimeChanged(const QDateTime &tnew);

    void on_cancelButton_clicked();//muss den Track aus der Liste des Atlas auch löschen.
    void on_openTimerButton_clicked();

    void on_transmitRangeButton_clicked();

    void on_newButton_clicked();

    void on_bluebird_clicked();

    void on_redbird_clicked();

    void on_pilot_textChanged(const QString &arg1);


    void on_expandRangeButton_clicked();

    void on_heightButton_clicked();

    void on_greenbird_clicked();


    void on_yellowbird_clicked();

private:
    Ui::TimedTrackDialog *ui;
};

#endif // TimedTrackDialog_H
