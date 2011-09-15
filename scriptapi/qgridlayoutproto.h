/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QGRIDLAYOUTPROTO_H__
#define __QGRIDLAYOUTPROTO_H__

#include <QGridLayout>
#include <QObject>
#include <QtScript>

class QString;
class QWidget;

Q_DECLARE_METATYPE(QGridLayout*)

void setupQGridLayoutProto(QScriptEngine *engine);
QScriptValue constructQGridLayout(QScriptContext *context, QScriptEngine *engine);

class QGridLayoutProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QGridLayoutProto(QObject *parent);

    Q_INVOKABLE bool  activate();
    Q_INVOKABLE void  addItem(QLayoutItem *item, int row, int column, int rowspan = 1, int columnSpan = 1, Qt::Alignment alignment = 0);
    Q_INVOKABLE void  addLayout(QLayout *layout, int row, int column, Qt::Alignment = 0);
    Q_INVOKABLE void  addLayout(QLayout *layout, int row, int column, int rowSpan, int columnSpan, Qt::Alignment = 0);
    Q_INVOKABLE void  addWidget(QWidget *widget, int row, int column, Qt::Alignment = 0);
    Q_INVOKABLE void  addWidget(QWidget *widget, int fromRow, int fromColumn, int rowSpan, int columnSpan, Qt::Alignment = 0);
    Q_INVOKABLE QRect cellRect(int row, int column)     const;
    Q_INVOKABLE int   columnCount()                     const;
    Q_INVOKABLE int   columnMinimumWidth(int column)    const;
    Q_INVOKABLE int   columnStretch(int column)         const;
    Q_INVOKABLE QRect contentsRect()                    const;
    Q_INVOKABLE int   count()                           const;
    Q_INVOKABLE Qt::Orientations      expandingDirections() const;
    Q_INVOKABLE void getContentsMargins(int *left, int *top, int *right, int *bottom) const;
    Q_INVOKABLE void getItemPosition(int index, int *row, int *coliumn, int *rowSpan, int *columnSpan);
    Q_INVOKABLE int  horizontalSpacing() const;
    Q_INVOKABLE int  indexOf(QWidget *widget) const;
    Q_INVOKABLE void invalidate();
    Q_INVOKABLE bool isEnabled() const;
    Q_INVOKABLE QLayoutItem *itemAt(int index) const;
    Q_INVOKABLE QLayoutItem *itemAtPosition(int row, int column) const;
    Q_INVOKABLE QSize        maximumSize()  const;
    Q_INVOKABLE QWidget     *menuBar()      const;
    Q_INVOKABLE QSize        minimumSize()  const;
    Q_INVOKABLE Qt::Corner   originCorner() const;
    Q_INVOKABLE QWidget     *parentWidget() const;
    Q_INVOKABLE void removeItem(QLayoutItem *item);
    Q_INVOKABLE void removeWidget(QWidget *widget);
    Q_INVOKABLE int  rowCount()                const;
    Q_INVOKABLE int  rowMinimumHeight(int row) const;
    Q_INVOKABLE int  rowStretch(int row)       const;
    Q_INVOKABLE bool setAlignment(QWidget *w, Qt::Alignment alignment);
    Q_INVOKABLE void setAlignment(Qt::Alignment alignment);
    Q_INVOKABLE bool setAlignment(QLayout *l, Qt::Alignment alignment);
    Q_INVOKABLE void setColumnMinimumWidth(int column, int minSize);
    Q_INVOKABLE void setColumnStretch(int column, int stretch);
    Q_INVOKABLE void setContentsMargins(int left, int top, int right, int bottom);
    Q_INVOKABLE void setEnabled(bool enable);
    Q_INVOKABLE void setHorizontalSpacing(int spacing);
    Q_INVOKABLE void setMenuBar(QWidget *widget);
    Q_INVOKABLE void setOriginCorner(Qt::Corner corner);
    Q_INVOKABLE void setRowMinimumHeight(int row, int minSize);
    Q_INVOKABLE void setRowStretch(int row, int stretch);
    Q_INVOKABLE void setSpacing(int spacing);
    Q_INVOKABLE void setVerticalSpacing(int spacing);
    Q_INVOKABLE int  spacing()          const;
    Q_INVOKABLE QLayoutItem *takeAt(int index);
    Q_INVOKABLE void         update();
    Q_INVOKABLE int          verticalSpacing() const;

    //Q_INVOKABLE QString     toString()     const;
};

#endif
