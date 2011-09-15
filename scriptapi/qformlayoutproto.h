/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QFORMLAYOUTPROTO_H__
#define __QFORMLAYOUTPROTO_H__

#include <QFormLayout>
#include <QObject>
#include <QtScript>

class QString;
class QWidget;

Q_DECLARE_METATYPE(QFormLayout*)
Q_DECLARE_METATYPE(enum QFormLayout::FieldGrowthPolicy)
Q_DECLARE_METATYPE(enum QFormLayout::ItemRole)
Q_DECLARE_METATYPE(enum QFormLayout::RowWrapPolicy)

void setupQFormLayoutProto(QScriptEngine *engine);
QScriptValue constructQFormLayout(QScriptContext *context, QScriptEngine *engine);

class QFormLayoutProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QFormLayoutProto(QObject *parent);

    Q_INVOKABLE bool  activate();
    Q_INVOKABLE void  addItem(QLayoutItem *item);
    Q_INVOKABLE void  addRow(QWidget *label, QWidget *field);
    Q_INVOKABLE void  addRow(QWidget *label, QLayout *field);
    Q_INVOKABLE void  addRow(const QString &labelText, QWidget *field);
    Q_INVOKABLE void  addRow(const QString &labelText, QLayout *field);
    Q_INVOKABLE void  addRow(QWidget *widget);
    Q_INVOKABLE void  addRow(QLayout *layout);
    Q_INVOKABLE void  addWidget(QWidget *widget);
    Q_INVOKABLE QRect contentsRect()                                 const;
    Q_INVOKABLE int   count()                                        const;
    Q_INVOKABLE Qt::Orientations               expandingDirections() const;
    Q_INVOKABLE QFormLayout::FieldGrowthPolicy fieldGrowthPolicy()   const;
    Q_INVOKABLE Qt::Alignment                  formAlignment()       const;
    Q_INVOKABLE void getContentsMargins(int *left, int *top, int *right, int *bottom) const;
    Q_INVOKABLE void getItemPosition(int index, int *rowPtr, QFormLayout::ItemRole *rolePtr) const;
    Q_INVOKABLE void getLayoutPosition(QLayout *layout, int *rowPtr, QFormLayout::ItemRole *rolePtr) const;
    Q_INVOKABLE void getWidgetPosition(QWidget *widget, int *rowPtr, QFormLayout::ItemRole *rolePtr) const;
    Q_INVOKABLE int  horizontalSpacing()      const;
    Q_INVOKABLE int  indexOf(QWidget *widget) const;
    Q_INVOKABLE void insertRow(int row, QWidget *label, QWidget *field);
    Q_INVOKABLE void insertRow(int row, QWidget *label, QLayout *field);
    Q_INVOKABLE void insertRow(int row, const QString &labelText, QWidget *field);
    Q_INVOKABLE void insertRow(int row, const QString &labelText, QLayout *field);
    Q_INVOKABLE void insertRow(int row, QWidget *widget);
    Q_INVOKABLE void insertRow(int row, QLayout *layout);
    Q_INVOKABLE void invalidate();
    Q_INVOKABLE bool isEmpty()   const;
    Q_INVOKABLE bool isEnabled() const;
    Q_INVOKABLE QLayoutItem  *itemAt(int index, QFormLayout::ItemRole role) const;
    Q_INVOKABLE QLayoutItem  *itemAt(int index)             const;
    Q_INVOKABLE Qt::Alignment labelAlignment()              const;
    Q_INVOKABLE QWidget      *labelForField(QWidget *field) const;
    Q_INVOKABLE QWidget      *labelForField(QLayout *field) const;
    Q_INVOKABLE QSize         maximumSize()                 const;
    Q_INVOKABLE QWidget      *menuBar()                     const;
    Q_INVOKABLE QSize         minimumSize()                 const;
    Q_INVOKABLE QWidget      *parentWidget()                const;
    Q_INVOKABLE void          removeItem(QLayoutItem *item);
    Q_INVOKABLE void          removeWidget(QWidget *widget);
    Q_INVOKABLE int           rowCount()                    const;
    Q_INVOKABLE QFormLayout::RowWrapPolicy rowWrapPolicy()  const;
    Q_INVOKABLE bool setAlignment(QWidget *w, Qt::Alignment alignment);
    Q_INVOKABLE void setAlignment(Qt::Alignment alignment);
    Q_INVOKABLE bool setAlignment(QLayout *l, Qt::Alignment alignment);
    Q_INVOKABLE void setContentsMargins(int left, int top, int right, int bottom);
    Q_INVOKABLE void setEnabled(bool enable);
    Q_INVOKABLE void setFieldGrowthPolicy(QFormLayout::FieldGrowthPolicy policy);
    Q_INVOKABLE void setFormAlignment(Qt::Alignment alignment);
    Q_INVOKABLE void setHorizontalSpacing(int spacing);
    Q_INVOKABLE void setItem(int row, QFormLayout::ItemRole role, QLayoutItem *item);
    Q_INVOKABLE void setLabelAlignment(Qt::Alignment alignment);
    Q_INVOKABLE void setLayout(int row, QFormLayout::ItemRole role, QLayout *layout);
    Q_INVOKABLE void setMenuBar(QWidget *widget);
    Q_INVOKABLE void setRowWrapPolicy(QFormLayout::RowWrapPolicy policy);
    Q_INVOKABLE void setSpacing(int spacing);
    Q_INVOKABLE void setVerticalSpacing(int spacing);
    Q_INVOKABLE void setWidget(int row, QFormLayout::ItemRole role, QWidget *widget);
    Q_INVOKABLE int          spacing()         const;
    Q_INVOKABLE QLayoutItem *takeAt(int index);
    Q_INVOKABLE void         update();
    Q_INVOKABLE int          verticalSpacing() const;

    Q_INVOKABLE QString     toString()         const;
};

#endif
