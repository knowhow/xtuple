/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "reschedulePoitem.h"

#include <QMessageBox>
#include <QVariant>

reschedulePoitem::reschedulePoitem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_reschedule, SIGNAL(clicked()), this, SLOT(sReschedule()));
  connect(_po, SIGNAL(valid(bool)), _poitem, SLOT(setEnabled(bool)));
  connect(_po, SIGNAL(newId(int, QString)), this, SLOT(sPopulatePoitem(int)));
  connect(_poitem, SIGNAL(newID(int)), this, SLOT(sPopulate(int)));
  connect(_poitem, SIGNAL(valid(bool)), _reschedule, SLOT(setEnabled(bool)));
}

reschedulePoitem::~reschedulePoitem()
{
  // no need to delete child widgets, Qt does it all for us
}

void reschedulePoitem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse reschedulePoitem::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("pohead_id", &valid);
  if (valid)
  {
    _po->setId(param.toInt());
    _po->setReadOnly(true);
    _new->setFocus();
  }

  param = pParams.value("poitem_id", &valid);
  if (valid)
  {
    q.prepare( "SELECT pohead_id "
               "FROM pohead, poitem "
               "WHERE ((poitem_pohead_id=pohead_id)"
               "  AND (pohead_status='O')"
               "  AND (poitem_id=:poitem_id));" );
    q.bindValue(":poitem_id", param.toInt());
    q.exec();
    if (q.first())
    {
      _po->setId(q.value("pohead_id").toInt());
      _po->setReadOnly(true);
      _poitem->setId(param.toInt());
    }
    else
    {
      QMessageBox::critical(this, tr("P/O Not Open"),
        tr("The P/O line item you are trying to reschedule does\n"
           "not belong to an Open P/O.") );
      return UndefinedError;
    }

    _new->setFocus();
  }

  return NoError;
}

void reschedulePoitem::sPopulatePoitem(int pPoheadid)
{
  q.prepare( "SELECT poitem_id,"
             "       ( poitem_linenumber || '-' ||"
             "         COALESCE(item_number, poitem_vend_item_number) || ' (' ||"
             "         COALESCE(item_descrip1, firstLine(poitem_vend_item_descrip)) || ')' ) "
             "FROM poitem LEFT OUTER JOIN "
             "     ( itemsite JOIN item "
             "       ON (itemsite_item_id=item_id)"
             "     ) ON (poitem_itemsite_id=itemsite_id) "
             "WHERE ( (poitem_status <> 'C')"
             " AND (poitem_pohead_id=:pohead_id) ) "
             "ORDER BY poitem_linenumber;" );
  q.bindValue(":pohead_id", pPoheadid);
  q.exec();
  _poitem->populate(q);
}

void reschedulePoitem::sPopulate(int pPoitemid)
{
  if (pPoitemid == -1)
  {
    _current->clear();
    _new->clear();
  }
  else
  {
    q.prepare( "SELECT poitem_duedate "
               "FROM poitem "
               "WHERE (poitem_id=:poitem_id);" );
    q.bindValue(":poitem_id", pPoitemid);
    q.exec();
    if (q.first())
    {
      _current->setDate(q.value("poitem_duedate").toDate());
    }
  }
}

void reschedulePoitem::sReschedule()
{
  if (!_new->isValid())
  {
    QMessageBox::critical( this, tr("Invalid Reschedule Date"),
                           tr("<p>You must enter a reschedule due date before "
                              "you may save this Purchase Order Item.") );
    _new->setFocus();
    return;
  }

  q.prepare("SELECT changePoitemDueDate(:poitem_id, :dueDate);");
  q.bindValue(":poitem_id", _poitem->id());
  q.bindValue(":dueDate", _new->date());
  q.exec();

  omfgThis->sPurchaseOrdersUpdated(_po->id(), true);

  accept();
}

