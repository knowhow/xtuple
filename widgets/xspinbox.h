/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __XSPINBOX_H__
#define __XSPINBOX_H__

#include <QSpinBox>

#include "widgets.h"
#include "xdatawidgetmapper.h"

class XTUPLEWIDGETS_EXPORT XSpinBox : public QSpinBox
{
  Q_OBJECT
  Q_PROPERTY(QString fieldName   READ fieldName   WRITE setFieldName)
  Q_PROPERTY(QString defaultValue  READ defaultValue WRITE setDefaultValue)

  public:
    XSpinBox(QWidget * = 0);
    
    virtual QString defaultValue() const { return _default; };
    virtual QString fieldName()   const { return _fieldName; };

  public slots:
    virtual void setDataWidgetMap(XDataWidgetMapper* m);
    virtual void setDefaultValue(QString p)  { _default = p; };
    virtual void setFieldName(QString p) { _fieldName = p; };

  private:
    QString _default;
    QString _fieldName;};

#endif
