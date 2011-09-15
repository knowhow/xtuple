/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printShippingForm.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "editICMWatermark.h"
#include "mqlutil.h"

printShippingForm::printShippingForm(QWidget* parent, const char * name, Qt::WindowFlags fl)
    : XDialog(parent, name, fl)
{
  setupUi(this);

  // programatically hiding -- see issue # 5853.
  _shipchrg->hide();
  _shipchrgLit->hide();

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sPrint()));
  connect(_shipformNumOfCopies, SIGNAL(valueChanged(int)), this, SLOT(sHandleShippingFormCopies(int)));
  connect(_shipformWatermarks, SIGNAL(itemSelected(int)), this, SLOT(sEditShippingFormWatermark()));
  connect(_shipment,	SIGNAL(newId(int)),	this, SLOT(sHandleShipment()));
  connect(_order,	SIGNAL(numberChanged(QString,QString)),	this, SLOT(sHandleOrder()));

  _captive = FALSE;
  _order->setAllowedTypes(OrderLineEdit::Sales | OrderLineEdit::Transfer);
  _order->setLabel("");
  _shipment->setStatus(ShipmentClusterLineEdit::AllStatus);
  _shipment->setStrict(true);

  _shipformWatermarks->addColumn( tr("Copy #"),      _dateColumn, Qt::AlignCenter );
  _shipformWatermarks->addColumn( tr("Watermark"),   -1,          Qt::AlignLeft   );
  _shipformWatermarks->addColumn( tr("Show Prices"), _dateColumn, Qt::AlignCenter );
  _shipformNumOfCopies->setValue(_metrics->value("ShippingFormCopies").toInt());

  if (_shipformNumOfCopies->value())
  {
    for (int counter = 0; counter < _shipformWatermarks->topLevelItemCount(); counter++)
    {
    _shipformWatermarks->topLevelItem(counter)->setText(1, _metrics->value(QString("ShippingFormWatermark%1").arg(counter)));
    _shipformWatermarks->topLevelItem(counter)->setText(2, ((_metrics->boolean(QString("ShippingFormShowPrices%1").arg(counter))) ? tr("Yes") : tr("No")));
    }
  }
}

printShippingForm::~printShippingForm()
{
  // no need to delete child widgets, Qt does it all for us
}

void printShippingForm::languageChange()
{
  retranslateUi(this);
}

enum SetResponse printShippingForm::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("cosmisc_id", &valid);	// deprecated
  if (valid)
  {
    _shipment->setId(param.toInt());
    q.prepare( "SELECT cohead_id, cohead_shiptoname, cohead_shiptoaddress1, cosmisc_shipchrg_id,"
               "       COALESCE(cosmisc_shipform_id, cohead_shipform_id) AS shipform_id "
               "FROM cosmisc, cohead "
               "WHERE ( (cosmisc_cohead_id=cohead_id)"
               " AND (cosmisc_id=:cosmisc_id) );" );
    q.bindValue(":cosmisc_id", _shipment->id());
    q.exec();
    if (q.first())
    {
      _captive = TRUE;

      _order->setId(q.value("cohead_id").toInt(),"SO");
      _order->setEnabled(FALSE);

      _shipToName->setText(q.value("cohead_shiptoname").toString());
      _shipToAddr1->setText(q.value("cohead_shiptoaddress1").toString());
      _shippingForm->setId(q.value("shipform_id").toInt());
      _shipchrg->setId(q.value("cosmisc_shipchrg_id").toInt());
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
    _buttonBox->setFocus();
  }

  param = pParams.value("shiphead_id", &valid);
  if (valid)
  {
    int orderid = -1;
    QString ordertype;

    _shipment->setId(param.toInt());
    q.prepare( "SELECT shiphead_order_id, shiphead_order_type,"
	       "       shiphead_shipchrg_id, shiphead_shipform_id "
	       "FROM shiphead "
	       "WHERE (shiphead_id=:shiphead_id);" );
    q.bindValue(":shiphead_id", _shipment->id());
    q.exec();
    if (q.first())
    {
      ordertype = q.value("shiphead_order_type").toString();
      orderid = q.value("shiphead_order_id").toInt();
      if (! q.value("shiphead_shipform_id").isNull())
	_shippingForm->setId(q.value("shiphead_shipform_id").toInt());
      _shipchrg->setId(q.value("shiphead_shipchrg_id").toInt());
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }

    ParameterList headp;
    if (ordertype == "SO")
    {
      headp.append("sohead_id", orderid);
      _order->setId(orderid, "SO");
    }
    else if (ordertype == "TO")
    {
      headp.append("tohead_id", orderid);
      _order->setId(orderid,"TO");
    }

    QString heads = "<? if exists(\"sohead_id\") ?>"
		  "SELECT cohead_id AS order_id, cohead_shiptoname AS shipto,"
		  "      cohead_shiptoaddress1 AS addr1,"
		  "      cohead_shipform_id AS shipform_id "
		  "FROM cohead "
		  "WHERE (cohead_id=<? value(\"sohead_id\") ?>);"
		  "<? elseif exists(\"tohead_id\") ?>"
		  "SELECT tohead_id AS order_id, tohead_destname AS shipto,"
		  "      tohead_destaddress1 AS addr1,"
		  "      tohead_shipform_id AS shipform_id "
		  "FROM tohead "
		  "WHERE (tohead_id=<? value(\"tohead_id\") ?>);"
		  "<? endif ?>"
		  ;
    MetaSQLQuery headm(heads);
    XSqlQuery headq = headm.toQuery(headp);
    if (headq.first())
    {
      _captive = TRUE;

      _shipToName->setText(headq.value("shipto").toString());
      _shipToAddr1->setText(headq.value("addr1").toString());
      if (_shippingForm->id() <= 0)
	_shippingForm->setId(headq.value("shipform_id").toInt());

      _order->setEnabled(false);
    }
    else if (headq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, headq.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
    _buttonBox->setFocus();
  }

  return NoError;
}

void printShippingForm::sHandleShippingFormCopies(int pValue)
{
  if (_shipformWatermarks->topLevelItemCount() > pValue)
    _shipformWatermarks->takeTopLevelItem(_shipformWatermarks->topLevelItemCount() - 1);
  else
  {
    XTreeWidgetItem *last = static_cast<XTreeWidgetItem*>(_shipformWatermarks->topLevelItem(_shipformWatermarks->topLevelItemCount() - 1));
    for (int counter = (_shipformWatermarks->topLevelItemCount()); counter <= pValue; counter++)
      last = new XTreeWidgetItem(_shipformWatermarks, last, counter, QVariant(tr("Copy #%1").arg(counter)), QVariant(""), QVariant(tr("Yes")));
  }
}

void printShippingForm::sEditShippingFormWatermark()
{
  QList<XTreeWidgetItem*>selected = _shipformWatermarks->selectedItems();
  for (int counter = 0; counter < selected.size(); counter++)
  {
    XTreeWidgetItem *cursor = static_cast<XTreeWidgetItem*>(selected[counter]);
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
}

void printShippingForm::sPrint()
{
  if (!_shipment->isValid())
  {
    QMessageBox::warning(this, tr("Shipment Number Required"),
			       tr("<p>You must enter a Shipment Number.") );
    _shipment->setFocus();
    return;
  }

  if (_shippingForm->id() == -1)
  {
    QMessageBox::warning( this, tr("Cannot Print Shipping Form"),
                          tr("<p>You must select a Shipping Form to print.") );
    _shippingForm->setFocus();
    return;
  }

  q.prepare("SELECT shipform_report_name AS report_name "
	    "  FROM shipform "
	    " WHERE((shipform_id=:shipform_id));" );
  q.bindValue(":shipform_id", _shippingForm->id());
  q.exec();
  if (q.first())
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

    for (int counter = 0; counter < _shipformWatermarks->topLevelItemCount(); counter++ )
    {
      QTreeWidgetItem *cursor = _shipformWatermarks->topLevelItem(counter);
      ParameterList params;
      params.append("cosmisc_id",  _shipment->id()); // for backwards compat
      params.append("shiphead_id", _shipment->id());
      params.append("watermark",   cursor->text(1));
      params.append("shipchrg_id", _shipchrg->id());

      if (_metrics->boolean("MultiWhs"))
	params.append("MultiWhs");

      if (cursor->text(2) == tr("Yes"))
        params.append("showcosts");

      orReport report(q.value("report_name").toString(), params);
      if (report.print(&printer, setupPrinter))
        setupPrinter = FALSE;
      else
      {
        report.reportError(this);
	orReport::endMultiPrint(&printer);
        return;
      }
    }
    orReport::endMultiPrint(&printer);

    q.prepare( "UPDATE shiphead "
               "SET shiphead_sfstatus='P' "
               "WHERE (shiphead_id=:shiphead_id);" );
    q.bindValue(":shiphead_id", _shipment->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    if (_captive)
      accept();
    else
    {
      _shipment->setId(-1);
      _order->setId(-1);
      _order->setEnabled(true);
      _order->setFocus();
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    systemError(this, tr("<p>The Shipping Head Record cannot be found for the "
			 "selected order."));
}

void printShippingForm::sHandleShipment()
{
  if (_shipment->isValid())
  {
    ParameterList params;
    MetaSQLQuery mql = mqlLoad("shippingForm", "shipment");
    params.append("shiphead_id", _shipment->id());
    if (_metrics->boolean("MultiWhs"))
      params.append("MultiWhs");
    if (_order->isValid() && _order->isSO())
      params.append("sohead_id", _order->id());
    if (_order->isValid() && _order->isTO())
      params.append("tohead_id", _order->id());
    q = mql.toQuery(params);

    if (q.first())
    {
      int orderid = q.value("order_id").toInt();
      if ((q.value("shiphead_order_type").toString() == "SO") &&
          ((_order->id() != orderid) || (!_order->isSO())))
        _order->setId(orderid, "SO");
      else if ((q.value("shiphead_order_type").toString() == "TO") &&
               ((_order->id() != orderid) || (!_order->isTO())))
        _order->setId(orderid,"TO");

      _shipToName->setText(q.value("shipto").toString());
      _shipToAddr1->setText(q.value("addr1").toString());
      _shippingForm->setId(q.value("shipform_id").toInt());
      _shipchrg->setId(q.value("shiphead_shipchrg_id").toInt());
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else
    {
      QMessageBox::critical(this, tr("Could not find data"),
                            tr("<p>Could not find shipment on this order."));

      _shipment->setId(-1);
    }
  }
  else
  {
    depopulate();
  }
}

void printShippingForm::sHandleSo()
{
  _shipment->clear();
  _shipment->setType(ShipmentClusterLineEdit::SalesOrder);
  _shipment->limitToOrder(_order->id());

  QString sql("SELECT cohead_id AS order_id, cohead_shiptoname AS shipto, "
              "       cohead_shiptoaddress1 AS addr1, shiphead_order_type, "
              "       shiphead_id, shiphead_shipchrg_id, shiphead_shipped, "
              "	COALESCE(shiphead_shipform_id, cohead_shipform_id) AS shipform_id "
              "FROM cohead, shiphead "
              "WHERE ((cohead_id=shiphead_order_id)"
              "  AND  (shiphead_order_type='SO')"
              "  AND  (cohead_id=<? value(\"sohead_id\") ?> )"
              "<? if exists(\"shiphead_id\") ?>"
              "  AND  (shiphead_id=<? value(\"shiphead_id\") ?> )"
              "<? endif ?>"
              ") "
              "ORDER BY shiphead_shipped "
              "LIMIT 1;");

  ParameterList params;
  MetaSQLQuery mql(sql);
  params.append("sohead_id", _order->id());
  if (_shipment->isValid())
    params.append("shiphead_id", _shipment->id());
  q = mql.toQuery(params);

  if (q.first())
  {
    if (_shipment->id() != q.value("shiphead_id").toInt())
      _shipment->setId(q.value("shiphead_id").toInt());

    _shipToName->setText(q.value("shipto").toString());
    _shipToAddr1->setText(q.value("addr1").toString());
    _shippingForm->setId(q.value("shipform_id").toInt());
    _shipchrg->setId(q.value("shiphead_shipchrg_id").toInt());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    depopulate();
}

void printShippingForm::sHandleOrder()
{
  if (_order->isSO())
    sHandleSo();
  else if (_order->isTO())
    sHandleTo();
  else
  {
    _shipment->removeOrderLimit();
    _shipment->clear();
    depopulate();
  }
}

void printShippingForm::sHandleTo()
{
  _shipment->clear();
  _shipment->setType(ShipmentClusterLineEdit::TransferOrder);
  _shipment->limitToOrder(_order->id());

  QString sql("SELECT tohead_id AS order_id, tohead_destname AS shipto, "
              "       tohead_destaddress1 AS addr1, shiphead_order_type, "
              "       shiphead_id, shiphead_shipchrg_id, shiphead_shipped, "
              "	COALESCE(shiphead_shipform_id, tohead_shipform_id) AS shipform_id "
              "FROM tohead, shiphead "
              "WHERE ((tohead_id=shiphead_order_id)"
              "  AND  (shiphead_order_type='TO')"
              "  AND  (tohead_id=<? value(\"tohead_id\") ?> )"
              "<? if exists(\"shiphead_id\") ?>"
              "  AND  (shiphead_id=<? value(\"shiphead_id\") ?> )"
              "<? endif ?>"
              ") "
              "ORDER BY shiphead_shipped "
              "LIMIT 1;");

  ParameterList params;
  MetaSQLQuery mql(sql);
  params.append("tohead_id", _order->id());
  if (_shipment->isValid())
    params.append("shiphead_id", _shipment->id());
  q = mql.toQuery(params);

  if (q.first())
  {
    if (_shipment->id() != q.value("shiphead_id").toInt())
      _shipment->setId(q.value("shiphead_id").toInt());

    _shipToName->setText(q.value("shipto").toString());
    _shipToAddr1->setText(q.value("addr1").toString());
    _shippingForm->setId(q.value("shipform_id").toInt());
    _shipchrg->setId(q.value("shiphead_shipchrg_id").toInt());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    depopulate();
}

void printShippingForm::depopulate()
{
  //_shipment->clear();
  _shipToName->clear();
  _shipToAddr1->clear();
  _shippingForm->setId(-1);
  _shipchrg->setId(-1);
}
