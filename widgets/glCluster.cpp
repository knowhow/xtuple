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
#include <QMessageBox>
#include "glcluster.h"

GLClusterLineEdit::GLClusterLineEdit(QWidget* pParent, const char* pName) :
    VirtualClusterLineEdit(pParent, "accnt", "accnt_id", "accnt_name", "accnt_descrip", "accnt_extref", 0, pName, "accnt_active")
{
  setTitles(tr("Account"), tr("Accounts"));
  setUiName("accountNumber");
  setEditPriv("MaintainChartOfAccounts");
  setViewPriv("ViewChartOfAccounts");
  setNewPriv("MaintainChartOfAccounts");

  _showExternal = false;
  _ignoreCompany = false;
  if (_x_metrics)
  {
    if (_x_metrics->value("GLCompanySize").toInt() == 0)
      _ignoreCompany = true;
  }

  _query = "SELECT accnt_id AS id, accnt_name AS number, "
           "  accnt_descrip AS name, accnt_extref AS description, "
           "  accnt_active AS active, accnt_type, "
           "  COALESCE(company_id, -1) AS company_id, "
           "  COALESCE(company_yearend_accnt_id, -1) AS company_yearend_accnt_id, "
           "  COALESCE(company_gainloss_accnt_id, -1) AS company_gainloss_accnt_id, "
           "  COALESCE(company_dscrp_accnt_id, -1) AS company_dscrp_accnt_id "
           "FROM ONLY accnt "
           "  LEFT OUTER JOIN company ON (accnt_company=company_number) "
           "WHERE (true) ";

  _typeMap.insert("A", tr("Asset"));
  _typeMap.insert("L",tr("Liability"));
  _typeMap.insert("R", tr("Revenue"));
  _typeMap.insert("E",tr("Expense"));
  _typeMap.insert("Q", tr("Equity"));

  setType(GLCluster::cUndefined);
}

void GLClusterLineEdit::setId(const int pId)
{
  _query.replace("FROM ONLY accnt","FROM accnt");
  VirtualClusterLineEdit::setId(pId);
  _query.replace("FROM accnt","FROM ONLY accnt");
}

void GLClusterLineEdit::setType(unsigned int pType)
{
  _type = pType;
  buildExtraClause();
}

void GLClusterLineEdit::setShowExternal(bool p)
{
  _showExternal = p;
  buildExtraClause();
}

void GLClusterLineEdit::setIgnoreCompany(bool p)
{
  _ignoreCompany = p;
}

int GLClusterLineEdit::companyId()
{
  if (model())
  {
    if (model()->rowCount())
      return    model()->data(model()->index(0,6)).toInt();
  }
  return -1;
}

void GLClusterLineEdit::buildExtraClause()
{
  _extraClause.clear();
  _types.clear();

  if(_type > 0)
  {
    if(_type & GLCluster::cAsset ||
       _type & GLCluster::cUndefined)
      _types << ("A");
    if(_type & GLCluster::cLiability ||
       _type & GLCluster::cUndefined)
      _types << ("L");
    if(_type & GLCluster::cExpense ||
       _type & GLCluster::cUndefined)
      _types << ("E");
    if(_type & GLCluster::cRevenue ||
       _type & GLCluster::cUndefined)
      _types << ("R");
    if(_type & GLCluster::cEquity ||
       _type & GLCluster::cUndefined)
      _types << ("Q");
  }

  if(!_types.isEmpty())
    _extraClause = "(accnt_type IN ('" + _types.join("','") + "'))";

  if (!_extraClause.isEmpty() && !_showExternal)
    _extraClause += " AND ";

  if (!_showExternal)
    _extraClause += "(NOT COALESCE(company_external, false)) ";
}

void GLClusterLineEdit::sList()
{
  disconnect(this, SIGNAL(editingFinished()), this, SLOT(sParse()));

  accountList* newdlg = listFactory();
  if (newdlg)
  {
    ParameterList params;
    params.append("accnt_id", _id);
    params.append("type", _type);
    if (_showExternal)
      params.append("showExternal");
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

void GLClusterLineEdit::sSearch()
{
  disconnect(this, SIGNAL(editingFinished()), this, SLOT(sParse()));

  accountSearch* newdlg = searchFactory();
  if (newdlg)
  {
    ParameterList params;
    params.append("accnt_id", _id);
    params.append("type", _type);
    if (_showExternal)
      params.append("showExternal");
    newdlg->set(params);
    newdlg->setSearchText(text());
    int id = newdlg->exec();
    setId(id);
  }
  else
    QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__),
                          tr("%1::sSearch() not yet defined")
                          .arg(metaObject()->className()));

  connect(this, SIGNAL(editingFinished()), this, SLOT(sParse()));
}

void GLClusterLineEdit::sParse()
{
  int oldid = _id;
  setStrict(false);
  VirtualClusterLineEdit::sParse();

  if (_id != oldid &&
      model()->rowCount())
  {
    QString type = model()->data(model()->index(0,5)).toString();
    int yearendid = model()->data(model()->index(0,7)).toInt();
    int gainlossid = model()->data(model()->index(0,8)).toInt();
    int dscrpid = model()->data(model()->index(0,9)).toInt();

    if (!_ignoreCompany &&
        (yearendid == -1 ||
         gainlossid == -1 ||
         dscrpid == -1 ))
    {
      QMessageBox::critical(this,tr("Company Incomplete"),
                            tr("The Company associated with this Account has incomplete information. "
                               "You must complete the Company record to use this Account."));
      setId(-1);
    }
    else if (_type && !_types.contains(type))
    {
      QStringList _typeNameList;
      for (int i = 0; i < _types.count(); i++)
        _typeNameList.append(_typeMap.value(_types.at(i)));
      QString _typeNames = _typeNameList.join(" or ");

      QString msg = tr("The Account used here would typically be type %1. "
                       "Selecting this Account may cause unexpected results in your "
                       "General Ledger. Are you sure this is the Account you "
                       "want to use here?").arg(_typeNames);
      if (QMessageBox::warning(this, tr("Non-standard Account Type"),msg,
                               QMessageBox::No | QMessageBox::Default |
                               QMessageBox::Yes  | QMessageBox::Escape) == QMessageBox::No)
        setId(-1);
    }
  }
  setStrict(true);
}

accountList* GLClusterLineEdit::listFactory()
{
  return new accountList(this);
}

accountSearch* GLClusterLineEdit::searchFactory()
{
  return new accountSearch(this);
}

//////////////////////////////////////

GLCluster::GLCluster(QWidget *pParent, const char *pName) :
    VirtualCluster(pParent, pName)
{
  addNumberWidget(new GLClusterLineEdit(this, pName));
  _name->show();

  _projectVisible = false;

  _projectLit = new QLabel("-", this);
  _projectLit->setVisible(false);
  _grid->addWidget(_projectLit, 0, 2);

  _project = new ProjectLineEdit(this);
  _project->setAllowedStatuses(ProjectLineEdit::InProcess);
  _project->setEnabled(false);
  _project->setVisible(false);
  _project->setNullStr(tr("project"));
  _grid->addWidget(_project, 0, 3);

  setFocusProxy(_number);
  setTabOrder(_number, _project);
  setOrientation(Qt::Horizontal);
}

bool GLCluster::projectVisible()
{
  return _project->isVisible();
}

void GLCluster::setId(const int p)
{
  int id = p;

  if (_x_metrics)
  {
    if (_x_metrics->boolean("EnableProjectAccounting"))
    {
      XSqlQuery qry;
      qry.prepare("SELECT accnt_id, prjaccnt_prj_id "
                  "FROM xtprjaccnt.prjaccnt "
                  "WHERE (accnt_id=:accnt_id); ");
      qry.bindValue(":accnt_id", p);
      qry.exec();
      if (qry.first())
      {
        id = qry.value("accnt_id").toInt();
        if (qry.value("prjaccnt_prj_id").toInt() != -1 &&
            !_projectVisible)
          setProjectVisible(true);
        if (_projectVisible)
          _project->setId(qry.value("prjaccnt_prj_id").toInt());
      }
    }

    VirtualCluster::setId(id);
  }
}

bool GLCluster::setProjectVisible(bool p)
{
  if (_x_metrics && p == true)
  {
    if (!_x_metrics->boolean("EnableProjectAccounting"))
      return false;
  }

  GLClusterLineEdit* number = static_cast<GLClusterLineEdit*>(_number);
  number->disconnect(this, SLOT(sHandleProjectState(int)));
  _project->disconnect(this, SLOT(sHandleProjectId()));

  _projectLit->setVisible(p);
  _project->setVisible(p);
  _projectVisible = p;

  if (p)
  {
    setOrientation(Qt::Vertical);
    connect(number, SIGNAL(newId(int)), this, SLOT(sHandleProjectState(int)));
    connect(_project, SIGNAL(newId(int)), this, SLOT(sHandleProjectId()));
  }
  else
    setOrientation(Qt::Horizontal);

  return (p);
}

void GLCluster::sHandleProjectState(int p)
{
  if (p == -1)
  {
    _project->setId(-1);
    _project->setEnabled(false);
  }
  else if (_project->isEnabled())
    return;
  else
  {
    _project->setEnabled(true);
    _project->setFocus();
  }
}

void GLCluster::sHandleProjectId()
{
  XSqlQuery qry;
  qry.prepare("SELECT getPrjAccntId(:prj_id,:accnt_id) AS accnt_id");
  qry.bindValue(":prj_id", _project->id());
  qry.bindValue(":accnt_id", _number->id());
  qry.exec();
  qry.first();
  _number->setId(qry.value("accnt_id").toInt());
}

///////////////////////////////////////////////////////////////////////////////

accountList::accountList(QWidget* pParent, Qt::WindowFlags pFlags) :
    VirtualList(pParent, pFlags)
{
  setObjectName("accountList");
  setMinimumWidth(600);
  _search->hide();
  _searchLit->hide();
  disconnect(_search,  SIGNAL(textChanged(const QString&)), this, SLOT(sSearch(const QString&)));

  _listTab->setColumnCount(0);
  if (_x_metrics)
  {
    if (_x_metrics->value("GLCompanySize").toInt() > 0)
      _listTab->addColumn(tr("Company"), 50, Qt::AlignCenter, true, "accnt_company");

    if (_x_metrics->value("GLProfitSize").toInt() > 0)
      _listTab->addColumn(tr("Profit"), 50, Qt::AlignCenter,  true, "accnt_profit");
  }

  _listTab->addColumn(tr("Account Number"), 100, Qt::AlignCenter, true, "accnt_number");

  if (_x_metrics)
  {
    if (_x_metrics->value("GLSubaccountSize").toInt() > 0)
      _listTab->addColumn(tr("Sub."), 50, Qt::AlignCenter, true, "accnt_sub");
  }

  _listTab->addColumn(tr("Description"), -1, Qt::AlignLeft, true, "accnt_descrip");
  _listTab->addColumn(tr("Type"),            75, Qt::AlignLeft ,  true, "accnt_type");
  _listTab->addColumn(tr("Sub. Type Code"),  75, Qt::AlignLeft,  false, "subaccnttype_code");
  _listTab->addColumn(tr("Sub. Type"),      100, Qt::AlignLeft,  false, "subaccnttype_descrip");

  setWindowTitle(tr("Account Numbers"));
  _titleLit->setText(tr("Chart of Accounts"));
  _parent = (GLClusterLineEdit*)(pParent);
}

XTreeWidget* accountList::xtreewidget()
{
  return _listTab;
}

void accountList::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("accnt_id", &valid);
  if (valid)
    _accntid = param.toInt();

  param = pParams.value("type", &valid);
  if (valid)
    _type = param.toUInt();

  _showExternal = pParams.inList("showExternal");
}

void accountList::sFillList()
{
  QString sql("SELECT accnt_id, *, "
              "       CASE WHEN(accnt_type='A') THEN '" + tr("Asset") + "'"
              "            WHEN(accnt_type='E') THEN '" + tr("Expense") + "'"
              "            WHEN(accnt_type='L') THEN '" + tr("Liability") + "'"
              "            WHEN(accnt_type='Q') THEN '" + tr("Equity") + "'"
              "            WHEN(accnt_type='R') THEN '" + tr("Revenue") + "'"
              "            ELSE accnt_type"
              "       END AS accnt_type_qtdisplayrole "
              "FROM (ONLY accnt LEFT OUTER JOIN"
              "     company ON (accnt_company=company_number)) "
              "     LEFT OUTER JOIN subaccnttype ON (accnt_type=subaccnttype_accnt_type AND accnt_subaccnttype_code=subaccnttype_code) ");

  QStringList types;
  QStringList where;

  if(_type > 0)
  {
    if(_type & GLCluster::cAsset)
      types << ("'A'");
    if(_type & GLCluster::cLiability)
      types << ("'L'");
    if(_type & GLCluster::cExpense)
      types << ("'E'");
    if(_type & GLCluster::cRevenue)
      types << ("'R'");
    if(_type & GLCluster::cEquity)
      types << ("'Q'");
  }
  if(!types.isEmpty())
    where << ("(accnt_type IN (" + types.join(",") + ")) ");

  if (! _showExternal)
    where << "(NOT COALESCE(company_external, false)) ";

  where << "accnt_active ";
  where << "(company_yearend_accnt_id <> -1)";
  where << "(company_gainloss_accnt_id <> -1)";
  where << "(company_dscrp_accnt_id <> -1)";

  if (!where.isEmpty())
    sql += " WHERE " + where.join(" AND ");

  sql += "ORDER BY accnt_number, accnt_sub, accnt_profit;";

  _listTab->populate(sql, _accntid);
}

///////////////////////////

accountSearch::accountSearch(QWidget* pParent, Qt::WindowFlags pFlags)
    : VirtualSearch(pParent, pFlags)
{
  setAttribute(Qt::WA_DeleteOnClose);
  setObjectName( "accountSearch" );
  setMinimumWidth(600);

  _accntid = -1;
  _typeval = 0;

  setWindowTitle( tr( "Search for Account" ) );

  _typeLit = new QLabel(tr("Type:"));
  _type = new QComboBox();
  _type->addItem(tr("ALL"), QVariant(""));
  _type->addItem(tr("Asset"), QVariant("A"));
  _type->addItem(tr("Liability"), QVariant("L"));
  _type->addItem(tr("Expense"), QVariant("E"));
  _type->addItem(tr("Revenue"), QVariant("R"));
  _type->addItem(tr("Equity"), QVariant("Q"));

  _typeStrLyt = new QHBoxLayout(this);
  _typeStrLyt->setObjectName("typeStrLyt");
  _typeStrLyt->addWidget(_typeLit);
  _typeStrLyt->addWidget(_type);
  _typeStrLyt->addItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Fixed));
  searchLyt->addItem(_typeStrLyt);

  // signals and slots connections
  connect( _type, SIGNAL(currentIndexChanged(int)), this, SLOT( sFillList()));

  _listTab->setColumnCount(0);
  if (_x_metrics)
  {
    if (_x_metrics->value("GLCompanySize").toInt() > 0)
      _listTab->addColumn(tr("Company"), 50, Qt::AlignCenter, true, "accnt_company");

    if (_x_metrics->value("GLProfitSize").toInt() > 0)
      _listTab->addColumn(tr("Profit"), 50, Qt::AlignCenter, true, "accnt_profit");
  }

  _listTab->addColumn(tr("Account Number"), 100, Qt::AlignCenter, true, "accnt_number");

  if (_x_metrics)
  {
    if (_x_metrics->value("GLSubaccountSize").toInt() > 0)
      _listTab->addColumn(tr("Sub."), 50, Qt::AlignCenter, true, "accnt_sub");
  }

  _listTab->addColumn(tr("Description"), -1, Qt::AlignLeft, true, "accnt_descrip");
  _listTab->addColumn(tr("Type"),            75, Qt::AlignLeft ,  true, "accnt_type");
  _listTab->addColumn(tr("Sub. Type Code"),  75, Qt::AlignLeft,  false, "subaccnttype_code");
  _listTab->addColumn(tr("Sub. Type"),      100, Qt::AlignLeft,  false, "subaccnttype_descrip");

  disconnect(_searchNumber,  SIGNAL(clicked()),	        this, SLOT(sFillList()));
  disconnect(_searchDescrip, SIGNAL(clicked()),  	this, SLOT(sFillList()));
  _searchNumber->hide();
  _searchName->hide();
  _searchDescrip->hide();

  _showExternal = false;
}

void accountSearch::showEvent(QShowEvent* e)
{
  if (!_search->text().isEmpty())
    sFillList();
  VirtualSearch::showEvent(e);
}

void accountSearch::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("accnt_id", &valid);
  if (valid)
    _accntid = param.toInt();

  param = pParams.value("type", &valid);
  if (valid)
  {
    _typeval = param.toUInt();
    if(_typeval > 0)
    {
      if(!(_typeval & GLCluster::cEquity))
        _type->removeItem(5);
      if(!(_typeval & GLCluster::cRevenue))
        _type->removeItem(4);
      if(!(_typeval & GLCluster::cExpense))
        _type->removeItem(3);
      if(!(_typeval & GLCluster::cLiability))
        _type->removeItem(2);
      if(!(_typeval & GLCluster::cAsset))
        _type->removeItem(1);
    }
  }

  _showExternal = pParams.inList("showExternal");
}

void accountSearch::sFillList()
{
  QString sql("SELECT accnt_id, *,"
              "       CASE WHEN(accnt_type='A') THEN '" + tr("Asset") + "'"
              "            WHEN(accnt_type='E') THEN '" + tr("Expense") + "'"
              "            WHEN(accnt_type='L') THEN '" + tr("Liability") + "'"
              "            WHEN(accnt_type='Q') THEN '" + tr("Equity") + "'"
              "            WHEN(accnt_type='R') THEN '" + tr("Revenue") + "'"
              "            ELSE accnt_type"
              "       END AS accnt_type_qtdisplayrole "
              "FROM (ONLY accnt LEFT OUTER JOIN"
              "     company ON (accnt_company=company_number)) "
              "     LEFT OUTER JOIN subaccnttype ON (accnt_type=subaccnttype_accnt_type AND accnt_subaccnttype_code=subaccnttype_code) ");

  QStringList types;
  QStringList where;

  if(_type->currentIndex() > 0)
  {
    where << "(accnt_type = '" + _type->itemData(_type->currentIndex()).toString() + "')";
  }
  else if(_typeval > 0)
  {
    if(_typeval & GLCluster::cAsset)
      types << ("'A'");
    if(_typeval & GLCluster::cLiability)
      types << ("'L'");
    if(_typeval & GLCluster::cExpense)
      types << ("'E'");
    if(_typeval & GLCluster::cRevenue)
      types << ("'R'");
    if(_typeval & GLCluster::cEquity)
      types << ("'Q'");
  }
  if(!types.isEmpty())
    where << "(accnt_type IN (" + types.join(",") + "))";

  if (!_search->text().isEmpty())
    where << "((accnt_name ~* :descrip) OR (accnt_descrip ~* :descrip) OR (accnt_extref ~* :descrip))";

  if (! _showExternal)
    where << "(NOT COALESCE(company_external, false))";

  where << "accnt_active ";

  if (!where.isEmpty())
    sql += " WHERE " + where.join(" AND ");

  sql += "ORDER BY accnt_number, accnt_sub, accnt_profit;";

  XSqlQuery qry;
  qry.prepare(sql);
  qry.bindValue(":descrip", _search->text());
  qry.exec();
  _listTab->populate(qry, _accntid);
}



