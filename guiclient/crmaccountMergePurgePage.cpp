/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "crmaccountMergePurgePage.h"

#include <QMessageBox>

#include <metasql.h>
#include <mqlutil.h>

#include "errorReporter.h"

class CrmaccountMergePurgePagePrivate
{
  public:
    CrmaccountMergePurgePagePrivate(QWizardPage *parent)
      : _parent(parent),
        _mergedKey(1),
        _selectedKey(2)
    {
      QString errmsg;
      bool    ok = false;
      _mqlstr = MQLUtil::mqlLoad("crmaccountmerge", "wip", errmsg, &ok);
      if (!ok)
        ErrorReporter::error(QtCriticalMsg, _parent,
                             QT_TRANSLATE_NOOP("CrmaccountMergePurgePage",
                                               "Getting List of Merges"),
                             errmsg, __FILE__, __LINE__);
    }

    QWizardPage *_parent;
    int          _mergedKey;
    QString      _mqlstr;
    int          _selectedKey;
};

CrmaccountMergePurgePage::CrmaccountMergePurgePage(QWidget *parent)
  : QWizardPage(parent),
    _data(0)
{
  setupUi(this);
  _data = new CrmaccountMergePurgePagePrivate(this);

  _completed->addColumn(tr("CRM Account Number"), _orderColumn, Qt::AlignLeft, true, "crmacct_number");
  _completed->addColumn(tr("CRM Account Name"),             -1, Qt::AlignLeft, true, "crmacct_name");
  _completed->addColumn(tr("Status"),                       -1, Qt::AlignLeft, true, "status");

  connect(_completed, SIGNAL(itemSelectionChanged()), this, SLOT(sHandleButtons()));
  connect(_delete,                 SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_deleteAll,              SIGNAL(clicked()), this, SLOT(sDeleteAll()));

  setFinalPage(true);
}

CrmaccountMergePurgePage::~CrmaccountMergePurgePage()
{
  if (_data)
  {
    delete _data;
    _data = 0;
  }
}

void CrmaccountMergePurgePage::initializePage()
{
  sFillList();
}

void CrmaccountMergePurgePage::sDelete()
{
  if (QMessageBox::question(this, tr("Delete?"),
                            tr("<p>Are you sure you want to delete the "
                               "selected merge records?</p><p>Note that "
                               "obsolete CRM Accounts will be deleted "
                               "when purging completed merges.</p>"),
                            QMessageBox::No | QMessageBox::Default,
                            QMessageBox::Yes) == QMessageBox::No)
    return;

  QList<XTreeWidgetItem*> selected = _completed->selectedItems();
  foreach (XTreeWidgetItem *item, selected)
  {
    XSqlQuery delq;
    delq.prepare("SELECT purgecrmacctmerge(:destcrmacctid) AS RESULT;");
    delq.bindValue(":destcrmacctid", item->id());
    delq.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Purge Error"),
                             delq, __FILE__, __LINE__))
      return;
  }
  sFillList();
}

void CrmaccountMergePurgePage::sDeleteAll()
{
  if (QMessageBox::question(this, tr("Delete?"),
                            tr("<p>Are you sure you want to delete all "
                               "of these merge records?</p><p>Note that "
                               "obsolete CRM Accounts will be deleted "
                               "when purging completed merges.</p>"),
                            QMessageBox::No | QMessageBox::Default,
                            QMessageBox::Yes) == QMessageBox::No)
    return;

  for (int i = 0; i < _completed->topLevelItemCount(); i++)
  {
    XTreeWidgetItem *item = _completed->topLevelItem(i);
    XSqlQuery delq;
    delq.prepare("SELECT purgecrmacctmerge(:destcrmacctid) AS RESULT;");
    delq.bindValue(":destcrmacctid", item->id());
    delq.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Purge Error"),
                             delq, __FILE__, __LINE__))
      return;
  }
  sFillList();
}

void CrmaccountMergePurgePage::sFillList()
{
  qDebug("CrmaccountMergePurgePage::sFillList() entered");
  ParameterList params;
  params.append("merged",      tr("Merge complete"));
  params.append("mergedKey",   _data->_mergedKey);
  params.append("selected",    tr("Data selection in progress"));
  params.append("selectedKey", _data->_selectedKey);

  MetaSQLQuery mql(_data->_mqlstr);
  XSqlQuery getq = mql.toQuery(params);
  _completed->populate(getq, true);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting List of Merges"),
                           getq, __FILE__, __LINE__))
    return;

  _completed->expandAll();
  sHandleButtons();
}

void CrmaccountMergePurgePage::sHandleButtons()
{
  foreach (XTreeWidgetItem *item, _completed->selectedItems())
  {
    if (item->id() != item->altId())
      item->setSelected(false);
  }

  _delete->setEnabled(_completed->selectedItems().size() > 0);
  _deleteAll->setEnabled(_completed->topLevelItemCount() > 0);
}
