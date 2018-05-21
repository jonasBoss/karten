#include "upopupmenu.h"
#include <QtGui>
UPopupMenu::UPopupMenu(QString lab) :
  QDialog(0)
{
  layout.addWidget(new QLabel(lab,this));
  setLayout(&layout);
}
void UPopupMenu::addButton(const char *text, int id){
  QPushButton *b=new QPushButton(text,this);
  group.addButton(b,id);
  layout.addWidget(b);
}
int UPopupMenu::popup(QPoint pos){
  move(pos);
  connect(&group,SIGNAL(buttonClicked(int)),this,SLOT(done(int)));
  setWindowFlags(Qt::Popup);
  return exec();
}
