/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspCustomerInformationExport.h"

#include <QSqlError>
#include <QVariant>

#include "xtreewidget.h"

dspCustomerInformationExport::dspCustomerInformationExport(QWidget* parent, const char* name, Qt::WFlags fl)
  : display(parent, "dspCustomerInformationExport", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Customer Information Export"));
  setListLabel(tr("Information"));
  setMetaSQLOptions("customerInformationExport", "detail");
  setUseAltId(true);

  _customerType->setType(ParameterGroup::CustomerType);
}

void dspCustomerInformationExport::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspCustomerInformationExport::setParams(ParameterList &params)
{
  if (!_showInactive->isChecked())
    params.append("showInactive");

  _customerType->appendValue(params);

  list()->setColumnCount(0);

  struct {
    QCheckBox *checkbox;
    QString title;
    QString dbcol;
  } list2[] = {
    { _number,       tr("Customer Number"),   "cust_number" },
    { _name,         tr("Customer Name"),     "cust_name" },
    { _billAddress1, tr("Billing Address 1"), "cust_address1" },
    { _billAddress2, tr("Billing Address 2"), "cust_address2" },
    { _billAddress3, tr("Billing Address 3"), "cust_address3" },
    { _billCity,     tr("Billing City"),      "cust_city" },
    { _billState,    tr("Billing State"),     "cust_state" },
    { _billZipCode,  tr("Billing Zip Code"),  "cust_zipcode" },
    { _billCountry,  tr("Billing Country"),   "cust_country" },
    { _billContact,  tr("Billing Contact"),   "cust_contact" },
    { _billPhone,    tr("Billing Phone"),     "cust_phone" },
    { _billFAX,      tr("Billing FAX"),       "cust_fax" },
    { _billEmail,    tr("Billing Email"),     "cust_email" },
    { _corrAddress1, tr("Corr. Address 1"),   "cust_corraddress1" },
    { _corrAddress2, tr("Corr. Address 2"),   "cust_corraddress2" },
    { _corrAddress3, tr("Corr. Address 3"),   "cust_corraddress3" },
    { _corrCity,     tr("Corr. City"),        "cust_corrcity" },
    { _corrState,    tr("Corr. State"),       "cust_corrstate" },
    { _corrZipCode,  tr("Corr. Zip Code"),    "cust_corrzipcode" },
    { _corrCountry,  tr("Corr. Country"),     "cust_corrcountry" },
    { _corrContact,  tr("Corr. Contact"),     "cust_corrcontact" },
    { _corrPhone,    tr("Corr. Phone"),       "cust_corrphone" },
    { _corrFAX,      tr("Corr. FAX"),         "cust_corrfax" },
    { _corrEmail,    tr("Corr. Email"),       "cust_corremail" }
  };

  for (unsigned int i = 0; i < sizeof(list2) / sizeof(list2[0]); i++)
  {
    if (list2[i].checkbox->isChecked())
    {
      list()->addColumn(list2[i].title, 100, Qt::AlignLeft, true, list2[i].dbcol);
      params.append(list2[i].dbcol);
    }
  }
  
  return true;
}
