/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "display.h"
#include "xlineedit.h"
#include "ui_display.h"

#include <QSqlError>
#include <QMessageBox>
#include <QPrinter>
#include <QPrintDialog>
#include <QShortcut>
#include <QToolButton>

#include <metasql.h>
#include <mqlutil.h>
#include <orprerender.h>
#include <orprintrender.h>
#include <renderobjects.h>
#include <parameter.h>
#include <previewdialog.h>

class displayPrivate : public Ui::display
{
public:
  displayPrivate(::display * parent) : _parent(parent)
  {
    setupUi(_parent);
    _print->hide();
    _preview->hide();
    _new->hide();
    _queryonstart->hide(); // hide until query on start enabled
    _autoupdate->hide(); // hide until auto update is enabled
    _parameterWidget->hide(); // hide until user shows manually
    _search->hide();
    _searchLit->hide();
    _listLabelFrame->setVisible(false);
    _useAltId = false;
    _queryOnStartEnabled = false;
    _autoUpdateEnabled = false;

    bool useToolbar = _metrics->boolean("DisplaysUseToolbar");

    // Build Toolbar even if we hide it so we get actions
    _newBtn = new QToolButton(_toolBar);
    _newBtn->setObjectName("_newBtn");
    _newBtn->setFocusPolicy(Qt::NoFocus);
    _newAct = _toolBar->addWidget(_newBtn);
    _newAct->setVisible(false);

    _closeBtn = new QToolButton(_toolBar);
    _closeBtn->setObjectName("_closeBtn");
    _closeBtn->setFocusPolicy(Qt::NoFocus);
    _closeAct = _toolBar->addWidget(_closeBtn);

    _sep1 = _toolBar->addSeparator();

    // Move parameter widget controls into toolbar
    _moreBtn = new QToolButton(_toolBar);
    _moreBtn->setObjectName("_moreBtn");
    _moreBtn->setFocusPolicy(Qt::NoFocus);
    _moreBtn->setCheckable(true);
    _moreAct = _toolBar->addWidget(_moreBtn);
    _moreAct->setVisible(false);

    if (useToolbar)
    {
      QLabel* filterListLit = _parent->findChild<QLabel*>("_filterListLit");
      filterListLit->setContentsMargins(3,0,3,0);
      XComboBox* filterList = _parent->findChild<XComboBox*>("_filterList");

      _filterLitAct = _toolBar->insertWidget(_moreAct, filterListLit);
      _filterLitAct->setVisible(false);

      _filterAct = _toolBar->insertWidget(_moreAct, filterList);
      _filterAct->setVisible(false);
    }

    _sep2 = _toolBar->addSeparator();
    _sep2->setVisible(false);

    // Print buttons
    _printBtn = new QToolButton(_toolBar);
    _printBtn->setObjectName("_printBtn");
    _printBtn->setFocusPolicy(Qt::NoFocus);
    _printAct = _toolBar->addWidget(_printBtn);
    _printAct->setVisible(false); // hide the print button until a reportName is set

    _previewBtn = new QToolButton(_toolBar);
    _previewBtn->setObjectName("_previewBtn");
    _printBtn->setFocusPolicy(Qt::NoFocus);
    _previewAct = _toolBar->addWidget(_previewBtn);
    _previewAct->setVisible(false); // hide the preview button until a reportName is set
    _sep3 = _toolBar->addSeparator();
    _sep3->setVisible(false);

    if (useToolbar)
    {
      // Optional search widget in toolbar
      _search->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
      _searchAct = _toolBar->addWidget(_search);
      _searchAct->setVisible(false);
    }
    else
    {
      _searchAct = new QAction(_search);
      _search->addAction(_searchAct);
    }

    // Remaining buttons in toolbar
    _queryBtn = new QToolButton(_toolBar);
    _queryBtn->setObjectName("_queryBtn");
    _queryBtn->setFocusPolicy(Qt::NoFocus);
    _queryAct = _toolBar->addWidget(_queryBtn);

    // Menu actions for query options
    _queryMenu = new QMenu(_queryBtn);
    _queryOnStartAct = new QAction(_queryMenu);
    _queryOnStartAct->setCheckable(true);
    _queryOnStartAct->setVisible(false);
    _queryMenu->addAction(_queryOnStartAct);

    _autoUpdateAct = new QAction(_queryMenu);
    _autoUpdateAct->setCheckable(true);
    _autoUpdateAct->setVisible(false);
    _queryMenu->addAction(_autoUpdateAct);

    // Determine whether to show toolbar or buttons
    _toolBar->setVisible(useToolbar);

    _close->setHidden(useToolbar);
    _query->setHidden(useToolbar);

    if (useToolbar)
    {
      _parent->layout()->setContentsMargins(0,0,0,0);
      _parent->layout()->setSpacing(0);
    }
  }

  void print(ParameterList, bool);

  QString reportName;
  QString metasqlName;
  QString metasqlGroup;

  bool _useAltId;
  bool _queryOnStartEnabled;
  bool _autoUpdateEnabled;

  QAction* _newAct;
  QAction* _closeAct;
  QAction* _sep1;
  QAction* _filterLitAct;
  QAction* _filterAct;
  QAction* _moreAct;
  QAction* _sep2;
  QAction* _printAct;
  QAction* _previewAct;
  QAction* _sep3;
  QAction* _searchAct;
  QAction* _queryAct;
  QAction* _queryOnStartAct;
  QAction* _autoUpdateAct;

  QMenu* _queryMenu;

  QToolButton * _newBtn;
  QToolButton * _closeBtn;
  QToolButton * _moreBtn;
  QToolButton * _queryBtn;
  QToolButton * _previewBtn;
  QToolButton * _printBtn;

private:
  ::display * _parent;
};

void displayPrivate::print(ParameterList pParams, bool showPreview)
{
  int numCopies = 1;
  ParameterList params = pParams;
  if (!params.count())
  {
    if(!_parent->setParams(params))
      return;
  }

  XSqlQuery report;
  report.prepare("SELECT report_grade, report_source "
                 "  FROM report "
                 " WHERE (report_name=:report_name)"
                 " ORDER BY report_grade DESC LIMIT 1");
  report.bindValue(":report_name", reportName);
  report.exec();
  QDomDocument _doc;
  if (report.first())
  {
    QString errorMessage;
    int     errorLine;

    if (!_doc.setContent(report.value("report_source").toString(), &errorMessage, &errorLine))
    {
      QMessageBox::critical(_parent, ::display::tr("Error Parsing Report"),
        ::display::tr("There was an error Parsing the report definition. %1 %2").arg(errorMessage).arg(errorLine));
      return;
    }
  }
  else
  {
    QMessageBox::critical(_parent, ::display::tr("Report Not Found"),
      ::display::tr("The report %1 does not exist.").arg(reportName));
    return;
  }

  ORPreRender pre;
  pre.setDom(_doc);
  pre.setParamList(params);
  ORODocument * doc = pre.generate();

  if(doc)
  {
    QPrinter printer(QPrinter::HighResolution);
    printer.setNumCopies( numCopies );

    ORPrintRender render;
    render.setupPrinter(doc, &printer);

    if(showPreview)
    {
      QPrinter * tPrinter = &printer;
      QPrinter pdfPrinter(QPrinter::HighResolution);
      if(!printer.isValid())
      {
        render.setupPrinter(doc, &pdfPrinter);
        pdfPrinter.setOutputFormat(QPrinter::PdfFormat);
        tPrinter = &pdfPrinter;
      }
      PreviewDialog preview (doc, tPrinter, _parent);
      if (preview.exec() == QDialog::Rejected)
        return;
    }

    QPrintDialog pd(&printer);
    pd.setMinMax(1, doc->pages());
    if(pd.exec() == QDialog::Accepted)
    {
      render.setPrinter(&printer);
      render.render(doc);
    }
    delete doc;
  }
}

display::display(QWidget* parent, const char* name, Qt::WindowFlags flags)
    : XWidget(parent, name, flags)
{
  _data = new displayPrivate(this);

  QPushButton* filterButton = findChild<QPushButton*>("_filterButton");

  // Set text
  _data->_newBtn->setText(tr("New"));
  _data->_closeBtn->setText(tr("Close"));
  _data->_moreBtn->setText(tr("More"));
  _data->_printBtn->setText(tr("Print"));
  _data->_previewBtn->setText(tr("Preview"));
  _data->_queryBtn->setText(tr("Query"));
  _data->_queryOnStartAct->setText(tr("Query on start"));
  _data->_autoUpdateAct->setText(tr("Automatically Update"));

  // Set shortcuts
  _data->_newAct->setShortcut(QKeySequence::New);
  _data->_closeAct->setShortcut(QKeySequence::Close);
  _data->_queryAct->setShortcut(QKeySequence::Refresh);
  _data->_printAct->setShortcut(QKeySequence::Print);

  if (_metrics->boolean("DisplaysUseToolbar"))
  {
    _data->_search->setNullStr(tr("search"));
    _data->_searchAct->setShortcut(QKeySequence::InsertParagraphSeparator);
    _data->_searchAct->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  }
  else
    connect(_data->_search, SIGNAL(editingFinished()), this, SLOT(sFillList()));

  // Set tooltips
  _data->_newBtn->setToolTip(_data->_newBtn->text() + " " + _data->_newAct->shortcut().toString(QKeySequence::NativeText));
  _data->_closeBtn->setToolTip(_data->_closeBtn->text() + " " + _data->_closeAct->shortcut().toString(QKeySequence::NativeText));
  _data->_queryBtn->setToolTip(_data->_queryBtn->text() + " " + _data->_queryAct->shortcut().toString(QKeySequence::NativeText));
  _data->_printBtn->setToolTip(_data->_printBtn->text() + " " + _data->_printAct->shortcut().toString(QKeySequence::NativeText));

  connect(_data->_newBtn, SIGNAL(clicked()), _data->_newAct, SLOT(trigger()));
  connect(_data->_closeBtn, SIGNAL(clicked()), _data->_closeAct, SLOT(trigger()));
  connect(_data->_moreBtn, SIGNAL(clicked(bool)), _data->_parameterWidget, SLOT(setVisible(bool)));
  connect(_data->_moreBtn, SIGNAL(clicked(bool)), filterButton, SLOT(setChecked(bool)));
  connect(_data->_printBtn, SIGNAL(clicked()), _data->_printAct, SLOT(trigger()));
  connect(_data->_previewBtn, SIGNAL(clicked()), _data->_previewAct, SLOT(trigger()));
  connect(_data->_queryBtn, SIGNAL(clicked()), _data->_queryAct, SLOT(trigger()));
  // Connect these two simply so checkbox takes care of pref. memory.  Could separate out later.
  connect(_data->_autoupdate, SIGNAL(toggled(bool)), _data->_autoUpdateAct, SLOT(setChecked(bool)));
  connect(_data->_autoUpdateAct, SIGNAL(triggered(bool)), _data->_autoupdate, SLOT(setChecked(bool)));
  connect(_data->_queryonstart, SIGNAL(toggled(bool)), _data->_queryOnStartAct, SLOT(setChecked(bool)));
  connect(_data->_queryOnStartAct, SIGNAL(triggered(bool)), _data->_queryonstart, SLOT(setChecked(bool)));

  // Connect Actions
  connect(_data->_newAct, SIGNAL(triggered()), this, SLOT(sNew()));
  connect(_data->_closeAct, SIGNAL(triggered()), this, SLOT(close()));
  connect(_data->_queryAct, SIGNAL(triggered()), this, SLOT(sFillList()));
  connect(_data->_printAct, SIGNAL(triggered()), this, SLOT(sPrint()));
  connect(_data->_previewAct, SIGNAL(triggered()), this, SLOT(sPreview()));
  connect(_data->_searchAct, SIGNAL(triggered()), this, SLOT(sFillList()));
  connect(_data->_list, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*,int)));
  connect(_data->_autoupdate, SIGNAL(toggled(bool)), this, SLOT(sAutoUpdateToggled()));
  connect(filterButton, SIGNAL(toggled(bool)), _data->_moreBtn, SLOT(setChecked(bool)));

  // Connect push buttons
  connect(_data->_new, SIGNAL(clicked()), _data->_newAct, SLOT(trigger()));
  connect(_data->_close, SIGNAL(clicked()), _data->_closeAct, SLOT(trigger()));
  connect(_data->_print, SIGNAL(clicked()), _data->_printAct, SLOT(trigger()));
  connect(_data->_preview, SIGNAL(clicked()), _data->_previewAct, SLOT(trigger()));
  connect(_data->_query, SIGNAL(clicked()), _data->_queryAct, SLOT(trigger()));

}

display::~display()
{
  delete _data;
  _data = 0;
}

void display::languageChange()
{
  _data->retranslateUi(this);
}

void display::showEvent(QShowEvent * e)
{
  XWidget::showEvent(e);

  if (_data->_queryOnStartEnabled &&
      _data->_queryonstart->isChecked())
    sFillList();
}

QWidget * display::optionsWidget()
{
  return _data->_optionsFrame;
}

XTreeWidget * display::list()
{
  return _data->_list;
}

ParameterWidget * display::parameterWidget()
{
  return _data->_parameterWidget;
}

QToolBar * display::toolBar()
{
  return _data->_toolBar;
}

QAction * display::newAction()
{
  return _data->_newAct;
}

QAction * display::closeAction()
{
  return _data->_closeAct;
}

QAction * display::filterSeparator()
{
  return _data->_sep1;
}

QAction * display::printSeparator()
{
  return _data->_sep2;
}

QAction * display::printAction()
{
  return _data->_printAct;
}

QAction * display::previewAction()
{
  return _data->_previewAct;
}

QAction * display::querySeparator()
{
  return _data->_sep3;
}

QAction * display::searchAction()
{
  return _data->_searchAct;
}

QAction * display::queryAction()
{
  return _data->_queryAct;
}

QString display::searchText()
{
  if (!_data->_search->isNull())
    return _data->_search->text().trimmed();
  return QString("");
}

bool display::setParams(ParameterList & params)
{
  parameterWidget()->appendValue(params);
  if (!_data->_search->isNull())
    params.append("search_pattern", _data->_search->text());

  return true;
}

void display::setReportName(const QString & reportName)
{
  _data->reportName = reportName;
  if (_metrics->boolean("DisplaysUseToolbar"))
  {
    _data->_printAct->setVisible(!reportName.isEmpty());
    _data->_previewAct->setVisible(!reportName.isEmpty());
    _data->_sep3->setVisible(!reportName.isEmpty());
  }
  else
  {
    _data->_print->setVisible(!reportName.isEmpty());
    _data->_preview->setVisible(!reportName.isEmpty());
  }
}

QString display::reportName() const
{
  return _data->reportName;
}

void display::setMetaSQLOptions(const QString & group, const QString & name)
{
  _data->metasqlName = name;
  _data->metasqlGroup = group;
}

void display::setListLabel(const QString & pText)
{
  _data->_listLabelFrame->setHidden(pText.isEmpty());
  _data->_listLabel->setText(pText);
}

void display::setUseAltId(bool on)
{
  _data->_useAltId = on;
}

bool display::useAltId() const
{
  return _data->_useAltId;
}

void display::setNewVisible(bool show)
{
  if (_metrics->boolean("DisplaysUseToolbar"))
  {
    _data->_newAct->setVisible(show);
    _data->_sep1->setVisible(show || _data->_closeAct->isVisible());
  }
  else
    _data->_new->setVisible(show);
}

bool display::newVisible() const
{
  return _data->_newAct->isVisible();
}

void display::setCloseVisible(bool show)
{
  if (_metrics->boolean("DisplaysUseToolbar"))
  {
    _data->_closeAct->setVisible(show);
    _data->_sep1->setVisible(show || _data->_newAct->isVisible());
  }
  else
    _data->_close->setVisible(show);
}

bool display::closeVisible() const
{
  return _data->_closeAct->isVisible();
}

void display::setParameterWidgetVisible(bool show)
{
  if (_metrics->boolean("DisplaysUseToolbar"))
  {
    _data->_parameterWidget->_filterButton->hide(); // _moreBtn is what you see here
    _data->_moreAct->setVisible(show);
    _data->_filterLitAct->setVisible(show);
    _data->_filterAct->setVisible(show);
    _data->_sep2->setVisible(show);
  }
  else
    _data->_parameterWidget->setVisible(show);
}

bool display::parameterWidgetVisible() const
{
  return _data->_parameterWidget->isVisible();
}

bool display::searchVisible() const
{
  return _data->_search->isVisible();
}

void display::setSearchVisible(bool show)
{
  _data->_search->setVisible(show);
  _data->_searchAct->setVisible(show && _metrics->boolean("DisplaysUseToolbar"));
  _data->_searchLit->setVisible(show && !_metrics->boolean("DisplaysUseToolbar"));
}

void display::setQueryOnStartEnabled(bool on)
{
  _data->_queryOnStartEnabled = on;
  if (_metrics->boolean("DisplaysUseToolbar"))
  {
    _data->_queryOnStartAct->setVisible(on);

    if (_data->_queryOnStartEnabled ||
        _data->_autoUpdateEnabled)
    {
      _data->_queryBtn->setPopupMode(QToolButton::MenuButtonPopup);
      _data->_queryBtn->setMenu(_data->_queryMenu);
    }
    else
    {
      _data->_queryBtn->setPopupMode(QToolButton::DelayedPopup);
      _data->_queryBtn->setMenu(0);
    }
  }
  else
    _data->_queryonstart->setVisible(on);

  _data->_queryonstart->setForgetful(!on);

  // Ensure query on start is checked by default
  if (on)
  {
    QString prefname = window()->objectName() + "/" +
                       _data->_queryonstart->objectName() + "/checked";
    XSqlQuery qry;
    qry.prepare("SELECT usrpref_id "
                "FROM usrpref "
                "WHERE ((usrpref_username=current_user) "
                " AND (usrpref_name=:prefname));");
    qry.bindValue(":prefname", prefname);
    qry.exec();
    if (!qry.first())
      _preferences->set(prefname, 2);
  }
}

bool display::queryOnStartEnabled() const
{
  return _data->_queryonstart->isVisible();
}

void display::setAutoUpdateEnabled(bool on)
{
  _data->_autoUpdateEnabled = on;

  if (_metrics->boolean("DisplaysUseToolbar"))
  {
    _data->_autoUpdateAct->setVisible(on);

    if (_data->_queryOnStartEnabled ||
        _data->_autoUpdateEnabled)
    {
      _data->_queryBtn->setPopupMode(QToolButton::MenuButtonPopup);
      _data->_queryBtn->setMenu(_data->_queryMenu);
    }
    else
    {
      _data->_queryBtn->setPopupMode(QToolButton::DelayedPopup);
      _data->_queryBtn->setMenu(0);
    }
  }
  else
    _data->_autoupdate->setVisible(on);

  sAutoUpdateToggled(); 
}

bool display::autoUpdateEnabled() const
{
  return _data->_autoUpdateEnabled;
}

void display::sNew()
{
}

void display::sPrint()
{
  sPrint(ParameterList());
}

void display::sPrint(ParameterList pParams)
{
  _data->print(pParams, false);
}

void display::sPreview()
{
  sPreview(ParameterList());
}

void display::sPreview(ParameterList pParams)
{
  _data->print(pParams, true);
}

void display::sFillList()
{
  ParameterList params;
  if (!setParams(params))
    return;
  int itemid = _data->_list->id();
  bool ok = true;
  QString errorString;
  MetaSQLQuery mql = MQLUtil::mqlLoad(_data->metasqlGroup, _data->metasqlName, errorString, &ok);
  if(!ok)
  {
    systemError(this, errorString, __FILE__, __LINE__);
    return;
  }
  XSqlQuery xq = mql.toQuery(params);
  _data->_list->populate(xq, itemid, _data->_useAltId);
  if (xq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, xq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void display::sPopulateMenu(QMenu *, QTreeWidgetItem *, int)
{
}

void display::sAutoUpdateToggled()
{
  bool update = _data->_autoUpdateEnabled && _data->_autoupdate->isChecked();
  if (update)
    connect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
  else
    disconnect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
}

ParameterList display::getParams()
{
  ParameterList params;
  bool ret = setParams(params);
  params.append("checkParamsReturn", ret);
    
  return params;
}
