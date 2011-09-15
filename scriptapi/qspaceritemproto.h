/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QSPACERITEMPROTO_H__
#define __QSPACERITEMPROTO_H__

#include <QSpacerItem>
#include <QObject>
#include <QtScript>

class QString;

Q_DECLARE_METATYPE(QSpacerItem*)

void setupQSpacerItem(QScriptEngine *engine);
QScriptValue constructQSpacerItem(QScriptContext *context, QScriptEngine *engine);

class QSpacerItemProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QSpacerItemProto(QObject *parent);

    Q_INVOKABLE Qt::Alignment    alignment() const;
    Q_INVOKABLE void             changeSize(int w, int h, QSizePolicy::Policy hPolicy = QSizePolicy::Minimum, QSizePolicy::Policy vPolicy = QSizePolicy::Minimum);
    QSizePolicy::ControlTypes    controlTypes ()                const;
    Q_INVOKABLE Qt::Orientations expandingDirections()          const;
    Q_INVOKABLE QRect            geometry()                     const;
    Q_INVOKABLE bool             hasHeightForWidth()            const;
    Q_INVOKABLE int              heightForWidth(int w)          const;
    Q_INVOKABLE void             invalidate();
    Q_INVOKABLE bool             isEmpty()                      const;
    Q_INVOKABLE QLayout         *layout();
    Q_INVOKABLE QSize            maximumSize()                  const;
    Q_INVOKABLE int              minimumHeightForWidth(int w)   const;
    Q_INVOKABLE QSize            minimumSize()                  const;
    Q_INVOKABLE void             setAlignment(Qt::Alignment alignment);
    Q_INVOKABLE void             setGeometry(const QRect &r);
    Q_INVOKABLE QSize            sizeHint()                     const;
    Q_INVOKABLE QSpacerItem     *spacerItem();
    Q_INVOKABLE QWidget         *widget();
};

#endif
