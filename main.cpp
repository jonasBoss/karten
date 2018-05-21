#include <qapplication.h>
#include "Atlas.h"
#include "mapservermanager.h"
#include "track.h"
#include "settingdialog.h"
#include "timedtrack.h"
#include <QtGui>
#include <QtCore>
int main( int argc, char ** argv )
{
     QApplication a( argc, argv );
     a.setWindowIcon(QPixmap(":/images/weltkugel.png"));
    //w.setWindowIcon(QIcon("icon.png"));
     QTranslator qtTranslator;
        qtTranslator.load("qt_" + QLocale::system().name(),
            QLibraryInfo::location(QLibraryInfo::TranslationsPath));
     a.installTranslator(&qtTranslator);
    if( QSettings("UBoss","karten").allKeys().isEmpty()){
        SettingDialog dial;
        dial.exec();
    }
    Atlas w;
    w.show();
    w.newPaint();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    return a.exec();
}
