#ifndef UPOPUPMENU_H
#define UPOPUPMENU_H
#include <QtGui>
/** die Klasse UPopupMenu stellt ein einfaches Popupmenu mit einem Label (Frage) und mehreren
  PushButtons zur VerfÃ¼gung. int popup(QPoint pos) lÃ¤sst es an der gegebenen Position erscheinen und
  liefert die id des gedrÃ¼ckten Buttons **/
class UPopupMenu : public QDialog
{
public:
  UPopupMenu(QString lab);//Ãberschrift
  void addButton(const char* text,int id);//Buttonbeschriftung und id
  int popup(QPoint pos);//liefert die id des gedrÃ¼ckten Buttons, pos muss globale Koordinaten enthalten.
private:
  QVBoxLayout layout;
  QButtonGroup group;
};

#endif // UPOPUPMENU_H
