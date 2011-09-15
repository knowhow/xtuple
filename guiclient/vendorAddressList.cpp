/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "vendorAddressList.h"

#include <QVariant>

/*
 *  Constructs a vendorAddressList as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
vendorAddressList::vendorAddressList(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_select, SIGNAL(clicked()), this, SLOT(sSelect()));
  connect(_vendaddr, SIGNAL(itemSelected(int)), _select, SLOT(animateClick()));
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));

  _vendaddr->addColumn(tr("Code"),    _orderColumn, Qt::AlignLeft,   true,  "code" );
  _vendaddr->addColumn(tr("Name"),    -1,           Qt::AlignLeft,   true,  "name" );
  _vendaddr->addColumn(tr("Address"), 100,          Qt::AlignLeft,   true,  "address" );
}

/*
 *  Destroys the object and frees any allocated resources
 */
vendorAddressList::~vendorAddressList()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void vendorAddressList::languageChange()
{
  retranslateUi(this);
}

enum SetResponse vendorAddressList::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("vend_id", &valid);
  if (valid)
  {
    _vendid = param.toInt();
    q.prepare("SELECT (vend_number||' - '||vend_name) AS f_name,"
              "       vend_address1"
              "  FROM vend"
              " WHERE(vend_id=:vend_id);");
    q.bindValue(":vend_id", _vendid);
    q.exec();
    if(q.first())
    {
      _vendName->setText(q.value("f_name").toString());
      _vendAddr1->setText(q.value("vend_address1").toString());
    }
  }

  sFillList();

  return NoError;
}

void vendorAddressList::sSelect()
{
  done(_vendaddr->id());
}

void vendorAddressList::sClose()
{
  done(_vendaddrid);
}

void vendorAddressList::sFillList()
{
  q.prepare( "SELECT -1 AS id, 'Main' AS code, vend_name AS name, vend_address1 AS address,"
             "       0 AS orderby "
             "FROM vend "
             "WHERE (vend_id=:vend_id) "
             "UNION "
             "SELECT vendaddr_id AS id, vendaddr_code AS code, vendaddr_name AS name, vendaddr_address1 AS address,"
             "       1 AS orderby "
             "FROM vendaddr "
             "WHERE (vendaddr_vend_id=:vend_id) "
             "ORDER BY orderby, code;" );
  q.bindValue(":vend_id", _vendid);
  q.exec();
  _vendaddr->populate(q, _vendid);
}

