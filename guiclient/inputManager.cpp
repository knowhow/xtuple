/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QObject>
#include <QList>
#include <QKeyEvent>
#include <QEvent>
#include <QDebug>

#include <xsqlquery.h>

#include "guiclient.h"

#include "inputManager.h"

typedef struct
{
  int  event;
  char string[5];
  int  length1;
  int  length2;
  int  length3;
} InputEvent;

#define cBCCProlog    "\x0b\x38"
#define cBCPrologSize 2
#define cBCTypeSize   4

#define cIdle         0x01
#define cProlog       0x02
#define cType         0x04
#define cHeader       0x08
#define cData         0x10

#ifdef Q_WS_MACX
#define cPrologCtrl   0x80    /* Macintosh-only */
#endif


static InputEvent _eventList[] =
{
  { cBCWorkOrder,          "WOXX", 1, 1, 0 },
  { cBCWorkOrderMaterial,  "WOMR", 1, 1, 1 },
  { cBCWorkOrderOperation, "WOOP", 1, 1, 1 },
  { cBCPurchaseOrder,         "POXX", 1, 0, 0 },
  { cBCPurchaseOrderLineItem, "POLI", 1, 1, 0 },
  { cBCSalesOrder,         "SOXX", 1, 0, 0 },
  { cBCSalesOrderLineItem, "SOLI", 1, 1, 0 },
  { cBCTransferOrder,         "TOXX", 1, 0, 0 },
  { cBCTransferOrderLineItem, "TOLI", 1, 1, 0 },
  { cBCItemSite,           "ISXX", 2, 1, 0 },
  { cBCItem,               "ITXX", 2, 0, 0 },
  { cBCUPCCode,            "ITUP", 0, 0, 0 },
  { cBCEANCode,            "ITEA", 0, 0, 0 },
  { cBCCountTag,           "CTXX", 2, 0, 0 },
  { cBCLocation,           "LOXX", 1, 2, 0 },
  { cBCLocationIssue,      "LOIS", 1, 2, 0 },
  { cBCLocationContents,   "LOCN", 1, 2, 0 },
  { cBCUser,               "USER", 1, 0, 0 },
  { cBCLotSerialNumber,    "LSNX", 1, 0, 0 }, // TODO: itemloc? lsdetail?
  { -1,                    "",     0, 0, 0 }
};


class ReceiverItem
{
  public:
    ReceiverItem()
    {
      _null = TRUE;
    };

    ReceiverItem(int pType, QObject *pParent, QObject *pTarget, const QString &pSlot)
    {
      _type   = pType;
      _parent = pParent;
      _target = pTarget;
      _slot   = pSlot;
      _null   = FALSE;
    };

    inline int type()        { return _type;   };
    inline QObject *parent() { return _parent; };
    inline QObject *target() { return _target; };
    inline char* slot()    { return _slot.toAscii().data();   };
    inline bool isNull()     { return _null;   };
    bool operator==(const ReceiverItem &value) const
    {
      return (_null && value._null) || (_target == value._target) ;
    };

  private:
    int     _type;
    QObject *_parent;
    QObject *_target;
    QString _slot;
    bool    _null;
};


class InputManagerPrivate
{
  public:
    InputManagerPrivate()
    {
      _state = cIdle;
    };

    QList<ReceiverItem> _receivers;
    int                      _state;
    int                      _cursor;
    int                      _eventCursor;
    int                      _length1;
    int                      _length2;
    int                      _length3;
    int                      _type;
    QString                  _buffer;

    ReceiverItem findReceiver(int pMask)
    {
      for (int counter = 0; counter < _receivers.count(); counter++)
        if (_receivers[counter].type() & pMask)
          return _receivers[counter];

      return ReceiverItem();
    };
};


InputManager::InputManager()
{
  _private = new InputManagerPrivate();
}

void InputManager::notify(int pType, QObject *pParent, QObject *pTarget, const QString &pSlot)
{
  _private->_receivers.prepend(ReceiverItem(pType, pParent, pTarget, pSlot));
  connect(pTarget, SIGNAL(destroyed(QObject *)), this, SLOT(sRemove(QObject *)));
}

/* Application scripts need to pass strings as the last argument to
   InputManager::notify() but don't have access to the C++ SLOT() macro
   to generate it. This function mimics the SLOT() macro and will need
   to be maintained as Qt changes.
 */
QString InputManager::slotName(const QString &slotname)
{
  return QString("1") + slotname;
}

void InputManager::sRemove(QObject *pTarget)
{
  for (int counter = 0; counter < _private->_receivers.count(); counter++)
    if (_private->_receivers[counter].target() == pTarget)
      _private->_receivers.removeAt(counter);
}

bool InputManager::eventFilter(QObject *, QEvent *pEvent)
{
  if (pEvent->type() == QEvent::KeyPress)
  {
//  Swallow the Shift key as the Symbol Bar Code readers express a Shift key
//  for each upper case alpha character
    if (((QKeyEvent *)pEvent)->key() == Qt::Key_Shift)
      return FALSE;

    int character = ((QKeyEvent *)pEvent)->text().data()->toAscii();
    /* qDebug("Scanned %d (key %d) at _private->_state=%d",
           character, ((QKeyEvent *)pEvent)->key(), _private->_state);
     */

    /* The Macintosh seems to handle control characters differently than
       Linux and Windows.  Apparently we need an extra state to look for
       the CTRL key (which Qt treats as Qt::Key_Meta, using Qt::Key_Control
       for the Apple/Command key).
     */
    switch (_private->_state)
    {
      case cIdle:
#ifdef Q_WS_MACX
        if (((QKeyEvent *)pEvent)->key() == Qt::Key_Meta)
        {
          _private->_state = cPrologCtrl;
          _private->_cursor = 0;
        }
#else
        if (character == QString(cBCCProlog)[0])
        {
          _private->_state = cProlog;
          _private->_cursor = 0;
        }
#endif
        else
          return FALSE;

        break;

#ifdef Q_WS_MACX
      case cPrologCtrl:
        // why does character come back as 0?
	// on an Intel Mac with Qt 4 the key() came back as Key_PageUp
	// but with PowerPC Mac with Qt 3 the key() came back as 'V'.
	// Accept either for now.
        if (((QKeyEvent *)pEvent)->key() - 64 == QString(cBCCProlog)[0] ||
            ((QKeyEvent *)pEvent)->key()      == Qt::Key_PageUp)
        {
          _private->_state = cProlog;
          _private->_cursor = 0;
        }
        else
          _private->_state = cIdle;

        break;
#endif

      case cProlog:
        _private->_cursor++;
        if (character == *(cBCCProlog + _private->_cursor))
        {
          if (_private->_cursor == (cBCPrologSize - 1))
          {
            _private->_state = cType;
            _private->_cursor = 0;
            _private->_buffer = "";
          }
        }
        else
          _private->_state = cIdle;

        break;

      case cType:
        _private->_buffer += character;
        if (++_private->_cursor == cBCTypeSize)
        {
          int cursor;
          for (cursor = 0, _private->_type = 0; _eventList[cursor].string[0] != '\0'; cursor++)
          {
            if (_private->_buffer == QString(_eventList[cursor].string))
            {
              _private->_eventCursor = cursor;
              _private->_type = _eventList[cursor].event;
	      break;
	    }
	  }

	  if (_private->_type == 0)
            _private->_state = cIdle;
          else
          {
            _private->_state = cHeader;
            _private->_cursor = 0;
            _private->_buffer = "";
	  }
        }

        break;

      case cHeader:
        _private->_buffer += character;
        _private->_cursor++;

        if (_private->_cursor == ( _eventList[_private->_eventCursor].length1 +
                         _eventList[_private->_eventCursor].length2 +
                         _eventList[_private->_eventCursor].length3 ) )
        {
          bool check;

          if (_eventList[_private->_eventCursor].length1)
          {
            _private->_length1 = _private->_buffer.left(_eventList[_private->_eventCursor].length1).toInt(&check);
            if (!check)
            {
              _private->_state = cIdle;
              break;
            }
          }
          else
            _private->_length1 = 0;

          if (_eventList[_private->_eventCursor].length2)
          {
            _private->_length2 = _private->_buffer.mid( _eventList[_private->_eventCursor].length1,
                                                        _eventList[_private->_eventCursor].length2 ).toInt(&check);
            if (!check)
            {
              _private->_state = cIdle;
              break;
            }
          }
          else
            _private->_length2 = 0;

          if (_eventList[_private->_eventCursor].length3)
          {
            _private->_length3 = _private->_buffer.right(_eventList[_private->_eventCursor].length3).toInt(&check);	    
            if (!check)
            {
              _private->_state = cIdle;
              break;
            }
          }
          else
            _private->_length3 = 0;

          _private->_cursor = 0;
          _private->_buffer = "";
          _private->_state = cData;
	}

	break;

      case cData:
        _private->_buffer += character;
        _private->_cursor++;

        if (_private->_cursor == (_private->_length1 + _private->_length2 + _private->_length3))
        {
          _private->_state = cIdle;

          switch (_private->_type)
          {
            case cBCWorkOrderOperation:
              dispatchWorkOrderOperation();
              break;

            case cBCPurchaseOrder:
              dispatchPurchaseOrder();
              break;

            case cBCSalesOrder:
              dispatchSalesOrder();
              break;

            case cBCTransferOrder:
              dispatchTransferOrder();
              break;

            case cBCCountTag:
              dispatchCountTag();
              break;

            case cBCWorkOrder:
              dispatchWorkOrder();
              break;

            case cBCSalesOrderLineItem:
              dispatchSalesOrderLineItem();
              break;

            case cBCPurchaseOrderLineItem:
              dispatchPurchaseOrderLineItem();
              break;

            case cBCTransferOrderLineItem:
              dispatchTransferOrderLineItem();
              break;

            case cBCItemSite:
              dispatchItemSite();
              break;

            case cBCItem:
              dispatchItem();
              break;

            case cBCUPCCode:
              dispatchUPCCode();
              break;

            case cBCLocation:
              dispatchLocation();
              break;

            case cBCLocationIssue:
              dispatchLocationIssue();
              break;

            case cBCLocationContents:
              dispatchLocationContents();
              break;

	    case cBCLotSerialNumber:
	      dispatchLotSerialNumber();
	      break;

            case cBCUser:
              dispatchUser();
              break;

            default:
              _private->_state = cIdle;
              break;
          }
	}

        break;
    }

    return TRUE;
  }
  else
    return FALSE;
}

void InputManager::dispatchWorkOrder()
{
  ReceiverItem receiver = _private->findReceiver(cBCWorkOrder);
  if (!receiver.isNull())
  {
    QString number    = _private->_buffer.left(_private->_length1);
    QString subNumber = _private->_buffer.right(_private->_length2);

    if (receiver.type() == cBCWorkOrder)
    {
      XSqlQuery woid;
      woid.prepare( "SELECT wo_id "
                    "FROM wo "
                    "WHERE ( (wo_number=:wo_number)"
                    " AND (wo_subnumber=:wo_subnumber) );" );
      woid.bindValue(":wo_number", number);
      woid.bindValue(":wo_subnumber", subNumber);
      woid.exec();
      if (woid.first())
      {
        message( tr("Scanned Work Order #%1-%2.")
                 .arg(number)
                 .arg(subNumber), 1000 );

        if (connect(this, SIGNAL(readWorkOrder(int)), receiver.target(), receiver.slot()))
        {
          emit readWorkOrder(woid.value("wo_id").toInt());
          disconnect(this, SIGNAL(readWorkOrder(int)), receiver.target(), receiver.slot());
        }
      }
      else
        message( tr("Work Order #%1-%2 does not exist in the Database.")
                 .arg(number)
                 .arg(subNumber), 1000 );
    }
  }
}

void InputManager::dispatchWorkOrderOperation()
{
  ReceiverItem receiver = _private->findReceiver((cBCWorkOrderOperation | cBCWorkOrder));
  if (!receiver.isNull())
  {
    QString number    = _private->_buffer.left(_private->_length1);
    QString subNumber = _private->_buffer.mid(_private->_length1, _private->_length2);
    QString seqNumber = _private->_buffer.right(_private->_length3);

    XSqlQuery wooperid;
    wooperid.prepare( "SELECT wo_id, wooper_id "
                      "FROM wo, wooper "
                      "WHERE ( (wooper_wo_id=wo_id)"
                      " AND (wo_number=:wo_number)"
                      " AND (wo_subnumber=:wo_subnumber)"
                      " AND (wooper_seqnumber=:wooper_seqnumber) );" );
    wooperid.bindValue(":wo_number", number);
    wooperid.bindValue(":wo_subnumber", subNumber);
    wooperid.bindValue(":wooper_seqnumber", seqNumber);
    wooperid.exec();
    if (wooperid.first())
    {
      message( tr("Scanned Work Order #%1-%2, Operation %3.")
               .arg(number)
               .arg(subNumber)
               .arg(seqNumber), 1000 );

      if (receiver.type() == cBCWorkOrderOperation)
      {
        if (connect(this, SIGNAL(readWorkOrderOperation(int)), receiver.target(), receiver.slot()))
        {
          emit readWorkOrderOperation(wooperid.value("wooper_id").toInt());
          disconnect(this, SIGNAL(readWorkOrderOperation(int)), receiver.target(), receiver.slot());
        }
      }
      else if (receiver.type() == cBCWorkOrder)
      {
        if (connect(this, SIGNAL(readWorkOrder(int)), receiver.target(), receiver.slot()))
        {
          emit readWorkOrder(wooperid.value("wo_id").toInt());
          disconnect(this, SIGNAL(readWorkOrder(int)), receiver.target(), receiver.slot());
        }
      }
    }
    else
      message( tr("Work Order #%1-%2, Operation %3 does not exist in the Database.")
               .arg(number)
               .arg(subNumber)
               .arg(seqNumber), 1000 );
  }
}

void InputManager::dispatchPurchaseOrder()
{
  ReceiverItem receiver = _private->findReceiver(cBCPurchaseOrder);
  if (!receiver.isNull())
  {
    QString number = _private->_buffer.left(_private->_length1);

    XSqlQuery poheadid;
    poheadid.prepare( "SELECT pohead_id "
                      "FROM pohead "
                      "WHERE (pohead_number=:pohead_number);" );
    poheadid.bindValue(":pohead_number", number);
    poheadid.exec();
    if (poheadid.first())
    {
      message( tr("Scanned Purchase Order #%1.")
               .arg(number), 1000 );

      if (connect(this, SIGNAL(readPurchaseOrder(int)), receiver.target(), receiver.slot()))
      {
        emit readPurchaseOrder(poheadid.value("pohead_id").toInt());
        disconnect(this, SIGNAL(readPurchaseOrder(int)), receiver.target(), receiver.slot());
      }
    }
    else
      message( tr("Purchase Order #%1 does not exist in the Database.")
               .arg(number), 1000 );
  }
}

void InputManager::dispatchSalesOrder()
{
  ReceiverItem receiver = _private->findReceiver(cBCSalesOrder);
  if (!receiver.isNull())
  {
    QString number = _private->_buffer.left(_private->_length1);

    XSqlQuery soheadid;
    soheadid.prepare( "SELECT cohead_id "
                      "FROM cohead "
                      "WHERE (cohead_number=:sohead_number);" );
    soheadid.bindValue(":sohead_number", number);
    soheadid.exec();
    if (soheadid.first())
    {
      message( tr("Scanned Sales Order #%1.")
               .arg(number), 1000 );

      if (connect(this, SIGNAL(readSalesOrder(int)), receiver.target(), receiver.slot()))
      {
        emit readSalesOrder(soheadid.value("cohead_id").toInt());
        disconnect(this, SIGNAL(readSalesOrder(int)), receiver.target(), receiver.slot());
      }
    }
    else
      message( tr("Sales Order #%1 does not exist in the Database.")
               .arg(number), 1000 );
  }
}

void InputManager::dispatchTransferOrder()
{
  ReceiverItem receiver = _private->findReceiver(cBCTransferOrder);
  if (!receiver.isNull())
  {
    QString number = _private->_buffer.left(_private->_length1);

    XSqlQuery toheadid;
    toheadid.prepare( "SELECT tohead_id "
                      "FROM tohead "
                      "WHERE (tohead_number=:tohead_number);" );
    toheadid.bindValue(":tohead_number", number);
    toheadid.exec();
    if (toheadid.first())
    {
      message( tr("Scanned Transfer Order #%1.")
               .arg(number), 1000 );

      if (connect(this, SIGNAL(readTransferOrder(int)), receiver.target(), receiver.slot()))
      {
        emit readTransferOrder(toheadid.value("tohead_id").toInt());
        disconnect(this, SIGNAL(readTransferOrder(int)), receiver.target(), receiver.slot());
      }
    }
    else
      message( tr("Transfer Order #%1 does not exist in the Database.")
               .arg(number), 1000 );
  }
}

void InputManager::dispatchPurchaseOrderLineItem()
{
  ReceiverItem receiver = _private->findReceiver((cBCPurchaseOrderLineItem | cBCPurchaseOrder | cBCItemSite | cBCItem));
  if (!receiver.isNull())
  {
    QString number    = _private->_buffer.left(_private->_length1);
    QString subNumber = _private->_buffer.right(_private->_length2);

    QString lineNumber = subNumber;
    QString subSubNumber = "0";
    int subsep = subNumber.indexOf(".");
    if(subsep >= 0)
    {
      lineNumber = subNumber.left(subsep);
      subSubNumber = subNumber.right(subNumber.length() - (subsep + 1));
    }

    if ( (receiver.type() == cBCPurchaseOrderLineItem) ||
         (receiver.type() == cBCPurchaseOrder) )
    {
      XSqlQuery poitemid;
      poitemid.prepare( "SELECT pohead_id, poitem_id "
                        "FROM pohead, poitem "
                        "WHERE ( (poitem_pohead_id=pohead_id)"
                        " AND (pohead_number=:pohead_number)"
                        " AND (poitem_linenumber=:poitem_linenumber) );" );
      poitemid.bindValue(":pohead_number", number);
      poitemid.bindValue(":poitem_linenumber", lineNumber);
      poitemid.exec();
      if (poitemid.first())
      {
        message( tr("Scanned Purchase Order Line #%1-%2.")
                 .arg(number)
                 .arg(subNumber), 1000 );

        if (receiver.type() == cBCPurchaseOrderLineItem)
        {
          if (connect(this, SIGNAL(readPurchaseOrderLineItem(int)), receiver.target(), receiver.slot()))
          {
            emit readPurchaseOrderLineItem(poitemid.value("poitem_id").toInt());
            disconnect(this, SIGNAL(readPurchaseOrderLineItem(int)), receiver.target(), receiver.slot());
          }
        }
        else if (receiver.type() == cBCPurchaseOrder)
       {
          if (connect(this, SIGNAL(readPurchaseOrder(int)), receiver.target(), receiver.slot()))
          {
            emit readPurchaseOrder(poitemid.value("pohead_id").toInt());
            disconnect(this, SIGNAL(readPurchaseOrder(int)), receiver.target(), receiver.slot());
          }
        }
      }
      else
        message( tr("Purchase Order Line #%1-%2 does not exist in the Database.")
                 .arg(number)
                 .arg(subNumber), 1000 );
    }
    else if ( (receiver.type() == cBCItemSite) ||
              (receiver.type() == cBCItem) )
    {
      XSqlQuery itemsiteid;
      itemsiteid.prepare( "SELECT itemsite_id, itemsite_item_id "
                          "FROM pohead, poitem, itemsite "
                          "WHERE ( (poitem_cohead_id=pohead_id)"
                          " AND (poitem_itemsite_id=itemsite_id)"
                          " AND (pohead_number=:pohead_number)"
                          " AND (poitem_linenumber=:poitem_linenumber) );" );
      itemsiteid.bindValue(":pohead_number", number);
      itemsiteid.bindValue(":poitem_linenumber", lineNumber);
      itemsiteid.exec();
      if (itemsiteid.first())
      {
        message( tr("Scanned Purchase Order Line #%1-%2.")
                 .arg(number)
                 .arg(subNumber), 1000 );

        if (receiver.type() == cBCItemSite)
        {
          if (connect(this, SIGNAL(readItemSite(int)), receiver.target(), receiver.slot()))
          {
            emit readItemSite(itemsiteid.value("itemsite_id").toInt());
            disconnect(this, SIGNAL(readItemSite(int)), receiver.target(), receiver.slot());
          }
        }
        else if (receiver.type() == cBCItem)
        {
          if (connect(this, SIGNAL(readItem(int)), receiver.target(), receiver.slot()))
          {
            emit readItem(itemsiteid.value("itemsite_item_id").toInt());
            disconnect(this, SIGNAL(readItem(int)), receiver.target(), receiver.slot());
          }
        }
      }
      else
        message( tr("Purchase Order Line #%1-%2 does not exist in the Database.")
                 .arg(number)
                 .arg(subNumber), 1000 );
    }
  }
}

void InputManager::dispatchSalesOrderLineItem()
{
  ReceiverItem receiver = _private->findReceiver((cBCSalesOrderLineItem | cBCSalesOrder | cBCItemSite | cBCItem));
  if (!receiver.isNull())
  {
    QString number    = _private->_buffer.left(_private->_length1);
    QString subNumber = _private->_buffer.right(_private->_length2);

    QString lineNumber = subNumber;
    QString subSubNumber = "0";
    int subsep = subNumber.indexOf(".");
    if(subsep >= 0)
    {
      lineNumber = subNumber.left(subsep);
      subSubNumber = subNumber.right(subNumber.length() - (subsep + 1));
    }

    if ( (receiver.type() == cBCSalesOrderLineItem) ||
         (receiver.type() == cBCSalesOrder) )
    {
      XSqlQuery soitemid;
      soitemid.prepare( "SELECT cohead_id, coitem_id "
                        "FROM cohead, coitem "
                        "WHERE ( (coitem_cohead_id=cohead_id)"
                        " AND (cohead_number=:sohead_number)"
                        " AND (coitem_linenumber=:soitem_linenumber)"
                        " AND (coitem_subnumber=:soitem_subnumber) );" );
      soitemid.bindValue(":sohead_number", number);
      soitemid.bindValue(":soitem_linenumber", lineNumber);
      soitemid.bindValue(":soitem_subnumber", subSubNumber);
      soitemid.exec();
      if (soitemid.first())
      {
        message( tr("Scanned Sales Order Line #%1-%2.")
                 .arg(number)
                 .arg(subNumber), 1000 );

        if (receiver.type() == cBCSalesOrderLineItem)
        {
          if (connect(this, SIGNAL(readSalesOrderLineItem(int)), receiver.target(), receiver.slot()))
          {
            emit readSalesOrderLineItem(soitemid.value("coitem_id").toInt());
            disconnect(this, SIGNAL(readSalesOrderLineItem(int)), receiver.target(), receiver.slot());
          }
        }
        else if (receiver.type() == cBCSalesOrder)
       {
          if (connect(this, SIGNAL(readSalesOrder(int)), receiver.target(), receiver.slot()))
          {
            emit readSalesOrder(soitemid.value("cohead_id").toInt());
            disconnect(this, SIGNAL(readSalesOrder(int)), receiver.target(), receiver.slot());
          }
        }
      }
      else
        message( tr("Sales Order Line #%1-%2 does not exist in the Database.")
                 .arg(number)
                 .arg(subNumber), 1000 );
    }
    else if ( (receiver.type() == cBCItemSite) ||
              (receiver.type() == cBCItem) )
    {
      XSqlQuery itemsiteid;
      itemsiteid.prepare( "SELECT itemsite_id, itemsite_item_id "
                          "FROM cohead, coitem, itemsite "
                          "WHERE ( (coitem_cohead_id=cohead_id)"
                          " AND (coitem_itemsite_id=itemsite_id)"
                          " AND (cohead_number=:sohead_number)"
                          " AND (coitem_linenumber=:soitem_linenumber)"
                          " AND (coitem_subnumber=:soitem_subnumber) );" );
      itemsiteid.bindValue(":sohead_number", number);
      itemsiteid.bindValue(":soitem_linenumber", lineNumber);
      itemsiteid.bindValue(":soitem_subnumber", subSubNumber);
      itemsiteid.exec();
      if (itemsiteid.first())
      {
        message( tr("Scanned Sales Order Line #%1-%2.")
                 .arg(number)
                 .arg(subNumber), 1000 );

        if (receiver.type() == cBCItemSite)
        {
          if (connect(this, SIGNAL(readItemSite(int)), receiver.target(), receiver.slot()))
          {
            emit readItemSite(itemsiteid.value("itemsite_id").toInt());
            disconnect(this, SIGNAL(readItemSite(int)), receiver.target(), receiver.slot());
          }
        }
        else if (receiver.type() == cBCItem)
        {
          if (connect(this, SIGNAL(readItem(int)), receiver.target(), receiver.slot()))
          {
            emit readItem(itemsiteid.value("itemsite_item_id").toInt());
            disconnect(this, SIGNAL(readItem(int)), receiver.target(), receiver.slot());
          }
        }
      }
      else
        message( tr("Sales Order Line #%1-%2 does not exist in the Database.")
                 .arg(number)
                 .arg(subNumber), 1000 );
    }
  }
}

void InputManager::dispatchTransferOrderLineItem()
{
  ReceiverItem receiver = _private->findReceiver((cBCTransferOrderLineItem | cBCTransferOrder | cBCItem));
  if (!receiver.isNull())
  {
    QString number    = _private->_buffer.left(_private->_length1);
    QString subNumber = _private->_buffer.right(_private->_length2);

    if ( (receiver.type() == cBCTransferOrderLineItem)	||
         (receiver.type() == cBCTransferOrder)		||
	 (receiver.type() == cBCItem) )
    {
      XSqlQuery toitemid;
      toitemid.prepare( "SELECT tohead_id, toitem_id, toitem_item_id "
                        "FROM tohead, toitem "
                        "WHERE ( (toitem_tohead_id=tohead_id)"
                        " AND (tohead_number=:tohead_number)"
                        " AND (toitem_linenumber=:toitem_linenumber) );" );
      toitemid.bindValue(":tohead_number", number);
      toitemid.bindValue(":toitem_linenumber", subNumber);
      toitemid.exec();
      if (toitemid.first())
      {
        message( tr("Scanned Transfer Order Line #%1-%2.")
                 .arg(number)
                 .arg(subNumber), 1000 );

        if (receiver.type() == cBCTransferOrderLineItem)
        {
          if (connect(this, SIGNAL(readTransferOrderLineItem(int)), receiver.target(), receiver.slot()))
          {
            emit readTransferOrderLineItem(toitemid.value("toitem_id").toInt());
            disconnect(this, SIGNAL(readTransferOrderLineItem(int)), receiver.target(), receiver.slot());
          }
        }
        else if (receiver.type() == cBCTransferOrder)
        {
          if (connect(this, SIGNAL(readTransferOrder(int)), receiver.target(), receiver.slot()))
          {
            emit readTransferOrder(toitemid.value("tohead_id").toInt());
            disconnect(this, SIGNAL(readTransferOrder(int)), receiver.target(), receiver.slot());
          }
        }
        else if (receiver.type() == cBCItem)
        {
          if (connect(this, SIGNAL(readItem(int)), receiver.target(), receiver.slot()))
          {
            emit readItem(toitemid.value("toitem_item_id").toInt());
            disconnect(this, SIGNAL(readItem(int)), receiver.target(), receiver.slot());
          }
        }
      }
      else
        message( tr("Transfer Order Line #%1-%2 does not exist in the Database.")
                 .arg(number)
                 .arg(subNumber), 1000 );
    }
  }
}

void InputManager::dispatchItemSite()
{
  ReceiverItem receiver = _private->findReceiver((cBCItemSite | cBCItem));
  if (!receiver.isNull())
  {
    QString itemNumber    = _private->_buffer.left(_private->_length1);
    QString warehouseCode = _private->_buffer.right(_private->_length2);

    XSqlQuery itemsiteid;
    itemsiteid.prepare( "SELECT itemsite_id, itemsite_item_id "
                        "FROM itemsite, item, warehous "
                        "WHERE ( (itemsite_warehous_id=warehous_id)"
                        " AND (itemsite_item_id=item_id)"
                        " AND (item_number=:item_number)"
                        " AND (warehous_code=:warehous_code) );" );
    itemsiteid.bindValue(":item_number", itemNumber);
    itemsiteid.bindValue(":warehous_code", warehouseCode);
    itemsiteid.exec();
    if (itemsiteid.first())
    {
      message( tr("Scanned Item %1, Site %2.")
               .arg(itemNumber)
               .arg(warehouseCode), 1000 );

      if (receiver.type() == cBCItemSite)
      {
        if (connect(this, SIGNAL(readItemSite(int)), receiver.target(), receiver.slot()))
        {
          emit readItemSite(itemsiteid.value("itemsite_id").toInt());
          disconnect(this, SIGNAL(readItemSite(int)), receiver.target(), receiver.slot());
        }
      }
      else if (receiver.type() == cBCItem)
      {
        if (connect(this, SIGNAL(readItem(int)), receiver.target(), receiver.slot()))
        {
          emit readItem(itemsiteid.value("itemsite_item_id").toInt());
          disconnect(this, SIGNAL(readItem(int)), receiver.target(), receiver.slot());
        }
      }
    }
    else
      message( tr("Item %1, Site %2 does not exist in the Database.")
               .arg(itemNumber)
               .arg(warehouseCode), 1000 );
  }
}

void InputManager::dispatchItem()
{
  ReceiverItem receiver = _private->findReceiver(cBCItem);
  if (!receiver.isNull())
  {
    QString itemNumber    = _private->_buffer.left(_private->_length1);

    XSqlQuery itemid;
    itemid.prepare( "SELECT item_id "
                    "FROM item "
                    "WHERE (item_number=:item_number);" );
    itemid.bindValue(":item_number", itemNumber);
    itemid.exec();
    if (itemid.first())
    {
      message( tr("Scanned Item %1.")
               .arg(itemNumber), 1000 );

      if (connect(this, SIGNAL(readItem(int)), receiver.target(), receiver.slot()))
      {
        emit readItem(itemid.value("item_id").toInt());
        disconnect(this, SIGNAL(readItem(int)), receiver.target(), receiver.slot());
      }
    }
    else
      message( tr("Item %1 does not exist in the Database.")
               .arg(itemNumber), 1000 );
  }
}

void InputManager::dispatchUPCCode()
{
  ReceiverItem receiver = _private->findReceiver(cBCItem);
  if (!receiver.isNull())
  {
    QString upcCode = _private->_buffer.left(_private->_length1);

    XSqlQuery itemid;
    itemid.prepare( "SELECT item_id, item_number "
                    "FROM item "
                    "WHERE (item_upccode=:item_upccode);" );
    itemid.bindValue(":item_upccode", upcCode);
    itemid.exec();
    if (itemid.first())
    {
      message( tr("Scanned UPC %1 for Item %2.")
               .arg(upcCode)
               .arg(itemid.value("item_number").toString()), 1000 );

      if (connect(this, SIGNAL(readItem(int)), receiver.target(), receiver.slot()))
      {
        emit readItem(itemid.value("item_id").toInt());
        disconnect(this, SIGNAL(readItem(int)), receiver.target(), receiver.slot());
      }
    }
    else
      message( tr("UPC Code %1 does not exist in the Database.")
               .arg(upcCode), 1000 );
  }
}

void InputManager::dispatchCountTag()
{
  ReceiverItem receiver = _private->findReceiver(cBCCountTag);
  if (!receiver.isNull())
  {
    QString tagNumber = _private->_buffer.left(_private->_length1);

    XSqlQuery cnttagid;
    cnttagid.prepare( "SELECT invcnt_id "
                      "FROM invcnt "
                      "WHERE (invcnt_tagnumber=:tagnumber);" );
    cnttagid.bindValue(":tagnumber", tagNumber);
    cnttagid.exec();
    if (cnttagid.first())
    {
      message( tr("Scanned Count Tag %1.")
               .arg(tagNumber), 1000 );

      if (connect(this, SIGNAL(readCountTag(int)), receiver.target(), receiver.slot()))
      {
        emit readCountTag(cnttagid.value("invcnt_id").toInt());
        disconnect(this, SIGNAL(readCountTag(int)), receiver.target(), receiver.slot());
      }
    }
    else
      message( tr("Item %1 does not exist in the Database.")
               .arg(tagNumber), 1000 );
  }
}

void InputManager::dispatchLocation()
{
  ReceiverItem receiver = _private->findReceiver(cBCLocation);
  if (!receiver.isNull())
  {
    QString warehouseCode = _private->_buffer.left(_private->_length1);
    QString locationCode  = _private->_buffer.right(_private->_length2);

    XSqlQuery locationid;
    locationid.prepare( "SELECT location_id "
                        "FROM location, warehous "
                        "WHERE ( (location_warehous_id=warehous_id)"
                        " AND (warehous_code=:warehous_code)"
                        " AND (location_name=:location_name) );" );
    locationid.bindValue(":warehous_code", warehouseCode);
    locationid.bindValue(":location_name", locationCode);
    locationid.exec();
    if (locationid.first())
    {
      message( tr("Scanned Site %1, Location %2.")
               .arg(warehouseCode) 
               .arg(locationCode), 1000 );

      if (connect(this, SIGNAL(readLocation(int)), receiver.target(), receiver.slot()))
      {
        emit readLocation(locationid.value("location_id").toInt());
        disconnect(this, SIGNAL(readLocation(int)), receiver.target(), receiver.slot());
      }
    }
    else
      message( tr("Site %1, Location %2 does not exist in the Database.")
               .arg(warehouseCode)
               .arg(locationCode), 1000 );
  }
}

void InputManager::dispatchLocationIssue()
{
  ReceiverItem receiver = _private->findReceiver((cBCLocation | cBCLocationIssue));
  if (!receiver.isNull())
  {
    QString warehouseCode = _private->_buffer.left(_private->_length1);
    QString locationCode  = _private->_buffer.right(_private->_length2);

    XSqlQuery locationid;
    locationid.prepare( "SELECT location_id "
                        "FROM location, warehous "
                        "WHERE ( (location_warehous_id=warehous_id)"
                        " AND (warehous_code=:warehous_code)"
                        " AND (location_name=:location_name) );" );
    locationid.bindValue(":warehous_code", warehouseCode);
    locationid.bindValue(":location_name", locationCode);
    locationid.exec();
    if (locationid.first())
    {
      message( tr("Scanned Site %1, Location %2.")
               .arg(warehouseCode) 
               .arg(locationCode), 1000 );

      if (receiver.type() == cBCLocationIssue)
      {
        if (connect(this, SIGNAL(readLocationIssue(int)), receiver.target(), receiver.slot()))
        {
          emit readLocationIssue(locationid.value("location_id").toInt());
          disconnect(this, SIGNAL(readLocationIssue(int)), receiver.target(), receiver.slot());
        }
      }
      else if (receiver.type() == cBCLocation)
      {
        if (connect(this, SIGNAL(readLocation(int)), receiver.target(), receiver.slot()))
        {
          emit readLocation(locationid.value("location_id").toInt());
          disconnect(this, SIGNAL(readLocation(int)), receiver.target(), receiver.slot());
        }
      }
    }
    else
      message( tr("Site %1, Location %2 does not exist in the Database.")
               .arg(warehouseCode)
               .arg(locationCode), 1000 );
  }
}

void InputManager::dispatchLocationContents()
{
  ReceiverItem receiver = _private->findReceiver((cBCLocation | cBCLocationContents));
  if (!receiver.isNull())
  {
    QString warehouseCode = _private->_buffer.left(_private->_length1);
    QString locationCode  = _private->_buffer.right(_private->_length2);

    XSqlQuery locationid;
    locationid.prepare( "SELECT location_id "
                        "FROM location, warehous "
                        "WHERE ( (location_warehous_id=warehous_id)"
                        " AND (warehous_code=:warehous_code)"
                        " AND (location_name=:location_name) );" );
    locationid.bindValue(":warehous_code", warehouseCode);
    locationid.bindValue(":location_name", locationCode);
    locationid.exec();
    if (locationid.first())
    {
      message( tr("Scanned Site %1, Location %2.")
               .arg(warehouseCode) 
               .arg(locationCode), 1000 );

      if (receiver.type() == cBCLocationContents)
      {
        if (connect(this, SIGNAL(readLocationContents(int)), receiver.target(), receiver.slot()))
        {
          emit readLocationContents(locationid.value("location_id").toInt());
          disconnect(this, SIGNAL(readLocationContents(int)), receiver.target(), receiver.slot());
        }
      }
      else if (receiver.type() == cBCLocation)
      {
        if (connect(this, SIGNAL(readLocation(int)), receiver.target(), receiver.slot()))
        {
          emit readLocation(locationid.value("location_id").toInt());
          disconnect(this, SIGNAL(readLocation(int)), receiver.target(), receiver.slot());
        }
      }
    }
    else
      message( tr("Site %1, Location %2 does not exist in the Database.")
               .arg(warehouseCode)
               .arg(locationCode), 1000 );
  }
}

void InputManager::dispatchUser()
{
  ReceiverItem receiver = _private->findReceiver((cBCUser));
  if (!receiver.isNull())
  {
    QString username = _private->_buffer.left(_private->_length1);

    XSqlQuery userid;
    userid.prepare( "SELECT usr_id "
                        "FROM usr "
                        "WHERE (usr_username=:username);" );
    userid.bindValue(":username", username);
    userid.exec();
    if (userid.first())
    {
      message( tr("Scanned User %1.")
               .arg(username), 1000 );

      if (connect(this, SIGNAL(readUser(int)), receiver.target(), receiver.slot()))
      {
	emit readUser(userid.value("usr_id").toInt());
	disconnect(this, SIGNAL(readUser(int)), receiver.target(), receiver.slot());
      }
    }
    else
      message( tr("User %1 not exist in the Database.")
               .arg(username), 1000 );
  }
}

void InputManager::dispatchLotSerialNumber()
{
  // qDebug("dispatchLotSerialNumber");
  ReceiverItem receiver = _private->findReceiver(cBCLotSerialNumber);
  if (!receiver.isNull())
  {
    QString lotserial = _private->_buffer.left(_private->_length1);

    XSqlQuery lsdetail;
    lsdetail.prepare( "SELECT lsdetail_id "
		      "FROM lsdetail "
		      "WHERE (formatlotserialnumber(lsdetail_ls_id)=:lotserial);" );
    lsdetail.bindValue(":lotserial", lotserial);
    lsdetail.exec();
    if (lsdetail.first())
    {
      message( tr("Scanned Lot/Serial # %1.").arg(lotserial), 1000);

      if (connect(this, SIGNAL(readLotSerialNumber(QString)), receiver.target(), receiver.slot()))
      {
        emit readLotSerialNumber(lotserial);
        disconnect(this, SIGNAL(readLotSerialNumber(QString)), receiver.target(), receiver.slot());
      }
    }
    else
      message( tr("Lot/Serial # %1 does not exist in the Database.")
               .arg(lotserial), 1000 );
  }
}
