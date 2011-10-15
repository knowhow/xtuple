#include "include.h"
#include <QMessageBox>

// http://tech-foo.blogspot.com/2008/05/qtscript-exposing-c-classes-part-two.html

void setupMsgBox(QScriptEngine *engine);

class Wrapper_QMessageBox: public QMessageBox, protected QScriptable
{
 Q_OBJECT
 public:
 
  Wrapper_QMessageBox(QScriptEngine *engine)
 : QMessageBox(0) {}
 
 Wrapper_QMessageBox(QWidget *parent =0)
 : QMessageBox(parent) {}

 Wrapper_QMessageBox(Icon icon, const QString & title, const QString & text, StandardButtons buttons = NoButton, QWidget * parent = 0, Qt::WindowFlags f = Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint)
    : QMessageBox(icon, title, text, buttons, parent, f) {}

 public slots:
  QScriptValue qscript_call(QWidget *parent = 0)
  {
   QMessageBox * const iface = new Wrapper_QMessageBox(parent);
   return engine()->newQObject(iface, QScriptEngine::AutoOwnership); 
  }

  QScriptValue qscript_call( Icon icon, const QString & title, const QString & text, StandardButtons buttons = NoButton, QWidget * parent = 0, Qt::WindowFlags f = Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint )
  {
   QMessageBox * const iface = new Wrapper_QMessageBox(icon,title,text,buttons,parent,f);
   return engine()->newQObject(iface, QScriptEngine::AutoOwnership);
  }
  
  void setWindowTitle ( const QString & title );

};

