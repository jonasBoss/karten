#include "hoehenprofil.h"
#include "region.h"
#include "track.h"
#include "Atlas.h"
#include <QtGui>
QString Hoehenprofil::help_leistungsprofil=trUtf8(
  "<p>Leistungsprofile legen fest, bei welcher Steigung mit welcher Geschwindigkeit gefahren bzw. gelaufen wird.</p>"
    "<p>Diese Daten sind in einer Textdatei festzulegen, die dann unter Einstellungen einzutragen ist.<br/>"
    "Ihr Format ist wie folgt:"
    "<h3>Format der Leistungsprofildatei <i>leistungsprofile.txt</i></h3>"
    "<p>eine Zeile: Bezeichnung des Profiles<br/>"
    "mehrere Zeilen: Profildaten. Jede Zeile hat zwei durch Semikolon getrennte Einträge:<br/>"
    "0.05 ; 2.8<br/>"
    "würde sagen: Bei 5% Steigung fahre ich mit 2.8 m/s<br/>"
    "nach der letzten Zeile mit Leistungsdaten: Leerzeile<br/>"
    "Danach können weitere Profildefinitionen folgen.</p>"
    "Zeilen, die mit # beginnen, werden ignoriert.");
Hoehenprofil::Hoehenprofil(QRadioButton *shPrBu, QWidget *parent, const Track * t) : QWidget(parent)
{
  showProfileButton=shPrBu;
  showProfileButton->setChecked(true);
  track=t;
  evaluationWidget=0;
  from=0; to=track==0?0:track->points.count()-1;
  fromL=0;toL=track==0?0:track->getLength();
  setAttribute(Qt::WA_DeleteOnClose);
  setAttribute(Qt::WA_MouseTracking);
  setFocusPolicy(Qt::ClickFocus);
  if (t==0)
    setWindowTitle(trUtf8("Höhenprofil"));
  else{
      setWindowTitle(trUtf8("Höhenprofil: ")+t->name);
      //t->calcLength(-1);
  }
  //resize(1000,100);
  setFixedHeight(80);
  setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

  hintLabel=new QLabel(this);
  hintLabel->setAutoFillBackground(true);
  hintLabel->setFrameShape(QFrame::Box);
  hintLabel->setText("");
  hintLabel->resize(hintLabel->sizeHint());
  hintLabel->move(0,-30);
  hintLabel->show();
  resizing=marking=false;mousey=segmentStart=segmentStop=0;
  connect(track->getAtlas(),SIGNAL(resized()),this,SLOT(adjustEvaluationWidget()));
  QAction * act;
  popup=new QMenu(trUtf8("Höhenprofilmenu"),this);
  addAction(popup->addAction(trUtf8("Auswertung erstellen"),this,SLOT(evaluate()),Qt::Key_A));
  closeAction=popup->addAction(trUtf8("Auswertung schließen"),this,
                               SLOT(closeEvaluationWidget()),Qt::Key_Escape);
  closeAction->setDisabled(true);closeAction->setVisible(false);
  addAction(closeAction);
  addAction(popup->addAction(trUtf8("Leistungsprofil auswählen"),this,SLOT(chooseLeistungsprofil())));
  act=popup->addAction("Hilfe",this,SLOT(help()),Qt::Key_F1);
  act->setShortcutContext(Qt::WidgetShortcut);
  addAction(act);
  act=(popup->addAction(trUtf8("Zoom in/out"),this,SLOT(zoom()),Qt::Key_Z));
  act->setShortcutContext(Qt::WidgetShortcut); addAction(act);
  addAction(popup->addAction(trUtf8("Höhenprofil ausblenden"),this,SLOT(deleteLater()),Qt::Key_Backspace));
}

Hoehenprofil::~Hoehenprofil()
{
    if(showProfileButton) showProfileButton->setChecked(false);
}

void Hoehenprofil::keyPressEvent(QKeyEvent *e)
{
    //qDebug()<<trUtf8("Hoehenprofil::keyPressevent key code ")<<e->key();
    //qDebug()<<"Modifiers "<<e->modifiers()<<"text()"<<e->text();
    if(e->modifiers()==Qt::ShiftModifier){
        switch(e->key()){
          case Qt::Key_Up :
                  setFixedHeight(height()+10);
            break;
           case Qt::Key_Down :
                setFixedHeight(height()-10);
                break;
            default:track->getAtlas()->keyPressEvent(e);
        }
    }else{//ohne Shift-Modifier
        track->getAtlas()->keyPressEvent(e);
    }
}

bool Hoehenprofil::inResizeArea(const QPoint &pos) const
{
    return pos.x()>=width()/2-15 && pos.x()<=width()/2+15 && pos.y()<=8;
}

void Hoehenprofil::mousePressEvent(QMouseEvent *e)
{
    if(track->points.count()<2) return;
    if(e->button()==Qt::LeftButton && inResizeArea(e->pos())){
        resizing=true;
        mousey=mapToGlobal(e->pos()).y();
        return;
    }
    if(e->button()==Qt::LeftButton){
        marking=true;
        segmentStart=track->indexBefore(
               ((toL-fromL)*e->pos().x()/width()+fromL)/track->getLength(),&segmentStartL,0);
        mousey=e->pos().x();
        return;
    }
    if(e->button()==Qt::RightButton){
        popup->popup(mapToGlobal(e->pos()));
    }
}

void Hoehenprofil::mouseReleaseEvent(QMouseEvent *e)
{
    if(track->points.count()<2) return;
  resizing=false;
  marking=false;
  if(e->pos().x()<=mousey+2 && e->pos().x()>=mousey-2){//mousey speichert Klickposition beim mousePressEvent
      segmentStop=segmentStart;
  }
  //segmentStop=track->indexBefore(1.0*e->pos().x()/width(),0,&segmentStopL)+1;
  if(segmentStart!=segmentStop)
          evaluate(segmentStart,segmentStop);
  repaint();
}


void Hoehenprofil::mouseMoveEvent(QMouseEvent *e)
{
    if(inResizeArea(e->pos()))
       setCursor(QCursor(Qt::/*SizeVerCursor*/SplitVCursor));
    else
        unsetCursor();//setCursor(QCursor(Qt::IBeamCursor));
    //QCursor::setShape(Qt::SizeVerCursor);
    if(resizing){
        setFixedHeight(height()+mousey-mapToGlobal(e->pos()).y());
        mousey=mapToGlobal(e->pos()).y();
    }
    if(to-from<1) return;
    double entf=round(e->pos().x()*(toL-fromL)/width());//wie weit vom Start des angezeigten Trackteils entfernt?
    hintLabel->setText(Track::printLength(entf));
    hintLabel->resize(hintLabel->sizeHint());
    hintLabel->move(e->pos()-QPoint(0,hintLabel->height()+5));
    emit mouseMoved((entf+fromL)/track->getLength());//Signal wird vom Atlas aufgefangen, der sein Hintlabel positioniert.
    if(marking){//ein Trackbereich wird gerade markiert, d.h. Linke Maustaste gedrückt.
        segmentStop=track->indexBefore(((toL-fromL)*e->pos().x()/width()+fromL)/track->getLength(),0,&segmentStopL)+1;
        if(segmentStop>=track->points.size()){
            segmentStop=track->points.size()-1;
         }
        repaint();
    }
}

void Hoehenprofil::paintEvent(QPaintEvent *)
{    
    QPainter painter(this);
    //murks=false;
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(track->farbe);
    painter.setFont(QFont("Arial", 10));
   /* if(track==0 || track->points.length()<2){
        painter.drawText(rect(), Qt::AlignCenter, trUtf8("Kein Track zu zeichnen."));
        return;
    }*/
    if(from-to==0 || track==0){
        painter.drawText(rect(), Qt::AlignCenter, trUtf8("Kein Track zu zeichnen."));
        return;
    }
    qreal gesamtlaenge=toL-fromL;
    qreal minH=track->points.at(from).altitude;
    qreal maxH=minH;//minimale und maximale Höhe im Track
    for(int i=from;i<to;i++){//minH und maxH ermitteln
        //gesamtlaenge+=Position(track->points.at(i+1)).distanceFrom(Position(track->points.at(i)));
        if(track->points.at(i+1).altitude>maxH) maxH=track->points.at(i+1).altitude;
        if(track->points.at(i+1).altitude<minH) minH=track->points.at(i+1).altitude;
    }
    if(minH==maxH){
        painter.drawText(rect(), Qt::AlignCenter, trUtf8("Alle Punkte auf gleicher Höhe"));
        return;
    }
    qreal scaleX=width()*1.0/gesamtlaenge;
    qreal scaleY=height()*1.0/((minH-maxH)!=0.0?minH-maxH:1000);
    qreal dy=-scaleY*maxH;
    QMatrix trackKoordinatenMatrix=QMatrix(scaleX,0,0,scaleY,0,dy);
    painter.setWorldMatrix(trackKoordinatenMatrix);
    //painter.drawLine(QPointF(0,minH),QPointF(gesamtlaenge,maxH));
    qreal lae=0.0;
    QLinearGradient grad(QPointF(gesamtlaenge/2,minH),QPointF(gesamtlaenge/2,maxH));
    grad.setColorAt(0,Qt::gray);grad.setColorAt(1,track->farbe);
    QPointF p[to-from +3]; //beachte Anzahl der punkte ist to-from+1
    p[0].setX(0);p[0].setY(minH);
    p[to-from+2].setX(gesamtlaenge);
    p[to-from+2].setY(minH);
    Position s1=Position(track->points.at(from));
    Position s2;
    for(int i=0;i<=to-from;i++){//Polygonzug
        p[i+1].setX(lae);
        p[i+1].setY(track->points.at(i+from).altitude);
        if(i<to-from) {
            s2=Position(track->points.at(i+from+1));
            lae+=s2.distanceFrom(s1);
            s1=s2;
        }
    }
    painter.setBrush(QBrush(grad));
    painter.drawPolygon(p,to-from+3);
/*****Jetzt kommen die Führungslinien********************/
    QPen gridPen=QPen(QColor(64,64,64,192));
    gridPen.setStyle(Qt::DashLine);
    QPen textPen=QPen(Qt::black);
    painter.setPen(gridPen);
    //painter.setWorldMatrix(trackKoordinatenMatrix);
    const int xPixelPerGridline=180;//ungefähr alle soviel Pixel kommt eine Gridline
    const int yPixelPerGridline=30;
    double gridLaenge,l;
    int delta;
    if(width()>xPixelPerGridline){
        gridLaenge=gesamtlaenge*xPixelPerGridline/width();//alle soviel Meter käme eine Gridline
        delta=Track::round125(gridLaenge);//alle soviel Meter tatsächlich eine Gridline
        if(delta>0){
            l=0.0;
            while(l<gesamtlaenge){
                painter.drawLine(QPointF(l,minH),QPointF(l,maxH));
                l+=delta;
            }
            painter.setWorldMatrix(QMatrix());
            painter.setPen(textPen);
            QPointF ersteLinie=trackKoordinatenMatrix.map(QPointF(delta,maxH));
            painter.translate(ersteLinie);
            painter.rotate(90);
            painter.drawText(0,0,Track::printLength(delta));
        }
    }
//jetzt die Höhengridlinien nach gleichem Muster.
    painter.setPen(gridPen);
    painter.setWorldMatrix(trackKoordinatenMatrix);
    gridLaenge=(maxH-minH)*yPixelPerGridline/height();
    delta=Track::round125(gridLaenge);
    if(delta>0){
        l=((int)(minH/delta))*delta;
        while(l<=maxH){
            if(l>=minH) painter.drawLine(QPointF(0,l),QPointF(gesamtlaenge,l));
            l+=delta;
        }
        //jetzt die Beschriftungen der Höhenlinien
        l=((int)(minH/delta))*delta;
        painter.setPen(textPen);
        painter.setWorldMatrix(QMatrix());
        while(l<=maxH){
            QPointF dahin=trackKoordinatenMatrix.map(QPointF(0,l));
            painter.drawText(dahin,Track::printLength(l));
            l+=delta;
        }
    }

/****Das Label mit den Gesamtdaten***************************/
    QString t=trUtf8(" Länge: %1, Höhe von %2 bis %3 ").arg(Track::printLength(gesamtlaenge)).arg(minH).arg(maxH);
    painter.setPen(track->farbe);
    painter.setBackgroundColor(QColor(0xFF,0xFF,0xFF));
    painter.setBackgroundMode(Qt::OpaqueMode);
    painter.setWorldTransform(QTransform());
    painter.drawText(rect(), Qt::AlignBottom | Qt::AlignHCenter, t);
/****Ein kleines Dreieck zum Ziehen*************************/
    painter.setPen(textPen);
    QPoint ps[3]={QPoint(width()/2,0),QPoint(width()/2-15,7),QPoint(width()/2+15,7)};
    painter.drawPolygon(ps,3);
/****Der markierte Bereich************************************/
    if(segmentStart!=segmentStop){
        painter.setWorldMatrix(trackKoordinatenMatrix);
        painter.setBrush(QColor(0x80,0x80,0x80,0x80));
        painter.drawRect(QRectF(segmentStartL-fromL,minH,segmentStopL-segmentStartL,maxH-minH));
    }
}


void Hoehenprofil::evaluate(int von, int bis)
{
    if(von==0 && bis==0){
        von=from; bis=to;
    }
    if(von==bis && von!=0) return;
    if(bis<von){von^=bis; bis^=von; von^=bis;}//von und bis werden vertauscht.
    if(evaluationWidget!=0){
        evaluationWidget->close();
        evaluationWidget=0;
    }
    double bergauf=0.0;
    double bergab=0.0;
    double horizontal=0.0;
    double hoehenmeter=0.0;
    double tiefenmeter=0.0;
    Position sp=Position(track->points.at(von));
    for (int i=von;i<bis;i++){
      Position zp=Position(track->points.at(i+1));
      double strecke=zp.distanceFrom(sp);
      double hoehe=track->points.at(i+1).altitude-track->points.at(i).altitude;
      double steigung=hoehe/strecke;
      if(steigung>0.004) bergauf+=strecke;
      else if(steigung<=0.004 && steigung>-0.01) horizontal+=strecke;
      else bergab+=strecke;
      if(hoehe>0) hoehenmeter+=hoehe;
      else tiefenmeter+=hoehe;
      sp=zp;
    }
    evaluationWidget=new QFrame(track->getAtlas());
    evaluationWidget->setFrameStyle(QFrame::Raised);
    evaluationWidget->setFrameShape(QFrame::Panel);
    evaluationWidget->setMidLineWidth(2);evaluationWidget->setLineWidth(2);
    //evaluationWidget->setFrameRect();
    //evaluationWidget->setWindowTitle(trUtf8("Trackauswertung"));
    evaluationWidget->setAttribute(Qt::WA_DeleteOnClose );
    evaluationWidget->setAttribute(Qt::WA_QuitOnClose,false);//Progr. kann sich beenden auch wenn dieses Fenster noch offen ist.
    //anzeige->setBackgroundRole(QPalette::Window);
    evaluationWidget->setBackgroundColor(QColor(240,240,240,210));
    evaluationWidget->setAutoFillBackground(true);
    connect(this,SIGNAL(destroyed(QObject*)),evaluationWidget,SLOT(close()));
    QGridLayout * layout=new QGridLayout(evaluationWidget);
/***Hier wird das Fenster mit Daten befüllt***********************/
    layout->addWidget(new QLabel(trUtf8("<h2>Trackauswertung mit Höhenprofil</h2>")),0,0,1,-1,Qt::AlignCenter);
    layout->addWidget(new QLabel(trUtf8("Strecke bergauf (>0.4%)")),1,0);
    layout->addWidget(new QLabel(Track::printLength(bergauf)),1,1);
    layout->addWidget(new QLabel(trUtf8("Höhenmeter gesamt")));
    layout->addWidget(new QLabel(bergauf==0?trUtf8("--"):
                          trUtf8("%1 m (ø %2 %)").arg(hoehenmeter).arg(round(hoehenmeter/bergauf*1000)/10.0)));
    layout->addWidget(new QLabel(trUtf8("Strecke horizontal (> -1 %)")));
    layout->addWidget(new QLabel(Track::printLength(horizontal)));
    layout->addWidget(new QLabel("Strecke bergab (<-1%)"));
    layout->addWidget(new QLabel(Track::printLength(bergab)));
    layout->addWidget(new QLabel(trUtf8("Höhenmeter bergab")));
    layout->addWidget(new QLabel(bergab==0?"--":
                          trUtf8("%1 m (ø %2 %)").arg(round(tiefenmeter)).arg(round(tiefenmeter/bergab*1000)/10.0)));
    int zeit=track->fahrzeit(leistungsprofil,von,bis);
    int stunden=zeit/3600;
    zeit-=stunden*3600;
    int minuten=zeit/60;
    zeit-=minuten*60;
    QLabel * fahrzeitLabel=new QLabel("Fahrzeit nach Leistungsprofil");
    fahrzeitLabel->setToolTip(leistungsprofil.name);
    layout->addWidget(fahrzeitLabel);
    layout->addWidget(new QLabel(leistungsprofil.name.isEmpty()?
                   trUtf8("kein Profil gewählt"):
                   QString("%1h %2' %3\"").arg(stunden).arg(minuten).arg(zeit)
                          ));
    evaluationWidget->resize(evaluationWidget->sizeHint());
    Atlas*at=track->getAtlas();
    evaluationWidget->move(at->width()-evaluationWidget->width(),at->height()-evaluationWidget->height());
    //anzeige->move(QCursor::pos()-QPoint(anzeige->width()/2,anzeige->height()));
    evaluationWidget->show();
    closeAction->setEnabled(true);closeAction->setVisible(true);
}

void Hoehenprofil::adjustEvaluationWidget()
{
  if(evaluationWidget==0) return;
  Atlas * at=track->getAtlas();
  evaluationWidget->move(at->width()-evaluationWidget->width(),at->height()-evaluationWidget->height());
}

void Hoehenprofil::closeEvaluationWidget()
{
    if(evaluationWidget!=0){
        evaluationWidget->deleteLater();
        evaluationWidget=0;
        segmentStart=segmentStop=0;
    }
    closeAction->setEnabled(false);closeAction->setVisible(false);
}

void Hoehenprofil::chooseLeistungsprofil()
{
    QSettings settings("UBoss","karten");
    QString filename=settings.value("leistungsprofildatei","none").toString();
    QFile f(filename);
    if (filename=="none" || filename=="" || !f.open(QIODevice::ReadOnly)){
        QMessageBox::information(0,"Leistungsprofile",
             trUtf8(
       "<H2>Kein Leistungsprofil verfügbar</H2>")+help_leistungsprofil);
    }else{
        QTextStream stream(&f);
        QList<Leistungsprofil> profilliste;
        while(!stream.atEnd()){
            Leistungsprofil profil;
            stream>>profil;
            profilliste.append(profil);
        }
        for(auto it=profilliste.begin();it!=profilliste.end();++it){
            if ((*it).name=="") profilliste.remove(it);
        }
        if(profilliste.isEmpty()){
            QMessageBox::warning(0,"Achtung!",trUtf8("die Datei %1 enthielt keine gülten Leistungsprofile").arg(filename));
            return;
        }
        QDialog dial(0);
        dial.setWindowTitle(trUtf8("Leistunsprofil wählen"));
        QVBoxLayout * layout=new QVBoxLayout(&dial);
        layout->addWidget(new QLabel(trUtf8("Wähle das Profil für die Auswertungen:")));
        QComboBox * combo=new QComboBox;
        for(Leistungsprofil profil:profilliste) combo->addItem(profil.name);
        layout->addWidget(combo);
        QDialogButtonBox*bb=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Help);
        connect(bb,SIGNAL(accepted()),&dial,SLOT(accept()));
        connect(bb,SIGNAL(helpRequested()),&dial,SLOT(reject()));
        connect(bb,SIGNAL(helpRequested()),this,SLOT(help()));
        bb->setCenterButtons(true);
        layout->addWidget(bb,Qt::AlignCenter);
        dial.resize(dial.sizeHint());
        //dial.move(mapToGlobal(QPoint(width()/2-50,-dial.height())));
        dial.move(QCursor::pos()-QPoint(0,dial.height()+20));
        if(dial.exec()!=QDialog::Accepted) return;
        leistungsprofil=profilliste.at( combo->currentIndex());
        if(evaluationWidget!=0){
            evaluate();
        }
    }
}

void Hoehenprofil::zoom()
{
    murks=true;
    if(segmentStop<=segmentStart) {
        zoomTo(0,track->points.count()-1);
    }else{
        from=segmentStart; to=segmentStop;
        fromL=segmentStartL;toL=segmentStopL;
        segmentStart=segmentStop;
        repaint();
    }
}

void Hoehenprofil::zoomTo(int afrom, int ato)
{
    if(afrom<0 || afrom>=track->points.count() || ato<0 || ato>=track->points.count()||ato<=afrom) return;
    if(afrom<=from){
        segmentStart=from;segmentStartL=fromL;
    }
    if(ato>=to){
        segmentStop=to;segmentStopL=toL;
    }
    from=afrom; to=ato;
    fromL=track->calcLength(from); toL=track->calcLength(to);
    //murks=true;
    repaint();
}

void Hoehenprofil::help()
{
    //Helpsystem::showPage(QUrl("qrc://));
    Helpsystem::helpsystem->help("/hilfeseiten/hoehenprofile.html",trUtf8("Höhenprofile"));
}

