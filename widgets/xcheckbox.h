/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __XCHECKBOX_H__
#define __XCHECKBOX_H__

#include <QCheckBox>
#include <QPixmap>

#include "widgets.h"
#include "xdatawidgetmapper.h"

class QScriptEngine;

void setupXCheckBox(QScriptEngine *engine);

class XTUPLEWIDGETS_EXPORT XCheckBox : public QCheckBox
{
  Q_OBJECT
  Q_PROPERTY(bool    defaultChecked   READ defaultChecked   WRITE setDefaultChecked)
  Q_PROPERTY(QString fieldName        READ fieldName        WRITE setFieldName)
  Q_PROPERTY(bool    forgetful        READ forgetful        WRITE setForgetful)

  public:
    XCheckBox(QWidget * = 0);
    XCheckBox(const QString &, QWidget * = 0);
    ~XCheckBox();

    virtual bool    defaultChecked() const { return _default; };
    virtual bool    forgetful()      const { return _forgetful; };
    virtual QString fieldName()      const { return _fieldName; };

  public slots:
    virtual void setDataWidgetMap(XDataWidgetMapper* m);
    virtual void setDefaultChecked(bool p)                { _default = p; };
    virtual void setFieldName(QString p)                  { _fieldName = p; };
    virtual void setForgetful(bool p);
    virtual void setData();
    virtual void init();

  protected:
    virtual void showEvent(QShowEvent *);

    XDataWidgetMapper *_mapper;
    
  private:
    virtual void constructor();

    bool    _default;
    bool    _forgetful;
    bool    _initialized;
    QString _settingsName;
    QString _fieldName;

    static QPixmap *_checkedIcon;
};

Q_DECLARE_METATYPE(XCheckBox*)

#endif
