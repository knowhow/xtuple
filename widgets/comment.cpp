/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlError>
#include <QToolTip>
#include <QVBoxLayout>
#include <QVariant>
#include <QWhatsThis>
#include <QVector>
#include <QCheckBox>

#include <parameter.h>

#include "xtextedit.h"
#include "xcombobox.h"
#include "comment.h"
#include "shortcuts.h"

#define cNew  1
#define cEdit 2
#define cView 3

comment::comment( QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl ) :
  QDialog( parent, fl )
{
  if(name)
    setObjectName(name);
  setWindowTitle(tr("Comment"));
  if(modal)
    setWindowModality(Qt::WindowModal);

  _commentid = -1;
  _targetId = -1;
  _mode = cNew;

  if (!name)
    setObjectName("comment");

  QVBoxLayout *moreLayout = new QVBoxLayout(this);
  moreLayout->setContentsMargins(5, 5, 5, 5);
  moreLayout->setSpacing(7);
  moreLayout->setObjectName("moreLayout");

  QHBoxLayout *commentLayout = new QHBoxLayout(this);
  commentLayout->setContentsMargins(5, 5, 5, 5);
  commentLayout->setSpacing(7);
  commentLayout->setObjectName("commentLayout");

  QVBoxLayout *layout11  = new QVBoxLayout(this);
  layout11->setSpacing(5);
  layout11->setObjectName("layout11");

  QHBoxLayout *layout9   = new QHBoxLayout(this);
  layout9->setObjectName("layout9");

  QBoxLayout *layout8    = new QHBoxLayout(this);
  layout8->setSpacing(5);
  layout8->setObjectName("layout8");

  QLabel *_cmnttypeLit = new QLabel(tr("Comment Type:"), this);
  _cmnttypeLit->setObjectName("_cmnttypeLit");
  layout8->addWidget( _cmnttypeLit );

  _cmnttype = new XComboBox( FALSE, this);
  _cmnttype->setObjectName("_cmnttype" );
  layout8->addWidget( _cmnttype );
  layout9->addLayout( layout8 );

  QSpacerItem* spacer = new QSpacerItem( 66, 10, QSizePolicy::Expanding, QSizePolicy::Minimum );
  layout9->addItem( spacer );

  _public = new QCheckBox(tr("Public"), this);
  _public->setObjectName("_public");
  if(!(_x_metrics && _x_metrics->boolean("CommentPublicPrivate")))
    _public->hide();
  _public->setChecked(_x_metrics && _x_metrics->boolean("CommentPublicDefault"));
  layout9->addWidget(_public);
  layout11->addLayout( layout9 );

  _comment = new XTextEdit( this);
  _comment->setObjectName("_comment" );
  _comment->setSpellEnable(true);
  layout11->addWidget( _comment );
  commentLayout->addLayout( layout11 );

  QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
  buttonBox->setOrientation(Qt::Vertical);
  buttonBox->setStandardButtons(QDialogButtonBox::Save | QDialogButtonBox::Cancel);

  _close = buttonBox->button(QDialogButtonBox::Cancel);
  _close->setObjectName("_close");

  _save = buttonBox->button(QDialogButtonBox::Save);
  _save->setObjectName("_save");

  _prev = buttonBox->addButton(tr("&Previous"), QDialogButtonBox::ActionRole);
  _prev->setObjectName("_prev");

  _next = buttonBox->addButton(tr("&Next"), QDialogButtonBox::ActionRole);
  _next->setObjectName("_next");

  _more = buttonBox->addButton(tr("&More"), QDialogButtonBox::ActionRole);
  _more->setObjectName("_more");
  _more->setCheckable(true);

  commentLayout->addWidget(buttonBox);

  moreLayout->addLayout(commentLayout);

  _comments = new Comments(this);
  _comments->setObjectName("_comments");
  _comments->setReadOnly(true);
  _comments->findChild<XCheckBox*>("_verbose")->setForgetful(true);
  _comments->findChild<XCheckBox*>("_verbose")->hide();
  _comments->findChild<XCheckBox*>("_verbose")->setChecked(false);
  _comments->_newComment->setVisible(false);
  _comments->setVerboseCommentList(true);
  _comments->setVisible(false);
  _comments->setEditable(false);
  moreLayout->addWidget(_comments);

  resize( QSize(524, 270).expandedTo(minimumSizeHint()) );
  //clearWState( WState_Polished );

// signals and slots connections
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_next, SIGNAL(clicked()), this, SLOT(sNextComment()));
  connect(_prev, SIGNAL(clicked()), this, SLOT(sPrevComment()));
  connect(_more, SIGNAL(toggled(bool)), _comments, SLOT(setVisible(bool)));

// tab order
  setTabOrder( _cmnttype, _comment );
  setTabOrder( _comment, _save );
  setTabOrder( _save, _close );

  _source = Comments::Uninitialized;
  _cmnttype->setAllowNull(true);

  shortcuts::setStandardKeys(this);
}

void comment::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("comment_id", &valid);
  if (valid)
  {
    _cmnttype->setType(XComboBox::AllCommentTypes);
    _commentid = param.toInt();
    populate();
  }

  param = pParams.value("commentIDList", &valid);
  if (valid)
  {
    _commentIDList = param.toList();
    _commentLocation = _commentIDList.indexOf(_commentid);

    if((_commentLocation-1) >= 0)
      _prev->setEnabled(true); 
    else
      _prev->setEnabled(false); 

    if((_commentLocation+1) < _commentIDList.size())
      _next->setEnabled(true); 
    else
      _next->setEnabled(false); 
  }

  param = pParams.value("cust_id", &valid);
  if (valid)
  {
    _source = Comments::Customer;
    _cmnttype->setType(XComboBox::CustomerCommentTypes);
    _targetId = param.toInt();
  }

  param = pParams.value("vend_id", &valid);
  if (valid)
  {
    _source = Comments::Vendor;
    _cmnttype->setType(XComboBox::VendorCommentTypes);
    _targetId = param.toInt();
  }

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _source = Comments::Item;
    _cmnttype->setType(XComboBox::ItemCommentTypes);
    _targetId = param.toInt();
  }

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _source = Comments::ItemSite;
    _cmnttype->setType(XComboBox::AllCommentTypes);
    _targetId = param.toInt();
  }


//  Quotes
  param = pParams.value("quhead_id", &valid);
  if (valid)
  {
    _source = Comments::Quote;
    _cmnttype->setType(XComboBox::AllCommentTypes);
    _targetId = param.toInt();
  }

  param = pParams.value("quitem_id", &valid);
  if (valid)
  {
    _source = Comments::QuoteItem;
    _cmnttype->setType(XComboBox::AllCommentTypes);
    _targetId = param.toInt();
  }


//  Sales Orders
  param = pParams.value("sohead_id", &valid);
  if (valid)
  {
    _source = Comments::SalesOrder;
    _cmnttype->setType(XComboBox::AllCommentTypes);
    _targetId = param.toInt();
  }

  param = pParams.value("soitem_id", &valid);
  if (valid)
  {
    _source = Comments::SalesOrderItem;
    _cmnttype->setType(XComboBox::AllCommentTypes);
    _targetId = param.toInt();
  }

//  Sales Orders
  param = pParams.value("tohead_id", &valid);
  if (valid)
  {
    _source = Comments::TransferOrder;
    _cmnttype->setType(XComboBox::AllCommentTypes);
    _targetId = param.toInt();
  }

//  Return Authorizations
  param = pParams.value("rahead_id", &valid);
  if (valid)
  {
    _source = Comments::ReturnAuth;
    _cmnttype->setType(XComboBox::AllCommentTypes);
    _targetId = param.toInt();
  }

  param = pParams.value("raitem_id", &valid);
  if (valid)
  {
    _source = Comments::ReturnAuthItem;
    _cmnttype->setType(XComboBox::AllCommentTypes);
    _targetId = param.toInt();
  }

//  Purchase Orders
  param = pParams.value("pohead_id", &valid);
  if (valid)
  {
    _source = Comments::PurchaseOrder;
    _cmnttype->setType(XComboBox::AllCommentTypes);
    _targetId = param.toInt();
  }

  param = pParams.value("poitem_id", &valid);
  if (valid)
  {
    _source = Comments::PurchaseOrderItem;
    _cmnttype->setType(XComboBox::AllCommentTypes);
    _targetId = param.toInt();
  }


  param = pParams.value("lsdetail_id", &valid);
  if (valid)
  {
    _source = Comments::LotSerial;
    _cmnttype->setType(XComboBox::LotSerialCommentTypes);
    _targetId = param.toInt();
  }

  param = pParams.value("prj_id", &valid);
  if (valid)
  {
    _source = Comments::Project;
    _cmnttype->setType(XComboBox::ProjectCommentTypes);
    _targetId = param.toInt();
  }

  param = pParams.value("warehous_id", &valid);
  if (valid)
  {
    _source = Comments::Warehouse;
    _cmnttype->setType(XComboBox::WarehouseCommentTypes);
    _targetId = param.toInt();
  }

  param = pParams.value("todoitem_id", &valid);
  if (valid)
  {
    _source = Comments::TodoItem;
    _cmnttype->setType(XComboBox::TodoItemCommentTypes);
    _targetId = param.toInt();
  }

  param = pParams.value("prjtask_id", &valid);
  if (valid)
  {
    _source = Comments::Task;
    _cmnttype->setType(XComboBox::TaskCommentTypes);
    _targetId = param.toInt();
  }

  param = pParams.value("addr_id", &valid);
  if (valid)
  {
    _source = Comments::Address;
    _cmnttype->setType(XComboBox::AllCommentTypes);
    _targetId = param.toInt();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      
      _cmnttype->setFocus();
      _next->setVisible(false);
      _prev->setVisible(false);
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _next->setVisible(false);
      _prev->setVisible(false);
      _cmnttype->setEnabled(false);
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _next->setVisible(true);
      _prev->setVisible(true);
      _cmnttype->setEnabled(false);
      _comment->setReadOnly(true);
      _save->hide();
      _close->setText(tr("&Close"));
      _close->setShortcut(QKeySequence::Close);
      _more->hide();

      _close->setFocus();
    }
  }

  param = pParams.value("sourceType", &valid);
  if (valid)
  {
    _source = (enum Comments::CommentSources)param.toInt();
    if(!(_mode == cEdit || _mode == cView))
    {
      switch (_source)
      {
        case Comments::Address:
          _cmnttype->setType(XComboBox::AddressCommentTypes);
          break;
        case Comments::BBOMHead:
          _cmnttype->setType(XComboBox::BBOMHeadCommentTypes);
          break;
        case Comments::BBOMItem:
          _cmnttype->setType(XComboBox::BBOMItemCommentTypes);
          break;
        case Comments::BOMHead:
          _cmnttype->setType(XComboBox::BOMHeadCommentTypes);
          break;
        case Comments::BOMItem:
          _cmnttype->setType(XComboBox::BOMItemCommentTypes);
          break;
        case Comments::BOOHead:
          _cmnttype->setType(XComboBox::BOOHeadCommentTypes);
          break;
        case Comments::BOOItem:
          _cmnttype->setType(XComboBox::BOOItemCommentTypes);
          break;
        case Comments::CRMAccount:
          _cmnttype->setType(XComboBox::CRMAccountCommentTypes);
          break;
        case Comments::Contact:
          _cmnttype->setType(XComboBox::ContactCommentTypes);
          break;
        case Comments::Customer:
          _cmnttype->setType(XComboBox::CustomerCommentTypes);
          break;
        case Comments::Employee:
          _cmnttype->setType(XComboBox::EmployeeCommentTypes);
          break;
        case Comments::Incident:
          _cmnttype->setType(XComboBox::IncidentCommentTypes);
          break;
        case Comments::Item:
          _cmnttype->setType(XComboBox::ItemCommentTypes);
          break;
        case Comments::ItemSite:
          _cmnttype->setType(XComboBox::ItemSiteCommentTypes);
          break;
        case Comments::ItemSource:
          _cmnttype->setType(XComboBox::ItemSourceCommentTypes);
          break;
        case Comments::Location:
          _cmnttype->setType(XComboBox::LocationCommentTypes);
          break;
        case Comments::LotSerial:
          _cmnttype->setType(XComboBox::LotSerialCommentTypes);
          break;
        case Comments::Opportunity:
          _cmnttype->setType(XComboBox::OpportunityCommentTypes);
          break;
        case Comments::Project:
          _cmnttype->setType(XComboBox::ProjectCommentTypes);
          break;
        case Comments::PurchaseOrder:
          _cmnttype->setType(XComboBox::PurchaseOrderCommentTypes);
          break;
        case Comments::PurchaseOrderItem:
          _cmnttype->setType(XComboBox::PurchaseOrderItemCommentTypes);
          break;
        case Comments::ReturnAuth:
          _cmnttype->setType(XComboBox::ReturnAuthCommentTypes);
          break;
        case Comments::ReturnAuthItem:
          _cmnttype->setType(XComboBox::ReturnAuthItemCommentTypes);
          break;
        case Comments::Quote:
          _cmnttype->setType(XComboBox::QuoteCommentTypes);
          break;
        case Comments::QuoteItem:
          _cmnttype->setType(XComboBox::QuoteItemCommentTypes);
          break;
        case Comments::SalesOrder:
          _cmnttype->setType(XComboBox::SalesOrderCommentTypes);
          break;
        case Comments::SalesOrderItem:
          _cmnttype->setType(XComboBox::SalesOrderItemCommentTypes);
          break;
        case Comments::Task:
          _cmnttype->setType(XComboBox::TaskCommentTypes);
          break;
        case Comments::TodoItem:
          _cmnttype->setType(XComboBox::TodoItemCommentTypes);
          break;
        case Comments::TransferOrder:
          _cmnttype->setType(XComboBox::TransferOrderCommentTypes);
          break;
        case Comments::TransferOrderItem:
          _cmnttype->setType(XComboBox::TransferOrderItemCommentTypes);
          break;
        case Comments::Vendor:
          _cmnttype->setType(XComboBox::VendorCommentTypes);
          break;
        case Comments::Warehouse:
          _cmnttype->setType(XComboBox::WarehouseCommentTypes);
          break;
        case Comments::WorkOrder:
          _cmnttype->setType(XComboBox::WorkOrderCommentTypes);
          break;
        default:
          _cmnttype->setType(XComboBox::AllCommentTypes);
          break;
      }
    }
  }

  param = pParams.value("source_id", &valid);
  if (valid)
  {
    _targetId = param.toInt();
  }

  _comments->setType(_source);
  _comments->setId(_targetId);
}

void comment::sSave()
{
  if (_cmnttype->id() == -1)
  {
    QMessageBox::critical( this, tr("Cannot Post Comment"),
                           tr("<p>You must select a Comment Type for this "
                              "Comment before you may post it.") );
    _cmnttype->setFocus();
    return;
  }

  int result = -1;
  if(_mode == cNew)
  {
    _query.prepare("SELECT postComment(:cmnttype_id, :source, :source_id, :text, :public) AS result;");
    _query.bindValue(":cmnttype_id", _cmnttype->id());
    _query.bindValue(":source", Comments::_commentMap[_source].ident);
    _query.bindValue(":source_id", _targetId);
    _query.bindValue(":text", _comment->toPlainText().trimmed());
    _query.bindValue(":public", _public->isChecked());
    _query.exec();
    if (_query.first())
    {
      int result = _query.value("result").toInt();
      if (result < 0)
      {
        QMessageBox::critical(this, tr("Cannot Post Comment"),
                              tr("<p>A Stored Procedure failed to run "
                                 "properly.<br>(%1, %2)<br>")
                                .arg("postComment").arg(result));
      }
    }
    else if (_query.lastError().type() != QSqlError::NoError)
    {
      QMessageBox::critical(this, tr("Cannot Post Comment"),
                            _query.lastError().databaseText());
    }
  }
  else if(_mode == cEdit)
  {
    result = _commentid;
    _query.prepare("UPDATE comment SET comment_text=:text, comment_public=:public WHERE comment_id=:comment_id");
    _query.bindValue(":text", _comment->toPlainText().trimmed());
    _query.bindValue(":comment_id", _commentid);
    _query.bindValue(":public", _public->isChecked());
    _query.exec();
    if(_query.lastError().type() != QSqlError::NoError)
    {
      QMessageBox::critical(this, tr("Cannot Post Comment"),
                            _query.lastError().databaseText());
      return;
    }
  }

  if(result < 0)
    done(result);
  else
    reject();
}

void comment::populate()
{
  _query.prepare( "SELECT comment_cmnttype_id, comment_text, comment_public "
                  "FROM comment "
                  "WHERE (comment_id=:comment_id);" );
  _query.bindValue(":comment_id", _commentid);
  _query.exec();
  if (_query.first())
  {
    _cmnttype->setId(_query.value("comment_cmnttype_id").toInt());
    _comment->setText(_query.value("comment_text").toString());
    _public->setChecked(_query.value("comment_public").toBool());
  }
  else if (_query.lastError().type() != QSqlError::NoError)
  {
    QMessageBox::critical(this, tr("Error Selecting Comment"),
                          _query.lastError().databaseText());
    return;
  }
}

void comment::sNextComment()
{
  if((_commentLocation+1) < _commentIDList.size())
  {
    _commentLocation++;
    _commentid = _commentIDList[_commentLocation].toInt();
    populate();
    if((_commentLocation+1) < _commentIDList.size())
      _next->setEnabled(true); 
    else
      _next->setEnabled(false); 

    if((_commentLocation-1) >= 0)
      _prev->setEnabled(true); 
    else
      _prev->setEnabled(false); 
  }
}

void comment::sPrevComment()
{
  if((_commentLocation-1) >= 0)
  {
    _commentLocation--;
    _commentid = _commentIDList[_commentLocation].toInt();
    populate();
    if((_commentLocation-1) >= 0)
      _prev->setEnabled(true); 
    else
      _prev->setEnabled(false); 

    if((_commentLocation+1) < _commentIDList.size())
      _next->setEnabled(true); 
    else
      _next->setEnabled(false); 
  }
}
