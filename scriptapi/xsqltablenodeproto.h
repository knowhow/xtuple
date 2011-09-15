/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __XSQLTABLENODEPROTO_H__
#define __XSQLTABLENODEPROTO_H__

#include <QMap>
#include <QModelIndex>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlIndex>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QtScript>

#include "xsqltablemodel.h"

class QMimeData;

Q_DECLARE_METATYPE(XSqlTableNode*)

void setupXSqlTableNodeProto(QScriptEngine *engine);
QScriptValue constructXSqlTableNode(QScriptContext *context, QScriptEngine *engine);

class XSqlTableNodeProto : public QObject, public QScriptable
{
  Q_OBJECT

public:
  XSqlTableNodeProto(QObject *parent = 0);
  virtual ~XSqlTableNodeProto();

  Q_INVOKABLE int index() const;
  Q_INVOKABLE int count() const;
  Q_INVOKABLE void appendChild(XSqlTableNode *child);
  Q_INVOKABLE void removeChild(int index);
  Q_INVOKABLE XSqlTableNode* appendChild(const QString &tableName, ParameterList &relations);

  Q_INVOKABLE QMap<QPair<XSqlTableModel*, int>, XSqlTableModel* > modelMap();
  Q_INVOKABLE QString tableName();
  Q_INVOKABLE QList<XSqlTableNode *> children();
  Q_INVOKABLE ParameterList relations();
  Q_INVOKABLE XSqlTableNode* child(int index);
  Q_INVOKABLE XSqlTableNode* child(const QString &tableName);
  Q_INVOKABLE XSqlTableNode* parent() const;
  Q_INVOKABLE XSqlTableModel* model(XSqlTableModel* parent = 0, int row = 0);

  Q_INVOKABLE void clear();
  Q_INVOKABLE void load(QPair<XSqlTableModel*, int> key);
  Q_INVOKABLE bool save();

};

#endif
