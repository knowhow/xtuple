/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QBOXLAYOUTPROTO_H__
#define __QBOXLAYOUTPROTO_H__

#include <QBoxLayout>
#include <QObject>
#include <QtScript>

class QString;

Q_DECLARE_METATYPE(QBoxLayout*)

void setupQBoxLayoutProto(QScriptEngine *engine);
QScriptValue constructQBoxLayout(QScriptContext *context, QScriptEngine *engine);

class QBoxLayoutProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QBoxLayoutProto(QObject *parent);

    Q_INVOKABLE bool activate();
    Q_INVOKABLE void addItem(QLayoutItem *item);
    Q_INVOKABLE void addLayout(QLayout *layout, int stretch = 0);
    Q_INVOKABLE void addSpacerItem(QSpacerItem *spacerItem);
    Q_INVOKABLE void addSpacing(int size);
    Q_INVOKABLE void addStretch(int stretch = 0);
    Q_INVOKABLE void addStrut(int size);
    Q_INVOKABLE void addWidget(QWidget *widget, int stretch = 0, Qt::Alignment alignment = 0);
    Q_INVOKABLE QRect                 contentsRect() const;
    Q_INVOKABLE int                   count() const;
    Q_INVOKABLE QBoxLayout::Direction direction() const;
    Q_INVOKABLE Qt::Orientations      expandingDirections() const;
    Q_INVOKABLE void getContentsMargins(int *left, int *top, int *right, int *bottom) const;
    Q_INVOKABLE int  indexOf(QWidget *widget) const;
    Q_INVOKABLE void insertLayout(int index, QLayout *layout, int stretch = 0);
    Q_INVOKABLE void insertSpacerItem(int index, QSpacerItem *spacerItem);
    Q_INVOKABLE void insertSpacing(int index, int size);
    Q_INVOKABLE void insertStretch(int index, int stretch = 0);
    Q_INVOKABLE void insertWidget(int index, QWidget *widget, int stretch = 0, Qt::Alignment alignment = 0);
    Q_INVOKABLE void invalidate();
    Q_INVOKABLE bool isEnabled() const;
    Q_INVOKABLE QLayoutItem *itemAt(int index) const;
    Q_INVOKABLE QSize        maximumSize() const;
    Q_INVOKABLE QWidget     *menuBar() const;
    Q_INVOKABLE QSize        minimumSize() const;
    Q_INVOKABLE QWidget     *parentWidget() const;
    Q_INVOKABLE void removeItem(QLayoutItem *item);
    Q_INVOKABLE void removeWidget(QWidget *widget);
    Q_INVOKABLE void setAlignment(Qt::Alignment alignment);
    Q_INVOKABLE void setContentsMargins(int left, int top, int right, int bottom);
    //Q_INVOKABLE void setDirection(QBoxLayout::Direction direction);
    Q_INVOKABLE void setDirection(int direction);
    Q_INVOKABLE void setEnabled(bool enable);
    Q_INVOKABLE void setMenuBar(QWidget *widget);
    Q_INVOKABLE void setStretch(int index, int stretch);
    Q_INVOKABLE bool setAlignment(QLayout *l, Qt::Alignment alignment);
    Q_INVOKABLE bool setAlignment(QWidget *w, Qt::Alignment alignment);
    Q_INVOKABLE bool setStretchFactor(QLayout *layout, int stretch);
    Q_INVOKABLE bool setStretchFactor(QWidget *widget, int stretch);
    Q_INVOKABLE int  stretch(int index) const;
    Q_INVOKABLE QLayoutItem *takeAt(int index);
    Q_INVOKABLE void         update();

    //Q_INVOKABLE QString     toString()     const;
};

#endif
