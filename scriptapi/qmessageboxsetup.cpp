/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qmessageboxsetup.h"

QScriptValue MessageBoxButtonRoletoScriptValue(QScriptEngine *engine, const enum QMessageBox::ButtonRole &p)
{
  return QScriptValue(engine, (int)p);
}

void MessageBoxButtonRolefromScriptValue(const QScriptValue &obj, enum QMessageBox::ButtonRole &p)
{
  p = (enum QMessageBox::ButtonRole)obj.toInt32();
}

QScriptValue MessageBoxIcontoScriptValue(QScriptEngine *engine, const enum QMessageBox::Icon &p)
{
  return QScriptValue(engine, (int)p);
}

void MessageBoxIconfromScriptValue(const QScriptValue &obj, enum QMessageBox::Icon &p)
{
  p = (enum QMessageBox::Icon)obj.toInt32();
}

QScriptValue MessageBoxStandardButtontoScriptValue(QScriptEngine *engine, const enum QMessageBox::StandardButton &p)
{
  return QScriptValue(engine, (int)p);
}

void MessageBoxStandardButtonfromScriptValue(const QScriptValue &obj, enum QMessageBox::StandardButton &p)
{
  p = (enum QMessageBox::StandardButton)obj.toInt32();
}

QScriptValue scriptAbout(QScriptContext *context, QScriptEngine * /*engine*/)
{
  if (context->argumentCount() >= 3 &&
      qscriptvalue_cast<QWidget*>(context->argument(0)) &&
      context->argument(1).isString() &&
      context->argument(2).isString())
    QMessageBox::about(qscriptvalue_cast<QWidget*>(context->argument(0)),
                       context->argument(1).toString(),
                       context->argument(2).toString());
  else
    context->throwError(QScriptContext::UnknownError,
                        "could not find an appropriate QMessageBox::about()");

  return QScriptValue();
}

QScriptValue scriptAboutQt(QScriptContext *context, QScriptEngine * /*engine*/)
{
  if (context->argumentCount() >= 2 &&
      qscriptvalue_cast<QWidget*>(context->argument(0)) &&
      context->argument(1).isString())
    QMessageBox::aboutQt(qscriptvalue_cast<QWidget*>(context->argument(0)),
                         context->argument(1).toString());
  else
    context->throwError(QScriptContext::UnknownError,
                        "could not find an appropriate QMessageBox::aboutQt()");

  return QScriptValue();
}

QScriptValue scriptCritical(QScriptContext *context, QScriptEngine * /*engine*/)
{
  QMessageBox::StandardButton result = QMessageBox::NoButton;

  if (context->argumentCount() == 3 &&
      qscriptvalue_cast<QWidget*>(context->argument(0)) &&
      context->argument(1).isString() &&
      context->argument(2).isString())
    result = QMessageBox::critical(qscriptvalue_cast<QWidget*>(context->argument(0)),
                                   context->argument(1).toString(),
                                   context->argument(2).toString());

  else if (context->argumentCount() == 4 &&
      qscriptvalue_cast<QWidget*>(context->argument(0)) &&
      context->argument(1).isString() &&
      context->argument(2).isString() &&
      context->argument(3).isNumber())
    result = QMessageBox::critical(qscriptvalue_cast<QWidget*>(context->argument(0)),
                                   context->argument(1).toString(),
                                   context->argument(2).toString(),
                                   (QMessageBox::StandardButton)(context->argument(3).toInt32()));

  else if (context->argumentCount() >= 5 &&
      qscriptvalue_cast<QWidget*>(context->argument(0)) &&
      context->argument(1).isString() &&
      context->argument(2).isString() &&
      context->argument(3).isNumber() &&
      context->argument(4).isNumber())
    result = QMessageBox::critical(qscriptvalue_cast<QWidget*>(context->argument(0)),
                                   context->argument(1).toString(),
                                   context->argument(2).toString(),
                                   (QMessageBox::StandardButtons)(context->argument(3).toInt32()),
                                   (QMessageBox::StandardButton)(context->argument(4).toInt32()));

  else
    context->throwError(QScriptContext::UnknownError,
                        "could not find an appropriate QMessageBox::critical()");
  return QScriptValue((int)result);
}

QScriptValue scriptInformation(QScriptContext *context, QScriptEngine * /*engine*/)
{
  QMessageBox::StandardButton result = QMessageBox::NoButton;

  if (context->argumentCount() == 3 &&
      qscriptvalue_cast<QWidget*>(context->argument(0)) &&
      context->argument(1).isString() &&
      context->argument(2).isString())
    result = QMessageBox::information(qscriptvalue_cast<QWidget*>(context->argument(0)),
                                   context->argument(1).toString(),
                                   context->argument(2).toString());

  else if (context->argumentCount() == 4 &&
      qscriptvalue_cast<QWidget*>(context->argument(0)) &&
      context->argument(1).isString() &&
      context->argument(2).isString() &&
      context->argument(3).isNumber())
    result = QMessageBox::information(qscriptvalue_cast<QWidget*>(context->argument(0)),
                                   context->argument(1).toString(),
                                   context->argument(2).toString(),
                                   (QMessageBox::StandardButton)(context->argument(3).toInt32()));

  else if (context->argumentCount() >= 5 &&
      qscriptvalue_cast<QWidget*>(context->argument(0)) &&
      context->argument(1).isString() &&
      context->argument(2).isString() &&
      context->argument(3).isNumber() &&
      context->argument(4).isNumber())
    result = QMessageBox::information(qscriptvalue_cast<QWidget*>(context->argument(0)),
                                   context->argument(1).toString(),
                                   context->argument(2).toString(),
                                   (QMessageBox::StandardButtons)(context->argument(3).toInt32()),
                                   (QMessageBox::StandardButton)(context->argument(4).toInt32()));

  else
    context->throwError(QScriptContext::UnknownError,
                        "could not find an appropriate QMessageBox::information()");
  return QScriptValue((int)result);
}
QScriptValue scriptQuestion(QScriptContext *context, QScriptEngine * /*engine*/)
{
  QMessageBox::StandardButton result = QMessageBox::NoButton;

  if (context->argumentCount() == 3 &&
      qscriptvalue_cast<QWidget*>(context->argument(0)) &&
      context->argument(1).isString() &&
      context->argument(2).isString())
    result = QMessageBox::question(qscriptvalue_cast<QWidget*>(context->argument(0)),
                                   context->argument(1).toString(),
                                   context->argument(2).toString());

  else if (context->argumentCount() == 4 &&
      qscriptvalue_cast<QWidget*>(context->argument(0)) &&
      context->argument(1).isString() &&
      context->argument(2).isString() &&
      context->argument(3).isNumber())
    result = QMessageBox::question(qscriptvalue_cast<QWidget*>(context->argument(0)),
                                   context->argument(1).toString(),
                                   context->argument(2).toString(),
                                   (QMessageBox::StandardButtons)(context->argument(3).toInt32()));

  else if (context->argumentCount() >= 5 &&
      qscriptvalue_cast<QWidget*>(context->argument(0)) &&
      context->argument(1).isString() &&
      context->argument(2).isString() &&
      context->argument(3).isNumber() &&
      context->argument(4).isNumber())
    result = QMessageBox::question(qscriptvalue_cast<QWidget*>(context->argument(0)),
                                   context->argument(1).toString(),
                                   context->argument(2).toString(),
                                   (QMessageBox::StandardButtons)(context->argument(3).toInt32()),
                                   (QMessageBox::StandardButton)(context->argument(4).toInt32()));

  else
    context->throwError(QScriptContext::UnknownError,
                        "could not find an appropriate QMessageBox::question()");
  return QScriptValue((int)result);
}
QScriptValue scriptWarning(QScriptContext *context, QScriptEngine * /*engine*/)
{
  QMessageBox::StandardButton result = QMessageBox::NoButton;

  if (context->argumentCount() == 3 &&
      qscriptvalue_cast<QWidget*>(context->argument(0)) &&
      context->argument(1).isString() &&
      context->argument(2).isString())
    result = QMessageBox::warning(qscriptvalue_cast<QWidget*>(context->argument(0)),
                                   context->argument(1).toString(),
                                   context->argument(2).toString());

  else if (context->argumentCount() == 4 &&
      qscriptvalue_cast<QWidget*>(context->argument(0)) &&
      context->argument(1).isString() &&
      context->argument(2).isString() &&
      context->argument(3).isNumber())
    result = QMessageBox::warning(qscriptvalue_cast<QWidget*>(context->argument(0)),
                                   context->argument(1).toString(),
                                   context->argument(2).toString(),
                                   (QMessageBox::StandardButton)(context->argument(3).toInt32()));

  else if (context->argumentCount() >= 5 &&
      qscriptvalue_cast<QWidget*>(context->argument(0)) &&
      context->argument(1).isString() &&
      context->argument(2).isString() &&
      context->argument(3).isNumber() &&
      context->argument(4).isNumber())
    result = QMessageBox::warning(qscriptvalue_cast<QWidget*>(context->argument(0)),
                                   context->argument(1).toString(),
                                   context->argument(2).toString(),
                                   (QMessageBox::StandardButtons)(context->argument(3).toInt32()),
                                   (QMessageBox::StandardButton)(context->argument(4).toInt32()));

  else
    context->throwError(QScriptContext::UnknownError,
                        "could not find an appropriate QMessageBox::warning()");
  return QScriptValue((int)result);
}
void setupQMessageBox(QScriptEngine *engine)
{
  QScriptValue widget = engine->newObject();

  qScriptRegisterMetaType(engine, MessageBoxButtonRoletoScriptValue,
                          MessageBoxButtonRolefromScriptValue);
  widget.setProperty("InvalidRole",    QScriptValue(engine, QMessageBox::InvalidRole),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AcceptRole",     QScriptValue(engine, QMessageBox::AcceptRole),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("RejectRole",     QScriptValue(engine, QMessageBox::RejectRole),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("DestructiveRole",QScriptValue(engine, QMessageBox::DestructiveRole),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ActionRole",     QScriptValue(engine, QMessageBox::ActionRole),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("HelpRole",       QScriptValue(engine, QMessageBox::HelpRole),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("YesRole",        QScriptValue(engine, QMessageBox::YesRole),        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("NoRole",         QScriptValue(engine, QMessageBox::NoRole),         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ApplyRole",      QScriptValue(engine, QMessageBox::ApplyRole),      QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ResetRole",      QScriptValue(engine, QMessageBox::ResetRole),      QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, MessageBoxIcontoScriptValue,
                          MessageBoxIconfromScriptValue);
  widget.setProperty("NoIcon",         QScriptValue(engine, QMessageBox::NoIcon),         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Question",       QScriptValue(engine, QMessageBox::Question),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Information",    QScriptValue(engine, QMessageBox::Information),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Warning",        QScriptValue(engine, QMessageBox::Warning),        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Critical",       QScriptValue(engine, QMessageBox::Critical),       QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, MessageBoxStandardButtontoScriptValue,
                          MessageBoxStandardButtonfromScriptValue);
  widget.setProperty("Ok",             QScriptValue(engine, QMessageBox::Ok),             QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Open",           QScriptValue(engine, QMessageBox::Open),           QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Save",           QScriptValue(engine, QMessageBox::Save),           QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Cancel",         QScriptValue(engine, QMessageBox::Cancel),         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Close",          QScriptValue(engine, QMessageBox::Close),          QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Discard",        QScriptValue(engine, QMessageBox::Discard),        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Apply",          QScriptValue(engine, QMessageBox::Apply),          QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Reset",          QScriptValue(engine, QMessageBox::Reset),          QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("RestoreDefaults",QScriptValue(engine, QMessageBox::RestoreDefaults),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Help",           QScriptValue(engine, QMessageBox::Help),           QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SaveAll",        QScriptValue(engine, QMessageBox::SaveAll),        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Yes",            QScriptValue(engine, QMessageBox::Yes),            QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("YesToAll",       QScriptValue(engine, QMessageBox::YesToAll),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("No",             QScriptValue(engine, QMessageBox::No),             QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("NoToAll",        QScriptValue(engine, QMessageBox::NoToAll),        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Abort",          QScriptValue(engine, QMessageBox::Abort),          QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Retry",          QScriptValue(engine, QMessageBox::Retry),          QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Ignore",         QScriptValue(engine, QMessageBox::Ignore),         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("NoButton",       QScriptValue(engine, QMessageBox::NoButton),       QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("QMessageBox", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);

  widget.setProperty("about",      engine->newFunction(scriptAbout));
  widget.setProperty("aboutQt",    engine->newFunction(scriptAboutQt));
  widget.setProperty("critical",   engine->newFunction(scriptCritical));
  widget.setProperty("information",engine->newFunction(scriptInformation));
  widget.setProperty("question",   engine->newFunction(scriptQuestion));
  widget.setProperty("warning",    engine->newFunction(scriptWarning));
}
