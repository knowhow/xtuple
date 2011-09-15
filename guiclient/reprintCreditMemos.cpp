/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "reprintCreditMemos.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>
#include "editICMWatermark.h"

reprintCreditMemos::reprintCreditMemos(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_numOfCopies, SIGNAL(valueChanged(int)), this, SLOT(sHandleCopies(int)));
  connect(_watermarks, SIGNAL(itemSelected(int)), this, SLOT(sEditWatermark()));

  _cmhead->addColumn( tr("C/M #"),   _orderColumn, Qt::AlignRight, true, "cmhead_number");
  _cmhead->addColumn( tr("Doc. Date"),_dateColumn, Qt::AlignCenter,true, "cmhead_docdate");
  _cmhead->addColumn( tr("Customer"),          -1, Qt::AlignLeft,  true, "customer");

  _cmhead->setSelectionMode(QAbstractItemView::ExtendedSelection);

  _watermarks->addColumn( tr("Copy #"),      _dateColumn, Qt::AlignCenter );
  _watermarks->addColumn( tr("Watermark"),   -1,          Qt::AlignLeft   );
  _watermarks->addColumn( tr("Show Prices"), _dateColumn, Qt::AlignCenter );

  _numOfCopies->setValue(_metrics->value("CreditMemoCopies").toInt() + 1);
  if (_numOfCopies->value())
  {
    for (int i = 0; i < _watermarks->topLevelItemCount(); i++)
    {
      _watermarks->topLevelItem(i)->setText(1, _metrics->value(QString("CreditMemoWatermark%1").arg(i)));
      _watermarks->topLevelItem(i)->setText(2, ((_metrics->value(QString("CreditMemoShowPrices%1").arg(i)) == "t") ? tr("Yes") : tr("No")));
    }
  }

  _cmhead->populate( "SELECT cmhead_id, cust_id,"
                     "       cmhead_number, cmhead_docdate,"
                     "       (TEXT(cust_number) || ' - ' || cust_name) AS customer "
                     "FROM ( SELECT cmhead_id, cust_id,"
                     "              cmhead_number, cmhead_docdate,"
                     "              cust_number, cust_name "
                     "       FROM cmhead, cust "
                     "       WHERE (cmhead_cust_id=cust_id) ) AS data "
                     "WHERE (checkCreditMemoSitePrivs(cmhead_id)) "
                     "ORDER BY cmhead_docdate DESC;", TRUE );
}

reprintCreditMemos::~reprintCreditMemos()
{
  // no need to delete child widgets, Qt does it all for us
}

void reprintCreditMemos::languageChange()
{
  retranslateUi(this);
}

void reprintCreditMemos::sPrint()
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
  QList<XTreeWidgetItem*> selected = _cmhead->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XTreeWidgetItem *cursor = (XTreeWidgetItem*)selected[i];
    for (int j = 0; j < _watermarks->topLevelItemCount(); j++)
    {
      QTreeWidgetItem *watermark = _watermarks->topLevelItem(j);
      q.prepare("SELECT findCustomerForm(:cust_id, 'C') AS _reportname;");
      q.bindValue(":cust_id", cursor->altId());
      q.exec();
      if (q.first())
      {
	ParameterList params;
	params.append("cmhead_id", cursor->id());
	params.append("showcosts", ((watermark->text(2) == tr("Yes")) ? "TRUE" : "FALSE") );
	params.append("watermark", watermark->text(1));

	orReport report(q.value("_reportname").toString(), params);
	if (report.isValid())
	{
	  if (report.print(&printer, setupPrinter))
	      setupPrinter = FALSE;
	  else
	  {
	    orReport::endMultiPrint(&printer);
	    return;
	  }
	}
	else
	  QMessageBox::critical(this, tr("Cannot Find Credit Memo Form"),
				tr("<p>The Credit Memo Form '%1' cannot be found. "
                                   "One or more of the selected Credit Memos "
                                   "cannot be printed until a Customer Form "
                                   "Assignment is updated to remove any "
                                   "references to this Credit Memo Form or "
                                   "this Credit Memo Form is created." )
				 .arg(q.value("_reportname").toString()) );
      }
    }

    emit finishedPrinting(cursor->id());
  }
  orReport::endMultiPrint(&printer);

  _cmhead->clearSelection();
  _close->setText(tr("&Close"));
  _print->setEnabled(FALSE);
}

void reprintCreditMemos::sHandleCopies(int pValue)
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

void reprintCreditMemos::sEditWatermark()
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
