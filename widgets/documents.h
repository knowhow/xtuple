/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef documents_h
#define documents_h

#include <QWidget>

#include <xsqlquery.h>

#include "widgets.h"
#include "guiclientinterface.h"

#include "ui_documents.h"

#define cNew                  1
#define cEdit                 2
#define cView                 3

class XTUPLEWIDGETS_EXPORT Documents : public QWidget, public Ui::documents
{
  Q_OBJECT

  Q_ENUMS(DocumentSources)

  Q_PROPERTY(DocumentSources type READ type WRITE setType)

  friend class image;
  friend class file;

  public:
    Documents(QWidget *);

    // if you add to this then add to the _documentMap[] below
    enum DocumentSources
    {
      Uninitialized,
      Address,          BBOMHead,           BBOMItem,
      BOMHead,          BOMItem,            BOOHead,
      BOOItem,          CRMAccount,         Contact, 
      Customer,         Employee,           Incident,   
      Item,             ItemSite,           ItemSource,
      Location,		LotSerial,          Opportunity,
      Project,		PurchaseOrder,      PurchaseOrderItem,
      ReturnAuth,       ReturnAuthItem,     Quote, 
      QuoteItem,        SalesOrder,         SalesOrderItem,
      Todo,             TransferOrder,      TransferOrderItem,
      Vendor,           Warehouse,          WorkOrder
    };

    static GuiClientInterface *_guiClientInterface;

    inline int  sourceid()             { return _sourceid; }
    inline enum DocumentSources type() { return _source;   }

    struct DocumentMap
    {
      enum DocumentSources source;
      const char *        ident;

      DocumentMap(enum DocumentSources s, const char * i)
      {
        source = s;
        ident = i;
      }
    };
    static const struct DocumentMap _documentMap[]; // see Documents.cpp for init

  public slots:
    void setType(enum DocumentSources);
    void setId(int);
    void setReadOnly(bool);
    void sNewDoc(QString type, QString ui);
    void sNewImage();
    void sNewToDo();
    void sNewIncdt();
    void sNewOpp();
    void sNewProj();
    void sInsertDocass(QString, int);
    void sAttachDoc();
    void sViewDoc();
    void sEditDoc();
    void sOpenDoc(QString mode = "edit");
    void sDetachDoc();
    
    void refresh();

  private slots:
    void handleSelection(bool = false);
    void handleItemSelected();

  private:
    enum DocumentSources _source;
    int                  _sourceid;
    bool                 _readOnly;

};

#endif
