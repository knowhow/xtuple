/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef custCluster_h
#define custCluster_h

#include "parameter.h"
#include "widgets.h"
#include "virtualCluster.h"
#include "crmacctcluster.h"

class QLabel;
class QPushButton;
class QDragEnterEvent;
class QDropEvent;
class QMouseEvent;

class CustInfo;

#define __allCustomers    0x01
#define __activeCustomers 0x02

#define CREDITSTATUS    5
#define CRMACCT_ID      6
#define ISCUSTOMER      7

class XTUPLEWIDGETS_EXPORT CLineEdit : public VirtualClusterLineEdit
{
  Q_OBJECT

  Q_ENUMS(CLineEditTypes)

  Q_PROPERTY(CLineEditTypes type READ type WRITE setType )

  friend class CustInfo;

  public:
    CLineEdit(QWidget * = 0, const char * = 0);

    enum CLineEditTypes
    {
      AllCustomers, 		ActiveCustomers,
      AllProspects,		ActiveProspects,
      AllCustomersAndProspects,	ActiveCustomersAndProspects
    };

    inline CLineEditTypes	type()	    const { return _type;       }

    bool   canOpen();

    bool   canEdit();
    void   setCanEdit(bool);

    bool   editMode();

  public slots:
    bool setEditMode(bool);
    void sNew();
    void setId(const int);
    void setType(CLineEditTypes);

  protected slots:
    VirtualList*    listFactory();
    VirtualSearch*  searchFactory();
    void            sParse();
    void            sUpdateMenu();

  protected:
    QAction* _modeSep;
    QAction* _modeAct;

  signals:
    void newCrmacctId(int);
    void editable(bool);

  private:
    CLineEditTypes	_type;
    CRMAcctLineEdit::CRMAcctSubtype _subtype;
    int                 _crmacctId;
    bool                _canEdit;
    bool                _editMode;
};

class XTUPLEWIDGETS_EXPORT CustCluster : public VirtualCluster
{
  Q_OBJECT

  Q_PROPERTY(bool canEdit READ canEdit WRITE setCanEdit )
  Q_PROPERTY(CLineEdit::CLineEditTypes	type           READ type	  WRITE setType                          )

  public:
    CustCluster(QWidget *parent, const char *name = 0);

    inline CLineEdit::CLineEditTypes type()  const { return static_cast<CLineEdit*>(_number)->type();          };

    Q_INVOKABLE void   setType(CLineEdit::CLineEditTypes);

    bool canEdit() const { return static_cast<CLineEdit*>(_number)->canEdit(); }
    void setCanEdit(bool p) const { static_cast<CLineEdit*>(_number)->setCanEdit(p); }

    Q_INVOKABLE bool editMode() { return static_cast<CLineEdit*>(_number)->editMode(); }
    Q_INVOKABLE bool setEditMode(bool p) const;

  private slots:
   void sHandleEditMode(bool);

  signals:
    void newCrmacctId(int);
    void editable(bool);
    void editingFinished();
};

#endif
