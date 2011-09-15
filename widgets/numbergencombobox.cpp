/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QComboBox>
#include <QStandardItemModel>
#include <QtScript>

#include "numbergencombobox.h"
#include "xt.h"

#define DEBUG false

// prototype for reimplementing XComboBox using Qt's model/view.

NumberGenComboBox::NumberGenComboBox(QWidget *pParent) :
  QComboBox(pParent),
  _allowedMethods(Manual | Automatic | Override | Shared)
{
  setAllowedMethods(_allowedMethods);
  connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(handleIndexChanged(int)));
}

void NumberGenComboBox::append(NumberGenComboBox::GenMethod id,
                               QString text,
                               QString code)
{
  int index = count();
  insertItem(index, text);
  setItemData(index, id, Xt::IdRole);
  setItemData(index, code.isEmpty() ? text : code, Xt::RawRole);
}

NumberGenComboBox::~NumberGenComboBox()
{
}

NumberGenComboBox::GenMethods NumberGenComboBox::allowedMethods() const
{
  return _allowedMethods;
}

QString NumberGenComboBox::automaticText() const
{
  int i = indexOfMethod(Automatic);
  return (i >= 0) ? itemText(i) : QString();
}

QString NumberGenComboBox::manualText() const
{
  int i = indexOfMethod(Manual);
  return (i >= 0) ? itemText(i) : QString();
}

// equivalent to XComboBox::id()
NumberGenComboBox::GenMethod NumberGenComboBox::method() const
{
  return (GenMethod)itemData(currentIndex(), Xt::IdRole).toInt();
}

// equivalent to XComboBox::code()
QString NumberGenComboBox::methodCode() const
{
  return itemData(currentIndex(), Xt::RawRole).toString();
}

QString NumberGenComboBox::overrideText() const
{
  int i = indexOfMethod(Override);
  return (i >= 0) ? itemText(i) : QString();
}

QString NumberGenComboBox::sharedText() const
{
  int i = indexOfMethod(Shared);
  return (i >= 0) ? itemText(i) : QString();
}

QString NumberGenComboBox::methodToCode(const GenMethod method)
{
  QString result;
  switch (method)
  {
    case Manual:    result = "M";    break;
    case Automatic: result = "A";    break;
    case Override:  result = "O";    break;
    case Shared:    result = "S";    break;
  }
  return result;
}

void NumberGenComboBox::handleIndexChanged(const int /*index*/)
{
  emit newMethod(method());
  emit newMethod((int)method());
  emit newMethodCode(methodCode());
}

void NumberGenComboBox::setAllowedMethods(const GenMethods methods)
{
  if (DEBUG)
    qDebug("%s::setAllowedMethods(0x%x) entered",
           qPrintable(objectName()), (int)methods);

  clear();

  if (methods & (int)Manual)
    append(Manual, tr("Manual"), methodToCode(Manual));
  if (methods & (int)Automatic)
    append(Automatic, tr("Automatic"), methodToCode(Automatic));
  if (methods & (int)Override)
    append(Override, tr("Automatic With Override"), methodToCode(Override));
  if (methods & (int)Shared)
    append(Shared, tr("Shared"), methodToCode(Shared));

  _allowedMethods = methods;
  
  if (DEBUG)
    qDebug("%s::setAllowedMethods(%x) returning",
           qPrintable(objectName()), (int)methods);
}

void NumberGenComboBox::setAutomaticText(const QString text)
{
  int i = indexOfMethod(Automatic);
  if (i >= 0)
    setItemText(i, text);
}

void NumberGenComboBox::setManualText(const QString text)
{
  int i = indexOfMethod(Manual);
  if (i >= 0)
    setItemText(i, text);
}

// this is equivalent to XComboBox::setId()
void NumberGenComboBox::setMethod(const NumberGenComboBox::GenMethod method)
{
  int i = indexOfMethod(method);
  setCurrentIndex(i);
}

void NumberGenComboBox::setMethod(const QString method)
{
  int index = -1;

  for (int i = 0; i < count(); i++)
    if (itemData(i, Xt::RawRole).toString() == method)
    {
      index = i;
      break;
    }

  setCurrentIndex(index);
}

void NumberGenComboBox::setOverrideText(const QString text)
{
  int i = indexOfMethod(Override);
  if (i >= 0)
    setItemText(i, text);
}

void NumberGenComboBox::setSharedText(const QString text)
{
  int i = indexOfMethod(Shared);
  if (i >= 0)
    setItemText(i, text);
}

int NumberGenComboBox::indexOfMethod(const NumberGenComboBox::GenMethod method) const
{
  for (int i = 0; i < count(); i++)
    if ((GenMethod)itemData(i, Xt::IdRole).toInt() == method)
      return i;
  return -1;
}

// script exposure ////////////////////////////////////////////////////////////

void NumberGenComboBoxFromScriptValue(const QScriptValue &obj,
                                      NumberGenComboBox * &item)
{
  item = qobject_cast<NumberGenComboBox*>(obj.toQObject());
}

QScriptValue NumberGenComboBoxToScriptValue(QScriptEngine *engine,
                                            NumberGenComboBox *const &item)
{
  return engine->newQObject(item);
}

QScriptValue constructNumberGenComboBox(QScriptContext *context,
                                        QScriptEngine  *engine)
{
  NumberGenComboBox *obj = 0;

  if (context->argumentCount() >= 1 &&
      qscriptvalue_cast<QWidget*>(context->argument(0)))
    obj = new NumberGenComboBox(qscriptvalue_cast<QWidget*>(context->argument(0)));
  else
    obj = new NumberGenComboBox();

  return engine->toScriptValue(obj);
}

#define CONST(a)    QScriptValue(engine, a)
#define CONSTFLAGS  QScriptValue::ReadOnly | QScriptValue::Undeletable

void setupNumberGenComboBox(QScriptEngine *engine)
{
  QScriptValue glob = engine->newFunction(constructNumberGenComboBox);

  qScriptRegisterMetaType(engine, NumberGenComboBoxToScriptValue, NumberGenComboBoxFromScriptValue);

  glob.setProperty("Manual",    CONST(NumberGenComboBox::Manual),    CONSTFLAGS);
  glob.setProperty("Automatic", CONST(NumberGenComboBox::Automatic), CONSTFLAGS);
  glob.setProperty("Override",  CONST(NumberGenComboBox::Override),  CONSTFLAGS);
  glob.setProperty("Shared",    CONST(NumberGenComboBox::Shared),    CONSTFLAGS);

  engine->globalObject().setProperty("NumberGenComboBox", glob, CONSTFLAGS);
}
