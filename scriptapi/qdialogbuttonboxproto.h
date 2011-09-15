/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QDIALOGBUTTONBOX_H__
#define __QDIALOGBUTTONBOX_H__

#include <QObject>
#include <QtScript>
#include <QDialogButtonBox>

Q_DECLARE_METATYPE(QDialogButtonBox*)
Q_DECLARE_METATYPE(QDialogButtonBox::ButtonLayout)
Q_DECLARE_METATYPE(QDialogButtonBox::ButtonRole)
Q_DECLARE_METATYPE(QDialogButtonBox::StandardButton)

void setupQDialogButtonBoxProto(QScriptEngine *engine);
QScriptValue constructQDialogButtonBox(QScriptContext *context, QScriptEngine *engine);

class QDialogButtonBoxProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QDialogButtonBoxProto(QObject *parent);

    Q_INVOKABLE  void	addButton ( QAbstractButton * button, QDialogButtonBox::ButtonRole role );
    Q_INVOKABLE  QPushButton *	addButton ( const QString & text, QDialogButtonBox::ButtonRole role );
    Q_INVOKABLE  QPushButton *	addButton ( QDialogButtonBox::StandardButton button );
    Q_INVOKABLE  QPushButton *	button ( QDialogButtonBox::StandardButton which ) const;
    Q_INVOKABLE  QDialogButtonBox::ButtonRole	buttonRole ( QAbstractButton * button ) const;
    Q_INVOKABLE  QList<QAbstractButton *>	buttons () const;
    Q_INVOKABLE  void	clear ();
    Q_INVOKABLE  void	removeButton ( QAbstractButton * button );
    Q_INVOKABLE  QDialogButtonBox::StandardButton	standardButton ( QAbstractButton * button ) const;
};

#endif
