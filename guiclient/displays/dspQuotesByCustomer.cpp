/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspQuotesByCustomer.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>

#include "salesOrder.h"

dspQuotesByCustomer::dspQuotesByCustomer(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspQuotesByCustomer", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Quote Lookup by Customer"));
  setListLabel(tr("Quotes"));
  setMetaSQLOptions("quotes", "detail");

  connect(_cust, SIGNAL(newId(int)), this, SLOT(sPopulatePo()));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setStartCaption(tr("Starting Order Date"));
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);
  _dates->setEndCaption(tr("Ending Order Date"));

  list()->addColumn(tr("Quote #"),     _orderColumn, Qt::AlignLeft,   true,  "quhead_number"   );
  list()->addColumn(tr("Quote Date"),  _dateColumn,  Qt::AlignRight,  true,  "quhead_quotedate"  );
  list()->addColumn(tr("Ship-to"),     -1,           Qt::AlignLeft,   true,  "quhead_shiptoname"   );
  list()->addColumn(tr("Cust. P/O #"), 200,          Qt::AlignLeft,   true,  "quhead_custponumber"   );
  list()->addColumn(tr("Status"),     _statusColumn,  Qt::AlignCenter, true,  "quhead_status" );

  _cust->setFocus();
  connect(omfgThis, SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sFillList())  );
}

void dspQuotesByCustomer::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}


void dspQuotesByCustomer::sPopulatePo()
{
  _poNumber->clear();

  if ((_cust->isValid()) && (_dates->allValid()))
  {
    q.prepare( "SELECT MIN(quhead_id), quhead_custponumber "
               "FROM quhead "
               "WHERE ( (quhead_cust_id=:cust_id)"
               " AND (quhead_quotedate BETWEEN :startDate AND :endDate) ) "
               "GROUP BY quhead_custponumber "
               "ORDER BY quhead_custponumber;" );
    _dates->bindValue(q);
    q.bindValue(":cust_id", _cust->id());
    q.exec();
    _poNumber->populate(q);
  }
}

void dspQuotesByCustomer::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem*, int)
{
  menuThis->addAction(tr("Edit..."), this, SLOT(sEditOrder()));
  menuThis->addAction(tr("View..."), this, SLOT(sViewOrder()));
  
  if (_privileges->check("ConvertQuotes"))
  {
    menuThis->addSeparator();
    menuThis->addAction(tr("Convert..."), this, SLOT(sConvert()));
  }

}

void dspQuotesByCustomer::sEditOrder()
{
  if (!checkSitePrivs(list()->id()))
    return;
    
  ParameterList params;
  params.append("mode", "editQuote");
  params.append("quhead_id", list()->id());
      
  salesOrder *newdlg = new salesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspQuotesByCustomer::sViewOrder()
{
  if (!checkSitePrivs(list()->id()))
    return;
    
  ParameterList params;
  params.append("mode", "viewQuote");
  params.append("quhead_id", list()->id());
      
  salesOrder *newdlg = new salesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

bool dspQuotesByCustomer::setParams(ParameterList & params)
{
  if ( !(( (_allPOs->isChecked()) ||
         ( (_selectedPO->isChecked()) && (_poNumber->currentIndex() != -1) ) ) &&
       (_dates->allValid())  ))
  {
    QMessageBox::warning(this, tr("Invalid Options"),
      tr("One or more options are not complete."));
    return false;
  }

  _dates->appendValue(params);
  params.append("cust_id", _cust->id());
  params.append("showExpired");
  params.append("customersOnly");
  if (_selectedPO->isChecked())
    params.append("poNumber", _poNumber->currentText());
  if (_showConverted->isChecked())
    params.append("showConverted", true);

  return true;
}

void dspQuotesByCustomer::sConvert()
{
  if ( QMessageBox::information( this, tr("Convert Selected Quote(s)"),
                                 tr("Are you sure that you want to convert the selected Quote(s) to Sales Order(s)?" ),
                                 tr("&Yes"), tr("&No"), QString::null, 0, 1 ) == 0)
  {
    XSqlQuery check;
    check.prepare( "SELECT quhead_number, quhead_status, cust_creditstatus "
                   "FROM quhead, cust "
                   "WHERE ( (quhead_cust_id=cust_id)"
                   " AND (quhead_id=:quhead_id) );" );

    XSqlQuery convert;
    convert.prepare("SELECT convertQuote(:quhead_id) AS sohead_id;");

    int counter = 0;
    int soheadid = -1;

    QList<XTreeWidgetItem*> selected = list()->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
      if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
      {
        XTreeWidgetItem *cursor = (XTreeWidgetItem*)selected[i];
        check.bindValue(":quhead_id", cursor->id());
        check.exec();
        if (check.first())
        {
          if (check.value("quhead_status").toString() == "C")
          {
            QMessageBox::warning( this, tr("Cannot Convert Quote"),
                                    tr( "Quote #%1 has already been converted to a Sales Order." )
                            .arg(check.value("quhead_number").toString()) );
            return;
          }

          if ( (check.value("cust_creditstatus").toString() == "H") && (!_privileges->check("CreateSOForHoldCustomer")) )
          {
            QMessageBox::warning( this, tr("Cannot Convert Quote"),
					tr( "Quote #%1 is for a Customer that has been placed on a Credit Hold and you do not have\n"
				    "privilege to create Sales Orders for Customers on Credit Hold.  The selected\n"
				    "Customer must be taken off of Credit Hold before you may create convert this Quote." )
                                .arg(check.value("quhead_number").toString()) );
            return;
          }

          if ( (check.value("cust_creditstatus").toString() == "W") && (!_privileges->check("CreateSOForWarnCustomer")) )
          {
            QMessageBox::warning( this, tr("Cannot Convert Quote"),
			    	tr( "Quote #%1 is for a Customer that has been placed on a Credit Warning and you do not have\n"
				    "privilege to create Sales Orders for Customers on Credit Warning.  The selected\n"
				    "Customer must be taken off of Credit Warning before you may create convert this Quote." )
                                .arg(check.value("quhead_number").toString()) );
            return;
          }
        }
        else
        {
	      systemError( this, tr("A System Error occurred at %1::%2.")
			   .arg(__FILE__)
			   .arg(__LINE__) );
	      continue;
        }

        convert.bindValue(":quhead_id", cursor->id());
        convert.exec();
        if (convert.first())
        { 
	      soheadid = convert.value("sohead_id").toInt();
	      if(soheadid < 0)
	      {
	        QMessageBox::warning( this, tr("Cannot Convert Quote"),
				tr( "Quote #%1 has one or more line items without a warehouse specified.\n"
				    "These line items must be fixed before you may convert this quote." )
				.arg(check.value("quhead_number").toString()) );
	        return;
	      }
	      counter++;
	      omfgThis->sSalesOrdersUpdated(soheadid);
		}
        else
        {
	      systemError( this, tr("A System Error occurred at %1::%2.")
			   .arg(__FILE__)
			   .arg(__LINE__) );
	      return;
        }
	  }
    }

    if (counter)
      omfgThis->sQuotesUpdated(-1);

    if (counter == 1)
      salesOrder::editSalesOrder(soheadid, true);
  }
}

bool dspQuotesByCustomer::checkSitePrivs(int orderid)
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
