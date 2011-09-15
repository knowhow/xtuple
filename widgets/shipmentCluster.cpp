/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "shipmentcluster.h"

#include <QMessageBox>

ShipmentCluster::ShipmentCluster(QWidget* pParent, const char* pName) :
    VirtualCluster(pParent, pName)
{
    addNumberWidget(new ShipmentClusterLineEdit(this, pName));
}

// should limitToOrder and removeOrderLimit be at the lineedit level?
void ShipmentCluster::limitToOrder(const int head_id)
{
  if (head_id > 0)
  {
    switch (static_cast<ShipmentClusterLineEdit*>(_number)->type())
    {
      case ShipmentClusterLineEdit::SalesOrder:
        setExtraClause(QString("   ((shiphead_order_id=%1)"
                               "AND (shiphead_order_type='SO')) ").arg(head_id));
        break;
      case ShipmentClusterLineEdit::TransferOrder:
        setExtraClause(QString("   ((shiphead_order_id=%1)"
                               "AND (shiphead_order_type='TO')) ").arg(head_id));
        break;
      case ShipmentClusterLineEdit::All:
      default:
        setExtraClause(QString(" (shiphead_order_id=%1) ").arg(head_id));
        break;
    }
    switch (static_cast<ShipmentClusterLineEdit*>(_number)->status())
    {
      case ShipmentClusterLineEdit::Shipped:
        if (!extraClause().isEmpty())
          setExtraClause(extraClause().append(" AND "));
        setExtraClause(extraClause().append(" (shiphead_shipped = TRUE) "));
        break;
      case ShipmentClusterLineEdit::Unshipped:
        if (!extraClause().isEmpty())
          setExtraClause(extraClause().append(" AND "));
        setExtraClause(extraClause().append(" (shiphead_shipped = FALSE) "));
        break;
      case ShipmentClusterLineEdit::AllStatus:
      default:
        // do nothing;
        break;
    }
  }
//  else
//    removeOrderLimit();
}

void ShipmentCluster::setId(const int pid)
{
//  removeOrderLimit();
  static_cast<ShipmentClusterLineEdit*>(_number)->setId(pid);
}

void ShipmentCluster::removeOrderLimit()
{
  ShipmentClusterLineEdit::ShipmentType type = static_cast<ShipmentClusterLineEdit*>(_number)->type();
  clearExtraClause();
  setType(type);

  ShipmentClusterLineEdit::ShipmentStatus status = static_cast<ShipmentClusterLineEdit*>(_number)->status();
  clearExtraClause();
  setStatus(status);
}

void ShipmentClusterLineEdit::setId(const int pid)
{
  //setType(All);
  VirtualClusterLineEdit::setId(pid);
}

ShipmentClusterLineEdit::ShipmentType ShipmentCluster::type()
{
  return (static_cast<ShipmentClusterLineEdit*>(_number))->type();
}

void ShipmentCluster::setType(QString ptype)
{
  return (static_cast<ShipmentClusterLineEdit*>(_number))->setType(ptype);
}

void ShipmentCluster::setType(ShipmentClusterLineEdit::ShipmentType ptype)
{
  return (static_cast<ShipmentClusterLineEdit*>(_number))->setType(ptype);
}

ShipmentClusterLineEdit::ShipmentStatus ShipmentCluster::status()
{
  return (static_cast<ShipmentClusterLineEdit*>(_number))->status();
}

void ShipmentCluster::setStatus(QString pstatus)
{
  return (static_cast<ShipmentClusterLineEdit*>(_number))->setStatus(pstatus);
}

void ShipmentCluster::setStatus(ShipmentClusterLineEdit::ShipmentStatus pstatus)
{
  return (static_cast<ShipmentClusterLineEdit*>(_number))->setStatus(pstatus);
}

ShipmentClusterLineEdit::ShipmentClusterLineEdit(QWidget* pParent, const char* pName) :
    VirtualClusterLineEdit(pParent, "shiphead", "shiphead_id", "shiphead_number",
                           "formatdate(shiphead_shipdate)", "shiphead_tracknum", 0, pName)
{
    _type = All;
    _status = AllStatus;
    setStrict(false);
    setType(SalesOrder);
    setStatus(Shipped);
    setTitles(tr("Shipment"), tr("Shipments"));
}

ShipmentClusterLineEdit::ShipmentType ShipmentClusterLineEdit::type()
{
  return _type;
}

void ShipmentClusterLineEdit::setType(ShipmentType ptype)
{
  if (ptype != _type)
  {
    switch (ptype)
    {
      case All:
        clearExtraClause();
        break;
      case SalesOrder:
        setExtraClause(" (shiphead_order_type='SO') ");
        break;
      case TransferOrder:
        setExtraClause(" (shiphead_order_type='TO') ");
        break;
      default:
        QMessageBox::critical(this, tr("Invalid Shipment Type"),
                              tr("<p>ShipmentClusterLineEdit::setType received "
                                 "an invalid ShipmentType %1").arg(ptype));
        return;
        break;
    }
  }
  _type = ptype;
}

void ShipmentClusterLineEdit::setType(QString ptype)
{
  if (ptype == "SO")
    setType(SalesOrder);
  else if (ptype == "TO")
    setType(TransferOrder);
  else
  {
    QMessageBox::critical(this, tr("Invalid Shipment Type"),
                          tr("ShipmentClusterLineEdit::setType received "
                             "an invalid ShipmentType %1").arg(ptype));
    setType(All);
  }
}

ShipmentClusterLineEdit::ShipmentStatus ShipmentClusterLineEdit::status()
{
  return _status;
}

void ShipmentClusterLineEdit::setStatus(ShipmentStatus pstatus)
{
  if (pstatus != _status)
  {
    switch (pstatus)
    {
      case AllStatus:
        clearExtraClause();
        break;
      case Shipped:
        setExtraClause(" (shiphead_shipped = TRUE) ");
        break;
      case Unshipped:
        setExtraClause(" (shiphead_shipped = FALSE) ");
        break;
      default:
        QMessageBox::critical(this, tr("Invalid Shipment Status"),
                              tr("<p>ShipmentClusterLineEdit::setStatus received "
                                 "an invalid ShipmentStatus %1").arg(pstatus));
        return;
        break;
    }
  }
  _status = pstatus;
}

void ShipmentClusterLineEdit::setStatus(QString pstatus)
{
  if (pstatus == "Shipped")
    setStatus(Shipped);
  else if (pstatus == "Unshipped")
    setStatus(Unshipped);
  else
  {
    QMessageBox::critical(this, tr("Invalid Shipment Status"),
                          tr("ShipmentClusterLineEdit::setStatus received "
                             "an invalid ShipmentStatus %1").arg(pstatus));
    setStatus(AllStatus);
  }
}

VirtualList* ShipmentClusterLineEdit::listFactory()
{
  return new ShipmentList(this);
}

VirtualSearch* ShipmentClusterLineEdit::searchFactory()
{
  return new ShipmentSearch(this);
}

ShipmentList::ShipmentList(QWidget* pParent, Qt::WindowFlags pFlags) :
  VirtualList(pParent, pFlags)
{
  _listTab->headerItem()->setText(1, tr("Shipped Date"));
  _listTab->headerItem()->setText(2, tr("Tracking Number"));
}

ShipmentSearch::ShipmentSearch(QWidget* pParent, Qt::WindowFlags pFlags) :
  VirtualSearch(pParent, pFlags)
{
  _listTab->headerItem()->setText(1, tr("Shipped Date"));
  _listTab->headerItem()->setText(2, tr("Tracking Number"));

  _searchName->setText(tr("Search through Shipped Date"));
  _searchDescrip->setText(tr("Search through Tracking Number"));
}
