/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qmenuproto.h"

#define DEBUG false

void setupQMenuProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QMenuProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QMenu*>(), proto);
  //engine->setDefaultPrototype(qMetaTypeId<QMenu>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQMenu,
                                                 proto);
  engine->globalObject().setProperty("QMenu", constructor);
}

QScriptValue constructQMenu(QScriptContext *context,
                            QScriptEngine  *engine)
{
  QMenu *obj = 0;
  if (context->argumentCount() == 1 && context->argument(0).isQObject())
    obj = new QMenu(qobject_cast<QWidget*>(context->argument(0).toQObject()));
  else if (context->argumentCount() == 2 && context->argument(1).isQObject())
    obj = new QMenu(context->argument(0).toString(),
                    qobject_cast<QWidget*>(context->argument(1).toQObject()));
  else
    obj = new QMenu();
  return engine->toScriptValue(obj);
}

QMenuProto::QMenuProto(QObject *parent)
    : QObject(parent)
{
}

QAction *QMenuProto::actionAt(const QPoint &pt) const
{
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    return item->actionAt(pt);
  return 0;
}

QRect QMenuProto::actionGeometry(QAction *act) const
{
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    return item->actionGeometry(act);
  return QRect();
}

QAction *QMenuProto::activeAction() const
{
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    return item->activeAction();
  return 0;
}

QList<QAction*> QMenuProto::actions() const
{
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    return item->actions();
  return QList<QAction*>();
}

QAction *QMenuProto::addAction(const QString &text)
{
  if (DEBUG) qDebug("addAction(QString = %s)", qPrintable(text));
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    return item->addAction(text);
  return 0;
}

QAction *QMenuProto::addAction(const QIcon &icon, const QString &text)
{
  if (DEBUG) qDebug("addAction(QIcon, QString = %s)", qPrintable(text));
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    return item->addAction(icon, text);
  return 0;
}

QAction *QMenuProto::addAction(const QString &text, const QObject *receiver, const char *member, const QKeySequence &shortcut)
{
  if (DEBUG) qDebug("addAction(QString = %s, QObject, char* = %s, QKeySequence)", qPrintable(text), member);
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    return item->addAction(text, receiver, member, shortcut);
  return 0;
}

QAction *QMenuProto::addAction(const QIcon &icon, const QString &text, const QObject *receiver, const char *member, const QKeySequence &shortcut)
{
  if (DEBUG) qDebug("addAction(QIcon, QString = %s, QObject, char* = %s, QKeySequence)", qPrintable(text), member);
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    return item->addAction(icon, text, receiver, member, shortcut);
  return 0;
}

void QMenuProto::appendAction(QAction *action)
{
  // Can't use addAction here because of ambiguity problem
  if (DEBUG) qDebug("addAction(QAction)");
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    item->addAction(action);
}

QAction *QMenuProto::addMenu(QMenu *menu)
{
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    return item->addMenu(menu);
  return 0;
}

QMenu *QMenuProto::addMenu(const QString &title)
{
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    return item->addMenu(title);
  return 0;
}

QMenu *QMenuProto::addMenu(const QIcon &icon, const QString &title)
{
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    return item->addMenu(icon, title);
  return 0;
}

QAction *QMenuProto::addSeparator()
{
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    return item->addSeparator();
  return 0;
}

void QMenuProto::clear()
{
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    item->clear();
}

QAction *QMenuProto::defaultAction() const
{
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    return item->defaultAction();
  return 0;
}

QAction *QMenuProto::exec()
{
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    return item->exec();
  return 0;
}

QAction *QMenuProto::exec(const QPoint &p, QAction *action)
{
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    return item->exec(p, action);
  return 0;
}

void QMenuProto::hideTearOffMenu()
{
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    item->hideTearOffMenu();
}

QIcon QMenuProto::icon() const
{
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    return item->icon();
  return QIcon();
}

void QMenuProto::insertAction(QAction *before, QAction *action)
{
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    item->insertAction(before, action);
}

void QMenuProto::insertActions(QAction *before, QList<QAction *>actions)
{
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    item->insertActions(before, actions);
}

QAction *QMenuProto::insertMenu(QAction *before, QMenu *menu)
{
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    return item->insertMenu(before, menu);
  return 0;
}

QAction *QMenuProto::insertSeparator(QAction *before)
{
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    return item->insertSeparator(before);
  return 0;
}

bool QMenuProto::isEmpty() const
{
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    return item->isEmpty();
  return false;
}

bool QMenuProto::isTearOffEnabled() const
{
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    return item->isTearOffEnabled();
  return false;
}

bool QMenuProto::isTearOffMenuVisible() const
{
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    return item->isTearOffMenuVisible();
  return false;
}

QAction *QMenuProto::menuAction() const
{
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    return item->menuAction();
  return 0;
}

void QMenuProto::popup(const QPoint &p, QAction *atAction)
{
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    item->popup(p, atAction);
}

void QMenuProto::removeAction(QAction *action)
{
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    item->removeAction(action);
}

bool QMenuProto::separatorsCollapsible() const
{
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    return item->separatorsCollapsible();
  return false;
}

void QMenuProto::setActiveAction(QAction *act)
{
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    item->setActiveAction(act);
}

void QMenuProto::setDefaultAction(QAction *act)
{
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    item->setDefaultAction(act);
}

void QMenuProto::setIcon(const QIcon &icon)
{
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    item->setIcon(icon);
}

void QMenuProto::setSeparatorsCollapsible(bool collapse)
{
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    item->setSeparatorsCollapsible(collapse);
}

void QMenuProto::setTearOffEnabled(bool enabled)
{
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    item->setTearOffEnabled(enabled);
}

void QMenuProto::setTitle(const QString &title)
{
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    item->setTitle(title);
}

QString QMenuProto::title() const
{
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    return item->title();
  return QString();
}

/*
HMENU QMenuProto::wceMenu(bool create)
{
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    return item->wceMenu(create);
  return HMENU();
}
*/

QString QMenuProto::toString() const
{
  QMenu *item = qscriptvalue_cast<QMenu*>(thisObject());
  if (item)
    return QString("[QMenu named %1: %2]")
                  .arg(item->objectName())
                  .arg(item->title());
  return QString("QMenu(unknown)");
}
