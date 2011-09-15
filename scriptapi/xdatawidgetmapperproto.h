/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __XWIDGETDATAMAPPERPROTO_H__
#define __XWIDGETDATAMAPPERPROTO_H__

#include <QObject>
#include <QtScript>

class QAbstractItemDelegate;
class XDataWidgetMapper;

Q_DECLARE_METATYPE(XDataWidgetMapper*)

void setupXDataWidgetMapperProto(QScriptEngine *engine);

class XDataWidgetMapperProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    XDataWidgetMapperProto(QObject *parent = 0);

    Q_INVOKABLE void addMapping(QWidget *widget, QString fieldName);
    Q_INVOKABLE void addMapping(QWidget *widget, QString fieldName, const QByteArray &propertyName);
    Q_INVOKABLE void addMapping(QWidget *widget, QString fieldName, const QByteArray &propertyName, const QByteArray &defaultName);
    /* compiler errors (confusion between int and char*)
    Q_INVOKABLE void addMapping(QWidget *widget, int section);
    Q_INVOKABLE void addMapping(QWidget *widget, int section, const QByteArray &propertyName);
    */
    Q_INVOKABLE void                   clearMapping();
    Q_INVOKABLE int                    currentIndex() const;
    Q_INVOKABLE QAbstractItemDelegate *itemDelegate() const;
    Q_INVOKABLE QByteArray             mappedDefaultName(QWidget *widget);
    Q_INVOKABLE QByteArray             mappedPropertyName(QWidget *widget) const;
    Q_INVOKABLE int                    mappedSection(QWidget *widget) const;
    Q_INVOKABLE QWidget               *mappedWidgetAt(int section) const;
    Q_INVOKABLE QAbstractItemModel    *model() const;
    Q_INVOKABLE int         orientation() const;
    Q_INVOKABLE void        removeDefault(QWidget *widget);
    Q_INVOKABLE void        removeMapping(QWidget *widget);
    Q_INVOKABLE QModelIndex rootIndex() const;
    Q_INVOKABLE void        setItemDelegate(QAbstractItemDelegate *delegate);
    Q_INVOKABLE void        setModel(QAbstractItemModel *model);
    Q_INVOKABLE void        setOrientation(int orientation);
    Q_INVOKABLE void        setRootIndex(const QModelIndex &index);
    Q_INVOKABLE void        setSubmitPolicy(int policy);
    Q_INVOKABLE int         submitPolicy() const;

  public slots:
    virtual QString toString() const;
};

#endif
