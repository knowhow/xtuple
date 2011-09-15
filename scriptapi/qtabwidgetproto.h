/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QTABWIDGETPROTO_H__
#define __QTABWIDGETPROTO_H__

#include <QTabWidget>
#include <QObject>
#include <QtScript>

class QString;

Q_DECLARE_METATYPE(QTabWidget*)
//Q_DECLARE_METATYPE(QTabWidget)

void setupQTabWidgetProto(QScriptEngine *engine);
void TabPositionScriptValue(const QScriptValue &obj, enum QTabWidget::TabPosition &p);
void TabShapeScriptValue(const QScriptValue &obj, enum QTabWidget::TabShape &p);
QScriptValue constructQTabWidget(QScriptContext *context, QScriptEngine *engine);
QScriptValue TabPositionScriptValue(QScriptEngine *engine, const enum QTabWidget::TabPosition &p);
QScriptValue TabShapeScriptValue(QScriptEngine *engine, const enum QTabWidget::TabShape &p);

class QTabWidgetProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QTabWidgetProto(QObject *parent);

    Q_INVOKABLE int      addTab(QWidget *page, const QString &label);
    Q_INVOKABLE int      addTab(QWidget *page, const QIcon &icon, const QString &label);
    Q_INVOKABLE void     clear();
    Q_INVOKABLE QWidget *cornerWidget(int corner = 1) const;
    Q_INVOKABLE int      count() const;
    Q_INVOKABLE int      currentIndex() const;
    Q_INVOKABLE QWidget *currentWidget() const;
#if QT_VERSION >= 0x040500
    Q_INVOKABLE bool     documentMode() const;
#endif
    Q_INVOKABLE int      elideMode() const;
    Q_INVOKABLE QSize    iconSize() const;
    Q_INVOKABLE int      indexOf(QWidget *w) const;
    Q_INVOKABLE int      insertTab(int index, QWidget *page, const QString &label);
    Q_INVOKABLE int      insertTab(int index, QWidget *page, const QIcon &icon, const QString & label);
#if QT_VERSION >= 0x040500
    Q_INVOKABLE bool     isMovable() const;
#endif
    Q_INVOKABLE bool     isTabEnabled(int index) const;
    Q_INVOKABLE void     removeTab(int index);
    Q_INVOKABLE void     setCornerWidget(QWidget *widget, int corner = 1);
#if QT_VERSION >= 0x040500
    Q_INVOKABLE void     setDocumentMode(bool set);
#endif
    Q_INVOKABLE void     setElideMode(int);
    Q_INVOKABLE void     setIconSize(const QSize &size);
#if QT_VERSION >= 0x040500
    Q_INVOKABLE void     setMovable(bool movable);
#endif
    Q_INVOKABLE void     setTabEnabled(int index, bool enable);
    Q_INVOKABLE void     setTabIcon(int index, const QIcon &icon);
    Q_INVOKABLE void     setTabPosition(int position);
    Q_INVOKABLE void     setTabShape(int s);
    Q_INVOKABLE void     setTabText(int index, const QString &label);
    Q_INVOKABLE void     setTabToolTip(int index, const QString &tip);
    Q_INVOKABLE void     setTabWhatsThis(int index, const QString &text);
#if QT_VERSION >= 0x040500
    Q_INVOKABLE void     setTabsClosable(bool closeable);
#endif
    Q_INVOKABLE void     setUsesScrollButtons(bool useButtons);
    Q_INVOKABLE QIcon    tabIcon(int index) const;
    Q_INVOKABLE int      tabPosition() const;
    Q_INVOKABLE int      tabShape() const;
    Q_INVOKABLE QString  tabText(int index) const;
    Q_INVOKABLE QString  tabToolTip(int index) const;
    Q_INVOKABLE QString  tabWhatsThis(int index) const;
#if QT_VERSION >= 0x040500
    Q_INVOKABLE bool     tabsClosable() const;
#endif
    Q_INVOKABLE bool     usesScrollButtons() const;
    Q_INVOKABLE QWidget *widget(int index) const;

};

#endif
