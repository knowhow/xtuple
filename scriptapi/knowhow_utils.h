#include "include.h"

class Wrapper_QMessageBox: public QMessageBox, protected QScriptable
{
 Q_OBJECT
 public:
 
 Wrapper_QMessageBox(QWidget *parent =0)
 : QMessageBox(parent) {}
 
 public slots:
  QScriptValue qscript_call(QWidget *parent = 0)
  {
   QMessageBox * const iface = new Wrapper_QMessageBox(parent);
   return engine()->newQObject(iface, QScriptEngine::AutoOwnership); 
  }

};

