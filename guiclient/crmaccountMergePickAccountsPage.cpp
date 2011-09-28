/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "crmaccountMergePickAccountsPage.h"

#include <QSqlError>
#include <QString>
#include <QStringList>

#include <mqlutil.h>

#include "crmaccountMerge.h"
#include "errorReporter.h"

class CrmaccountMergePickAccountsPagePrivate
{
  public:
    CrmaccountMergePickAccountsPagePrivate(QWidget *parent)
      : _parent(parent),
        _targetid_cache(-1)
    {
      QString errmsg;
      bool ok = false;
      _mqlstr = MQLUtil::mqlLoad("crmaccounts", "detail", errmsg, &ok);
      if (!ok)
        ErrorReporter::error(QtCriticalMsg, _parent,
                             QT_TRANSLATE_NOOP("CrmaccountMergePickAccountsPage",
                                               "Error Getting CRM Accounts"),
                             errmsg, __FILE__, __LINE__);
    }

    QList<int>  _ids_cache;
    QWidget    *_parent;
    QString     _mqlstr;
    int         _targetid_cache;
};

CrmaccountMergePickAccountsPage::CrmaccountMergePickAccountsPage(QWidget *parent)
  : QWizardPage(parent)
{
  setupUi(this);

  _data = new CrmaccountMergePickAccountsPagePrivate(this);

  connect(_query,               SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_sources,SIGNAL(itemSelectionChanged()), this, SLOT(sHandleButtons()));
  connect(_target,            SIGNAL(valid(bool)), this, SIGNAL(completeChanged()));

  _sources->addColumn(tr("Number"), _orderColumn, Qt::AlignLeft,  true, "crmacct_number");
  _sources->addColumn(tr("Name"),             -1, Qt::AlignLeft,  true, "crmacct_name");
  _sources->addColumn(tr("First"),            50, Qt::AlignLeft,  true, "cntct_first_name" );
  _sources->addColumn(tr("Last"),             -1, Qt::AlignLeft,  true, "cntct_last_name" );
  _sources->addColumn(tr("Phone"),           100, Qt::AlignLeft,  true, "cntct_phone" );
  _sources->addColumn(tr("Email"),           100, Qt::AlignLeft,  true, "cntct_email" );
  _sources->addColumn(tr("Address"),          -1, Qt::AlignLeft,  false, "addr_line1" );
  _sources->addColumn(tr("City"),             75, Qt::AlignLeft,  false, "addr_city" );
  _sources->addColumn(tr("State"),            50, Qt::AlignLeft,  false, "addr_state" );
  _sources->addColumn(tr("Country"),         100, Qt::AlignLeft,  false, "addr_country" );
  _sources->addColumn(tr("Postal Code"),      75, Qt::AlignLeft,  false, "addr_postalcode" );
  _sources->addColumn(tr("Customer"),  _ynColumn, Qt::AlignCenter,true,  "cust");
  _sources->addColumn(tr("Prospect"),  _ynColumn, Qt::AlignCenter,true,  "prospect");
  _sources->addColumn(tr("Vendor"),    _ynColumn, Qt::AlignCenter,true,  "vend");
  _sources->addColumn(tr("Competitor"),_ynColumn, Qt::AlignCenter,false, "competitor");
  _sources->addColumn(tr("Partner"),   _ynColumn, Qt::AlignCenter,false, "partner");
  _sources->addColumn(tr("Tax Auth."), _ynColumn, Qt::AlignCenter,false, "taxauth");
  _sources->addColumn(tr("User"),      _ynColumn, Qt::AlignCenter,false, "usr");
  _sources->addColumn(tr("Employee"),  _ynColumn, Qt::AlignCenter,false, "emp");
  _sources->addColumn(tr("Sales Rep"), _ynColumn, Qt::AlignCenter,false, "salesrep");

  _filter->append(tr("Hide Merges in Progress"),"excludeMergeWIP",        ParameterWidget::Exists, true);
  _filter->append(tr("Show Inactive"),          "showInactive",           ParameterWidget::Exists, true);
  _filter->append(tr("Account Number Pattern"), "crmacct_number_pattern", ParameterWidget::Text);
  _filter->append(tr("Account Name Pattern"),   "crmacct_name_pattern",   ParameterWidget::Text);
  _filter->append(tr("Contact Name Pattern"),   "cntct_name_pattern",     ParameterWidget::Text);
  _filter->append(tr("Phone Pattern"),          "cntct_phone_pattern",    ParameterWidget::Text);
  _filter->append(tr("Email Pattern"),          "cntct_email_pattern",    ParameterWidget::Text);
  _filter->append(tr("Street Pattern"),         "addr_street_pattern",    ParameterWidget::Text);
  _filter->append(tr("City Pattern"),           "addr_city_pattern",      ParameterWidget::Text);
  _filter->append(tr("State Pattern"),          "addr_state_pattern",     ParameterWidget::Text);
  _filter->append(tr("Postal Code Pattern"),    "addr_postalcode_pattern",ParameterWidget::Text);
  _filter->append(tr("Country Pattern"),        "addr_country_pattern",   ParameterWidget::Text);
  _filter->applyDefaultFilterSet();

  registerField("_target", _target, "text", "currentIndexChanged(QString)");
}

CrmaccountMergePickAccountsPage::~CrmaccountMergePickAccountsPage()
{
  if (_data)
    delete _data;
}

void CrmaccountMergePickAccountsPage::cleanupPage()
{
  _data->_targetid_cache = _target->id();
  _data->_ids_cache.clear();
  for (int i = 0; i < _target->count(); i++)
    _data->_ids_cache.append(_target->id(i));
}

void CrmaccountMergePickAccountsPage::initializePage()
{
  _data->_targetid_cache = _target->id();
  for (int i = 0; i < _data->_ids_cache.size(); i++)
    _target->append(_data->_ids_cache[i],       // sFillList will set real codes
                    QString::number(_data->_ids_cache[i]));
  sFillList();
}

bool CrmaccountMergePickAccountsPage::isComplete() const
{
  return _sources->selectedItems().size() > 1 && _target->isValid();
}

bool CrmaccountMergePickAccountsPage::validatePage()
{
  XSqlQuery begin("BEGIN;");
  XSqlQuery rollback;

  rollback.prepare("ROLLBACK;");

  XSqlQuery delq;
  delq.prepare("DELETE FROM crmacctsel"
               " WHERE crmacctsel_dest_crmacct_id=:destid"
               "   AND crmacctsel_src_crmacct_id=:srcid;");
  for (int n = 0; n < _data->_ids_cache.size(); n++)
  {
    if (_target->id(_data->_ids_cache[n]) == -1) // source id
    {
      delq.bindValue(":destid", _data->_targetid_cache);
      delq.bindValue(":srcid", _data->_ids_cache[n]);
      delq.exec();
      if (delq.lastError().type() != QSqlError::NoError)
      {
        rollback.exec();
        ErrorReporter::error(QtCriticalMsg, this, tr("Clearing Old Selections"),
                             delq, __FILE__, __LINE__);
        return false;
      }
    }
  }

  if (_target->id() != _data->_targetid_cache)
  {
    XSqlQuery updq;
    updq.prepare("UPDATE crmacctsel SET crmacctsel_dest_crmacct_id=:new,"
                 " crmacctsel_mrg_crmacct_active         = (crmacctsel_src_crmacct_id=:new),"
                 " crmacctsel_mrg_crmacct_cntct_id_1     = (crmacctsel_src_crmacct_id=:new),"
                 " crmacctsel_mrg_crmacct_cntct_id_2     = (crmacctsel_src_crmacct_id=:new),"
                 " crmacctsel_mrg_crmacct_competitor_id  = (crmacctsel_src_crmacct_id=:new),"
                 " crmacctsel_mrg_crmacct_cust_id        = (crmacctsel_src_crmacct_id=:new),"
                 " crmacctsel_mrg_crmacct_emp_id         = (crmacctsel_src_crmacct_id=:new),"
                 " crmacctsel_mrg_crmacct_name           = (crmacctsel_src_crmacct_id=:new),"
                 " crmacctsel_mrg_crmacct_notes          = (crmacctsel_src_crmacct_id=:new),"
                 " crmacctsel_mrg_crmacct_owner_username = (crmacctsel_src_crmacct_id=:new),"
                 " crmacctsel_mrg_crmacct_parent_id      = (crmacctsel_src_crmacct_id=:new),"
                 " crmacctsel_mrg_crmacct_partner_id     = (crmacctsel_src_crmacct_id=:new),"
                 " crmacctsel_mrg_crmacct_prospect_id    = (crmacctsel_src_crmacct_id=:new),"
                 " crmacctsel_mrg_crmacct_salesrep_id    = (crmacctsel_src_crmacct_id=:new),"
                 " crmacctsel_mrg_crmacct_taxauth_id     = (crmacctsel_src_crmacct_id=:new),"
                 " crmacctsel_mrg_crmacct_type           = (crmacctsel_src_crmacct_id=:new),"
                 " crmacctsel_mrg_crmacct_usr_username   = (crmacctsel_src_crmacct_id=:new),"
                 " crmacctsel_mrg_crmacct_vend_id        = (crmacctsel_src_crmacct_id=:new) "
                 " WHERE crmacctsel_dest_crmacct_id=:old;");
    updq.bindValue(":new", _target->id());
    updq.bindValue(":old", _data->_targetid_cache);
    updq.exec();
    if (updq.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Updating Selections"),
                           updq, __FILE__, __LINE__);
      return false;
    }
  }

  QStringList addme;
  for (int n = 0; n < _target->count(); n++)
    if (! _data->_ids_cache.contains(_target->id(n)))
      addme << QString::number(_target->id(n));
  if (addme.size() > 0)
  {
    MetaSQLQuery mql("INSERT INTO crmacctsel VALUES ("
                     " <? value('srcid') ?>,"
                     " <? value('destid') ?>,"
                     " <? value('srcid') ?> = <? value('destid') ?>,"
                     " <? value('srcid') ?> = <? value('destid') ?>,"
                     " <? value('srcid') ?> = <? value('destid') ?>,"
                     " <? value('srcid') ?> = <? value('destid') ?>,"
                     " <? value('srcid') ?> = <? value('destid') ?>,"
                     " <? value('srcid') ?> = <? value('destid') ?>,"
                     " <? value('srcid') ?> = <? value('destid') ?>,"
                     " <? value('srcid') ?> = <? value('destid') ?>,"
                     " <? value('srcid') ?> = <? value('destid') ?>,"
                     " <? value('srcid') ?> = <? value('destid') ?>,"
                     " <? value('srcid') ?> = <? value('destid') ?>,"
                     " <? value('srcid') ?> = <? value('destid') ?>,"
                     " <? value('srcid') ?> = <? value('destid') ?>,"
                     " <? value('srcid') ?> = <? value('destid') ?>,"
                     " <? value('srcid') ?> = <? value('destid') ?>,"
                     " <? value('srcid') ?> = <? value('destid') ?>,"
                     " <? value('srcid') ?> = <? value('destid') ?>"
                     ");");
    ParameterList params;
    params.append("destid", _target->id());

    QString srcidstr("srcid");

    for (int i = 0; i < addme.size(); i++)
    {
      params.remove(srcidstr);
      params.append(srcidstr, addme[i]);
      XSqlQuery insq = mql.toQuery(params);
      if (insq.lastError().type() != QSqlError::NoError)
      {
        rollback.exec();
        ErrorReporter::error(QtCriticalMsg, this, tr("Inserting New Selections"),
                             insq, __FILE__, __LINE__);
        return false;
      }
    }
  }

  XSqlQuery commit("COMMIT;");

  _data->_targetid_cache = _target->id();
  _data->_ids_cache.clear();
  for (int i = 0; i < _target->count(); i++)
    _data->_ids_cache.append(_target->id(i));

  wizard()->page(crmaccountMerge::Page_PickTask)->initializePage();
  setField("_existingMerge", field("_target"));

  return true;
}

void CrmaccountMergePickAccountsPage::sFillList()
{
  // save the current target and selection
  int prevtarget = _target->id();
  QList<int> ids;
  for (int i = 0; i < _target->count(); i++)
    ids.append(_target->id(i));

  // repopulate
  _target->clear();
  MetaSQLQuery mql(_data->_mqlstr);
  XSqlQuery qry = mql.toQuery(_filter->parameters());
  _sources->populate(qry);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Getting CRM Account"),
                           qry, __FILE__, __LINE__))
    return;

  // restore the selection
  disconnect(_sources,SIGNAL(itemSelectionChanged()), this, SLOT(sHandleButtons()));
  for (int j = 0; j < ids.size(); j++)
  {
    for (int i = 0; i < _sources->topLevelItemCount(); i++)
    {
      XTreeWidgetItem *item = dynamic_cast<XTreeWidgetItem*>(_sources->topLevelItem(i));
      if (item && item->id() == ids[j])
      {
        _sources->setCurrentItem(item, 0,
                                 QItemSelectionModel::Select |
                                 QItemSelectionModel::Rows);
        _target->append(item->id(), item->text("crmacct_number"));
      }
    }
  }
  connect(_sources,SIGNAL(itemSelectionChanged()), this, SLOT(sHandleButtons()));
  sHandleButtons();

  _target->setId(prevtarget);
}

void CrmaccountMergePickAccountsPage::sHandleButtons()
{
  QList<XTreeWidgetItem *> items = _sources->selectedItems();
  _target->setEnabled(items.size() > 0);

  int targetid = _target->id();

  _target->clear();
  for (int i = 0; i < items.size(); i++)
    _target->append(items[i]->id(), items[i]->text("crmacct_number"));

  _target->setId(targetid);

  emit completeChanged();
}
