/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QMENUPROTO_H__
#define __QMENUPROTO_H__

#include <QMenu>
#include <QObject>
#include <QtScript>

#include "qactionproto.h"

Q_DECLARE_METATYPE(QMenu*)

void setupQMenuProto(QScriptEngine *engine);
QScriptValue constructQMenu(QScriptContext *context, QScriptEngine *engine);

class QMenuProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QMenuProto(QObject *parent);

    Q_INVOKABLE QAction *actionAt(const QPoint &pt)	const;
    Q_INVOKABLE QRect    actionGeometry(QAction *act)	const;
    Q_INVOKABLE QAction *activeAction()	                const;
    Q_INVOKABLE QList<QAction*> actions()               const;
    Q_INVOKABLE QAction *addAction(const QString &text);
    Q_INVOKABLE QAction *addAction(const QIcon &icon, const QString &text);
    Q_INVOKABLE QAction *addAction(const QString &text, const QObject *receiver, const char *member, const QKeySequence &shortcut = 0);
    Q_INVOKABLE QAction *addAction(const QIcon &icon, const QString &text, const QObject *receiver, const char *member, const QKeySequence &shortcut = 0);
    Q_INVOKABLE void     appendAction(QAction *action);
    Q_INVOKABLE QAction *addMenu(QMenu *menu);
    Q_INVOKABLE QMenu   *addMenu(const QString &title);
    Q_INVOKABLE QMenu   *addMenu(const QIcon &icon, const QString &title);
    Q_INVOKABLE QAction *addSeparator();
    Q_INVOKABLE void     clear();
    Q_INVOKABLE QAction *defaultAction()	        const;
    Q_INVOKABLE QAction *exec();
    Q_INVOKABLE QAction *exec(const QPoint &p, QAction *action = 0);
    Q_INVOKABLE void     hideTearOffMenu();
    Q_INVOKABLE QIcon    icon()	                        const;
    Q_INVOKABLE void     insertAction(QAction *before, QAction *action);
    Q_INVOKABLE void     insertActions(QAction *before, QList<QAction*>actions);
    Q_INVOKABLE QAction *insertMenu(QAction *before, QMenu *menu);
    Q_INVOKABLE QAction *insertSeparator(QAction *before);
    Q_INVOKABLE bool     isEmpty()	                const;
    Q_INVOKABLE bool     isTearOffEnabled()	        const;
    Q_INVOKABLE bool     isTearOffMenuVisible()	        const;
    Q_INVOKABLE QAction *menuAction()	                const;
    Q_INVOKABLE void     popup(const QPoint &p, QAction *atAction = 0);
    Q_INVOKABLE void     removeAction(QAction *action);
    Q_INVOKABLE bool     separatorsCollapsible()	const;
    Q_INVOKABLE void     setActiveAction(QAction *act);
    Q_INVOKABLE void     setDefaultAction(QAction *act);
    Q_INVOKABLE void     setIcon(const QIcon &icon);
    Q_INVOKABLE void     setSeparatorsCollapsible(bool collapse);
    Q_INVOKABLE void     setTearOffEnabled(bool);
    Q_INVOKABLE void     setTitle(const QString &title);
    Q_INVOKABLE QString  title()	                const;
    // Q_INVOKABLE HMENU    wceMenu(bool create = false);

    Q_INVOKABLE QString  toString()                     const;

};

#endif
