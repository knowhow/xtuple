#include "include.h"
#include <QMessageBox>

// http://tech-foo.blogspot.com/2008/05/qtscript-exposing-c-classes-part-two.html

class Wrapper_QMessageBox: public QMessageBox, protected QScriptable
{
 Q_OBJECT
 public:
 
  Wrapper_QMessageBox(QScriptEngine *engine)
 : QMessageBox(0) {}
 
 Wrapper_QMessageBox(QWidget *parent =0)
 : QMessageBox(parent) {}
 
 public slots:
  QScriptValue qscript_call(QWidget *parent = 0)
  {
   QMessageBox * const iface = new Wrapper_QMessageBox(parent);
   return engine()->newQObject(iface, QScriptEngine::AutoOwnership); 
  }

};

