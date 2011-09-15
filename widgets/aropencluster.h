/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __AROPENCLUSTER_H__
#define __AROPENCLUSTER_H__

#include "virtualCluster.h"

class XTUPLEWIDGETS_EXPORT AropenLineEdit : public VirtualClusterLineEdit
{
  Q_OBJECT
  Q_ENUMS (DocTypes)
  Q_PROPERTY(DocTypes allowedDocTypes READ allowedDocTypes WRITE setAllowedDocTypes)

  public:
    AropenLineEdit(QWidget *, const char * = 0);

    enum DocType
    {
      AnyType = 0x00,
      CreditMemo = 0x01, DebitMemo   = 0x02,
      Invoice    = 0x04, CashDeposit = 0x08
    };
    Q_DECLARE_FLAGS(DocTypes, DocType)

    Q_INVOKABLE virtual DocTypes allowedDocTypes()  const;
    Q_INVOKABLE virtual void     setExtraClause(const QString &);
    Q_INVOKABLE virtual DocType  type()             const;
    Q_INVOKABLE virtual QString  typeString()       const;

  public slots:
    virtual void setCustId(int pCustid);
    virtual void setAllowedDocTypes(const DocTypes ptypes);

  protected:
    virtual QString	buildExtraClause();

  private:
    DocTypes _allowedTypes;
    QString  _custClause;
    int      _custId;
    QString  _standardExtraClause;
    QString  _typeClause;
    QString  _userExtraClause;

};
Q_DECLARE_OPERATORS_FOR_FLAGS(AropenLineEdit::DocTypes);

class XTUPLEWIDGETS_EXPORT AropenCluster : public VirtualCluster
{
  Q_OBJECT
  Q_ENUMS   (AropenLineEdit::DocTypes)
  Q_PROPERTY(AropenLineEdit::DocTypes allowedDocTypes READ allowedDocTypes WRITE setAllowedDocTypes)

  public:
    AropenCluster(QWidget *, const char * = 0);

    Q_INVOKABLE virtual AropenLineEdit::DocTypes allowedDocTypes()  const;
    Q_INVOKABLE virtual void                     setExtraClause(const QString&);
    Q_INVOKABLE virtual AropenLineEdit::DocType  type()             const;
    Q_INVOKABLE virtual QString                  typeString()       const;

  public slots:
    virtual void setCustId(int pCustid);
    virtual void setAllowedDocTypes(const AropenLineEdit::DocTypes ptypes);

};

#endif
