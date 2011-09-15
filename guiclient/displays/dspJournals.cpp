/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspJournals.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "glTransactionDetail.h"
#include "dspGLSeries.h"
#include "invoice.h"
#include "purchaseOrder.h"
#include "voucher.h"
#include "miscVoucher.h"
#include "dspShipmentsByShipment.h"
#include "apOpenItem.h"
#include "arOpenItem.h"
#include "salesOrder.h"
#include "dspWoHistoryByNumber.h"
#include "transactionInformation.h"
#include "storedProcErrorLookup.h"
#include "parameterwidget.h"
#include "creditMemo.h"

dspJournals::dspJournals(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspJournals", fl)
{
  setWindowTitle(tr("Journals"));
  setListLabel(tr("Journal Entries"));
  setReportName("Journals");
  setMetaSQLOptions("journals", "detail");
  setParameterWidgetVisible(true);

  QString qryType = QString( "SELECT  1, '%1' UNION "
                             "SELECT  2, '%2' UNION "
                             "SELECT  3, '%3' UNION "
                             "SELECT  4, '%4' UNION "
                             "SELECT  5, '%5'")
      .arg(tr("Asset"))
      .arg(tr("Expense"))
      .arg(tr("Liability"))
      .arg(tr("Equity"))
      .arg(tr("Revenue"));

  QString qrySubType = QString("SELECT subaccnttype_id, "
                               "       (subaccnttype_code || '-' || subaccnttype_descrip) "
                               "FROM subaccnttype "
                               "ORDER BY subaccnttype_code;");

  QString qrySource = QString("SELECT 1 AS id, 'A/P' AS source  UNION "
                              "SELECT 2 AS id, 'A/R' AS source  UNION "
                              "SELECT 3 AS id, 'G/L' AS source  UNION "
                              "SELECT 4 AS id, 'I/M' AS source  UNION "
                              "SELECT 5 AS id, 'P/D' AS source  UNION "
                              "SELECT 6 AS id, 'P/O' AS source  UNION "
                              "SELECT 7 AS id, 'S/O' AS source  UNION "
                              "SELECT 8 AS id, 'S/R' AS source  UNION "
                              "SELECT 9 AS id, 'W/O' AS source;");

  QString qryAccNum = QString("SELECT min(accnt_id) as num_id,"
                              "       accnt_number AS num_number "
                              "FROM accnt "
                              "GROUP BY accnt_number "
                              "ORDER BY accnt_number;");
 
  list()->addColumn(tr("Date"),      _dateColumn,    Qt::AlignCenter, true, "sltrans_date");
  list()->addColumn(tr("Source"),    _orderColumn,   Qt::AlignCenter, true, "sltrans_source");
  list()->addColumn(tr("Doc. Type"), _docTypeColumn, Qt::AlignCenter, true, "sltrans_doctype");
  list()->addColumn(tr("Doc. #"),    _orderColumn,   Qt::AlignCenter, true, "docnumber");
  list()->addColumn(tr("Reference"), -1,             Qt::AlignLeft,   true, "notes");
  list()->addColumn(tr("Journal#"),  _orderColumn,   Qt::AlignLeft,   false,"sltrans_journalnumber");
  list()->addColumn(tr("Account"),   -1,             Qt::AlignLeft,   true, "account");
  list()->addColumn(tr("Debit"),     _moneyColumn,   Qt::AlignRight,  true, "debit");
  list()->addColumn(tr("Credit"),    _moneyColumn,   Qt::AlignRight,  true, "credit");
  list()->addColumn(tr("Posted"),    _ynColumn,      Qt::AlignCenter, true, "sltrans_posted");
  list()->addColumn(tr("GL Journal #"),_orderColumn,   Qt::AlignLeft,   false, "sltrans_gltrans_journalnumber");
  list()->addColumn(tr("Username"),  _userColumn,    Qt::AlignLeft,   false, "sltrans_username");

  parameterWidget()->append(tr("Start Date"), "startDate", ParameterWidget::Date, QDate::currentDate(), true);
  parameterWidget()->append(tr("End Date"),   "endDate",   ParameterWidget::Date, QDate::currentDate(), true);
  parameterWidget()->append(tr("GL Account"), "accnt_id",  ParameterWidget::GLAccount);
  parameterWidget()->append(tr("Document #"), "docnum",    ParameterWidget::Text);
  parameterWidget()->appendComboBox(tr("Source"), "source_id",    qrySource);
  parameterWidget()->append(tr("Source Pattern"), "source_pattern", ParameterWidget::Text);
  if (_metrics->value("GLCompanySize").toInt() > 0)
  parameterWidget()->appendComboBox(tr("Company"), "company_id", XComboBox::Companies);
  if (_metrics->value("GLProfitSize").toInt() >  0)
    parameterWidget()->appendComboBox(tr("Profit Center"), "prfcntr_id", XComboBox::ProfitCenters);
  parameterWidget()->appendComboBox(tr("Main Segment"), "num_id", qryAccNum);
  if (_metrics->value("GLSubaccountSize").toInt() > 0)
    parameterWidget()->appendComboBox(tr("Sub Account"), "subaccnt_id", XComboBox::Subaccounts);
  parameterWidget()->appendComboBox(tr("Account Type"), "accnttype_id", qryType);
  parameterWidget()->appendComboBox(tr("Sub Type"), "subType",   qrySubType);
  parameterWidget()->append(tr("Posted"), "posted", ParameterWidget::CheckBox);
  parameterWidget()->append(tr("GL Journal"), "journalnumber",    ParameterWidget::Text);

  parameterWidget()->applyDefaultFilterSet();

  _sources << "None" << "A/P" << "A/R" << "G/L" << "I/M" << "P/D" << "P/O" << "S/O" << "S/R" << "W/O";
}

enum SetResponse dspJournals::set(const ParameterList &pParams)
{
  XWidget::set(pParams);

  parameterWidget()->setSavedFilters();

  QVariant param;
  bool     valid;

  param = pParams.value("accnt_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("GL Account"), param);

  param = pParams.value("startDate", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Start Date"), param);

  param = pParams.value("endDate", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("End Date"), param);

  param = pParams.value("posted", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Posted"), param);

  param = pParams.value("journalnumber", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("GL Journal"), param);

  param = pParams.value("source", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Source Pattern"), param.toString());

  param = pParams.value("period_id", &valid);
  if (valid)
  {
    q.prepare( "SELECT period_start, period_end "
               "FROM period "
               "WHERE (period_id=:period_id);" );
    q.bindValue(":period_id", param.toInt());
    q.exec();
    if (q.first())
    {
      parameterWidget()->setDefault(tr("Start Date"), q.value("period_start").toDate());
      parameterWidget()->setDefault(tr("End Date"), q.value("period_end").toDate());
    }
  }

  parameterWidget()->applyDefaultFilterSet();

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspJournals::sPopulateMenu(QMenu * menuThis, QTreeWidgetItem* pItem, int)
{
  XTreeWidgetItem * item = (XTreeWidgetItem*)pItem;
  if(0 == item)
    return;

  menuThis->addAction(tr("View..."), this, SLOT(sViewTrans()));
  QAction* viewSeriesAct = menuThis->addAction(tr("View Journal Series..."), this, SLOT(sViewSeries()));
  viewSeriesAct->setDisabled(item->data(0, Xt::DeletedRole).toBool());

  if(item->rawValue("sltrans_doctype").toString() == "VO")
    menuThis->addAction(tr("View Voucher..."), this, SLOT(sViewDocument()));
  else if(item->rawValue("sltrans_doctype").toString() == "IN")
    menuThis->addAction(tr("View Invoice..."), this, SLOT(sViewDocument()));
  else if(item->rawValue("sltrans_doctype").toString() == "PO")
    menuThis->addAction(tr("View Purchase Order..."), this, SLOT(sViewDocument()));
  else if(item->rawValue("sltrans_doctype").toString() == "SH")
    menuThis->addAction(tr("View Shipment..."), this, SLOT(sViewDocument()));
  else if(item->rawValue("sltrans_doctype").toString() == "CM")
    menuThis->addAction(tr("View Credit Memo..."), this, SLOT(sViewDocument()));
  else if(item->rawValue("sltrans_doctype").toString() == "DM")
    menuThis->addAction(tr("View Debit Memo..."), this, SLOT(sViewDocument()));
  else if(item->rawValue("sltrans_doctype").toString() == "SO")
    menuThis->addAction(tr("View Sales Order..."), this, SLOT(sViewDocument()));
  else if(item->rawValue("sltrans_doctype").toString() == "WO")
    menuThis->addAction(tr("View WO History..."), this, SLOT(sViewDocument()));
  else if(item->rawValue("sltrans_source").toString() == "I/M")
    menuThis->addAction(tr("View Inventory History..."), this, SLOT(sViewDocument()));
}

bool dspJournals::setParams(ParameterList &params)
{
  if (!display::setParams(params))
    return false;

  bool valid;
  QVariant param;

  param = params.value("accnttype_id", &valid);
  if (valid)
  {
    int typid = param.toInt();
    QString type;

    if (typid == 1)
      type = "A";
    else if (typid ==2)
      type = "E";
    else if (typid ==3)
      type = "L";
    else if (typid ==4)
      type = "Q";
    else if (typid ==5)
      type = "R";

    params.append("accntType", type);
  }

  param = params.value("source_id", &valid);
  if (valid)
    params.append("source", _sources.at(param.toInt()));

  param = params.value("num_id", &valid);
  if (valid)
  {
    XSqlQuery num;
    num.prepare("SELECT accnt_number "
                "FROM accnt "
                "WHERE (accnt_id=:accnt_id);");
    num.bindValue(":accnt_id", params.value("num_id").toInt());
    num.exec();
    if (num.first())
      params.append("accnt_number", num.value("accnt_number").toString());
  }

  return true;
}

void dspJournals::sViewTrans()
{
  ParameterList params;

  params.append("sltrans_id", list()->id());

  glTransactionDetail newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspJournals::sViewSeries()
{
  q.prepare("SELECT sltrans_date, sltrans_journalnumber"
            "  FROM sltrans"
            " WHERE (sltrans_id=:sltrans_id)");
  q.bindValue(":sltrans_id", list()->id());
  q.exec();
  if(!q.first())
    return;

  ParameterList params;

  params.append("startDate", q.value("SLtrans_date").toDate());
  params.append("endDate", q.value("SLtrans_date").toDate());
  params.append("journalnumber", q.value("SLtrans_journalnumber").toString());
  params.append("journal", true);

  dspGLSeries *newdlg = new dspGLSeries();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspJournals::sViewDocument()
{
  XTreeWidgetItem * item = (XTreeWidgetItem*)list()->currentItem();
  if(0 == item)
    return;

  ParameterList params;
  if(item->rawValue("sltrans_doctype").toString() == "VO")
  {
    q.prepare("SELECT vohead_id, vohead_misc"
              "  FROM vohead"
              " WHERE (vohead_number=:vohead_number)");
    q.bindValue(":vohead_number", item->rawValue("docnumber").toString());
    q.exec();
    if(!q.first())
      return;

    params.append("vohead_id", q.value("vohead_id").toInt());
    params.append("mode", "view");
    
    if(q.value("vohead_misc").toBool())
    {
      miscVoucher *newdlg = new miscVoucher();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
    else
    {
      voucher *newdlg = new voucher();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }

  }
  else if(item->rawValue("sltrans_doctype").toString() == "IN")
  {
    q.prepare("SELECT invchead_id"
              "  FROM invchead"
              " WHERE (invchead_invcnumber=:invchead_invcnumber)");
    q.bindValue(":invchead_invcnumber", item->rawValue("docnumber").toString());
    q.exec();
    if(!q.first())
      return;

    invoice::viewInvoice(q.value("invchead_id").toInt());
  }
  else if(item->rawValue("sltrans_doctype").toString() == "PO")
  {
    QStringList docnumber = item->rawValue("docnumber").toString().split("-");
    q.prepare("SELECT pohead_id"
              "  FROM pohead"
              " WHERE (pohead_number=:docnumber)");
    q.bindValue(":docnumber", docnumber[0]);
    q.exec();
    if(!q.first())
      return;

    params.append("pohead_id", q.value("pohead_id").toInt());
    params.append("mode", "view");

    purchaseOrder *newdlg = new purchaseOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(item->rawValue("sltrans_doctype").toString() == "SH")
  {
    q.prepare("SELECT shiphead_id"
              "  FROM shiphead"
              " WHERE (shiphead_number=:shiphead_number)");
    q.bindValue(":shiphead_number", item->rawValue("docnumber").toString());
    q.exec();
    if(!q.first())
      return;

    params.append("shiphead_id", q.value("shiphead_id").toInt());

    dspShipmentsByShipment *newdlg = new dspShipmentsByShipment();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if( (item->rawValue("sltrans_doctype").toString() == "CM") || (item->rawValue("sltrans_doctype").toString() == "DM") )
  {
    if(item->rawValue("sltrans_source").toString() == "A/P")
    {
      q.prepare("SELECT apopen_id"
                "  FROM apopen"
                " WHERE ( (apopen_docnumber=:docnumber) "
                "  AND (apopen_doctype IN ('C', 'D')) );");
      q.bindValue(":docnumber", item->rawValue("docnumber").toString());
      q.exec();
      if(!q.first())
        return;

      params.append("mode", "view");
      params.append("apopen_id", q.value("apopen_id").toInt());
      apOpenItem newdlg(this, "", true);
      newdlg.set(params);
      newdlg.exec();
    }
    else if(item->rawValue("sltrans_source").toString() == "A/R")
    {
      q.prepare("SELECT aropen_id"
                "  FROM aropen"
                " WHERE ( (aropen_docnumber=:docnumber) "
                "  AND (aropen_doctype IN ('C', 'D')) );");
      q.bindValue(":docnumber", item->rawValue("docnumber").toString());
      q.exec();
      if(!q.first())
        return;

      params.append("mode", "view");
      params.append("aropen_id", q.value("aropen_id").toInt());
      arOpenItem newdlg(this, "", true);
      newdlg.set(params);
      newdlg.exec();
    }
    else if(item->rawValue("sltrans_source").toString() == "S/O")
    {
      q.prepare("SELECT cmhead_id"
                "  FROM cmhead"
                " WHERE (cmhead_number=:docnumber);");
      q.bindValue(":docnumber", item->rawValue("docnumber").toString());
      q.exec();
      if(!q.first())
        return;

      params.append("mode", "view");
      params.append("cmhead_id", q.value("cmhead_id").toInt());
      creditMemo *newdlg = new creditMemo();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
  }
  else if(item->rawValue("sltrans_doctype").toString() == "SO")
  {
    QStringList docnumber = item->rawValue("docnumber").toString().split("-");
    q.prepare("SELECT cohead_id"
              "  FROM cohead"
              " WHERE (cohead_number=:docnumber)");
    q.bindValue(":docnumber", docnumber[0]);
    q.exec();
    if(q.first())
      salesOrder::viewSalesOrder(q.value("cohead_id").toInt(), this);
  }
  else if(item->rawValue("sltrans_doctype").toString() == "WO")
  {
    QStringList docnumber = item->rawValue("docnumber").toString().split("-");
    params.append("wo_number", docnumber[0]);

    dspWoHistoryByNumber *newdlg = new dspWoHistoryByNumber();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(item->rawValue("sltrans_source").toString() == "I/M")
  {
    q.prepare("SELECT sltrans_misc_id"
              "  FROM sltrans"
              " WHERE (sltrans_id=:sltrans_id)");
    q.bindValue(":sltrans_id", item->id());
    q.exec();
    if(!q.first())
      return;

    params.append("mode", "view");
    params.append("invhist_id", q.value("sltrans_misc_id").toInt());

    transactionInformation newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }
}

