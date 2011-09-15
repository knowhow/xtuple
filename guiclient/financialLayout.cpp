/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#include "financialLayout.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "financialLayoutColumns.h"
#include "financialLayoutItem.h"
#include "financialLayoutGroup.h"
#include "financialLayoutSpecial.h"

#define cFlRoot  0
#define cFlItem  1
#define cFlGroup 2
#define cFlSpec  3

#define cIncome    0
#define cBalance  1
#define cCash    2
#define cAdHoc    3

financialLayout::financialLayout(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_name, SIGNAL(lostFocus()), this, SLOT(sCheck()));
  connect(_layout, SIGNAL(itemSelectionChanged()), this, SLOT(sHandleButtons()));
  connect(_addTopLevelGroup, SIGNAL(clicked()), this, SLOT(sAddTopLevelGroup()));
  connect(_addGroup, SIGNAL(clicked()), this, SLOT(sAddGroup()));
  connect(_layout, SIGNAL(itemSelected(int)), this, SLOT(sHandleSelection()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_addAccount, SIGNAL(clicked()), this, SLOT(sAddAccount()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_moveUp, SIGNAL(clicked()), this, SLOT(sMoveUp()));
  connect(_moveDown, SIGNAL(clicked()), this, SLOT(sMoveDown()));
  connect(_addSpecial, SIGNAL(clicked()), this, SLOT(sAddSpecial()));
  connect(_income, SIGNAL(clicked()), this, SLOT(sSetType()));
  connect(_balance, SIGNAL(clicked()), this, SLOT(sSetType()));
  connect(_cash, SIGNAL(clicked()), this, SLOT(sSetType()));
  connect(_adHoc, SIGNAL(clicked()), this, SLOT(sSetType()));
  connect(_addCol, SIGNAL(clicked()), this, SLOT(sAddCol()));
  connect(_editCol, SIGNAL(clicked()), this, SLOT(sEditCol()));
  connect(_viewCol, SIGNAL(clicked()), this, SLOT(sEditCol()));
  connect(_deleteCol, SIGNAL(clicked()), this, SLOT(sDeleteCol()));
  connect(_layouts, SIGNAL(itemSelectionChanged()), this, SLOT(sHandleButtonsCol()));
  connect(_layouts, SIGNAL(itemSelected(int)), this, SLOT(sEditCol()));
  connect(_showTotal, SIGNAL(toggled(bool)), this, SLOT(sUncheckAltGrandTotal()));
  connect(_showGrandTotal, SIGNAL(toggled(bool)), this, SLOT(sUncheckAltGrandTotal()));

  _layout->setRootIsDecorated(TRUE);
  _layout->addColumn( tr("Group/Account Name"), -1, Qt::AlignLeft   );

  _layouts->addColumn( tr("Name"), _itemColumn, Qt::AlignLeft, true, "flcol_name");
  _layouts->addColumn( tr("Description"),   -1, Qt::AlignLeft, true, "flcol_descrip");
  
  _cachedType=cIncome;

}

financialLayout::~financialLayout()
{
  // no need to delete child widgets, Qt does it all for us
}

void financialLayout::languageChange()
{
  retranslateUi(this);
}

enum SetResponse financialLayout::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("flhead_id", &valid);
  if (valid)
  {
    _flheadid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      
      q.prepare("SELECT NEXTVAL('flhead_flhead_id_seq') AS flhead_id;");
      q.exec();
      if (q.first())
      {
        _flheadid = q.value("flhead_id").toInt();
        q.prepare("INSERT INTO flhead (flhead_id) values(:flhead_id);");
        q.bindValue(":flhead_id", _flheadid);
        q.exec();
      }
      
      sSetType();
//  ToDo

      _name->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _buttonBox->setFocus();
      _view->setHidden(TRUE);
      _viewCol->setHidden(TRUE);
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _name->setEnabled(FALSE);
      _descrip->setEnabled(FALSE);
      _showTotal->setEnabled(FALSE);
      _showGrandTotal->setEnabled(FALSE);
      _altLabels->setEnabled(FALSE);
      _addTopLevelGroup->setHidden(TRUE);
      _addGroup->setHidden(TRUE);
      _addAccount->setHidden(TRUE);
      _edit->setHidden(TRUE);
      _delete->setHidden(TRUE);
      _moveUp->setHidden(TRUE);
      _moveDown->setHidden(TRUE);
      _reportType->setEnabled(FALSE);
      _addCol->setHidden(TRUE);
      _editCol->setHidden(TRUE);
      _deleteCol->setHidden(TRUE);
    }
  }

  return NoError;
}

void financialLayout::sCheck()
{

  _name->setText(_name->text().trimmed());
  if ((_mode == cNew) || (_name->text().length()))
  {
    q.prepare( "SELECT flhead_id "
               "FROM flhead "
               "WHERE (UPPER(flhead_name)=UPPER(:flhead_name));" );
    q.bindValue(":flhead_name", _name->text());
    q.exec();
    if (q.first())
    {
      _flheadid = q.value("flhead_id").toInt();
      _mode = cEdit;
      _layout->setColumnCount(1);
      populate();

      _name->setEnabled(FALSE);
    }
  }
}

void financialLayout::sSave()
{
  if (_name->text().length() == 0)
  {
    QMessageBox::warning( this, tr("Layout Name is Invalid"),
                          tr("You must enter a valid name for this Financial Report.") );
    _name->setFocus();
    return;
  }
  
  q.prepare( "UPDATE flhead "
             "SET flhead_name=:flhead_name, flhead_descrip=:flhead_descrip,"
             "    flhead_showtotal=:showtotal,"
             "    flhead_showstart=:flhead_showstart,"
             "    flhead_showend=:flhead_showend,"
             "    flhead_showdelta=:flhead_showdelta,"
             "    flhead_showbudget=:flhead_showbudget,"
             "    flhead_showdiff=:flhead_showdiff,"
             "    flhead_showcustom=:flhead_showcustom,"
             "    flhead_type=:flhead_type,"
             "    flhead_active=:flhead_active,"
             "    flhead_custom_label=:flhead_custom_label,"
             "    flhead_usealtbegin=:flhead_usealtbegin,"
             "    flhead_altbegin=:flhead_altbegin,"
             "    flhead_usealtend=:flhead_usealtend,"
             "    flhead_altend=:flhead_altend,"
             "    flhead_usealtdebits=:flhead_usealtdebits,"
             "    flhead_altdebits=:flhead_altdebits,"
             "    flhead_usealtcredits=:flhead_usealtcredits,"
             "    flhead_altcredits=:flhead_altcredits,"
             "    flhead_usealtbudget=:flhead_usealtbudget,"
             "    flhead_altbudget=:flhead_altbudget,"
             "    flhead_usealtdiff=:flhead_usealtdiff,"
             "    flhead_altdiff=:flhead_altdiff,"
             "    flhead_usealttotal=:flhead_usealttotal,"
             "    flhead_alttotal=:flhead_alttotal,"
             "    flhead_notes=:flhead_notes"
             " WHERE (flhead_id=:flhead_id);" );

  q.bindValue(":flhead_name", _name->text());
  q.bindValue(":flhead_descrip", _descrip->text());
  q.bindValue(":showtotal", QVariant((_showTotal->isChecked()) || (_showGrandTotal->isChecked())));
  q.bindValue(":flhead_showstart", QVariant((_showTotal->isChecked() && _showStart->isChecked())));
  q.bindValue(":flhead_showend", QVariant((_showTotal->isChecked() && _showEnd->isChecked()) ||
    ((_showGrandTotal->isChecked()) && (_balance->isChecked()))));
  q.bindValue(":flhead_showdelta", QVariant((_showTotal->isChecked() && _showDelta->isChecked()) ||
     ((_showGrandTotal->isChecked()) && (_cash->isChecked()))));
  q.bindValue(":flhead_showbudget", QVariant((_showTotal->isChecked() && _showBudget->isChecked()) ||
     ((_showGrandTotal->isChecked()) && (!_adHoc->isChecked()))));
  q.bindValue(":flhead_showdiff", QVariant((_showTotal->isChecked() && _showDiff->isChecked()) ||
     ((_showGrandTotal->isChecked()) && (((_income->isChecked())) || (_cash->isChecked())))));
  q.bindValue(":flhead_showcustom", QVariant((_showTotal->isChecked() && _showCustom->isChecked())));
  if ( _income->isChecked() )
    q.bindValue(":flhead_type","I");
  else if ( _balance->isChecked() )
    q.bindValue(":flhead_type","B");
  else if ( _cash->isChecked() )
    q.bindValue(":flhead_type","C");
  else if ( _adHoc->isChecked() )
    q.bindValue(":flhead_type","A");
  q.bindValue(":flhead_active", QVariant(_active->isChecked()));
  q.bindValue(":flhead_id", _flheadid);
  q.bindValue(":flhead_custom_label", _customText->text());
  q.bindValue(":flhead_usealttotal", QVariant(_altTotal->isChecked()));
  q.bindValue(":flhead_alttotal", _altTotalText->text());
  q.bindValue(":flhead_usealtbegin", QVariant(_altBegin->isChecked()));
  q.bindValue(":flhead_altbegin", _altBeginText->text());
  q.bindValue(":flhead_usealtend", QVariant(_altEnd->isChecked()));
  q.bindValue(":flhead_altend", _altEndText->text());
  q.bindValue(":flhead_usealtdebits", QVariant(_altDebits->isChecked()));
  q.bindValue(":flhead_altdebits", _altDebitsText->text());
  q.bindValue(":flhead_usealtcredits", QVariant(_altCredits->isChecked()));
  q.bindValue(":flhead_altcredits", _altCreditsText->text());
  q.bindValue(":flhead_usealtbudget", QVariant(_altBudget->isChecked()));
  q.bindValue(":flhead_altbudget", _altBudgetText->text());
  q.bindValue(":flhead_usealtdiff", QVariant(_altDiff->isChecked()));
  q.bindValue(":flhead_altdiff", _altDiffText->text());
  q.bindValue(":flhead_id", _flheadid);
  q.bindValue(":flhead_notes", _notes->toPlainText());
  q.exec();
  
  done(_flheadid);
}

void financialLayout::populate()
{
  q.prepare( "SELECT flhead_name, flhead_descrip, flhead_showtotal,"
             "       flhead_showstart, flhead_showend,"
             "       flhead_showdelta, flhead_showbudget, flhead_showdiff,"
             "       flhead_showcustom, flhead_type, flhead_active, "
             "       flhead_custom_label,"
             "       flhead_usealtbegin, flhead_altbegin,"
             "       flhead_usealtend, flhead_altend,"
             "       flhead_usealtdebits, flhead_altdebits,"
             "       flhead_usealtcredits, flhead_altcredits,"
             "       flhead_usealtbudget, flhead_altbudget,"
             "       flhead_usealtdiff, flhead_altdiff,"
             "       flhead_usealttotal, flhead_alttotal, "
             "       flhead_notes "
             "FROM flhead "
             "WHERE (flhead_id=:flhead_id);" );
  q.bindValue(":flhead_id", _flheadid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("flhead_name").toString());
    _descrip->setText(q.value("flhead_descrip").toString());
    _notes->setPlainText(q.value("flhead_notes").toString());
    if(_showTotal->isChecked())
    {
      _showStart->setChecked(q.value("flhead_showstart").toBool());
      _showEnd->setChecked(q.value("flhead_showend").toBool());
      _showDelta->setChecked(q.value("flhead_showdelta").toBool());
      _showBudget->setChecked(q.value("flhead_showbudget").toBool());
      _showDiff->setChecked(q.value("flhead_showdiff").toBool());
      _showCustom->setChecked(q.value("flhead_showcustom").toBool());
    }
    if ( q.value("flhead_type").toString() == "I" )
    {
      _showGrandTotal->setChecked(q.value("flhead_showtotal").toBool());
      _cachedType = cIncome;
      _income->setChecked(true);
    }
    else if ( q.value("flhead_type").toString() == "B" )
    {
      _showGrandTotal->setChecked(q.value("flhead_showtotal").toBool());
      _cachedType = cBalance;
      _balance->setChecked(true);
    }
    else if ( q.value("flhead_type").toString() == "C" )
    {
      _showGrandTotal->setChecked(q.value("flhead_showtotal").toBool());
      _cachedType = cCash;
      _cash->setChecked(true);
    }
    else if ( q.value("flhead_type").toString() == "A" )
    {
      _showTotal->setChecked(q.value("flhead_showtotal").toBool());
      _cachedType = cAdHoc;
      _adHoc->setChecked(true);
    }
    _active->setChecked(q.value("flhead_active").toBool());
    _customText->setText(q.value("flhead_custom_label").toString());
    _altTotal->setChecked(q.value("flhead_usealttotal").toBool());
    _altTotalText->setText(q.value("flhead_alttotal").toString());
    _altBegin->setChecked(q.value("flhead_usealtbegin").toBool());
    _altBeginText->setText(q.value("flhead_altbegin").toString());
    _altEnd->setChecked(q.value("flhead_usealtend").toBool());
    _altEndText->setText(q.value("flhead_altend").toString());
    _altDebits->setChecked(q.value("flhead_usealtdebits").toBool());
    _altDebitsText->setText(q.value("flhead_altdebits").toString());
    _altCredits->setChecked(q.value("flhead_usealtcredits").toBool());
    _altCreditsText->setText(q.value("flhead_altcredits").toString());
    _altBudget->setChecked(q.value("flhead_usealtbudget").toBool());
    _altBudgetText->setText(q.value("flhead_altbudget").toString());
    _altDiff->setChecked(q.value("flhead_usealtdiff").toBool());
    _altDiffText->setText(q.value("flhead_altdiff").toString());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  
  sSetType();
}

void financialLayout::sFillList()
{
  int pId = -1, pType = -1;

  XTreeWidgetItem * item = (XTreeWidgetItem*)(_layout->currentItem());
  if(0 != item)
  {
    pId = item->id();
    pType = item->altId();
  }

  _layout->clear();
  _lastStack.clear();
  _last = 0;
  if (_adHoc->isChecked())
  {
    insertFlGroupAdHoc(-1, "", 0, pId, pType);
  }
  else
  {
    insertFlGroup(-1, "", 0, pId, pType);
   }
  _layout->expandAll();

  item = (XTreeWidgetItem*)(_layout->currentItem());
  if(0 != item)
    _layout->scrollToItem(item);

  q.prepare ("SELECT flcol_id, flcol_name, flcol_descrip "
             " FROM flcol "
             " WHERE flcol_flhead_id=:flhead_id "
             " ORDER BY flcol_name, flcol_descrip; ");
  q.bindValue(":flhead_id", _flheadid);
  q.exec();
  _layouts->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void financialLayout::insertFlGroup(int pFlgrpid, QString pFlgrpname, XTreeWidgetItem *pParent, int pId, int pType)
{
  XTreeWidgetItem *root = 0;
  XSqlQuery thisLevel;

  if (pFlgrpid != -1)
  {
    thisLevel.prepare("SELECT CASE WHEN(flgrp_summarize) THEN 'Summarized'"
                      "            WHEN(flgrp_subtotal) THEN 'Subtotal'"
                      "            ELSE ''"
                      "       END AS summary,"
                      "       flgrp_subtract as subtract "
                      "  FROM flgrp"
                      " WHERE ((flgrp_id=:flgrp_id)"
                      "   AND  (flgrp_flhead_id=:flhead_id) ); ");
    thisLevel.bindValue(":flgrp_id", pFlgrpid);
    thisLevel.bindValue(":flhead_id", _flheadid);
    thisLevel.exec();
    thisLevel.first();
    if (pParent)
      _last = root = new XTreeWidgetItem(pParent, _last, pFlgrpid, cFlGroup, pFlgrpname,
                               thisLevel.value("summary").toString(),
                               ( thisLevel.value("subtract").toBool() ? tr("-") : tr("+") ) );
    else
      _last = root = new XTreeWidgetItem(_layout, _last, pFlgrpid, cFlGroup, pFlgrpname,
                               thisLevel.value("summary").toString(),
                               ( thisLevel.value("subtract").toBool() ? tr("-") : tr("+") ) );

    if ( (pFlgrpid == pId) && (pType == cFlGroup) )
    {
      _layout->setCurrentItem(root);
      _layout->scrollToItem(root);
    }

    _lastStack.push(_last);
    _last = 0;
  }

  thisLevel.prepare( "SELECT flgrp_id AS id, :group AS type, flgrp_order AS orderby,"
                     "       flgrp_name AS name,"
                     "       flgrp_subtract AS subtract "
                     "FROM flgrp "
                     "WHERE ( (flgrp_flgrp_id=:flgrp_id)"
                     " AND (flgrp_flhead_id=:flhead_id) ) "

                     "UNION SELECT flitem_id AS id, :item AS type, flitem_order AS orderby,"
                     "             formatflitemdescrip(flitem_id) AS name,"
                     "             flitem_subtract AS subtract "
                     "FROM flitem "
                     "WHERE (flitem_flgrp_id=:flgrp_id) "

                     "UNION SELECT flspec_id AS id, :spec AS type, flspec_order AS orderby,"
                     "             flspec_name AS name,"
                     "             flspec_subtract AS subtract "
                     "FROM flspec "
                     "WHERE (flspec_flgrp_id=:flgrp_id) "

                     "ORDER BY orderby;" );
  thisLevel.bindValue(":group", cFlGroup);
  thisLevel.bindValue(":item", cFlItem);
  thisLevel.bindValue(":spec", cFlSpec);
  thisLevel.bindValue(":flgrp_id", pFlgrpid);
  thisLevel.bindValue(":flhead_id", _flheadid);
  thisLevel.exec();
  while (thisLevel.next())
  {
    if (thisLevel.value("type").toInt() == cFlGroup)
      insertFlGroup(thisLevel.value("id").toInt(), thisLevel.value("name").toString(), root, pId, pType);
    else if (pFlgrpid != -1)
    {
      _last = new XTreeWidgetItem( root, _last,
                                thisLevel.value("id").toInt(), thisLevel.value("type").toInt(),
                                thisLevel.value("name"), "",
                                ( thisLevel.value("subtract").toBool() ? tr("-") : tr("+") ) );

      if ( (thisLevel.value("id").toInt() == pId) && (thisLevel.value("type").toInt() == pType) )
      {
        _layout->setCurrentItem(_last);
        _layout->scrollToItem(root);
      }
    }
  }

  _last = _lastStack.pop();
}

void financialLayout::insertFlGroupAdHoc(int pFlgrpid, QString pFlgrpname, XTreeWidgetItem *pParent, int pId, int pType)
{
  XTreeWidgetItem *root = 0;
  XSqlQuery thisLevel;

  if (pFlgrpid != -1)
  {
    thisLevel.prepare("SELECT CASE WHEN(flgrp_summarize OR flgrp_subtotal) THEN formatBoolYN(flgrp_showstart)"
                      "            ELSE ''"
                      "       END AS showbeg,"
                      "       CASE WHEN(flgrp_summarize OR flgrp_subtotal) THEN formatBoolYN(flgrp_showend)"
                      "            ELSE ''"
                      "       END AS showend,"
                      "       CASE WHEN(flgrp_summarize OR flgrp_subtotal) THEN formatBoolYN(flgrp_showdelta)"
                      "            ELSE ''"
                      "       END AS showdbcr,"
                      "       CASE WHEN(flgrp_summarize OR flgrp_subtotal) THEN formatBoolYN(flgrp_showbudget)"
                      "            ELSE ''"
                      "       END AS showbudget,"
                      "       CASE WHEN(flgrp_summarize OR flgrp_subtotal) THEN formatBoolYN(flgrp_showdiff)"
                      "            ELSE ''"
                      "       END AS showdiff,"
                      "       CASE WHEN(flgrp_summarize OR flgrp_subtotal) THEN formatBoolYN(flgrp_showcustom)"
                      "            ELSE ''"
                      "       END AS showcustom,"
                      "       CASE WHEN(flgrp_summarize) THEN 'Summarized'"
                      "            WHEN(flgrp_subtotal) THEN 'Subtotal'"
                      "            ELSE ''"
                      "       END AS summary,"
                      "       flgrp_subtract as subtract "
                      "  FROM flgrp"
                      " WHERE ((flgrp_id=:flgrp_id)"
                      "   AND  (flgrp_flhead_id=:flhead_id) ); ");
    thisLevel.bindValue(":flgrp_id", pFlgrpid);
    thisLevel.bindValue(":flhead_id", _flheadid);
    thisLevel.exec();
    thisLevel.first();
    if (pParent)
      _last = root = new XTreeWidgetItem(pParent, _last, pFlgrpid, cFlGroup, pFlgrpname,
                               thisLevel.value("showbeg").toString(),
                               thisLevel.value("showend").toString(),
                               thisLevel.value("showdbcr").toString(),
                               thisLevel.value("showbudget").toString(),
                               thisLevel.value("showdiff").toString(),
                               thisLevel.value("showcustom").toString(),
                               thisLevel.value("summary").toString(),
                               ( thisLevel.value("subtract").toBool() ? tr("-") : tr("+") ) );
    else
      _last = root = new XTreeWidgetItem(_layout, _last, pFlgrpid, cFlGroup, pFlgrpname,
                               thisLevel.value("showbeg").toString(),
                               thisLevel.value("showend").toString(),
                               thisLevel.value("showdbcr").toString(),
                               thisLevel.value("showbudget").toString(),
                               thisLevel.value("showdiff").toString(),
                               thisLevel.value("showcustom").toString(),
                               thisLevel.value("summary").toString(),
                               ( thisLevel.value("subtract").toBool() ? tr("-") : tr("+") ) );

    if ( (pFlgrpid == pId) && (pType == cFlGroup) )
    {
      _layout->setCurrentItem(root);
      _layout->scrollToItem(root);
    }

    _lastStack.push(_last);
    _last = 0;
  }

  thisLevel.prepare( "SELECT flgrp_id AS id, :group AS type, flgrp_order AS orderby,"
                     "       flgrp_name AS name,"
                     "       '' AS showbeg,"
                     "       '' AS showend,"
                     "       '' AS showdbcr,"
                     "       '' AS showbudget,"
                     "       '' AS showdiff,"
                     "       '' AS showcustom,"
                     "       flgrp_subtract AS subtract "
                     "FROM flgrp "
                     "WHERE ( (flgrp_flgrp_id=:flgrp_id)"
                     " AND (flgrp_flhead_id=:flhead_id) ) "

                     "UNION SELECT flitem_id AS id, :item AS type, flitem_order AS orderby,"
                     "             formatflitemdescrip(flitem_id) AS name,"
                     "             formatBoolYN(flitem_showstart) AS showbeg,"
                     "             formatBoolYN(flitem_showend) AS showend,"
                     "             formatBoolYN(flitem_showdelta) AS showdbcr,"
                     "             formatBoolYN(flitem_showbudget) AS showbudget,"
                     "             formatBoolYN(flitem_showdiff) AS showdiff,"
                     "             CASE WHEN(NOT flitem_showcustom) THEN formatBoolYN(flitem_showcustom)"
                     "                  WHEN(flitem_custom_source='S') THEN text('BB')"
                     "                  WHEN(flitem_custom_source='E') THEN text('EB')"
                     "                  WHEN(flitem_custom_source='D') THEN text('DB')"
                     "                  WHEN(flitem_custom_source='C') THEN text('CR')"
                     "                  WHEN(flitem_custom_source='B') THEN text('BU')"
                     "                  WHEN(flitem_custom_source='F') THEN text('DI')"
                     "                  ELSE flitem_custom_source"
                     "             END AS showcustom,"
                     "             flitem_subtract AS subtract "
                     "FROM flitem "
                     "WHERE (flitem_flgrp_id=:flgrp_id) "

                     "UNION SELECT flspec_id AS id, :spec AS type, flspec_order AS orderby,"
                     "             flspec_name AS name,"
                     "             formatBoolYN(flspec_showstart) AS showbeg,"
                     "             formatBoolYN(flspec_showend) AS showend,"
                     "             formatBoolYN(flspec_showdelta) AS showdbcr,"
                     "             formatBoolYN(flspec_showbudget) AS showbudget,"
                     "             formatBoolYN(flspec_showdiff) AS showdiff,"
                     "             CASE WHEN(NOT flspec_showcustom) THEN formatBoolYN(flspec_showcustom)"
                     "                  WHEN(flspec_custom_source='S') THEN text('BB')"
                     "                  WHEN(flspec_custom_source='E') THEN text('EB')"
                     "                  WHEN(flspec_custom_source='D') THEN text('DB')"
                     "                  WHEN(flspec_custom_source='C') THEN text('CR')"
                     "                  WHEN(flspec_custom_source='B') THEN text('BU')"
                     "                  WHEN(flspec_custom_source='F') THEN text('DI')"
                     "                  ELSE flspec_custom_source"
                     "             END AS showcustom,"
                     "             flspec_subtract AS subtract "
                     "FROM flspec "
                     "WHERE (flspec_flgrp_id=:flgrp_id) "

                     "ORDER BY orderby;" );
  thisLevel.bindValue(":group", cFlGroup);
  thisLevel.bindValue(":item", cFlItem);
  thisLevel.bindValue(":spec", cFlSpec);
  thisLevel.bindValue(":flgrp_id", pFlgrpid);
  thisLevel.bindValue(":flhead_id", _flheadid);
  thisLevel.exec();
  while (thisLevel.next())
  {
    if (thisLevel.value("type").toInt() == cFlGroup)
      insertFlGroupAdHoc(thisLevel.value("id").toInt(), thisLevel.value("name").toString(), root, pId, pType);
    else if (pFlgrpid != -1)
    {
      _last = new XTreeWidgetItem( root, _last,
                                thisLevel.value("id").toInt(), thisLevel.value("type").toInt(),
                                thisLevel.value("name"), thisLevel.value("showbeg"),
                                thisLevel.value("showend"), thisLevel.value("showdbcr"),
                                thisLevel.value("showbudget"), thisLevel.value("showdiff"),
                                thisLevel.value("showcustom"), "",
                                ( thisLevel.value("subtract").toBool() ? tr("-") : tr("+") ) );

      if ( (thisLevel.value("id").toInt() == pId) && (thisLevel.value("type").toInt() == pType) )
      {
        _layout->setCurrentItem(_last);
        _layout->scrollToItem(root);
      }
    }
  }

  _last = _lastStack.pop();
}

void financialLayout::sHandleButtons()
{
    if (_layout->altId() == cFlRoot)
    {
      _addGroup->setEnabled(TRUE);
      _addAccount->setEnabled(FALSE);
      _addSpecial->setEnabled(FALSE);
      _edit->setEnabled(FALSE);
      _view->setEnabled(FALSE);
      _delete->setEnabled(FALSE);
      _moveUp->setEnabled(FALSE);
      _moveDown->setEnabled(FALSE);
    }
    else if (_layout->altId() == cFlGroup)
    {
      _addGroup->setEnabled(TRUE);
      _addAccount->setEnabled(TRUE);
      _edit->setEnabled(TRUE);
      _view->setEnabled(TRUE);
      _delete->setEnabled(TRUE);
      _moveUp->setEnabled(TRUE);
      _moveDown->setEnabled(TRUE);
      if (_adHoc->isChecked())
        _addSpecial->setEnabled(TRUE);
      else
        _addSpecial->setEnabled(FALSE);
    }
    else if ((_layout->altId() == cFlItem) || (_layout->altId() == cFlSpec))
    {
      _addGroup->setEnabled(FALSE);
      _addAccount->setEnabled(FALSE);
      _addSpecial->setEnabled(FALSE);
      _edit->setEnabled(TRUE);
      _view->setEnabled(TRUE);
      _delete->setEnabled(TRUE);
      _moveUp->setEnabled(TRUE);
      _moveDown->setEnabled(TRUE);
    }
}

void financialLayout::sHandleButtonsCol()
{
      _editCol->setEnabled(TRUE);
      _viewCol->setEnabled(TRUE);
      _deleteCol->setEnabled(TRUE);
}

void financialLayout::sHandleSelection()
{
  if ( (_mode == cEdit) || (_mode == cNew) )
    _edit->animateClick();
}

void financialLayout::sAddTopLevelGroup()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("flhead_id", _flheadid);
  params.append("flgrp_flgrp_id", -1);
  if (_income->isChecked())
    params.append("type", "income");
  else if (_balance->isChecked())
    params.append("type", "balance");
  else if (_cash->isChecked())
    params.append("type", "cash");
  else if (_adHoc->isChecked())
      params.append("type", "adHoc");
  
  financialLayoutGroup newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void financialLayout::sAddGroup()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("flhead_id", _flheadid);
  params.append("flgrp_flgrp_id", _layout->id());
  if (_income->isChecked())
    params.append("type", "income");
  else if (_balance->isChecked())
    params.append("type", "balance");
  else if (_cash->isChecked())
    params.append("type", "cash");
  else if (_adHoc->isChecked())
      params.append("type", "adHoc");
  
  financialLayoutGroup newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void financialLayout::sAddAccount()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("flhead_id", _flheadid);
  params.append("flgrp_id", _layout->id());
  if (_income->isChecked())
    params.append("type", "income");
  else if (_balance->isChecked())
    params.append("type", "balance");
  else if (_cash->isChecked())
    params.append("type", "cash");
  else if (_adHoc->isChecked())
      params.append("type", "adHoc");
  
  financialLayoutItem newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void financialLayout::sAddSpecial()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("flhead_id", _flheadid);
  params.append("flgrp_id", _layout->id());
 
  financialLayoutSpecial newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void financialLayout::sEdit()
{
  bool ok = false;
  ParameterList params;
  if ( (_mode == cEdit) || (_mode == cNew) )
	params.append("mode", "edit");
  else
	params.append("mode", "view");

  if(_layout->altId() == cFlGroup)
  {
    params.append("flgrp_id", _layout->id());
    if (_income->isChecked())
      params.append("type", "income");
    else if (_balance->isChecked())
      params.append("type", "balance");
    else if (_cash->isChecked())
      params.append("type", "cash");
    else if (_adHoc->isChecked())
      params.append("type", "adHoc");

    financialLayoutGroup newdlg(this, "", TRUE);
    newdlg.set(params);
    ok = (newdlg.exec() != XDialog::Rejected);
  }
  else if(_layout->altId() == cFlItem)
  {
    params.append("flitem_id", _layout->id());
    if (_income->isChecked())
      params.append("type", "income");
    else if (_balance->isChecked())
      params.append("type", "balance");
    else if (_cash->isChecked())
      params.append("type", "cash");
    else if (_adHoc->isChecked())
      params.append("type", "adHoc");
  
    financialLayoutItem newdlg(this, "", TRUE);
    newdlg.set(params);
    ok = (newdlg.exec() != XDialog::Rejected);
  }
  else if(_layout->altId() == cFlSpec)
  {
    params.append("flspec_id", _layout->id());
  
    financialLayoutSpecial newdlg(this, "", TRUE);
    newdlg.set(params);
    ok = (newdlg.exec() != XDialog::Rejected);
  }

  if(ok)
    sFillList();
}

void financialLayout::sDelete()
{
  if(_layout->altId() == cFlGroup)
    q.prepare("SELECT deleteFlgrp(:item_id) AS result;");
  else if(_layout->altId() == cFlItem)
    q.prepare( "DELETE FROM flitem WHERE (flitem_id=:item_id);" );
  else if(_layout->altId() == cFlSpec)
    q.prepare( "DELETE FROM flspec WHERE (flspec_id=:item_id);" );

  q.bindValue(":item_id", _layout->id());
  q.exec();

  XTreeWidgetItem *item = _layout->currentItem();
  if (0 != item)
    _layout->setCurrentItem(_layout->topLevelItem(_layout->indexOfTopLevelItem(item) - 1));
  sFillList();
}

void financialLayout::sMoveUp()
{
  if (_layout->altId() == cFlGroup)
    q.prepare("SELECT moveFlGroupUp(:item_id) AS result;");
  else if (_layout->altId() == cFlItem)
    q.prepare("SELECT moveFlItemUp(:item_id) AS result;");
  else if (_layout->altId() == cFlSpec)
    q.prepare("SELECT moveFlSpecUp(:item_id) AS result;");

  q.bindValue(":item_id", _layout->id());
  q.exec();

  sFillList();
}

void financialLayout::sMoveDown()
{
  if (_layout->altId() == cFlGroup)
    q.prepare("SELECT moveFlGroupDown(:item_id) AS result;");
  else if (_layout->altId() == cFlItem)
    q.prepare("SELECT moveFlItemDown(:item_id) AS result;");
  else if (_layout->altId() == cFlSpec)
    q.prepare("SELECT moveFlSpecDown(:item_id) AS result;");

  q.bindValue(":item_id", _layout->id());
  q.exec();

  sFillList();
}

void financialLayout::sSetType()
{
  //Check for column layout definitions and respond
  if ((_income->isChecked()  && _cachedType != cIncome)  || 
      (_balance->isChecked() && _cachedType != cBalance) ||
      (_cash->isChecked()    && _cachedType != cCash)    ||
      (_adHoc->isChecked()   && _cachedType != cAdHoc))
  {
    q.prepare(  "SELECT flcol_id FROM flcol WHERE (flcol_flhead_id=:flheadid); ");
    q.bindValue(":flheadid",_flheadid);
    q.exec();
    if (q.first())
    {
        QMessageBox::critical( this, tr("Cannot Change Type"),
              tr("All column layouts must be deleted before changing the type.") );
        if (_cachedType == cAdHoc)
          _adHoc->setChecked(TRUE);
        else if (_cachedType == cIncome)
          _income->setChecked(TRUE);
        else if (_cachedType == cBalance)
          _balance->setChecked(TRUE);
        else if (_cachedType == cCash)
          _cash->setChecked(TRUE);
        return;
    }
  }

  //Check to see if groups and items are defined, if so warn about changes
  if ((_income->isChecked()  && _cachedType != cIncome)  || 
      (_balance->isChecked() && _cachedType != cBalance) ||
      (_cash->isChecked()    && _cachedType != cCash))
  {    
    q.prepare(  "SELECT flgrp_id FROM flgrp WHERE (flgrp_flhead_id=:flheadid) "
          "UNION "
          "SELECT flitem_id FROM flitem WHERE (flitem_flhead_id=:flheadid) "
          "UNION "
          "SELECT flhead_id FROM flhead WHERE (flhead_id=:flheadid);");
    q.bindValue(":flheadid",_flheadid);
    q.exec();

    if (q.first())
    {
      if ( QMessageBox::warning( this, tr("Report type changed"),
          tr( "Existing row definitions will be changed.\n"
          "Group percentage settings will be reset.\n"
          "Continue?" ),
          tr("Yes"), tr("No"), QString::null ) == 1 )
      {
        if (_cachedType == cAdHoc)
          _adHoc->setChecked(TRUE);
        else if (_cachedType == cIncome)
          _income->setChecked(TRUE);
        else if (_cachedType == cBalance)
          _balance->setChecked(TRUE);
        else if (_cachedType == cCash)
          _cash->setChecked(TRUE);
        return;
      }
      else
      {
        _showTotal->setChecked(FALSE);
          
        q.prepare("UPDATE flgrp "
              " SET flgrp_showstart=false,flgrp_showend=:showend,flgrp_showdelta=:showdelta,"
              "     flgrp_showbudget=true,flgrp_showdiff=:showdiff,flgrp_showcustom=false,"
              "     flgrp_showstartprcnt=false,flgrp_showendprcnt=false,flgrp_showdeltaprcnt=false,"
              "     flgrp_showbudgetprcnt=false,flgrp_showdiffprcnt=false,flgrp_showcustomprcnt=false"
              " WHERE (flgrp_flhead_id=:flhead_id); "
              " UPDATE flitem "
              " SET flitem_showstart=false,flitem_showend=:showend,flitem_showdelta=:showdelta,"
              "     flitem_showbudget=true,flitem_showdiff=:showdiff,flitem_showcustom=false,"
              "     flitem_showstartprcnt=false,flitem_showendprcnt=false,flitem_showdeltaprcnt=false,"
              "     flitem_showbudgetprcnt=false,flitem_showdiffprcnt=false,flitem_showcustomprcnt=false"
              " WHERE (flitem_flhead_id=:flhead_id); "
              " UPDATE flhead "
              " SET flhead_type=:type,flhead_showtotal=false,"
              "    flhead_showstart=false,flhead_showend=false,flhead_showdelta=false,"
              "     flhead_showbudget=true,flhead_showdiff=false,flhead_showcustom=false,"  
              "    flhead_custom_label=NULL,flhead_usealttotal=false,flhead_alttotal=NULL,"
              "     flhead_usealtbegin=false,flhead_altbegin=NULL,flhead_usealtend=false,"
              "     flhead_altend=NULL,flhead_usealtdebits=false,flhead_altdebits=NULL,"
              "     flhead_usealtcredits=false,flhead_altcredits=NULL,flhead_usealtbudget=false,"
              "     flhead_altbudget=NULL,flhead_usealtdiff=false,flhead_altdiff=NULL"
              " WHERE (flhead_id=:flhead_id);");
        
        if (_income->isChecked())
        {
          q.bindValue(":showend","false");
          q.bindValue(":showdelta","false");
          q.bindValue(":showdiff","true");
          q.bindValue(":type","I");  
          _cachedType=cIncome;  
        }
        else if (_balance->isChecked())
        {
          q.bindValue(":showend","true");
          q.bindValue(":showdelta","false");
          q.bindValue(":showdiff","false");
          q.bindValue(":type","B");
          _cachedType=cBalance;
        }
        if (_cash->isChecked())
        {
          q.bindValue(":showend","false");
          q.bindValue(":showdelta","true");
          q.bindValue(":showdiff","true");
          q.bindValue(":type","C");
          _cachedType=cCash;
        }
        q.bindValue(":flhead_id",_flheadid);
        q.exec();
      }
    } 
  }
      
  if ((_layout->columnCount() == 1) ||
      ((_layout->columnCount() <= 4) && (_adHoc->isChecked())) ||
      ((_layout->columnCount() > 4) && (!_adHoc->isChecked())) )
  {
    _layout->clear();
    _layout->setColumnCount(1);

    if ( _adHoc->isChecked() )
    {
      _tab->setTabEnabled(1,FALSE);
      _showTotal->setHidden(FALSE);
      _showGrandTotal->setHidden(TRUE);
      _addSpecial->setHidden(FALSE);
      _altBegin->setHidden(FALSE);
      _altBeginText->setHidden(FALSE);
      _altEnd->setHidden(FALSE);
      _altEndText->setHidden(FALSE);
      _customText->setHidden(FALSE);
      _altDiff->setHidden(FALSE);
      _altDiffText->setHidden(FALSE);
      _customLit->setHidden(FALSE);
      _cachedType=cAdHoc;

      _layout->addColumn( tr("Show\nBeg'ng"),        _ynColumn, Qt::AlignCenter );
      _layout->addColumn( tr("Show\nEnd"),           _ynColumn, Qt::AlignCenter );
      _layout->addColumn( tr("Show\nDb/Cr"),         _ynColumn, Qt::AlignCenter );
      _layout->addColumn( tr("Show\nBudget"),        _ynColumn, Qt::AlignCenter );
      _layout->addColumn( tr("Show\nDiff."),         _ynColumn, Qt::AlignCenter );
      _layout->addColumn( tr("Show\nCustom"),        _ynColumn, Qt::AlignCenter );
    }
    else
    {
      _tab->setTabEnabled(1,TRUE);
      _showTotal->setHidden(TRUE);
      _showGrandTotal->setHidden(FALSE);
      _addSpecial->setHidden(TRUE);
      _altBegin->setHidden(TRUE);
      _altBeginText->setHidden(TRUE);
      _altEnd->setHidden(TRUE);
      _altEndText->setHidden(TRUE);
      _altDiff->setHidden(TRUE);
      _altDiffText->setHidden(TRUE);
      _customText->setHidden(TRUE);
      _customLit->setHidden(TRUE);
    }

    _layout->addColumn( tr("Sub./Summ."),       _dateColumn, Qt::AlignCenter );
    _layout->addColumn( tr("Oper."),              _ynColumn, Qt::AlignCenter );
      
    sFillList();
  }

  //Change the display if necessary  
  if ((_cachedType == cCash) || (_cachedType == cAdHoc))
  {
    _altDebits->setHidden(FALSE);
    _altDebitsText->setHidden(FALSE);
    _altCredits->setHidden(FALSE);
    _altCreditsText->setHidden(FALSE);
  }
  else
  {
    _altDebits->setHidden(TRUE);
    _altDebitsText->setHidden(TRUE);
    _altCredits->setHidden(TRUE);
    _altCreditsText->setHidden(TRUE);
  }

}

void financialLayout::sAddCol()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("flhead_id", _flheadid);
  if (_income->isChecked())
    params.append("type", "income");
  if (_balance->isChecked())
    params.append("type", "balance");
  if (_cash->isChecked())
    params.append("type", "cash");
  if (_adHoc->isChecked())
    params.append("type", "adhoc");
  financialLayoutColumns newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void financialLayout::sEditCol()
{
  bool ok = false;
  ParameterList params;
  if ( (_mode == cEdit) || (_mode == cNew) )
	params.append("mode", "edit");
  else
	params.append("mode", "view");
  params.append("flhead_id", _flheadid);
  params.append("flcol_id", _layouts->id());
  if (_income->isChecked())
    params.append("type", "income");
  if (_balance->isChecked())
    params.append("type", "balance");
  if (_cash->isChecked())
    params.append("type", "cash");
  if (_adHoc->isChecked())
    params.append("type", "adhoc");
  financialLayoutColumns newdlg(this, "", TRUE);
  newdlg.set(params);
  ok = (newdlg.exec() != XDialog::Rejected);
  
  if(ok)
    sFillList();
}

void financialLayout::sDeleteCol()
{
  q.prepare( "DELETE FROM flcol WHERE (flcol_id=:flcol_id);" );

  q.bindValue(":flcol_id", _layouts->id());
  q.exec();

  XTreeWidgetItem *item = _layout->currentItem();
  if(0 != item)
    _layout->setCurrentItem(_layout->topLevelItem(_layout->indexOfTopLevelItem(item) - 1));
  sFillList();
}

void financialLayout::sUncheckAltGrandTotal()
{
  if ((!_showTotal->isChecked()) && (!_showGrandTotal->isChecked()))
    _altTotal->setChecked(FALSE);
}

void financialLayout::reject()
{
  if(cNew == _mode)
  {
    q.prepare("DELETE FROM flspec WHERE flspec_flhead_id=:flhead_id;"
              "DELETE FROM flitem WHERE flitem_flhead_id=:flhead_id;"
              "DELETE FROM flgrp  WHERE flgrp_flhead_id=:flhead_id;"
              "DELETE FROM flhead WHERE flhead_id=:flhead_id;");
    q.bindValue(":flhead_id", _flheadid);
    q.exec();
  }

  XDialog::reject();
}
