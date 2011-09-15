/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "menubutton.h"

#include <parameter.h>
#include <quuencode.h>
#include <xsqlquery.h>

#include <QImage>
#include <QMessageBox>
#include <QSqlError>
#include <QtScript>
#include <QVBoxLayout>

GuiClientInterface* MenuButton::_guiClientInterface = 0;

MenuButton::MenuButton(QWidget *pParent) :
  QWidget(pParent)
{
  setupUi(this);

  _action = 0;
  _shown = false;
  setEnabled(false);

  _button->setIconSize(QSize(48,48));
  _button->setIcon(QIcon(QPixmap(":/widgets/images/folder_zoom_64.png")));

#ifdef Q_WS_MAC
  QFont font = _label->font();
  font.setPointSize(11);
  _label->setFont(font);
#endif

  _label->setWordWrap(true);
  _label->setAlignment(Qt::AlignCenter);
}

MenuButton::~MenuButton()
{
}

void MenuButton::actionEvent(QActionEvent *event)
{
    QAction *action = event->action();
    switch (event->type())
    {
    case QEvent::ActionChanged:
        if (action == _action)
            setAction(action); // update button state
        break;
    case QEvent::ActionRemoved:
        if (_action == action)
            _action = 0;
        action->disconnect(this);
        break;
    default:
        ;
    }
    QWidget::actionEvent(event);
}

QString MenuButton::actionName() const
{
  if (_action)
    return _action->objectName();

  return QString();
}

QString MenuButton::image() const
{
  return _image;
}

QString MenuButton::label() const
{
  return _label->text();
}

void MenuButton::setLabel(const QString & text)
{
  _label->setText(text);
}

void MenuButton::setAction(QAction *action)
{
  _button->disconnect();
  if (!action)
    return;

  if (_action)
    removeAction(_action);
  _action = action;
  addAction(_action);
  connect(_button, SIGNAL(clicked()), _action, SLOT(trigger()));
  setEnabled(_action->isEnabled());
}

void MenuButton::setAction(const QString & name)
{
  if (actionName() == name)
    return;

  if (_guiClientInterface)
  {
    QAction *action = _guiClientInterface->findAction(name);
    if (!action)
    {
      // Create one so property can be saved.  Maybe gui client
      // action name provided will be valid in another instance.
      action = new QAction(this);
      action->setObjectName(name);
      action->setEnabled(false);
    }
    setAction(action);
  }
}

void MenuButton::setImage(const QString & image)
{
  _image = image;

  if (_shown)
  {
    XSqlQuery qry;
    qry.prepare("SELECT image_data "
                "FROM image "
                "WHERE (image_name=:image);");
    qry.bindValue(":image", _image);
    qry.exec();
    if (qry.first())
    {
      QImage img;
      img.loadFromData(QUUDecode(qry.value("image_data").toString()));
      _button->setIcon(QIcon(QPixmap::fromImage(img)));
      return;
    }
    else if (qry.lastError().type() != QSqlError::NoError)
      QMessageBox::critical(this, tr("A System Error occurred at %1::%2.")
                            .arg(__FILE__)
                            .arg(__LINE__),
                            qry.lastError().databaseText());
    _button->setIcon(QIcon(QPixmap(":/widgets/images/folder_zoom_64.png")));
  }
}

void MenuButton::showEvent(QShowEvent *e)
{
  if (!_shown)
  {
    _shown = true;
    if (!_image.isEmpty())
      setImage(_image);
  }
  QWidget::showEvent(e);
}

// scripting exposure /////////////////////////////////////////////////////////

QScriptValue MenuButtontoScriptValue(QScriptEngine *engine, MenuButton* const &item)
{
  return engine->newQObject(item);
}

void MenuButtonfromScriptValue(const QScriptValue &obj, MenuButton* &item)
{
  item = qobject_cast<MenuButton*>(obj.toQObject());
}

QScriptValue constructMenuButton(QScriptContext *context,
                                QScriptEngine  *engine)
{
  MenuButton *button = 0;

  if (context->argumentCount() == 0)
    button = new MenuButton();

  else if (context->argumentCount() == 1 &&
           qscriptvalue_cast<QWidget*>(context->argument(0)))
    button = new MenuButton(qscriptvalue_cast<QWidget*>(context->argument(0)));

  else
    context->throwError(QScriptContext::UnknownError,
                        QString("Could not find an appropriate MenuButton constructor"));

  return engine->toScriptValue(button);
}

void setupMenuButton(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, MenuButtontoScriptValue, MenuButtonfromScriptValue);
  QScriptValue widget = engine->newFunction(constructMenuButton);
  engine->globalObject().setProperty("MenuButton", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
