/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "taxTypes.h"

#include <QVariant>
#include <QMessageBox>
//#include <QStatusBar>
#include <parameter.h>
#include <openreports.h>
#include "taxType.h"

/*
 *  Constructs a taxTypes as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
taxTypes::taxTypes(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_taxtype, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));

//  statusBar()->hide();
  
  _taxtype->addColumn(tr("Name"),        70, Qt::AlignLeft,  true,  "taxtype_name" );
  _taxtype->addColumn(tr("Description"), -1, Qt::AlignLeft,  true,  "taxtype_descrip" );

  if (_privileges->check("MaintainTaxTypes"))
  {
    connect(_taxtype, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_taxtype, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_taxtype, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    connect(_taxtype, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

    _new->setEnabled(FALSE);
  }

  sFillList(-1);
}

/*
 *  Destroys the object and frees any allocated resources
 */
taxTypes::~taxTypes()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void taxTypes::languageChange()
{
  retranslateUi(this);
}

void taxTypes::sDelete()
{
  q.prepare("SELECT deleteTaxType(:taxtype_id) AS result;");
  q.bindValue(":taxtype_id", _taxtype->id());
  q.exec();
  if (q.first())
  {
    if (q.value("result").toInt() < 0)
    {
      QMessageBox::warning( this, tr("Cannot Delete Tax Type"),
                            tr( "You cannot delete the selected Tax Type because there are currently items assigned to it.\n"
                                "You must first re-assign these items before deleting the selected Tax Type." ) );
      return;
    }

    sFillList(-1);
  }
  else
    systemError(this, tr("A System Error occurred at %1::%2.")
                      .arg(__FILE__)
                      .arg(__LINE__) );
}

void taxTypes::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  taxType newdlg(this, "", TRUE);
  newdlg.set(params);
  
  int result = newdlg.exec();
  if (result != XDialog::Rejected)
    sFillList(result);
}

void taxTypes::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("taxtype_id", _taxtype->id());

  taxType newdlg(this, "", TRUE);
  newdlg.set(params);
  
  int result = newdlg.exec();
  if (result != XDialog::Rejected)
    sFillList(result);
}

void taxTypes::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("taxtype_id", _taxtype->id());

  taxType newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void taxTypes::sPrint()
{
  orReport report("TaxTypesMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void taxTypes::sFillList(int pId)
{
  _taxtype->populate( "SELECT taxtype_id, taxtype_name, taxtype_descrip "
                      "FROM taxtype "
                      "ORDER BY taxtype_name;", pId );
}

