/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemSourcePrice.h"

#include <QVariant>
#include <QMessageBox>
#include <QValidator>

/*
 *  Constructs a itemSourcePrice as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
itemSourcePrice::itemSourcePrice(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _qtyBreak->setValidator(omfgThis->qtyVal());
  _itemsrcpid = -1;
  _itemsrcid = -1;
}

/*
 *  Destroys the object and frees any allocated resources
 */
itemSourcePrice::~itemSourcePrice()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void itemSourcePrice::languageChange()
{
    retranslateUi(this);
}

enum SetResponse itemSourcePrice::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("itemsrc_id", &valid);
  if (valid)
    _itemsrcid = param.toInt();

  param = pParams.value("curr_id", &valid);
  if (valid)
    _price->setId(param.toInt());

  param = pParams.value("curr_effective", &valid);
  if (valid)
    _price->setEffective(param.toDate());

  param = pParams.value("itemsrcp_id", &valid);
  if (valid)
  {
    _itemsrcpid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _qtyBreak->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _qtyBreak->setEnabled(FALSE);
      _price->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void itemSourcePrice::sSave()
{
  q.prepare("SELECT itemsrcp_id"
            "  FROM itemsrcp" 
            " WHERE ((itemsrcp_id != :itemsrcp_id)"
            "   AND  (itemsrcp_itemsrc_id=:itemsrcp_itemsrc_id)"
            "   AND  (itemsrcp_qtybreak=:qtybreak));");
  q.bindValue(":itemsrcp_id", _itemsrcpid);
  q.bindValue(":itemsrcp_itemsrc_id", _itemsrcid);
  q.bindValue(":qtybreak", _qtyBreak->toDouble());
  q.exec();
  if(q.first())
  {
    QMessageBox::warning(this, tr("Duplicate Qty. Break"),
      tr("A Qty. Break with the specified Qty. already exists for this Item Source.") );
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('itemsrcp_itemsrcp_id_seq') AS itemsrcp_id;");
    if (q.first())
      _itemsrcpid = q.value("itemsrcp_id").toInt();
//  ToDo

    q.prepare( "INSERT INTO itemsrcp "
               "(itemsrcp_id, itemsrcp_itemsrc_id, itemsrcp_qtybreak, itemsrcp_price, itemsrcp_updated, itemsrcp_curr_id) "
               "VALUES "
               "(:itemsrcp_id, :itemsrcp_itemsrc_id, :itemsrcp_qtybreak, :itemsrcp_price, CURRENT_DATE, :itemsrcp_curr_id);" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE itemsrcp "
               "SET itemsrcp_qtybreak=:itemsrcp_qtybreak, "
	       "    itemsrcp_price=:itemsrcp_price, "
	       "    itemsrcp_updated=CURRENT_DATE, "
	       "    itemsrcp_curr_id=:itemsrcp_curr_id "
               "WHERE (itemsrcp_id=:itemsrcp_id);" );

  q.bindValue(":itemsrcp_id", _itemsrcpid);
  q.bindValue(":itemsrcp_itemsrc_id", _itemsrcid);
  q.bindValue(":itemsrcp_qtybreak", _qtyBreak->toDouble());
  q.bindValue(":itemsrcp_price", _price->localValue());
  q.bindValue(":itemsrcp_curr_id", _price->id());
  q.exec();

  done(_itemsrcpid);
}

void itemSourcePrice::populate()
{
  q.prepare( "SELECT itemsrcp_qtybreak,"
             "       itemsrcp_price, itemsrcp_curr_id,"
             "       itemsrcp_updated, itemsrcp_itemsrc_id "
             "FROM itemsrcp "
             "WHERE (itemsrcp_id=:itemsrcp_id);" );
  q.bindValue(":itemsrcp_id", _itemsrcpid);
  q.exec();
  if (q.first())
  {
    _itemsrcid = q.value("itemsrcp_itemsrc_id").toInt();
    _qtyBreak->setDouble(q.value("itemsrcp_qtybreak").toDouble());
    _price->setLocalValue(q.value("itemsrcp_price").toDouble());
    _price->setEffective(q.value("itemsrcp_updated").toDate());
    _price->setId(q.value("itemsrcp_curr_id").toInt());
  }
}
