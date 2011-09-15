/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QACTIONPROTO_H__
#define __QACTIONPROTO_H__

#include <QAction>
#include <QKeySequence>
#include <QList>
#include <QObject>
#include <QtScript>

class QActionGroup;
class QGraphicsWidget;
class QString;

Q_DECLARE_METATYPE(QAction*)
Q_DECLARE_METATYPE(QList<QAction*>)

void setupQActionProto(QScriptEngine *engine);
QScriptValue constructQAction(QScriptContext *context, QScriptEngine *engine);

class QActionProto : public QObject, public QScriptable
{
  Q_OBJECT

  Q_PROPERTY(bool     autoRepeat        READ autoRepeat  WRITE setAutoRepeat)
  Q_PROPERTY(bool     checkable         READ isCheckable WRITE setCheckable)
  Q_PROPERTY(bool     checked           READ isChecked   WRITE setChecked)
  Q_PROPERTY(bool     enabled           READ isEnabled   WRITE setEnabled)
  Q_PROPERTY(QFont    font              READ font        WRITE setFont)
  Q_PROPERTY(QIcon    icon              READ icon        WRITE setIcon)
  Q_PROPERTY(QString  iconText          READ iconText    WRITE setIconText)
  Q_PROPERTY(bool iconVisibleInMenu     READ isIconVisibleInMenu WRITE setIconVisibleInMenu)
  Q_PROPERTY(QAction::MenuRole menuRole READ menuRole    WRITE setMenuRole)
  Q_PROPERTY(QKeySequence shortcut      READ shortcut    WRITE setShortcut)
  Q_PROPERTY(Qt::ShortcutContext shortcutContext READ shortcutContext WRITE setShortcutContext)
  Q_PROPERTY(QString  statusTip         READ statusTip   WRITE setStatusTip)
  Q_PROPERTY(QString  text              READ text        WRITE setText)
  Q_PROPERTY(QString  toolTip           READ toolTip     WRITE setToolTip)
  Q_PROPERTY(bool     visible           READ isVisible   WRITE setVisible)
  Q_PROPERTY(QString  whatsThis         READ whatsThis   WRITE setWhatsThis)

  public:
    QActionProto(QObject *parent);

    Q_INVOKABLE QActionGroup       *actionGroup()                    const;
    Q_INVOKABLE void                activate(int event);
    Q_INVOKABLE QList<QGraphicsWidget *> associatedGraphicsWidgets() const;
    Q_INVOKABLE QList<QWidget *>    associatedWidgets()              const;
    Q_INVOKABLE bool                autoRepeat()                     const;
    Q_INVOKABLE QVariant            data()                           const;
    Q_INVOKABLE QFont               font()                           const;
    Q_INVOKABLE QIcon               icon()                           const;
    Q_INVOKABLE QString             iconText()                       const;
    Q_INVOKABLE bool                isCheckable()                    const;
    Q_INVOKABLE bool                isChecked()                      const;
    Q_INVOKABLE bool                isEnabled()                      const;
    Q_INVOKABLE bool                isIconVisibleInMenu()            const;
    Q_INVOKABLE bool                isSeparator()                    const;
    Q_INVOKABLE bool                isVisible()                      const;
    Q_INVOKABLE QMenu              *menu()                           const;
    Q_INVOKABLE QAction::MenuRole   menuRole()                       const;
    Q_INVOKABLE QWidget            *parentWidget()                   const;
    Q_INVOKABLE void                setActionGroup(QActionGroup *group);
    Q_INVOKABLE void                setAutoRepeat(bool);
    Q_INVOKABLE void                setCheckable(bool);
    Q_INVOKABLE void                setChecked(bool);
    Q_INVOKABLE void                setData(const QVariant &userData);
    Q_INVOKABLE void                setEnabled(bool);
    Q_INVOKABLE void                setFont(const QFont &font);
    Q_INVOKABLE void                setIcon(const QIcon &icon);
    Q_INVOKABLE void                setIconText(const QString &text);
    Q_INVOKABLE void                setIconVisibleInMenu(bool visible);
    Q_INVOKABLE void                setMenu(QMenu *menu);
    Q_INVOKABLE void                setMenuRole(QAction::MenuRole menuRole);
    Q_INVOKABLE void                setSeparator(bool b);
    Q_INVOKABLE void                setShortcut(const QKeySequence &shortcut);
    Q_INVOKABLE void                setShortcutContext(Qt::ShortcutContext context);
    Q_INVOKABLE void                setShortcuts(const QList<QKeySequence> &shortcuts);
    Q_INVOKABLE void                setShortcuts(int key);
    Q_INVOKABLE void                setStatusTip(const QString &statusTip);
    Q_INVOKABLE void                setText(const QString &text);
    Q_INVOKABLE void                setToolTip(const QString &tip);
    Q_INVOKABLE void                setVisible(bool);
    Q_INVOKABLE void                setWhatsThis(const QString &what);

    Q_INVOKABLE QKeySequence        shortcut()          const;
    Q_INVOKABLE Qt::ShortcutContext shortcutContext()   const;
    Q_INVOKABLE QList<QKeySequence> shortcuts()         const;
    Q_INVOKABLE bool                showStatusText(QWidget *widget = 0);
    Q_INVOKABLE QString             statusTip()         const;
    Q_INVOKABLE QString             text()              const;
    Q_INVOKABLE QString             toolTip()           const;
    Q_INVOKABLE QString             whatsThis()         const;

    Q_INVOKABLE QString             toString()          const;

  //signals:
  //  void changed();
  //  void hovered();
  //  void toggled(bool checked);
  //  void triggered(bool checked = false);
};

#endif
