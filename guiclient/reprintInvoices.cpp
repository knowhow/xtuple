/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "reprintInvoices.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include "mqlutil.h"
#include <openreports.h>

#include "editICMWatermark.h"
#include "submitAction.h"

reprintInvoices::reprintInvoices(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sQuery()));
  connect(_numOfCopies, SIGNAL(valueChanged(int)), this, SLOT(sHandleInvoiceCopies(int)));
  connect(_watermarks, SIGNAL(itemSelected(int)), this, SLOT(sEditWatermark()));

  _invoice->addColumn( tr("Invoice #"),    _orderColumn,    Qt::AlignRight, true, "invchead_invcnumber");
  _invoice->addColumn( tr("Doc. Date"),    _dateColumn,     Qt::AlignCenter,true, "invchead_invcdate");
  _invoice->addColumn( tr("Customer"),     -1,              Qt::AlignLeft,  true, "customer");
  _invoice->addColumn( tr("Total Amount"), _bigMoneyColumn, Qt::AlignRight, true, "extprice" );
  _invoice->addColumn( tr("Balance"),      _bigMoneyColumn, Qt::AlignRight, true, "balance" );
  _invoice->setSelectionMode(QAbstractItemView::ExtendedSelection);

  _watermarks->addColumn( tr("Copy #"),      _dateColumn, Qt::AlignCenter );
  _watermarks->addColumn( tr("Watermark"),   -1,          Qt::AlignLeft   );
  _watermarks->addColumn( tr("Show Prices"), _dateColumn, Qt::AlignCenter );

  _numOfCopies->setValue(_metrics->value("InvoiceCopies").toInt() + 1);
  if (_numOfCopies->value())
  {
    for (int i = 0; i < _watermarks->topLevelItemCount(); i++)
    {
      _watermarks->topLevelItem(i)->setText(1, _metrics->value(QString("InvoiceWatermark%1").arg(i)));
      _watermarks->topLevelItem(i)->setText(2, ((_metrics->value(QString("InvoiceShowPrices%1").arg(i)) == "t") ? tr("Yes") : tr("No")));
    }
  }
}

reprintInvoices::~reprintInvoices()
{
  // no need to delete child widgets, Qt does it all for us
}

void reprintInvoices::languageChange()
{
  retranslateUi(this);
}

void reprintInvoices::sQuery()
{
  MetaSQLQuery mql = mqlLoad("invoices", "detail");
  ParameterList params;
  params.append("invc_pattern", _invoicePattern->text().trimmed());
  if (_dates->allValid())
    _dates->appendValue(params);
  if (_balanceOnly->isChecked())
    params.append("balanceOnly");
  q = mql.toQuery(params);
  _invoice->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void reprintInvoices::sPrint()
{
  QPrinter printer(QPrinter::HighResolution);
  bool     setupPrinter = TRUE;

  bool userCanceled = false;
  if (orReport::beginMultiPrint(&printer, userCanceled) == false)
  {
    if(!userCanceled)
      systemError(this, tr("Could not initialize printing system for multiple reports."));
    return;
  }
  QList<XTreeWidgetItem*> selected = _invoice->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XTreeWidgetItem *cursor = (XTreeWidgetItem*)selected[i];

    for (int j = 0; j < _watermarks->topLevelItemCount(); j++)
    {
      q.prepare("SELECT findCustomerForm(:cust_id, 'I') AS _reportname;");
      q.bindValue(":cust_id", cursor->altId());
      q.exec();
      if (q.first())
      {
	ParameterList params;
	params.append("invchead_id", cursor->id());
	params.append("showcosts", ((_watermarks->topLevelItem(j)->text(2) == tr("Yes")) ? "TRUE" : "FALSE") );
	params.append("watermark", _watermarks->topLevelItem(j)->text(1));

	orReport report(q.value("_reportname").toString(), params);
	if (report.isValid())
	{
	  if (report.print(&printer, setupPrinter))
		 setupPrinter = FALSE;
	      else 
	      {
		report.reportError(this);
		orReport::endMultiPrint(&printer);
		return;
	      }
	}
	else
	  QMessageBox::critical(this, tr("Cannot Find Invoice Form"),
				tr("<p>The Invoice Form '%1' cannot be found. "
                                   "One or more of the selected Invoices "
                                   "cannot be printed until a Customer Form "
                                   "Assignment is updated to remove any "
                                   "references to this Invoice Form or this "
                                   "Invoice Form is created.")
                                  .arg(q.value("_reportname").toString()) );
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }

    emit finishedPrinting(cursor->id());
  }
  orReport::endMultiPrint(&printer);

  _invoice->clearSelection();
  _close->setText(tr("&Close"));
  _print->setEnabled(FALSE);
}

void reprintInvoices::sHandleInvoiceCopies(int pValue)
{
  if (_watermarks->topLevelItemCount() > pValue)
    _watermarks->takeTopLevelItem(_watermarks->topLevelItemCount() - 1);
  else
  {
    for (int i = (_watermarks->topLevelItemCount() + 1); i <= pValue; i++)
      new XTreeWidgetItem(_watermarks,
			  _watermarks->topLevelItem(_watermarks->topLevelItemCount() - 1),
			  i, i, tr("Copy #%1").arg(i), "", tr("Yes"));
  }
}

void reprintInvoices::sEditWatermark()
{
  QTreeWidgetItem *cursor = _watermarks->currentItem();
  ParameterList params;
  params.append("watermark", cursor->text(1));
  params.append("showPrices", (cursor->text(2) == tr("Yes")));

  editICMWatermark newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() == XDialog::Accepted)
  {
    cursor->setText(1, newdlg.watermark());
    cursor->setText(2, ((newdlg.showPrices()) ? tr("Yes") : tr("No")));
  }
}
