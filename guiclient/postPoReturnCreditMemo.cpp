/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "postPoReturnCreditMemo.h"

#include <QVariant>
#include <QMessageBox>
#include <openreports.h>

/*
 *  Constructs a postPoReturnCreditMemo as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
postPoReturnCreditMemo::postPoReturnCreditMemo(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));

  _qty->setPrecision(omfgThis->qtyVal());

  _porejectid = -1;

  _post->setFocus();
}

/*
 *  Destroys the object and frees any allocated resources
 */
postPoReturnCreditMemo::~postPoReturnCreditMemo()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void postPoReturnCreditMemo::languageChange()
{
  retranslateUi(this);
}

enum SetResponse postPoReturnCreditMemo::set(const ParameterList & pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("poreject_id", &valid);
  if(valid)
  {
    _porejectid = param.toInt();
    q.prepare("SELECT pohead_curr_id,"
              "       COALESCE(item_number, poitem_vend_item_number) AS itemnumber,"
              "       poreject_qty,"
              "       (poitem_unitprice * poreject_qty) AS itemAmount"
              "  FROM pohead, poreject, poitem"
              "       LEFT OUTER JOIN itemsite ON (poitem_itemsite_id=itemsite_id)"
              "       LEFT OUTER JOIN item ON (itemsite_item_id=item_id)"
              " WHERE((poreject_poitem_id=poitem_id)"
              "   AND (pohead_id=poitem_pohead_id)"
              "   AND (poreject_id=:poreject_id));");
    q.bindValue(":poreject_id", _porejectid);
    q.exec();
    if(q.first())
    {
      _item->setText(q.value("itemNumber").toString());
      _qty->setDouble(q.value("poreject_qty").toDouble());
      _amount->set(q.value("itemAmount").toDouble(), q.value("pohead_curr_id").toInt(), QDate::currentDate(), FALSE);
    }
  }

  return NoError;
}

void postPoReturnCreditMemo::sPost()
{
  q.prepare("SELECT postPoReturnCreditMemo(:poreject_id, :amount) AS result;");
  q.bindValue(":poreject_id", _porejectid);
  q.bindValue(":amount", _amount->localValue());
  if(!q.exec())
  {
    systemError( this, tr("A System Error occurred at postPoReturnCreditMemo::%1.")
                       .arg(__LINE__) );
    return;
  }

  accept();
}
