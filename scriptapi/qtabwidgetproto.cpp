/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qtabwidgetproto.h"

#include <QString>

void setupQTabWidgetProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QTabWidgetProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QTabWidget*>(), proto);
  //engine->setDefaultPrototype(qMetaTypeId<QTabWidget>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQTabWidget,
                                                 proto);
  engine->globalObject().setProperty("QTabWidget",  constructor);
}

QScriptValue constructQTabWidget(QScriptContext * /*context*/,
                                    QScriptEngine  *engine)
{
  QTabWidget *obj = 0;
  // if (context->argumentCount() > 0)
  // else
    obj = new QTabWidget();
  return engine->toScriptValue(obj);
}

QTabWidgetProto::QTabWidgetProto(QObject *parent)
    : QObject(parent)
{
}

int QTabWidgetProto::addTab(QWidget * page, const QString &label)
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    return item->addTab(page, label);
  return 0;
}

int QTabWidgetProto::addTab(QWidget * page, const QIcon &icon, const QString &label)
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    return item->addTab(page, icon, label);
  return 0;
}

void QTabWidgetProto::clear()
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    item->clear();
}

QWidget *QTabWidgetProto::cornerWidget(int corner) const
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    return item->cornerWidget((Qt::Corner)corner);
  return 0;
}

int QTabWidgetProto::count() const
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    return item->count();
  return 0;
}

int QTabWidgetProto::currentIndex() const
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    return item->currentIndex();
  return 0;
}

QWidget *QTabWidgetProto::currentWidget() const
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    return item->currentWidget();
  return 0;
}

#if QT_VERSION >= 0x040500
bool QTabWidgetProto::documentMode() const
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    return item->documentMode();
  return false;
}
#endif

int QTabWidgetProto::elideMode() const
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    return item->elideMode();
  return 0;
}

QSize QTabWidgetProto::iconSize() const
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    return item->iconSize();
  return QSize();
}

int QTabWidgetProto::indexOf(QWidget *w) const
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    return item->indexOf(w);
  return 0;
}

int QTabWidgetProto::insertTab(int index, QWidget *page, const QString &label)
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    return item->insertTab(index, page, label);
  return 0;
}

int QTabWidgetProto::insertTab(int index, QWidget *page, const QIcon &icon, const QString & label)
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    return item->insertTab(index, page, icon, label);
  return 0;
}

#if QT_VERSION >= 0x040500
bool QTabWidgetProto::isMovable() const
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    return item->isMovable();
  return false;
}
#endif

bool QTabWidgetProto::isTabEnabled(int index) const
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    return item->isTabEnabled(index);
  return false;
}

void QTabWidgetProto::removeTab(int index)
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    item->removeTab(index);
}

void QTabWidgetProto::setCornerWidget(QWidget *widget, int corner)
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    item->setCornerWidget(widget, (Qt::Corner)corner);
}

#if QT_VERSION >= 0x040500
void QTabWidgetProto::setDocumentMode(bool set)
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    item->setDocumentMode(set);
}
#endif

void QTabWidgetProto::setElideMode(int mode)
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    item->setElideMode((Qt::TextElideMode)mode);
}

void QTabWidgetProto::setIconSize(const QSize &size)
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    item->setIconSize(size);
}

#if QT_VERSION >= 0x040500
void QTabWidgetProto::setMovable(bool movable)
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    item->setMovable(movable);
}
#endif

void QTabWidgetProto::setTabEnabled(int index, bool enable)
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    item->setTabEnabled(index, enable);
}

void QTabWidgetProto::setTabIcon(int index, const QIcon &icon)
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    item->setTabIcon(index, icon);
}

void QTabWidgetProto::setTabPosition(int position)
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    item->setTabPosition((QTabWidget::TabPosition)position);
}

void QTabWidgetProto::setTabShape(int s)
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    item->setTabShape((QTabWidget::TabShape)s);
}

void QTabWidgetProto::setTabText(int index, const QString &label)
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    item->setTabText(index, label);
}

void QTabWidgetProto::setTabToolTip(int index, const QString &tip)
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    item->setTabToolTip(index, tip);
}

void QTabWidgetProto::setTabWhatsThis(int index, const QString &text)
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    item->setTabWhatsThis(index, text);
}

#if QT_VERSION >= 0x040500
void QTabWidgetProto::setTabsClosable(bool closeable)
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    item->setTabsClosable(closeable);
}
#endif

void QTabWidgetProto::setUsesScrollButtons(bool useButtons)
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    item->setUsesScrollButtons(useButtons);
}

QIcon QTabWidgetProto::tabIcon(int index) const
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    return item->tabIcon(index);
  return QIcon();
}

int QTabWidgetProto::tabPosition() const
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    return item->tabPosition();
  return 0;
}

int QTabWidgetProto::tabShape() const
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    return item->tabShape();
  return 0;
}

QString QTabWidgetProto::tabText(int index) const
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    return item->tabText(index);
  return QString();
}

QString QTabWidgetProto::tabToolTip(int index) const
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    return item->tabToolTip(index);
  return QString();
}

QString QTabWidgetProto::tabWhatsThis(int index) const
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    return item->tabWhatsThis(index);
  return QString();
}

#if QT_VERSION >= 0x040500
bool QTabWidgetProto::tabsClosable() const
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    return item->tabsClosable();
  return false;
}
#endif

bool QTabWidgetProto::usesScrollButtons() const
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    return item->usesScrollButtons();
  return false;
}

QWidget *QTabWidgetProto::widget(int index) const
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    return item->widget(index);
  return 0;
}

/*
QString QTabWidgetProto::toString() const
{
  QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
  if (item)
    return QString("QTabWidget()");
  return QString("QTabWidget(unknown)");
}
*/
