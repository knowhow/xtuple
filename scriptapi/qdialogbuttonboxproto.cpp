/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qdialogbuttonboxproto.h"

#include <QScriptEngine>
#include <QDialogButtonBox>

QScriptValue QDialogButtonBoxtoScriptValue(QScriptEngine *engine, QDialogButtonBox* const &item)
{
  return engine->newQObject(item);
}

void QDialogButtonBoxfromScriptValue(const QScriptValue &obj, QDialogButtonBox* &item)
{
  item = qobject_cast<QDialogButtonBox*>(obj.toQObject());
}

QScriptValue QDialogButtonBoxButtonLayouttoScriptValue(QScriptEngine *engine, const enum QDialogButtonBox::ButtonLayout &p)
{
  return QScriptValue(engine, (int)p);
}

void QDialogButtonBoxButtonLayoutfromScriptValue(const QScriptValue &obj, enum QDialogButtonBox::ButtonLayout &p)
{
  p = (enum QDialogButtonBox::ButtonLayout)obj.toInt32();
}

QScriptValue QDialogButtonBoxButtonRoletoScriptValue(QScriptEngine *engine, const enum QDialogButtonBox::ButtonRole &p)
{
  return QScriptValue(engine, (int)p);
}

void QDialogButtonBoxButtonRolefromScriptValue(const QScriptValue &obj, enum QDialogButtonBox::ButtonRole &p)
{
  p = (enum QDialogButtonBox::ButtonRole)obj.toInt32();
}

QScriptValue QDialogButtonBoxStandardButtontoScriptValue(QScriptEngine *engine,  const enum QDialogButtonBox::StandardButton &p)
{
  return QScriptValue(engine, (int)p);
}

void QDialogButtonBoxStandardButtonfromScriptValue(const QScriptValue &obj, enum QDialogButtonBox::StandardButton &p)
{
  p = (enum QDialogButtonBox::StandardButton)obj.toInt32();
}

void setupQDialogButtonBoxProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QDialogButtonBoxtoScriptValue, QDialogButtonBoxfromScriptValue);

  QScriptValue proto = engine->newQObject(new QDialogButtonBoxProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QDialogButtonBox*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQDialogButtonBox,
                                                 proto);

  engine->globalObject().setProperty("QDialogButtonBox",  constructor);

  // enum ButtonLayout
  qScriptRegisterMetaType(engine, QDialogButtonBoxButtonLayouttoScriptValue, QDialogButtonBoxButtonLayoutfromScriptValue);
  constructor.setProperty("WinLayout", QScriptValue(engine, QDialogButtonBox::WinLayout), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("MacLayout", QScriptValue(engine, QDialogButtonBox::MacLayout), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("KdeLayout", QScriptValue(engine, QDialogButtonBox::KdeLayout), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("GnomeLayout", QScriptValue(engine, QDialogButtonBox::GnomeLayout), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  // enum ButtonRole
  qScriptRegisterMetaType(engine, QDialogButtonBoxButtonRoletoScriptValue, QDialogButtonBoxButtonRolefromScriptValue);
  constructor.setProperty("InvalidRole", QScriptValue(engine, QDialogButtonBox::InvalidRole), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("AcceptRole", QScriptValue(engine, QDialogButtonBox::AcceptRole), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("RejectRole", QScriptValue(engine, QDialogButtonBox::RejectRole), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("DestructiveRole", QScriptValue(engine, QDialogButtonBox::DestructiveRole), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("ActionRole", QScriptValue(engine, QDialogButtonBox::ActionRole), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("HelpRole", QScriptValue(engine, QDialogButtonBox::HelpRole), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("YesRole", QScriptValue(engine, QDialogButtonBox::YesRole), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("NoRole", QScriptValue(engine, QDialogButtonBox::NoRole), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("ApplyRole", QScriptValue(engine, QDialogButtonBox::ApplyRole), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("ResetRole", QScriptValue(engine, QDialogButtonBox::ResetRole), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  // enum StandardButton
  qScriptRegisterMetaType(engine, QDialogButtonBoxStandardButtontoScriptValue, QDialogButtonBoxStandardButtonfromScriptValue);
  constructor.setProperty("Ok", QScriptValue(engine, QDialogButtonBox::Ok), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Open", QScriptValue(engine, QDialogButtonBox::Open), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Save", QScriptValue(engine, QDialogButtonBox::Save ), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Cancel", QScriptValue(engine, QDialogButtonBox::Cancel), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Close", QScriptValue(engine, QDialogButtonBox::Close), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Discard", QScriptValue(engine, QDialogButtonBox::Discard), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Apply", QScriptValue(engine, QDialogButtonBox::Apply), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Reset", QScriptValue(engine, QDialogButtonBox::Reset), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("RestoreDefaults", QScriptValue(engine, QDialogButtonBox::RestoreDefaults), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Help", QScriptValue(engine, QDialogButtonBox::Help), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("SaveAll", QScriptValue(engine, QDialogButtonBox::SaveAll), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Yes", QScriptValue(engine, QDialogButtonBox::Yes), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("YesToAll", QScriptValue(engine, QDialogButtonBox::YesToAll ), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("No", QScriptValue(engine, QDialogButtonBox::No), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("NoToAll", QScriptValue(engine, QDialogButtonBox::NoToAll), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Abort", QScriptValue(engine, QDialogButtonBox::Abort), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Retry", QScriptValue(engine, QDialogButtonBox::Retry), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Ignore", QScriptValue(engine, QDialogButtonBox::Ignore), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("NoButton", QScriptValue(engine, QDialogButtonBox::NoButton), QScriptValue::ReadOnly | QScriptValue::Undeletable);
}

QScriptValue constructQDialogButtonBox(QScriptContext * context,
                                    QScriptEngine  *engine)
{
  QDialogButtonBox *obj = 0;
  if (context->argumentCount() == 3)
  {
    obj = new QDialogButtonBox((QDialogButtonBox::StandardButton)(context->argument(0).toInt32()),
                               (Qt::Orientation)(context->argument(1).toInt32()),
                               qobject_cast<QWidget*>(context->argument(2).toQObject()));
  }
  else if (context->argumentCount() == 2)
  {
    obj = new QDialogButtonBox((Qt::Orientation)(context->argument(0).toInt32()),
                               qobject_cast<QWidget*>(context->argument(1).toQObject()));
  }
  else if (context->argumentCount() == 1)
  {
    obj = new QDialogButtonBox(qobject_cast<QWidget*>(context->argument(0).toQObject()));
  }
  else
    obj = new QDialogButtonBox();
  return engine->toScriptValue(obj);
}

QDialogButtonBoxProto::QDialogButtonBoxProto(QObject *parent)
    : QObject(parent)
{
}

void QDialogButtonBoxProto::addButton ( QAbstractButton * button, QDialogButtonBox::ButtonRole role )
{
  QDialogButtonBox *item = qscriptvalue_cast<QDialogButtonBox*>(thisObject());
  if (item)
    item->addButton ( button, role );
}

QPushButton * QDialogButtonBoxProto::addButton ( const QString & text, QDialogButtonBox::ButtonRole role )
{
  QDialogButtonBox *item = qscriptvalue_cast<QDialogButtonBox*>(thisObject());
  if (item)
    return item->addButton( text, role );

  return 0;
}

QPushButton * QDialogButtonBoxProto::addButton ( QDialogButtonBox::StandardButton button )
{
  QDialogButtonBox *item = qscriptvalue_cast<QDialogButtonBox*>(thisObject());
  if (item)
    return item->addButton( button );

  return 0;
}

QPushButton* QDialogButtonBoxProto::button ( QDialogButtonBox::StandardButton which ) const
{
  QDialogButtonBox *item = qscriptvalue_cast<QDialogButtonBox*>(thisObject());
  if (item)
    return item->button( which );

  return 0;
}

QDialogButtonBox::ButtonRole QDialogButtonBoxProto::buttonRole ( QAbstractButton * button ) const
{
  QDialogButtonBox *item = qscriptvalue_cast<QDialogButtonBox*>(thisObject());
  if (item)
    return item->buttonRole(button);

  return QDialogButtonBox::NoRole;
}

QList<QAbstractButton *> QDialogButtonBoxProto::buttons () const
{
  QDialogButtonBox *item = qscriptvalue_cast<QDialogButtonBox*>(thisObject());
  if (item)
    return item->buttons();

  return QList<QAbstractButton *>();
}

void QDialogButtonBoxProto::clear ()
{
  QDialogButtonBox *item = qscriptvalue_cast<QDialogButtonBox*>(thisObject());
  if (item)
    item->clear();
}

void QDialogButtonBoxProto::removeButton ( QAbstractButton * button )
{
  QDialogButtonBox *item = qscriptvalue_cast<QDialogButtonBox*>(thisObject());
  if (item)
    item->removeButton(button);
}

QDialogButtonBox::StandardButton QDialogButtonBoxProto::standardButton ( QAbstractButton * button ) const
{
  QDialogButtonBox *item = qscriptvalue_cast<QDialogButtonBox*>(thisObject());
  if (item)
    return item->standardButton(button);

  return QDialogButtonBox::NoButton;
}

