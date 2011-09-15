/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "taxCodes.h"

#include <QMessageBox>
#include <QSqlError>
//#include <QStatusBar>
#include <QVariant>

#include <parameter.h>

#include "guiclient.h"
#include "storedProcErrorLookup.h"
#include "taxCode.h"

taxCodes::taxCodes(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
    setupUi(this);

//    (void)statusBar();

    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_view, SIGNAL(clicked()), this, SLOT(sView()));

//    statusBar()->hide();
    
    _tax->addColumn(tr("Code"),        _itemColumn, Qt::AlignLeft,   true,  "tax_code" );
    _tax->addColumn(tr("Description"), -1,          Qt::AlignLeft,   true,  "tax_descrip" );

    if (_privileges->check("MaintainTaxCodes"))
    {
      connect(_tax, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_tax, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_tax, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    }
    else
    {
      _new->setEnabled(FALSE);
      connect(_tax, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    }

    sFillList();
}

taxCodes::~taxCodes()
{
    // no need to delete child widgets, Qt does it all for us
}

void taxCodes::languageChange()
{
    retranslateUi(this);
}

void taxCodes::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  taxCode newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void taxCodes::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("tax_id", _tax->id());

  taxCode newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void taxCodes::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("tax_id", _tax->id());

  taxCode newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void taxCodes::sDelete()
{
  if(QMessageBox::question(this, tr("Delete Tax Code?"),
			      tr("<p>Are you sure you want to delete this Tax Code "
				 "along with its associated Tax Code Rates?"),
				  QMessageBox::Yes,
				  QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    q.prepare("SELECT deleteTax(:tax_id) AS result;");
    q.bindValue(":tax_id", _tax->id());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
        systemError(this, storedProcErrorLookup("deleteTax", result),
	    	  __FILE__, __LINE__);
        return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    sFillList();
  }
}

void taxCodes::sFillList()
{
  _tax->clear();
  _tax->populate( "SELECT tax_id, tax_code, tax_descrip "
                  "FROM tax "
                  "ORDER BY tax_code;" );
}
