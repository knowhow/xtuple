/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#ifndef VENDORGROUP_H
#define VENDORGROUP_H

#include "widgets.h"
#include "ui_vendorgroup.h"

class ParameterList;

class XTUPLEWIDGETS_EXPORT VendorGroup : public QWidget, public Ui::VendorGroup
{
  Q_OBJECT

  Q_ENUMS(VendorGroupState)

  Q_PROPERTY(enum VendorGroupState state READ state WRITE setState)

  public:
    VendorGroup(QWidget * = 0, const char * = 0);
    virtual ~VendorGroup();
    virtual void synchronize(VendorGroup*);

    enum VendorGroupState
    {
      All, Selected, SelectedType, TypePattern
    };

    Q_INVOKABLE virtual void           appendValue(ParameterList &);
    Q_INVOKABLE virtual void           bindValue(XSqlQuery &);
    Q_INVOKABLE virtual QString        typePattern() { return _vendorType->text(); };
    enum VendorGroupState  state()      { return (VendorGroupState)_select->currentIndex(); };
    Q_INVOKABLE virtual int            vendId()     { return _vend->id(); };
    Q_INVOKABLE virtual int            vendTypeId() { return _vendorTypes->id(); };

    Q_INVOKABLE inline bool isAll()          { return _select->currentIndex() == All; };
    Q_INVOKABLE inline bool isSelectedVend() { return _select->currentIndex() == Selected; };
    Q_INVOKABLE inline bool isSelectedType() { return _select->currentIndex() == SelectedType; };
    Q_INVOKABLE inline bool isTypePattern() { return _select->currentIndex() == TypePattern; };
    Q_INVOKABLE virtual bool isValid();

  public slots:
    virtual void setVendId(int p);
    virtual void setVendTypeId(int p);
    virtual void setTypePattern(const QString &p);
    virtual void setState(int p) { setState((VendorGroupState)p); }
    virtual void setState(enum VendorGroupState p);

  signals:
    void newTypePattern(QString);
    void newState(int);
    void newVendId(int);
    void newVendTypeId(int);
    void updated();

  protected slots:
    virtual void languageChange();
    virtual void sTypePatternFinished();
};

#endif
