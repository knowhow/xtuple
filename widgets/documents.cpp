/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QDesktopServices>
#include <QMessageBox>
#include <QDebug>
#include <QDialog>
#include <QUrl>
#include <QMenu>
#include <QFileInfo>
#include <QDir>
#include <QSettings>

#include <openreports.h>
#include <parameter.h>
#include <xsqlquery.h>
#include <metasql.h>
#include "mqlutil.h"

#include "documents.h"
#include "imageview.h"
#include "imageAssignment.h"
#include "docAttach.h"

// CAUTION: This will break if the order of this list does not match
//          the order of the enumerated values as defined.
const Documents::DocumentMap Documents::_documentMap[] =
{
  DocumentMap( Uninitialized,     " "   ),
  DocumentMap( Address,           "ADDR"),
  DocumentMap( BBOMHead,          "BBH" ),
  DocumentMap( BBOMItem,          "BBI" ),
  DocumentMap( BOMHead,           "BMH" ),
  DocumentMap( BOMItem,           "BMI" ),
  DocumentMap( BOOHead,           "BOH" ),
  DocumentMap( BOOItem,           "BOI" ),
  DocumentMap( CRMAccount,        "CRMA"),
  DocumentMap( Contact,           "T"   ),
  DocumentMap( Customer,          "C"   ),
  DocumentMap( Employee,          "EMP" ),
  DocumentMap( Incident,          "INCDT"),
  DocumentMap( Item,              "I"   ),
  DocumentMap( ItemSite,          "IS"  ),
  DocumentMap( ItemSource,        "IR"  ),
  DocumentMap( Location,          "L"   ),
  DocumentMap( LotSerial,         "LS"   ),
  DocumentMap( Opportunity,       "OPP" ),
  DocumentMap( Project,           "J"   ),
  DocumentMap( PurchaseOrder,     "P"   ),
  DocumentMap( PurchaseOrderItem, "PI"  ),
  DocumentMap( ReturnAuth,        "RA"  ),
  DocumentMap( ReturnAuthItem,    "RI"  ),
  DocumentMap( Quote,             "Q"   ),
  DocumentMap( QuoteItem,         "QI"  ),
  DocumentMap( SalesOrder,        "S"   ),
  DocumentMap( SalesOrderItem,    "SI"  ),
  DocumentMap( Todo,              "TODO"),
  DocumentMap( TransferOrder,     "TO"  ),
  DocumentMap( TransferOrderItem, "TI"  ),
  DocumentMap( Vendor,            "V"   ),
  DocumentMap( Warehouse,         "WH"  ),
  DocumentMap( WorkOrder,         "W"   ),
};

GuiClientInterface* Documents::_guiClientInterface = 0;

Documents::Documents(QWidget *pParent) :
  QWidget(pParent)
{
  setupUi(this);
  
  _source = Uninitialized;
  _sourceid = -1;
  _readOnly = false;

  _doc->addColumn(tr("Type"),  _itemColumn,  Qt::AlignLeft, true, "target_type" );
  _doc->addColumn(tr("Number"), _itemColumn, Qt::AlignLeft, true, "target_number" );
  _doc->addColumn(tr("Name"), -1,  Qt::AlignLeft, true, "name" );
  _doc->addColumn(tr("Description"),  -1, Qt::AlignLeft, true, "description");
  _doc->addColumn(tr("Relationship"),  _itemColumn, Qt::AlignLeft,true, "purpose");
  
  connect(_attachDoc, SIGNAL(clicked()), this, SLOT(sAttachDoc()));
  connect(_editDoc, SIGNAL(clicked()), this, SLOT(sEditDoc()));
  connect(_viewDoc, SIGNAL(clicked()), this, SLOT(sViewDoc()));
  connect(_detachDoc, SIGNAL(clicked()), this, SLOT(sDetachDoc()));
  connect(_doc, SIGNAL(valid(bool)), this, SLOT(handleSelection()));
  connect(_doc, SIGNAL(itemSelected(int)), this, SLOT(handleItemSelected()));
  handleSelection();

  if (_x_privileges)
  {
    QMenu * newDocMenu = new QMenu;

    QAction* imgAct = new QAction(tr("Image"), this);
    imgAct->setEnabled(_x_privileges->check("MaintainImages"));
    connect(imgAct, SIGNAL(triggered()), this, SLOT(sNewImage()));
    newDocMenu->addAction(imgAct);

    QAction* incdtAct = new QAction(tr("Incident"), this);
    incdtAct->setEnabled(_x_privileges->check("MaintainPersonalIncidents") ||
                         _x_privileges->check("MaintainAllIncidents"));
    connect(incdtAct, SIGNAL(triggered()), this, SLOT(sNewIncdt()));
    newDocMenu->addAction(incdtAct);

    QAction* todoAct = new QAction(tr("To Do"), this);
    todoAct->setEnabled(_x_privileges->check("MaintainPersonalToDoItems") ||
                        _x_privileges->check("MaintainAllToDoItems"));
    connect(todoAct, SIGNAL(triggered()), this, SLOT(sNewToDo()));
    newDocMenu->addAction(todoAct);

    QAction* oppAct = new QAction(tr("Opportunity"), this);
    oppAct->setEnabled(_x_privileges->check("MaintainPersonalOpportunities") ||
                       _x_privileges->check("MaintainAllOpportunities"));
    connect(oppAct, SIGNAL(triggered()), this, SLOT(sNewOpp()));
    newDocMenu->addAction(oppAct);

    QAction* projAct = new QAction(tr("Project"), this);
    projAct->setEnabled(_x_privileges->check("MaintainPersonalProjects") ||
                        _x_privileges->check("MaintainAllProjects"));
    connect(projAct, SIGNAL(triggered()), this, SLOT(sNewProj()));
    newDocMenu->addAction(projAct);

    _newDoc->setMenu(newDocMenu);
  }
}

void Documents::setType(enum DocumentSources pSource)
{
  _source = pSource;
}

void Documents::setId(int pSourceid)
{
  _sourceid = pSourceid;
  refresh();
}

void Documents::setReadOnly(bool pReadOnly)
{
  _readOnly = pReadOnly;

  _newDoc->setEnabled(!pReadOnly);
  _attachDoc->setEnabled(!pReadOnly);
  _editDoc->setEnabled(!pReadOnly);
  _detachDoc->setEnabled(!pReadOnly);

  handleSelection();
}

void Documents::sNewDoc(QString type, QString ui)
{
  ParameterList params;
  params.append("mode", "new");
  int target_id;
  QDialog* newdlg = qobject_cast<QDialog*>(_guiClientInterface->openWindow(ui, params, parentWidget(),Qt::WindowModal, Qt::Dialog));
  target_id = newdlg->exec();
  if (target_id != QDialog::Rejected)
    sInsertDocass(type, target_id);
  refresh();
}

void Documents::sNewImage()
{
  ParameterList params;
  params.append("sourceType", _source);
  params.append("source_id", _sourceid);

  imageAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    refresh();
}

void Documents::sNewIncdt()
{
  sNewDoc("INCDT","incident");
}

void Documents::sNewToDo()
{
  sNewDoc("TODO", "todoItem");
}

void Documents::sNewProj()
{
  sNewDoc("J", "project");
}

void Documents::sNewOpp()
{
  sNewDoc("OPP","opportunity");
}

void Documents::sInsertDocass(QString target_type, int target_id)
{
  XSqlQuery ins;
  ins.prepare("INSERT INTO docass ( docass_source_id, docass_source_type, docass_target_id, docass_target_type )"
            "  VALUES ( :sourceid, :sourcetype::text, :targetid, :targettype); ");
  ins.bindValue(":sourceid", _sourceid);
  ins.bindValue(":sourcetype", _documentMap[_source].ident);
  ins.bindValue(":targetid", target_id);
  ins.bindValue(":targettype", target_type);
  ins.exec();
}

void Documents::sEditDoc()
{
  sOpenDoc("edit");
}

void Documents::sOpenDoc(QString mode)
{
  QString ui;
  QString docType = _doc->currentItem()->rawValue("target_type").toString();
  int targetid = _doc->currentItem()->id("target_number");
  ParameterList params;
  if (docType == "Q" && mode == "view")
    params.append("mode", "viewQuote");
  else if (docType == "Q" && mode == "edit")
    params.append("mode", "editQuote");
  else
    params.append("mode", mode);

  //image -- In the future this needs to be changed to use docass instead of imageass
  if (docType == "IMG")
  {
    XSqlQuery img;
    img.prepare("SELECT imageass_image_id "
                "FROM imageass "
                "WHERE (imageass_id=:imageass_id); ");
    img.bindValue(":imageass_id", _doc->id());
    img.exec();
    img.first();
    params.append("image_id", img.value("imageass_image_id").toInt());
    imageview newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.exec() != QDialog::Rejected)
      refresh();
    return;
  }
  //url -- In the future this needs to be changed to use docass instead of url
  else if (docType == "URL" || docType == "FILE")
  {
    if (mode == "edit")
    {
      ParameterList params;
      params.append("url_id", targetid);

      docAttach newdlg(this, "", TRUE);
      newdlg.set(params);
      newdlg.exec();

      refresh();
      return;
    }

    XSqlQuery qfile;
    qfile.prepare("SELECT url_id, url_source_id, url_source, url_title, url_url, url_stream"
                  " FROM url"
                  " WHERE (url_id=:url_id);");

    qfile.bindValue(":url_id", _doc->id());
    qfile.exec();

    // If file is in the database, copy to a temp. directory in the file system and open it.
    if (qfile.first() && (docType == "FILE"))
    {
      QFileInfo fi( qfile.value("url_url").toString() );
      QDir tdir;
      QString fileName = fi.fileName();
      QString filePath = tdir.tempPath() + "/xtTempDoc/" + qfile.value("url_id").toString() + "/";
      QFile tfile(filePath + fileName);

      // Remove any previous watches
      if (_guiClientInterface)
        _guiClientInterface->removeDocumentWatch(tfile.fileName());

      if (!tdir.exists(filePath))
        tdir.mkpath(filePath);

      if (!tfile.open(QIODevice::WriteOnly))
      {
        QMessageBox::warning( this, tr("File Open Error"),
                             tr("Could Not Create File %1.").arg(tfile.fileName()) );
        return;
      }
      tfile.write(qfile.value("url_stream").toByteArray());
      QUrl urldb;
      urldb.setUrl(tfile.fileName());
#ifndef Q_WS_WIN
      urldb.setScheme("file");
#endif
      tfile.close();
      QDesktopServices::openUrl(urldb);

      // Add a watch to the file that will save any changes made to the file back to the database.
      if (_guiClientInterface && !_readOnly) // TODO: only if NOT read-only
        _guiClientInterface->addDocumentWatch(tfile.fileName(),qfile.value("url_id").toInt());
      return;
    }
    else
    {
     //If url scheme is missing, we'll assume it is "file" for now.
      QUrl url(_doc->currentItem()->rawValue("description").toString());
      if (url.scheme().isEmpty())
        url.setScheme("file");
      QDesktopServices::openUrl(url);
      return;
    }
  }
  else if (docType == "INCDT")
  {
    params.append("incdt_id", targetid);
    ui = "incident";
  }
  else if (docType == "TODO")
  {
    params.append("todoitem_id", targetid);
    ui = "todoItem";
  }
  else if (docType == "OPP")
  {
    params.append("ophead_id", targetid);
    ui = "opportunity";
  }
  else if (docType == "J")
  {
    params.append("prj_id", targetid);
    ui = "project";
  }
  else if (docType == "CRMA")
  {
    params.append("crmacct_id", targetid);
    ui = "crmaccount";
  }
  else if (docType == "T")
  {
    params.append("cntct_id", targetid);
    ui = "contact";
  }
  else if (docType == "V")
  {
    params.append("vend_id", targetid);
    ui = "vendor";
  }
  else if (docType == "I")
  {
    params.append("item_id", targetid);
    ui = "item";
  }
  else if (docType == "Q")
  {
    params.append("quhead_id", targetid);
    ui = "salesOrder";
  }
  else if (docType == "S")
  {
    params.append("sohead_id", targetid);
    ui = "salesOrder";
  }
  else if (docType == "P")
  {
    params.append("pohead_id", targetid);
    ui = "purchaseOrder";
  }
  else if (docType == "W")
  {
    params.append("wo_id", targetid);
    ui = "workOrder";
  }
  else if (docType == "EMP")
  {
    params.append("emp_id", targetid);
    ui = "employee";
  }
  else if (docType == "C")
  {
    params.append("cust_id", targetid);
    ui = "customer";
  }
  else
  {
    QMessageBox::critical(this, tr("Error"),
                          tr("Unknown File Type."));
    return;
  }

  QWidget* w = 0;
  if (parentWidget()->window())
  {
    if (parentWidget()->window()->isModal())
      w = _guiClientInterface->openWindow(ui, params, parentWidget()->window() , Qt::WindowModal, Qt::Dialog);
    else
      w = _guiClientInterface->openWindow(ui, params, parentWidget()->window() , Qt::NonModal, Qt::Window);
  }

  if (w->inherits("QDialog"))
  {
    QDialog* newdlg = qobject_cast<QDialog*>(w);
    newdlg->exec();
  }

  refresh();
}

void Documents::sViewDoc()
{
  sOpenDoc("view");
}

void Documents::sAttachDoc()
{
  ParameterList params;
  params.append("sourceType", _source);
  params.append("source_id", _sourceid);

  docAttach newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();

  refresh();
}

void Documents::sDetachDoc()
{
  XSqlQuery detach;
  if (_doc->id() < 0)
    return;

  if(QMessageBox::question( this, tr("Confirm Detach"),
       tr("<p>You have requested to detach the selected document."
          " In some cases this may permanently remove the document from the system.</p>"
          "<p>Are you sure you want to continue?</p>"), QMessageBox::Yes|QMessageBox::No, QMessageBox::No) == QMessageBox::No)
    return; // user doesn't want to continue so get out of here

  if ( _doc->currentItem()->rawValue("target_type") == "IMG" )
  {
    detach.prepare( "DELETE FROM imageass "
                    "WHERE (imageass_id = :docid );" );
  }
  else if ( _doc->currentItem()->rawValue("target_type") == "URL" ||
            _doc->currentItem()->rawValue("target_type") == "FILE")
  {
    detach.prepare( "DELETE FROM url "
                    "WHERE (url_id = :docid );" );
  }
  else
  {
    detach.prepare( "DELETE FROM docass "
                    "WHERE (docass_id = :docid );" );
  }
  detach.bindValue(":docid", _doc->id());
  detach.exec();
  refresh();
}

void Documents::refresh()
{
  if(-1 == _sourceid)
  {
    _doc->clear();
    return;
  }

  XSqlQuery query;
  
  //Populate doc list
  QString sql("SELECT id, target_number, target_type, "
              " target_id AS target_number_xtidrole, source_type, source_id, purpose, "
              " name, description, "
              " CASE WHEN (purpose='I') THEN :inventory"
              " WHEN (purpose='P') THEN :product"
              " WHEN (purpose='E') THEN :engineering"
              " WHEN (purpose='M') THEN :misc"
              " WHEN (purpose='A') THEN :child"
              " WHEN (purpose='C') THEN :parent"
              " WHEN (purpose='S') THEN :sibling"
              " WHEN (purpose='D') THEN :dupe"
              " ELSE :other"
              " END AS purpose_qtdisplayrole, "
              " CASE WHEN (target_type='T') THEN :contact "
              " WHEN (target_type='CRMA') THEN :crma "
              " WHEN (target_type='C') THEN :cust "
              " WHEN (target_type='EMP') THEN :emp "
              " WHEN (target_type='URL') THEN :url "
              " WHEN (target_type='FILE') THEN :file "
              " WHEN (target_type='IMG') THEN :image "
              " WHEN (target_type='INCDT') THEN :incident "
              " WHEN (target_type='I') THEN :item "
              " WHEN (target_type='OPP') THEN :opp "
              " WHEN (target_type='J') THEN :project "
              " WHEN (target_type='P') THEN :po "
              " WHEN (target_type='S') THEN :so "
              " WHEN (target_type='Q') THEN :quote "
              " WHEN (target_type='V') THEN :vendor "
              " WHEN (target_type='W') THEN :wo "
              " WHEN (target_type='TODO') THEN :todo "
              " ELSE :error "
              " END AS target_type_qtdisplayrole "
              "FROM docinfo "
              "WHERE ((source_type=:source) "
              " AND (source_id=:sourceid)) "
              "ORDER by target_type_qtdisplayrole, target_number; ");
  query.prepare(sql);
  query.bindValue(":inventory", tr("Inventory Description"));
  query.bindValue(":product", tr("Product Description"));
  query.bindValue(":engineering", tr("Engineering Reference"));
  query.bindValue(":misc", tr("Miscellaneous"));

  query.bindValue(":parent", tr("Parent"));
  query.bindValue(":child", tr("Child"));
  query.bindValue(":sibling", tr("Related to"));
  query.bindValue(":dupe", tr("Duplicate of"));
  query.bindValue(":other", tr("Other"));

  query.bindValue(":po", tr("Purchase Order"));
  query.bindValue(":so", tr("Sales Order"));
  query.bindValue(":quote", tr("Quote"));
  query.bindValue(":wo", tr("Work Order"));
  query.bindValue(":image", tr("Image"));
  query.bindValue(":incident", tr("Incident"));
  query.bindValue(":todo", tr("To-Do"));
  query.bindValue(":task", tr("Task"));
  query.bindValue(":project", tr("Project"));
  query.bindValue(":item", tr("Item"));
  query.bindValue(":crma", tr("CRM Account"));
  query.bindValue(":cust", tr("Customer"));
  query.bindValue(":vendor", tr("Vendor"));
  query.bindValue(":contact", tr("Contact"));
  query.bindValue(":opp", tr("Opportunity"));
  query.bindValue(":url", tr("URL"));
  query.bindValue(":emp", tr("Employee"));
  query.bindValue(":file", tr("File"));

  query.bindValue(":source", _documentMap[_source].ident);
  query.bindValue(":sourceid", _sourceid);
  query.exec();
  _doc->populate(query,TRUE);
}

void Documents::handleSelection(bool /*pReadOnly*/)
{
  if (_doc->selectedItems().count() &&
      (_doc->currentItem()->rawValue("target_type").toString() == "URL" ||
       _doc->currentItem()->rawValue("target_type").toString() == "FILE" ))
  {
    _viewDoc->setText(tr("Open"));
  }
  else
  {
    _viewDoc->setText(tr("View"));
  }

  bool valid = (_doc->selectedItems().count() > 0);
  _editDoc->setEnabled(valid && !_readOnly);
  _viewDoc->setEnabled(valid);
}

void Documents::handleItemSelected()
{
  if(_readOnly ||
     (_doc->currentItem()->rawValue("target_type").toString() == "URL" ||
      _doc->currentItem()->rawValue("target_type").toString() == "FILE" ))
  {
    _viewDoc->animateClick();
  }
  else
  {
    _editDoc->animateClick();
  }
}
