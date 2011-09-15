/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __XTREEWIDGETITEMPROTO_H__
#define __XTREEWIDGETITEMPROTO_H__

#include <QObject>
#include <QtScript>

class XTreeWidgetItem;

Q_DECLARE_METATYPE(XTreeWidgetItem*)

void setupXTreeWidgetItemProto(QScriptEngine *engine);

class XTreeWidgetItemProto : public QObject, public QScriptable
{
  Q_OBJECT

  Q_PROPERTY(int id    READ id    WRITE setId    DESIGNABLE false)
  Q_PROPERTY(int altId READ altId WRITE setAltId DESIGNABLE false)

  public:
    XTreeWidgetItemProto(QObject *parent = 0);

    int  altId() const;
    int  id()    const;
    void setAltId(const int pId);
    void setId(const int pId);

    Q_INVOKABLE virtual XTreeWidgetItem *child(int idx) const;
    Q_INVOKABLE virtual int              id(const QString);
    Q_INVOKABLE virtual QVariant         rawValue(const QString);
    Q_INVOKABLE virtual void             setText(int, const QVariant &);
    Q_INVOKABLE virtual void             setTextColor(int column, const QColor & color);
    Q_INVOKABLE virtual void             setTextColor(const QColor &);
    Q_INVOKABLE virtual QString          text(int p)           const;
    Q_INVOKABLE virtual QString          text(const QString &) const;

  public slots:
    virtual QString toString() const;
};

#endif
