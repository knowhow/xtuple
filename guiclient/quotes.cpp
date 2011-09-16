/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "quotes.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>
#include <openreports.h>

#include "failedPostList.h"
#include "salesOrder.h"
#include "storedProcErrorLookup.h"
#include "customer.h"
#include "parameterwidget.h"

quotes::quotes(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "vendors", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Quotes"));
  setMetaSQLOptions("quotes", "detail");
  setParameterWidgetVisible(true);
  setNewVisible(true);
  setQueryOnStartEnabled(true);

  _convertedtoSo->setVisible(false);

  if (_metrics->boolean("MultiWhs"))
    parameterWidget()->append(tr("Site"), "warehous_id", ParameterWidget::Site);
  parameterWidget()->append(tr("Exclude Prospects"), "customersOnly", ParameterWidget::Exists);
  parameterWidget()->append(tr("Customer"), "cust_id", ParameterWidget::Customer);
  parameterWidget()->appendComboBox(tr("Customer Type"), "custtype_id", XComboBox::CustomerTypes);
  parameterWidget()->append(tr("Customer Type Pattern"), "custtype_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("P/O Number"), "poNumber", ParameterWidget::Text);
  parameterWidget()->appendComboBox(tr("Sales Rep."), "salesrep_id", XComboBox::SalesRepsActive);
  parameterWidget()->append(tr("Start Date"), "startDate", ParameterWidget::Date);
  parameterWidget()->append(tr("End Date"),   "endDate",   ParameterWidget::Date);

  list()->addColumn(tr("Quote #"),    _orderColumn, Qt::AlignRight, true, "quhead_number");
  list()->addColumn(tr("Customer"),   -1,           Qt::AlignLeft,  true, "quhead_billtoname");
  list()->addColumn(tr("P/O Number"), -1,           Qt::AlignLeft,  true, "quhead_custponumber");
  list()->addColumn(tr("Status"),    _dateColumn,   Qt::AlignCenter,true, "quhead_status");
  list()->addColumn(tr("Quote Date"), _dateColumn,  Qt::AlignCenter,true, "quhead_quotedate");
  list()->addColumn(tr("Expire Date"), _dateColumn,  Qt::AlignCenter,false, "quhead_expire");
  list()->addColumn(tr("Notes"),      -1,           Qt::AlignCenter,true, "notes");
  list()->setSelectionMode(QAbstractItemView::ExtendedSelection);

  if (_privileges->check("MaintainQuotes"))
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sEdit()));
  else
  {
    newAction()->setEnabled(FALSE);
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sView()));
  }

  connect(omfgThis, SIGNAL(quotesUpdated(int, bool)), this, SLOT(sFillList()));
}

enum SetResponse quotes::set(const ParameterList& pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool	   valid;
  
  param = pParams.value("run", &valid);
  if (valid)
    sFillList();

  return NoError;
}

void quotes::sPopulateMenu(QMenu * pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Print..."), this, SLOT(sPrint()));
  menuItem->setEnabled(_privileges->check("PrintQuotes"));

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Convert..."), this, SLOT(sConvert()));
  menuItem->setEnabled(_privileges->check("ConvertQuotes"));

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainQuotes"));

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));

  menuItem = pMenu->addAction(tr("Delete..."), this, SLOT(sDelete()));
  menuItem->setEnabled(_privileges->check("MaintainQuotes"));
}

void quotes::sPrint()
{
  QPrinter printer(QPrinter::HighResolution);
  bool setupPrinter = TRUE;
  q.prepare( "SELECT findCustomerForm(quhead_cust_id, 'Q') AS reportname "
             "FROM quhead "
             "WHERE (quhead_id=:quheadid); " );
  bool userCanceled = false;
  QList<XTreeWidgetItem*> selected = list()->selectedItems();

  if (selected.size() > 0 &&
      orReport::beginMultiPrint(&printer, userCanceled) == false)
  {
    if(!userCanceled)
      systemError(this, tr("<p>Could not initialize printing system for "
			   "multiple reports."));
    return;
  }
  QList<int> printedQuotes;
  for (int i = 0; i < selected.size(); i++)
  {
    int quheadid = ((XTreeWidgetItem*)(selected[i]))->id();
    q.bindValue(":quheadid", quheadid);
    q.exec();
    if (q.first())
    {
      if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
      {
        ParameterList params;
        params.append("quhead_id", quheadid);

        orReport report(q.value("reportname").toString(), params);
        if (report.isValid() && report.print(&printer, setupPrinter))
        {
          setupPrinter = FALSE;
          printedQuotes.append(quheadid);
        }
        else
        {
          report.reportError(this);
          break;
        }
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        break;
      }
    }
  }
  if (selected.size() > 0)
    orReport::endMultiPrint(&printer);

  for (int i = 0; i < printedQuotes.size(); i++)
    emit finishedPrinting(printedQuotes.at(i));
}

void quotes::sConvert()
{
  if (QMessageBox::question(this, tr("Convert Selected Quote(s)"),
			    tr("<p>Are you sure that you want to convert "
			       "the selected Quote(s) to Sales Order(s)?"),
			    QMessageBox::Yes,
			    QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    XSqlQuery convert;
    convert.prepare("SELECT convertQuote(:quhead_id) AS sohead_id;");

    XSqlQuery prospectq;
    prospectq.prepare("SELECT convertProspectToCustomer(quhead_cust_id) AS result "
		      "FROM quhead "
		      "WHERE (quhead_id=:quhead_id);");

    bool tryagain = false;
    QList<int> converted;
    do {
      int soheadid = -1;
      QList<XTreeWidgetItem*> selected = list()->selectedItems();
      QList<XTreeWidgetItem*> notConverted;

      for (int i = 0; i < selected.size(); i++)
      {
        if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
        {
          int quheadid = ((XTreeWidgetItem*)(selected[i]))->id();
          XSqlQuery check;
          check.prepare("SELECT * FROM quhead WHERE (quhead_id = :quhead_id) AND (quhead_status ='C');");
          check.bindValue(":quhead_id", quheadid);
          check.exec();
          if (check.first())
          {
            QMessageBox::critical(this, tr("Can not Convert"),
                                tr("<p>One or more of the Selected Quotes have already "
                                   " been converted to Sales Order."
                                   "You can not Convert a Sales Order back to Quote."));
            return;
          }
          else
          {
          convert.bindValue(":quhead_id", quheadid);
          convert.exec();
          }
          if (convert.first())
          {
            soheadid = convert.value("sohead_id").toInt();
            if (soheadid == -3)
            {
              if ((_metrics->value("DefaultSalesRep").toInt() > 0) &&
                  (_metrics->value("DefaultTerms").toInt() > 0) &&
                  (_metrics->value("DefaultCustType").toInt() > 0) && 
                  (_metrics->value("DefaultShipFormId").toInt() > 0)  && 
                  (_privileges->check("MaintainCustomerMasters"))) 
                {
                  if (QMessageBox::question(this, tr("Quote for Prospect"),
                                tr("<p>This Quote is for a Prospect, not "
                               "a Customer. Do you want to convert "
                               "the Prospect to a Customer using global "
                               "default values?"),
                          QMessageBox::Yes,
                          QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
                  {
                    prospectq.bindValue(":quhead_id", quheadid);
                    prospectq.exec();
                    if (prospectq.first())
                    {
                      int result = prospectq.value("result").toInt();
                      if (result < 0)
                      {
                        systemError(this,
                                    storedProcErrorLookup("convertProspectToCustomer",
                                    result), __FILE__, __LINE__);
                        notConverted.append(selected[i]);
                        continue;
                      }
                      convert.exec();
                      if (convert.first())
                      {
                        soheadid = convert.value("sohead_id").toInt();
                        if (soheadid < 0)
                        {
                          QMessageBox::warning(this, tr("Cannot Convert Quote"),
                                  storedProcErrorLookup("convertQuote", soheadid)
                                  .arg(selected[i] ? selected[i]->text(0) : ""));
                          notConverted.append(selected[i]);
                          continue;
                        }
                      }
                    }
                    else if (prospectq.lastError().type() != QSqlError::NoError)
                    {
                      systemError(this, prospectq.lastError().databaseText(),
                              __FILE__, __LINE__);
                      notConverted.append(selected[i]);
                      continue;
                    }
                  }
                  else
                  {
                    QMessageBox::information(this, tr("Quote for Prospect"),
                                tr("<p>The prospect must be manually "
                                   "converted to customer from either the "
                                   "CRM Account or Customer windows before "
                                   "coverting this quote."));
                    notConverted.append(selected[i]);
                    continue;
                  }
              }
              else
              {
                QMessageBox::information(this, tr("Quote for Prospect"),
                            tr("<p>The prospect must be manually "
                               "converted to customer from either the "
                               "CRM Account or Customer windows before "
                               "coverting this quote."));
                notConverted.append(selected[i]);
                continue;
              }
            }
            else if (soheadid < 0)
            {
              QMessageBox::warning(this, tr("Cannot Convert Quote"),
                      storedProcErrorLookup("convertQuote", soheadid)
                      .arg(selected[i] ? selected[i]->text(0) : ""));
              notConverted.append(selected[i]);
              continue;
            }
            converted << quheadid;
            omfgThis->sSalesOrdersUpdated(soheadid);

            salesOrder::editSalesOrder(soheadid, true);
          }
          else if (convert.lastError().type() != QSqlError::NoError)
          {
            notConverted.append(selected[i]);
            systemError(this, convert.lastError().databaseText(), __FILE__, __LINE__);
            continue;
          }
        }
      }

      if (notConverted.size() > 0)
      {
	failedPostList newdlg(this, "", true);
	newdlg.setLabel(tr("<p>The following Quotes could not be converted."));
        newdlg.sSetList(notConverted, list()->headerItem(), list()->header());
	tryagain = (newdlg.exec() == XDialog::Accepted);
	selected = notConverted;
	notConverted.clear();
      }
    } while (tryagain);
    for (int i = 0; i < converted.size(); i++)
    {
      omfgThis->sQuotesUpdated(converted[i]);
    }
  } // if user wants to convert
}

void quotes::sNew()
{
  ParameterList params;
  params.append("mode", "newQuote");
  parameterWidget()->appendValue(params); // To pick up customer id, if any
      
  salesOrder *newdlg = new salesOrder(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void quotes::sEdit()
{
  QList<XTreeWidgetItem*> selected = list()->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
    {
      ParameterList params;
      params.append("mode", "editQuote");
      params.append("quhead_id", ((XTreeWidgetItem*)(selected[i]))->id());
    
      salesOrder *newdlg = new salesOrder(this);
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
      break;
    }
  }
}

void quotes::sView()
{
  QList<XTreeWidgetItem*> selected = list()->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
    {
      ParameterList params;
      params.append("mode", "viewQuote");
      params.append("quhead_id", ((XTreeWidgetItem*)(selected[i]))->id());
      
      salesOrder *newdlg = new salesOrder(this);
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
      break;
    }
  }
}

void quotes::sDelete()
{
  if (QMessageBox::question(this, tr("Delete Selected Quotes"),
                            tr("<p>Are you sure that you want to delete the "
			       "selected Quotes?" ),
			    QMessageBox::Yes,
			    QMessageBox::No | QMessageBox::Default)
			    == QMessageBox::Yes)
  {
    q.prepare("SELECT deleteQuote(:quhead_id) AS result;");

    int counter = 0;
    QList<XTreeWidgetItem*> selected = list()->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
      if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
      {
        q.bindValue(":quhead_id", ((XTreeWidgetItem*)(selected[i]))->id());
        q.exec();
        if (q.first())
        {
          int result = q.value("result").toInt();
          if (result < 0)
          {
            systemError(this, storedProcErrorLookup("deleteQuote", result),
                        __FILE__, __LINE__);
            continue;
          }
          counter++;
        }
        else if (q.lastError().type() != QSqlError::NoError)
        {
          systemError(this, tr("A System Error occurred deleting Quote #%1\n%2.")
                             .arg(selected[i]->text(0))
                             .arg(q.lastError().databaseText()), __FILE__, __LINE__);
          continue;
        }
      }
    }

    if (counter)
      omfgThis->sQuotesUpdated(-1);
  }
}

bool quotes::checkSitePrivs(int orderid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkQuoteSitePrivs(:quheadid) AS result;");
    check.bindValue(":quheadid", orderid);
    check.exec();
    if (check.first())
    {
      if (!check.value("result").toBool())
      {
        QMessageBox::critical(this, tr("Access Denied"),
                              tr("You may not view, edit, or convert this Quote as it references "
                                 "a Site for which you have not been granted privileges.")) ;
        return false;
      }
    }
  }
  return true;
}

bool quotes::setParams(ParameterList &params)
{
  display::setParams(params);
  if(_showExpired->isChecked())
    params.append("showExpired");
  if(_convertedtoSo->isChecked())
    params.append("showConverted");

  return true;
}
