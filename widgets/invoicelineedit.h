/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef invoiceLineEdit_h
#define invoiceLineEdit_h

#include "xlineedit.h"

#include "widgets.h"

class XTUPLEWIDGETS_EXPORT InvoiceLineEdit : public XLineEdit
{
  Q_OBJECT

  public:
    InvoiceLineEdit(QWidget *, const char * = 0);

  inline int  custid()        { return _custid;        }
  inline int  coheadid()      { return _coheadid;      }
  inline QString invoiceNumber() { return _invoiceNumber; }

  public slots:
    void setInvoiceNumber(QString);
    void sParse();
    virtual void setId(const int);

  signals:
    void newCustid(int);
    void newCoheadid(int);
    void newInvoiceNumber(QString);
    void newId(int);
    void valid(bool);

  private:
    int  _custid;
    int  _coheadid;
    QString _invoiceNumber;
};

#endif
