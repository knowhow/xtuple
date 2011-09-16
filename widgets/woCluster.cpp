/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QLabel>
#include <QPushButton>
#include <QLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QtScript>

#include <parameter.h>
#include <xsqlquery.h>
#include <QMessageBox>

#include "warehousegroup.h"
#include "xcombobox.h"
#include "xlineedit.h"

#include "wocluster.h"

#include "../common/format.h"

void setupWoCluster(QScriptEngine *engine)
{
  QScriptValue widget = engine->newObject();

  widget.setProperty("Open", QScriptValue(engine, WoLineEdit::Open), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Exploded", QScriptValue(engine, WoLineEdit::Exploded), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Issued", QScriptValue(engine, WoLineEdit::Issued), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Released", QScriptValue(engine, WoLineEdit::Released), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Closed", QScriptValue(engine, WoLineEdit::Closed), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("WoLineEdit", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}

WoLineEdit::WoLineEdit(QWidget *pParent, const char *pName) :
    VirtualClusterLineEdit(pParent, "wo", "wo_id", "wo_number::text || '-' || wo_subnumber::text", "item_number", "(item_descrip1 || ' ' || item_descrip2) ", 0, pName)
{
  _woType = 0;
  init();
}

WoLineEdit::WoLineEdit(int pWoType, QWidget* pParent, const char* pName) :
    VirtualClusterLineEdit(pParent, "wo", "wo_id", "wo_number::text || '-' || wo_subnumber::text", "item_number", "(item_descrip1 || ' ' || item_descrip2) ", 0, pName)
{
  _woType = pWoType;
  init();
}

void WoLineEdit::init()
{
  setTitles(tr("Work Order"), tr("Work Orders"));
  setUiName("workOrder");
  setEditPriv("MaintainWorkOrders");
  setViewPriv("ViewWorkOrders");
  setNewPriv("MaintainWorkOrders");

  _warehouseid = -1;

  _qtyOrdered = 0.0;
  _qtyReceived = 0.0;

  _query =  QString("SELECT wo_id AS id, "
                    "       wo_number::text || '-' || wo_subnumber::text AS number,"
                    "       item_number AS name, "
                    "       (item_descrip1 || ' ' || item_descrip2) AS description, "
                    "       warehous_id,"
                    "       warehous_code, item_id, item_number, uom_name,"
                    "       item_descrip1, "
                    "       item_descrip2,"
                    "       abs(wo_qtyord) AS wo_qtyord,"
                    "       abs(wo_qtyrcv) AS wo_qtyrcv, "
                    "       CASE WHEN (wo_status = 'O') THEN '%3' "
                    "       WHEN (wo_status = 'E') THEN '%4' "
                    "       WHEN (wo_status = 'I') THEN '%5' "
                    "       WHEN (wo_status = 'R') THEN '%6' "
                    "       WHEN (wo_status = 'C') THEN '%7' "
                    "       END AS wo_status,"
                    "       wo_duedate,"
                    "       wo_startdate,"
                    "       noNeg(abs(wo_qtyord) - abs(wo_qtyrcv)) AS balance, "
                    "       CASE WHEN (wo_qtyord >= 0) THEN "
                    "         '%1' "
                    "       ELSE "
                    "         '%2' "
                    "       END AS wo_method, "
                    "       CASE WHEN (wo_qtyord >= 0) THEN "
                    "         'A' "
                    "       ELSE "
                    "         'D' "
                    "       END AS method "
                    "FROM wo "
                    "  JOIN itemsite ON (wo_itemsite_id=itemsite_id) "
                    "  JOIN item ON (itemsite_item_id=item_id) "
                    "  JOIN whsinfo ON (itemsite_warehous_id=warehous_id) "
                    "  JOIN uom ON (item_inv_uom_id=uom_id) "
                    "WHERE (true) " )
      .arg(tr("Assembly"))
      .arg(tr("Disassembly"))
      .arg(tr("Open"))
      .arg(tr("Exploded"))
      .arg(tr("In Process"))
      .arg(tr("Released"))
      .arg(tr("Closed"));
}

void WoLineEdit::silentSetId(const int pId)
{
  if (pId != -1)
  {
    XSqlQuery wo;

    wo.prepare(_query + _idClause +
               (_extraClause.isEmpty() || !_strict ? "" : " AND " + _extraClause) +
               QString(";"));
    wo.bindValue(":id", pId);
    wo.exec();
    if (wo.first())
    {
      if (_completer)
        static_cast<QSqlQueryModel* >(_completer->model())->setQuery(QSqlQuery());

      _id    = pId;
      _valid = TRUE;

      setText(wo.value("number").toString());

      _qtyOrdered  = wo.value("wo_qtyord").toDouble();
      _qtyReceived = wo.value("wo_qtyrcv").toDouble();
      _method = wo.value("method").toString();
      _currentWarehouseid = (wo.value("warehous_id").isNull())
                                      ? -1 : wo.value("warehous_id").toInt();

      emit newId(_id);
      emit newItemid(wo.value("item_id").toInt());
      emit warehouseChanged(wo.value("warehous_code").toString());
      emit itemNumberChanged(wo.value("item_number").toString());
      emit uomChanged(wo.value("uom_name").toString());
      emit itemDescrip1Changed(wo.value("item_descrip1").toString());
      emit itemDescrip2Changed(wo.value("item_descrip2").toString());
      emit startDateChanged(wo.value("wo_startdate").toDate());
      emit dueDateChanged(wo.value("wo_duedate").toDate());
      emit qtyOrderedChanged(wo.value("wo_qtyord").toDouble());
      emit qtyReceivedChanged(wo.value("wo_qtyrcv").toDouble());
      emit qtyBalanceChanged(wo.value("balance").toDouble());
      emit statusChanged(wo.value("wo_status").toString());
      emit methodChanged(wo.value("wo_method").toString());
      emit valid(TRUE);
    }
  }
  else
  {
    _id    = -1;
    _valid = FALSE;
    _currentWarehouseid = -1;

    XLineEdit::clear();

    emit newId(-1);
    emit newItemid(-1);
    emit warehouseChanged("");
    emit itemNumberChanged("");
    emit uomChanged("");
    emit itemDescrip1Changed("");
    emit itemDescrip2Changed("");
    emit startDateChanged(QDate());
    emit dueDateChanged(QDate());
    emit qtyOrderedChanged(0);
    emit qtyReceivedChanged(0);
    emit qtyBalanceChanged(0);
    emit statusChanged("");
    emit methodChanged("");
    emit valid(FALSE);
      
    _qtyOrdered  = 0;
    _qtyReceived = 0;
  }

  _parsed = TRUE;
  emit parsed();
}

void WoLineEdit::sList()
{
  disconnect(this, SIGNAL(editingFinished()), this, SLOT(sParse()));

  woList* newdlg = listFactory();
  if (newdlg)
  {
    ParameterList params;
    params.append("wo_id", _id);
    params.append("woType", _woType);

    if (_warehouseid != -1)
      params.append("warehous_id", _warehouseid);

    newdlg->set(params);

    int id = newdlg->exec();
    setId(id);
  }
  else
    QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__),
                          tr("%1::sList() not yet defined")
                          .arg(metaObject()->className()));

  connect(this, SIGNAL(editingFinished()), this, SLOT(sParse()));
}

void WoLineEdit::sSearch()
{
  disconnect(this, SIGNAL(editingFinished()), this, SLOT(sParse()));

  woSearch* newdlg = searchFactory();
  if (newdlg)
  {
    ParameterList params;
    params.append("wo_id", _id);
    params.append("woType", _woType);

    if (_warehouseid != -1)
      params.append("warehous_id", _warehouseid);

    newdlg->set(params);

    int id = newdlg->exec();
    setId(id);
  }
  else
    QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__),
                          tr("%1::sList() not yet defined")
                          .arg(metaObject()->className()));

  connect(this, SIGNAL(editingFinished()), this, SLOT(sParse()));
}

woList* WoLineEdit::listFactory()
{
  return new woList(this);
}

woSearch* WoLineEdit::searchFactory()
{
  return new woSearch(this);
}

void WoLineEdit::setType(int pWoType)
{
  if (_woType == pWoType)
    return;

  _woType = pWoType;

  buildExtraClause();
}

void WoLineEdit::setWarehouse(int pWarehouseid)
{
  if (_warehouseid == pWarehouseid)
    return;

  _warehouseid = pWarehouseid;

  buildExtraClause();
}

void WoLineEdit::buildExtraClause()
{
  if (!_woType && !_warehouseid)
  {
    _extraClause.clear();
    return;
  }

  if (_woType)
  {
    QStringList statuses;
    if (_woType & cWoOpen)
      statuses << "'O'";

    if (_woType & cWoExploded)
      statuses << "'E'";

    if (_woType & cWoReleased)
      statuses << "'R'";

    if (_woType & cWoIssued)
      statuses << "'I'";

    if (_woType & cWoClosed)
      statuses << "'C'";

    if(!statuses.isEmpty())
      _extraClause = "(wo_status IN (" + statuses.join(",") + "))";
  }

  if (!_extraClause.isEmpty() && _warehouseid > 0)
    _extraClause += " AND ";

  if (_warehouseid > 0)
    _extraClause += QString(" (warehous_id=%1) ").arg(_warehouseid);
}

//////////////////////////////////////////////////

WoCluster::WoCluster(QWidget *pParent, const char *name) :
  QWidget(pParent)
{
  setObjectName(name);
  constructor();
}

WoCluster::WoCluster(int pWoType, QWidget *pParent, const char *name) :
  QWidget(pParent)
{
  setObjectName(name);
  constructor();

  _woNumber->setType(pWoType);
}

void WoCluster::constructor()
{
  _mainLayout      = new QVBoxLayout(this);
  _woLayout        = new QHBoxLayout(0);
  _warehouseLayout = new QHBoxLayout(0);
  _line1Layout     = new QHBoxLayout(0);
  _itemLayout      = new QHBoxLayout(0);
  _uomLayout       = new QHBoxLayout(0);
  _line2Layout     = new QHBoxLayout(0);
  _statusLayout    = new QHBoxLayout(0);

  _mainLayout->setContentsMargins(0, 0, 0, 0);
  _woLayout->setContentsMargins(0, 0, 0, 0);
  _warehouseLayout->setContentsMargins(0, 0, 0, 0);
  _line1Layout->setContentsMargins(0, 0, 0, 0);
  _itemLayout->setContentsMargins(0, 0, 0, 0);
  _uomLayout->setContentsMargins(0, 0, 0, 0);
  _line2Layout->setContentsMargins(0, 0, 0, 0);
  _statusLayout->setContentsMargins(0, 0, 0, 0);

  _mainLayout->setSpacing(2); 
  _woLayout->setSpacing(5); 
  _warehouseLayout->setSpacing(5); 
  _line1Layout->setSpacing(7); 
  _itemLayout->setSpacing(5); 
  _uomLayout->setSpacing(5); 
  _line2Layout->setSpacing(7); 
  _statusLayout->setSpacing(5); 

  _woNumberLit = new QLabel(tr("Work Order #:"), this);
  _woNumberLit->setObjectName("woNumberLit");
  _woNumberLit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _woNumberLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  _woLayout->addWidget(_woNumberLit);

  _woNumber = new WoLineEdit(this);
  _woNumber->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _woLayout->addWidget(_woNumber);

  _line1Layout->addLayout(_woLayout);

  QLabel *warehouseLit = new QLabel(tr("Site:"), this);
  warehouseLit->setObjectName("warehouseLit");
  _woNumber->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  warehouseLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  _warehouseLayout->addWidget(warehouseLit);

  _warehouse = new QLabel(this);
  _warehouse->setObjectName("_warehouse");
  _warehouse->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  _warehouse->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  _warehouseLayout->addWidget(_warehouse);
  _line1Layout->addLayout(_warehouseLayout);
  _mainLayout->addLayout(_line1Layout);

  _itemNumberLit = new QLabel(tr("Item Number:"), this);
  _itemNumberLit->setObjectName("itemNumberLit");
  _itemNumberLit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _itemNumberLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  _itemLayout->addWidget(_itemNumberLit);

  _itemNumber = new QLabel(this);
  _itemNumber->setObjectName("_itemNumber");
  _itemNumber->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _itemNumber->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  _itemLayout->addWidget(_itemNumber);
  _line2Layout->addLayout(_itemLayout);

  _uomLit = new QLabel(tr("UOM:"), this);
  _uomLit->setObjectName("uomLit");
  _uomLit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _uomLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  _uomLayout->addWidget(_uomLit);

  _uom = new QLabel(this);
  _uom->setObjectName("_uom");
  _uom->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  _uom->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  _uomLayout->addWidget(_uom);
  _line2Layout->addLayout(_uomLayout);
  _mainLayout->addLayout(_line2Layout);

  _descrip1 = new QLabel(this);
  _descrip1->setObjectName("_descrip1");
  _descrip1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _descrip1->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  _mainLayout->addWidget(_descrip1);

  _descrip2 = new QLabel(this);
  _descrip2->setObjectName("_descrip2");
  _descrip2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _descrip2->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  _mainLayout->addWidget(_descrip2);

  _statusLit = new QLabel(tr("Status:"), this);
  _statusLit->setObjectName("statusLit");
  _statusLit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _statusLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  _statusLayout->addWidget(_statusLit);

  _status = new QLabel(this);
  _status->setObjectName("_status");
  _status->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _status->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  _statusLayout->addWidget(_status);

  _methodLit = new QLabel(tr("Method:"), this);
  _methodLit->setObjectName("methodLit");
  _methodLit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _methodLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  _statusLayout->addWidget(_methodLit);

  _method = new QLabel(this);
  _method->setObjectName("_method");
  _method->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _method->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  _statusLayout->addWidget(_method);

  _mainLayout->addLayout(_statusLayout);
  _orientation = Qt::Horizontal;
  setOrientation(Qt::Vertical);

//  Make some internal connections
  connect(_woNumber, SIGNAL(itemNumberChanged(const QString &)), _itemNumber, SLOT(setText(const QString &)));
  connect(_woNumber, SIGNAL(uomChanged(const QString &)), _uom, SLOT(setText(const QString &)));
  connect(_woNumber, SIGNAL(itemDescrip1Changed(const QString &)), _descrip1, SLOT(setText(const QString &)));
  connect(_woNumber, SIGNAL(itemDescrip2Changed(const QString &)), _descrip2, SLOT(setText(const QString &)));
  connect(_woNumber, SIGNAL(warehouseChanged(const QString &)), _warehouse, SLOT(setText(const QString &)));
  connect(_woNumber, SIGNAL(statusChanged(const QString &)), _status, SLOT(setText(const QString &)));
  connect(_woNumber, SIGNAL(methodChanged(const QString &)), _method, SLOT(setText(const QString &)));

  connect(_woNumber, SIGNAL(newId(int)), this, SIGNAL(newId(int)));
  connect(_woNumber, SIGNAL(newItemid(int)), this, SIGNAL(newItemid(int)));
  connect(_woNumber, SIGNAL(startDateChanged(const QDate &)), this, SIGNAL(startDateChanged(const QDate &)));
  connect(_woNumber, SIGNAL(dueDateChanged(const QDate &)), this, SIGNAL(dueDateChanged(const QDate &)));
  connect(_woNumber, SIGNAL(qtyOrderedChanged(const double)),  this, SIGNAL(qtyOrderedChanged(const double)));
  connect(_woNumber, SIGNAL(qtyReceivedChanged(const double)), this, SIGNAL(qtyReceivedChanged(const double)));
  connect(_woNumber, SIGNAL(qtyBalanceChanged(const double)),  this, SIGNAL(qtyBalanceChanged(const double)));
  connect(_woNumber, SIGNAL(valid(bool)), this, SIGNAL(valid(bool)));

  setFocusProxy(_woNumber);
}

void WoCluster::setReadOnly(bool pReadOnly)
{
  if (pReadOnly)
    _woNumber->setEnabled(FALSE);
  else
    _woNumber->setEnabled(TRUE);
}

void WoCluster::setDataWidgetMap(XDataWidgetMapper* m)
{
  m->addMapping(this, _fieldName, "number", "defaultNumber");
  //_woNumber->_mapper=m;
}

void WoCluster::setWoNumber(const QString& number)
{
  if (_woNumber->text() == number)
    return;
    
  _woNumber->setText(number);
  _woNumber->sParse();
}

void WoCluster::setId(int pId)
{
  _woNumber->setId(pId);
}

QString WoCluster::woNumber() const
{
  return _woNumber->text();
}

Qt::Orientation WoCluster::orientation()
{
  return _orientation;
}

void WoCluster::setOrientation(Qt::Orientation orientation)
{
  if (orientation == _orientation)
    return;

  if (orientation == Qt::Vertical)
  {
    _line1Layout->removeItem(_itemLayout);
    _line2Layout->addLayout(_itemLayout);
    _descrip1->setVisible(true);
    _descrip2->setVisible(true);
    _statusLit->setVisible(true);
    _status->setVisible(true);
    _uom->setVisible(true);
    _uomLit->setVisible(true);
    _method->setVisible(true);
    _methodLit->setVisible(true);
  }
  else
  {
    _line2Layout->removeItem(_itemLayout);
    _line1Layout->addLayout(_itemLayout);
    _descrip1->setVisible(false);
    _descrip2->setVisible(false);
    _statusLit->setVisible(false);
    _status->setVisible(false);
    _uom->setVisible(false);
    _uomLit->setVisible(false);
    _method->setVisible(false);
    _methodLit->setVisible(false);
  }

  _orientation = orientation;
}

QString WoCluster::label()
{
  return _woNumberLit->text();
}

void WoCluster::setLabel(const QString& p)
{
  _woNumberLit->setText(p);
}

WomatlCluster::WomatlCluster(QWidget *parent, const char *name) : QWidget(parent)
{
  setObjectName(name);
  constructor();
}

WomatlCluster::WomatlCluster(WoCluster *wocParent, QWidget *parent, const char *name) : QWidget(parent)
{
  setObjectName(name);
  constructor();

  setGeometry(0, 0, parent->width(), parent->height());

  _sense = 1;

  connect(wocParent, SIGNAL(newId(int)), SLOT(setWoid(int)));
  connect(this, SIGNAL(newId(int)), wocParent, SLOT(sPopulateInfo(int)));
}

void WomatlCluster::constructor()
{
  setupUi(this);

  _valid  = FALSE;
  _id     = -1;
  _woid   = -1;
  _type   = (Push | Pull | Mixed);

  _source = WoMaterial;
  _sourceId = -1;

  _required = 0.0;
  _issued = 0.0;

  connect(_itemNumber, SIGNAL(newID(int)), SLOT(sPopulateInfo(int)));

  if(_x_metrics)
  {
    _qtyIssued->setPrecision(decimalPlaces("qty"));
    _qtyFxd->setPrecision(decimalPlaces("qty"));
    _qtyPer->setPrecision(decimalPlaces("qtyper"));
    _qtyRequired->setPrecision(decimalPlaces("qty"));
    _scrap->setPrecision(decimalPlaces("percent"));
  }

  setFocusProxy(_itemNumber);
}

void WomatlCluster::languageChange()
{
  retranslateUi(this);
}

void WomatlCluster::setReadOnly(bool)
{
}

void WomatlCluster::setWooperid(int pWooperid)
{
  _source = Wooper;
  _sourceId = pWooperid;

  bool qual = FALSE;
  QString sql( "SELECT womatl_id AS womatlid, item_number,"
               "       wo_id, uom_name, item_descrip1, item_descrip2,"
               "       womatl_qtyfxd AS _qtyfxd, womatl_qtyreq AS _qtyreq, womatl_qtyiss AS _qtyiss,"
               "       formatQty(womatl_qtyfxd) AS qtyfxd,"
               "       formatQtyPer(womatl_qtyper) AS qtyper,"
               "       formatScrap(womatl_scrap) AS scrap,"
               "       formatQtyPer(womatl_qtyreq) AS qtyreq,"
               "       formatQtyPer(womatl_qtyiss) AS qtyiss,"
               "       formatQtyPer(womatl_qtywipscrap) AS qtywipscrap "
               "FROM womatl, wo, itemsite, item, uom "
               "WHERE ( (womatl_wo_id=wo_id)"
               " AND (womatl_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (womatl_uom_id=uom_id)"
               " AND (womatl_wooper_id=:wooper_id)"
               " AND (womatl_issuemethod IN (" );

  if (_type & Push)
  {
    sql += "'S'";
    qual = TRUE;
  }

  if (_type & Pull)
  {
    if (qual)
      sql += ",";
    else
      qual = TRUE;

    sql += "'L'";
  }

  if (_type & Mixed)
  {
    if (qual)
      sql += ",";

    sql += "'M'";
  }

  sql += ")) );";

  XSqlQuery query;
  query.prepare(sql);
  query.bindValue(":wooper_id", pWooperid);
  query.exec();
  if (query.first())
  {
    _womatl.prepare(sql);
    _womatl.bindValue(":wooper_id", pWooperid);
    _womatl.exec();
    _itemNumber->populate(query);
  }
  else
  {
    _id = -1;
    _woid = -1;
    _valid = FALSE;
    _required = 0.0;
    _issued  = 0.0;
    
    emit newId(-1);
    emit newQtyRequired(0.0);
    emit newQtyIssued(0.0);
    emit newQtyScrappedFromWIP(0.0);

    _itemNumber->clear();
  }
}

void WomatlCluster::setWoid(int pWoid)
{
  _source = WorkOrder;
  _sourceId = pWoid;

  bool qual = FALSE;
  QString sql( "SELECT womatl_id AS womatlid, item_number,"
               "       wo_id, wo_qtyord, uom_name, item_descrip1, item_descrip2,"
               "       womatl_qtyreq AS _qtyreq, womatl_qtyiss AS _qtyiss,"
               "       womatl_qtyfxd AS qtyfxd, womatl_qtyper AS qtyper,"
               "       womatl_scrap * 100 AS scrap,"
               "       ABS(womatl_qtyreq) AS qtyreq,"
               "       ABS(womatl_qtyiss)  AS qtyiss,"
               "       womatl_qtywipscrap AS qtywipscrap "
               "FROM womatl, wo, itemsite, item, uom "
               "WHERE ( (womatl_wo_id=wo_id)"
               " AND (womatl_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (womatl_uom_id=uom_id)"
               " AND (wo_id=:wo_id)"
               " AND (womatl_issuemethod IN (" );

  if (_type & Push)
  {
    sql += "'S'";
    qual = TRUE;
  }

  if (_type & Pull)
  {
    if (qual)
      sql += ",";
    else
      qual = TRUE;

    sql += "'L'";
  }

  if (_type & Mixed)
  {
    if (qual)
      sql += ",";

    sql += "'M'";
  }

  sql += ")) );";

  _womatl.prepare(sql);
  _womatl.bindValue(":wo_id", pWoid);
  _womatl.exec();
  _itemNumber->populate(_womatl);
  if (_womatl.first())
  {
    if (_womatl.value("wo_qtyord").toDouble() < 0)
    {
      _qtyRequiredLit->setText("Qty. to Return:");
      _qtyIssuedLit->setText("Qty. Returned:");
      _sense = -1;
    }
    else
    {
      _qtyRequiredLit->setText("Qty. Required:");
      _qtyIssuedLit->setText("Qty. Issued:");
      _sense = 1;
    }
  }
  else
  {
    _id = -1;
    _woid = -1;
    _valid = FALSE;
    _required = 0.0;
    _issued  = 0.0;
    
    emit newId(-1);
    emit newQtyRequired(0.0);
    emit newQtyIssued(0.0);
    emit newQtyScrappedFromWIP(0.0);

    _itemNumber->clear();
  }
}

void WomatlCluster::setId(int pWomatlid)
{
  _source = WoMaterial;
  _sourceId = pWomatlid;

  if (pWomatlid == -1)
    sPopulateInfo(-1);

  else
  {
    bool qual = FALSE;
    QString sql( "SELECT list.womatl_id AS womatlid, item_number, "
                 "       wo_id, uom_name, item_descrip1, item_descrip2,"
                 "       ABS(list.womatl_qtyreq) AS _qtyreq, "
                 "       ABS(list.womatl_qtyiss) AS _qtyiss,"
                 "       (list.womatl_qtyfxd) AS qtyfxd,"
                 "       (list.womatl_qtyper) AS qtyper,"
                 "       (list.womatl_scrap * 100) AS scrap,"
                 "       (abs(list.womatl_qtyreq)) AS qtyreq, "
                 "       (abs(list.womatl_qtyiss)) AS qtyiss, "
                 "       (list.womatl_qtywipscrap) AS qtywipscrap "
                 "FROM womatl AS list, womatl AS target, wo, itemsite, item, uom "
                 "WHERE ( (list.womatl_wo_id=wo_id)"
                 " AND (target.womatl_wo_id=wo_id)"
                 " AND (list.womatl_itemsite_id=itemsite_id)"
                 " AND (itemsite_item_id=item_id)"
                 " AND (list.womatl_uom_id=uom_id)"
                 " AND (target.womatl_id=:womatl_id)"
                 " AND (list.womatl_issuemethod IN (" );

    if (_type & Push)
    {
      sql += "'S'";
      qual = TRUE;
    }

    if (_type & Pull)
    {
      if (qual)
        sql += ",";
      else
        qual = TRUE;

      sql += "'L'";
    }

    if (_type & Mixed)
    {
      if (qual)
        sql += ",";

      sql += "'M'";
    }

    sql += ")) );";

    XSqlQuery query;
    query.prepare(sql);
    query.bindValue(":womatl_id", pWomatlid);
    query.exec();
    if (query.first())
    {
      _womatl.prepare(sql);
      _womatl.bindValue(":womatl_id", pWomatlid);
      _womatl.exec();

      emit newId(pWomatlid);

      _valid = TRUE;
      _id = pWomatlid;

      _itemNumber->populate(query);
      _itemNumber->setId(pWomatlid);
    }
    else
    {
      _valid = FALSE;
      _woid = -1;
      _id = -1;

      emit newId(-1);
      emit valid(FALSE);

      _itemNumber->clear();
    }
  }
}

void WomatlCluster::setType(int pType)
{
  if (pType)
    _type = pType;
}

void WomatlCluster::sRefresh()
{
  if (_source == WorkOrder)
    setWoid(_sourceId);
  else if (_source == Wooper)
    setWooperid(_sourceId);
  else if (_source == WoMaterial)
    setId(_sourceId);
}

void WomatlCluster::sPopulateInfo(int pWomatlid)
{
  if (pWomatlid == -1)
  {
    _itemNumber->setCurrentIndex(0);
    _uom->setText("");
    _descrip1->setText("");
    _descrip2->setText("");
    _qtyFxd->setText("");
    _qtyPer->setText("");
    _scrap->setText("");
    _qtyRequired->setText("");
    _qtyIssued->setText("");

    _id = -1;
    _valid = FALSE;
    _required = 0;
    _issued = 0;

    emit newId(-1);
    emit newQtyScrappedFromWIP(0.0);
    emit valid(FALSE);
  }
  else if (_womatl.findFirst("womatlid", pWomatlid) != -1)
  {
    _uom->setText(_womatl.value("uom_name").toString());
    _descrip1->setText(_womatl.value("item_descrip1").toString());
    _descrip2->setText(_womatl.value("item_descrip2").toString());
    _qtyFxd->setDouble(_womatl.value("qtyfxd").toDouble());
    _qtyPer->setDouble(_womatl.value("qtyper").toDouble());
    _scrap->setDouble(_womatl.value("scrap").toDouble());
    _qtyRequired->setDouble(_womatl.value("qtyreq").toDouble());
    _qtyIssued->setDouble(_womatl.value("qtyiss").toDouble());

    _id = pWomatlid;
    _valid = TRUE;
    _required = _womatl.value("_qtyreq").toDouble();
    _issued = _womatl.value("_qtyiss").toDouble();

    emit newId(_id);
    emit newQtyScrappedFromWIP(_womatl.value("qtywipscrap").toDouble());
    emit valid(TRUE);
  }
}

void WomatlCluster::setDataWidgetMap(XDataWidgetMapper* m)
{
  m->addMapping(_itemNumber, _fieldName, "code", "currentDefault");
}

/////////////////////////////////////////////

woList::woList(QWidget* pParent, Qt::WindowFlags pFlags) :
    VirtualList(pParent, pFlags)
{
  setObjectName("woList");
  setMinimumWidth(600);
  _search->hide();
  _searchLit->hide();
  disconnect(_search,  SIGNAL(textChanged(const QString&)), this, SLOT(sSearch(const QString&)));

  _warehouse = new WarehouseGroup(this, "_warehouse");
  QVBoxLayout* searchLyt = findChild<QVBoxLayout*>("searchLyt");
  searchLyt->addWidget(_warehouse);
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillList()));

  _listTab->setColumnCount(0);
  _listTab->addColumn(tr("Order#"),     _orderColumn, Qt::AlignLeft,  true, "number");
  _listTab->addColumn(tr("Status"),              40, Qt::AlignCenter,true, "wo_status");
  _listTab->addColumn(tr("Whs."),        _whsColumn, Qt::AlignCenter,true, "warehous_code");
  _listTab->addColumn(tr("Item Number"),_itemColumn, Qt::AlignLeft,  true, "item_number");
  _listTab->addColumn(tr("Description"),         -1, Qt::AlignLeft,  true, "description");
}

void woList::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
    _woid = param.toInt();

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());

  param = pParams.value("woType", &valid);
  if (valid)
    _type = param.toInt();
}

void woList::sFillList()
{
  QString sql;

  sql = "SELECT wo_id,"
        "       formatWONumber(wo_id) AS number,"
        "       wo_status, warehous_code, item_number,"
        "       (item_descrip1 || ' ' || item_descrip2) AS description "
        "FROM wo, itemsite, warehous, item "
        "WHERE ( (wo_itemsite_id=itemsite_id)"
        " AND (itemsite_warehous_id=warehous_id)"
        " AND (itemsite_item_id=item_id)";

  if (_type != 0)
  {
    QStringList statuslist;

    if (_type & cWoOpen)
      statuslist << "'O'";

    if (_type & cWoExploded)
      statuslist << "'E'";

    if (_type & cWoReleased)
      statuslist << "'R'";

    if (_type & cWoIssued)
      statuslist << "'I'";

    if (_type & cWoClosed)
      statuslist << "'C'";

    sql += "AND (wo_status IN (" + statuslist.join(",") + "))";
  }

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  sql += ") ORDER BY wo_number, wo_subnumber, warehous_code, item_number";

  XSqlQuery wo;
  wo.prepare(sql);
  _warehouse->bindValue(wo);
  wo.exec();

  _listTab->populate(wo, _woid);
}

///////////////////////////

woSearch::woSearch(QWidget* pParent, Qt::WindowFlags pFlags)
    : VirtualSearch(pParent, pFlags)
{
  setObjectName( "woSearch" );
  setMinimumWidth(600);

  _warehouse = new WarehouseGroup(this, "_warehouse");
  QVBoxLayout* searchLyt = findChild<QVBoxLayout*>("searchLyt");
  searchLyt->addWidget(_warehouse);
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillList()));

  _listTab->setColumnCount(0);
  _listTab->addColumn(tr("Order#"),     _orderColumn, Qt::AlignLeft,  true, "number");
  _listTab->addColumn(tr("Status"),              40, Qt::AlignCenter,true, "wo_status");
  _listTab->addColumn(tr("Whs."),        _whsColumn, Qt::AlignCenter,true, "warehous_code");
  _listTab->addColumn(tr("Item Number"),_itemColumn, Qt::AlignLeft,  true, "item_number");
  _listTab->addColumn(tr("Description"),         -1, Qt::AlignLeft,  true, "description");

  _searchName->hide();
}

void woSearch::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
    _woid = param.toInt();

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());

  param = pParams.value("woType", &valid);
  if (valid)
    _type = param.toInt();
}

void woSearch::sFillList()
{
  QString sql;

  sql = "SELECT wo_id,"
        "       formatWONumber(wo_id) AS number,"
        "       wo_status, warehous_code, item_number,"
        "       (item_descrip1 || ' ' || item_descrip2) AS description "
        "FROM wo, itemsite, warehous, item "
        "WHERE ( (wo_itemsite_id=itemsite_id)"
        " AND (itemsite_warehous_id=warehous_id)"
        " AND (itemsite_item_id=item_id)";

  if (_type != 0)
  {
    QStringList statuslist;

    if (_type & cWoOpen)
      statuslist << "'O'";

    if (_type & cWoExploded)
      statuslist << "'E'";

    if (_type & cWoReleased)
      statuslist << "'R'";

    if (_type & cWoIssued)
      statuslist << "'I'";

    if (_type & cWoClosed)
      statuslist << "'C'";

    sql += "AND (wo_status IN (" + statuslist.join(",") + "))";
  }

  if (!_search->text().isEmpty())
  {
    sql += " AND ((false)  ";
    if (_searchNumber->isChecked())
    {
      sql += " OR (formatWONumber(wo_id) ~* :search) "
             " OR (item_number ~* :search) ";
    }
    if (_searchDescrip->isChecked())
    {
      sql += " OR (item_descrip1 ~* :search) "
             " OR (item_descrip2 ~* :search) ";
    }
    sql += ") ";
  }

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  sql += ") ORDER BY wo_number, wo_subnumber, warehous_code, item_number";

  XSqlQuery wo;
  wo.prepare(sql);
  wo.bindValue(":search", _search->text());
  _warehouse->bindValue(wo);
  wo.exec();

  _listTab->populate(wo, _woid);
}

