/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "scripttoolbox.h"

#include <QBoxLayout>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QGridLayout>
#include <QIODevice>
#include <QLayout>
#include <QList>
#include <QMenu>
#include <QMessageBox>
#include <QScriptEngine>
#include <QScriptValueIterator>
#include <QSqlError>
#include <QStackedLayout>
#include <QTabWidget>
#include <QTextStream>
#include <QUrl>
#include <QWidget>
#include <QWebView>

#include <parameter.h>
#include <metasql.h>
#include <openreports.h>

#include "creditCard.h"
#include "creditcardprocessor.h"
#include "mqlutil.h"
#include "storedProcErrorLookup.h"
#include "xdialog.h"
#include "xmainwindow.h"
#include "xtreewidget.h"
#include "xuiloader.h"
#include "getscreen.h"

/** \ingroup scriptapi
  
    \class ScriptToolbox

    \brief The ScriptToolbox provides a collection of miscellaneous utility
           methods used when writing xTuple ERP extension %scripts.

    The ScriptToolbox class provides the C++ implementation of the global
    toolbox object. A toolbox object is created whenever xTuple ERP
    creates a QScriptEngine, such as starting the application and
    opening a new window.

    Scripts should use these methods by using the toolbox object
    in the %script's global namespace:
    \code
      var qry = toolbox.executeQuery("SELECT CURRENT_DATE AS result;");
    \endcode

    The methods in the class are intended to be used from %scripts only.
    Use the appropriate classes and methods directly if you are writing in C++.
    The descriptions of individual methods below should point you to
    the proper C++ classes to use.

    Historical note:
    Many methods in the ScriptToolbox are deprecated. They were created
    while we were learning how best to integrate QtScript with xTuple ERP.
    Please do not use these, as they will eventually be removed from the
    toolbox. The documentation for these deprecated methods refers to
    better ways to handle relevant situations.

    \see initMenu
    \see toolbox
    \see XDialog
    \see XMainWindow
    \see XWidget
 */

QWidget *ScriptToolbox::_lastWindow = 0;

/** \brief Create a new ScriptToolbox.

  This constructor should not be called directly except in a small number
  of places in the xTuple ERP core. It does not install the toolbox
  object in the engine's global namespace.

  \param engine The script engine in which to create the toolbox object
*/

ScriptToolbox::ScriptToolbox(QScriptEngine * engine)
  : QObject(engine)
{
  _engine = engine;
}

ScriptToolbox::~ScriptToolbox()
{
}

/** \brief Execute a simple database query.

    The passed-in string is sent to the database engine for execution.
    The resulting XSqlQuery object is returned.
    The calling script is responsible for navigating the results, including
    reading the query results and checking for errors.

    For example, let's say you want a warning every day when you log in
    about %currencies with missing exchange rates.
    The following lines %check the database for
    %currencies that do not have exchange rates defined for today:
    \dontinclude initMenu_executeQueryExample.js
    \skip var
    \until IS NULL

    Now the script must loop through the results to inform you of
    each row found:
    \skip next()
    \until curr_abbr

    Without this loop, the script would have asked the database for
    the missing exchange rates but nothing would have been done with the
    results.

    \param query The database query to execute

    \return The XSqlQuery object after calling XSqlQuery::exec()

    \see QSqlQuery
    \see XSqlQuery
*/
XSqlQuery ScriptToolbox::executeQuery(const QString & query)
{
  ParameterList params;
  MetaSQLQuery mql(query);
  return mql.toQuery(params);
}
/** \example initMenu_executeQueryExample.js */

/** \brief Execute a MetaSQL query.

  The passed-in string is processed as a MetaSQL query and then sent to
  the database engine for execution. The given ParameterList controls
  parsing of the string to generate the actual query to execute.

  This method executes
  the query but the caller is responsible for navigating the results
  and checking for errors.

  The following scripted example creates a ParameterList \c params,
  sets the \c site parameter to the internal id of the currently selected
  \c _site widget, then queries the database for a list of %items
  associated with that itemsite. The results are then used to populate
  an XTreeWidget.

  \dontinclude itemSiteViewItem.js
  \skip sFillList
  \until }

  \param query  The MetaSQL string from which to build the query
  \param params The ParameterList used by MetaSQL to formulate the final query

  \return The XSqlQuery object after calling XSqlQuery::exec()

  \see MetaSQLQuery
  \see XSqlQuery
  \see QSqlQuery

 */
XSqlQuery ScriptToolbox::executeQuery(const QString & query, const ParameterList & params)
{
  MetaSQLQuery mql(query);
  return mql.toQuery(params);
}
/** \example itemSiteViewItem.js */

/** \brief Execute a simple query loaded from the \c metasql table.

  This loads a query string from the \c metasql table in the xTuple ERP
  database.
  This string is then executed as a database query.
  If the string has MetaSQL tags in it, they are all treated as undefined
  (the ParameterList is empty).

  It is the caller's responsibility to ensure that there is a record
  in the \c metasql table that matches the given %group and name, and that
  processing the query string with an empty parameter list will result in
  valid SQL.

  \param %group The metasql_group value to use when searching for this query
  \param name  The metasql_name value to use when searching for this query

  \return The XSqlQuery object after calling XSqlQuery::exec()

  \see mqlLoad
  \see MetaSQLQuery
  \see XSqlQuery
  \see QSqlQuery
 */
XSqlQuery ScriptToolbox::executeDbQuery(const QString & group, const QString & name)
{
  ParameterList params;
  MetaSQLQuery mql = mqlLoad(group, name);
  return mql.toQuery(params);
}

/** \brief Execute a MetaSQL query loaded from the \c metasql table.

  This loads a query string from the \c metasql table in the xTuple ERP
  database.
  The given ParameterList controls
  parsing of the string to generate the actual query to execute.

  This method executes
  the query but the caller is responsible for navigating the results
  and checking for errors.

  The following scripted example creates a ParameterList \c params,
  populates it with the number of days preauthorized credit card charges
  are configured to be valid (defaults to 7) and defines
  some standard text values to display instead of internal codes
  (e.g. \c Preauthorization), then queries the database for a list of
  credit card transactions. The query itself it taken from the \c metasql
  table where the \c metasql_group is \c ccpayments and the
  \c metasql_name is \c list.
  The results are then used to populate
  an XTreeWidget.

  \dontinclude ccvoid.js
  \skip sPopulateTransactions
  \until }

  \param %group The metasql_group value to use when searching for this query
  \param name  The metasql_name value to use when searching for this query
  \param params The ParameterList used by MetaSQL to formulate the final query

  \return The XSqlQuery object after calling XSqlQuery::exec()

  \see MetaSQLQuery
  \see XSqlQuery
  \see QSqlQuery
 */
XSqlQuery ScriptToolbox::executeDbQuery(const QString & group, const QString & name, const ParameterList & params)
{
  MetaSQLQuery mql = mqlLoad(group, name);
  return mql.toQuery(params);
}
/** \example ccvoid.js */

/** \brief This is a convenience function that simply begins a database transaction.
 */
XSqlQuery ScriptToolbox::executeBegin()
{
  ParameterList params;
  MetaSQLQuery mql("BEGIN;");
  return mql.toQuery(params);
}

/** \brief This is a convenience function that simply commits the currently open
           database transaction.
 */
XSqlQuery ScriptToolbox::executeCommit()
{
  ParameterList params;
  MetaSQLQuery mql("COMMIT;");
  return mql.toQuery(params);
}

/** \brief This is a convenience function that simply rolls back the currently
           open database transaction.
 */
XSqlQuery ScriptToolbox::executeRollback()
{
  ParameterList params;
  MetaSQLQuery mql("ROLLBACK;");
  return mql.toQuery(params);
}

/** \brief Get a standard Quantity QValidator.
  \deprecated Use mainwindow.qtyVal() instead.
 */
QObject * ScriptToolbox::qtyVal()
{
  QValidator * val = omfgThis->qtyVal();
  return val;
}

/** \brief Get a standard Transfer Quantity QValidator.
  \deprecated Use mainwindow.transQtyVal() instead.
 */
QObject * ScriptToolbox::TransQtyVal()
{
  QValidator * val = omfgThis->transQtyVal();
  return val;
}

/** \brief Get a standard Quantity Per QValidator.
  \deprecated Use mainwindow.qtyPerVal() instead.
 */
QObject * ScriptToolbox::qtyPerVal()
{
  QValidator * val = omfgThis->qtyPerVal();
  return val;
}

/** \brief Get a standard Percent QValidator.
  \deprecated Use mainwindow.percentVal() instead.
 */
QObject * ScriptToolbox::percentVal()
{
  QValidator * val = omfgThis->percentVal();
  return val;
}

/** \brief Get a standard Money QValidator.
  \deprecated Use mainwindow.moneyVal() instead.
 */
QObject * ScriptToolbox::moneyVal()
{
  QValidator * val = omfgThis->moneyVal();
  return val;
}

/** \brief Get a standard Negative Money QValidator.
  \deprecated Use mainwindow.negMoneyVal() instead.
 */
QObject * ScriptToolbox::negMoneyVal()
{
  QValidator * val = omfgThis->negMoneyVal();
  return val;
}

/** \brief Get a standard Price QValidator.
  \deprecated Use mainwindow.priceVal() instead.
 */
QObject * ScriptToolbox::priceVal()
{
  QValidator * val = omfgThis->priceVal();
  return val;
}

/** \brief Get a standard Cost QValidator.
  \deprecated Use mainwindow.costVal() instead.
 */
QObject * ScriptToolbox::costVal()
{
  QValidator * val = omfgThis->costVal();
  return val;
}

/** \brief Get a standard Ratio QValidator.
  \deprecated Use mainwindow.ratioVal() instead.
 */
QObject * ScriptToolbox::ratioVal()
{
  QValidator * val = omfgThis->ratioVal();
  return val;
}

/** \brief Get a standard Weight QValidator.
  \deprecated Use mainwindow.weightVal() instead.
 */
QObject * ScriptToolbox::weightVal()
{
  QValidator * val = omfgThis->weightVal();
  return val;
}

/** \brief Get a standard Run Time QValidator.
  \deprecated Use mainwindow.runTimeVal() instead.
 */
QObject * ScriptToolbox::runTimeVal()
{
  QValidator * val = omfgThis->runTimeVal();
  return val;
}

/** \brief Get a standard Order QValidator.
  \deprecated Use mainwindow.orderVal() instead.
 */
QObject * ScriptToolbox::orderVal()
{
  QValidator * val = omfgThis->orderVal();
  return val;
}

/** \brief Get a standard Day QValidator.
  \deprecated Use mainwindow.dayVal() instead.
 */
QObject * ScriptToolbox::dayVal()
{
  QValidator * val = omfgThis->dayVal();
  return val;
}

/** \brief Create a new QGridLayout.
  \deprecated Use QGridLayout instead.
 */
QObject * ScriptToolbox::createGridLayout()
{
  QGridLayout * layout = new QGridLayout;
  return layout;
}

/** \brief Get the layout containing the given widget.

  Search the object hierarchy and find the smallest QLayout 
  that contains the given widget. This lets a %script create a new
  QWidget and insert it into the %user interface near another widget.

  \dontinclude addButton.js
  \skip var
  \until widgetGetLayout

  \param w The widget whose layout you want to find

  \return A pointer to the layout or 0 if no such layout could be found

*/
QObject * ScriptToolbox::widgetGetLayout(QWidget * w)
{
  QObject * p = w->parentWidget();
  while(p)
  {
    QList<QLayout*> list = p->findChildren<QLayout*>();
    for (int i = 0; i < list.size(); ++i)
    {
      if (list.at(i)->indexOf(w) != -1)
        return list.at(i);
    }
  }
  return NULL;
}
/** \example addButton.js */

/** \brief Insert a QGridLayout into an existing QGridLayout.
    \deprecated Use QGridLayout::addLayout instead
  */
void ScriptToolbox::layoutGridAddLayout(QObject * parent, QObject * child, int row, int column, int alignment)
{
  QGridLayout * parentLayout = qobject_cast<QGridLayout*>(parent);
  QGridLayout * childLayout = qobject_cast<QGridLayout*>(child);

  if(parentLayout)
    parentLayout->addLayout(childLayout, row, column, (Qt::Alignment)alignment);
}

/** \brief Insert a QWidget into an existing QBoxLayout.
    \deprecated Use QBoxLayout::insertWidget directly
  */
void ScriptToolbox::layoutBoxInsertWidget(QObject * obj, int index, QWidget * widget, int stretch, int alignment)
{
  QBoxLayout * layout = qobject_cast<QBoxLayout*>(obj);
  if(layout && widget)
    layout->insertWidget(index, widget, stretch, (Qt::Alignment)alignment);
}

/** \brief Insert a QWidget into an existing QGridLayout.
    \deprecated Use QGridLayout::addWidget instead.
  */
void ScriptToolbox::layoutGridAddWidget(QObject * obj, QWidget * widget, int row, int column, int alignment)
{
  QGridLayout * layout = qobject_cast<QGridLayout*>(obj);
  if(layout && widget)
    layout->addWidget(widget, row, column, (Qt::Alignment)alignment);
}

/** \brief Insert a QWidget into an existing QGridLayout.
    \deprecated Use QGridLayout::addWidget instead.
  */
void ScriptToolbox::layoutGridAddWidget(QObject * obj, QWidget * widget, int fromRow, int fromColumn, int rowSpan, int columnSpan, int alignment)
{
  QGridLayout * layout = qobject_cast<QGridLayout*>(obj);
  if(layout && widget)
    layout->addWidget(widget, fromRow, fromColumn, rowSpan, columnSpan, (Qt::Alignment)alignment);
}

/** \brief Insert a QWidget into an existing QStackedLayout.
    \deprecated Use QStackedWidget::insertWidget instead.
  */
void ScriptToolbox::layoutStackedInsertWidget(QObject * obj, int index, QWidget * widget)
{
  QStackedLayout * layout = qobject_cast<QStackedLayout*>(obj);
  if(layout && widget)
    layout->insertWidget(index, widget);
}

/** \brief Add an action to an existing QMenu.
    \deprecated Use QMenu::addAction instead.
  */
QObject * ScriptToolbox::menuAddAction(QObject * menu, const QString & text, const bool enabled)
{
  QMenu * m = qobject_cast<QMenu*>(menu);
  QAction * act = 0;
  if(m)
  {
    act = m->addAction(text);
    act->setEnabled(enabled);
  }
  return act;
}

/** \brief Add an action to an existing QMenu.
    \deprecated Use QMenu::addAction instead.
  */
QObject * ScriptToolbox::menuAddMenu(QObject * menu, const QString & text, const QString & name)
{
  QMenu * m = qobject_cast<QMenu*>(menu);
  QMenu * nm = 0;
  if(m)
  {
    nm = m->addMenu(text);
    if(!name.isEmpty())
      nm->setObjectName(name);
  }
  return nm;
}

/** \brief Add a separator to an existing QMenu.
    \deprecated Use QMenu::addSeparator instead.
  */
QObject * ScriptToolbox::menuAddSeparator(QObject * menu)
{
  QMenu * m = qobject_cast<QMenu*>(menu);
  QAction * na = 0;
  if(m)
    na = m->addSeparator();
  return na;
}

/** \brief Insert an action into an existing QMenu.
    \deprecated Use QMenu::insertAction instead.
  */
QObject * ScriptToolbox::menuInsertAction(QObject * menu, QObject * before, const QString & name, const bool enabled)
{
  QMenu * m = qobject_cast<QMenu*>(menu);
  QAction * ba = qobject_cast<QAction*>(before);
  QAction * na = new QAction(name,m);
  na->setEnabled(enabled);
  if(m && ba && na)
  {
    m->insertAction(ba, na);
    return na;
  }
  return 0;
}

/** \brief Insert a QMenu into an existing QMenu.
    \deprecated Use QMenu::insertMenu instead.
  */
QObject * ScriptToolbox::menuInsertMenu(QObject * menu, QObject * before, const QString & name)
{
  QMenu * m = qobject_cast<QMenu*>(menu);
  QAction * ba = qobject_cast<QAction*>(before);
  QMenu * nm = new QMenu(name);

  if (!ba && nm)
  {
    QMenu * bm =  qobject_cast<QMenu*>(before);
    if (bm)
      ba = bm->menuAction();
  }
  if (ba && nm)
    m->insertMenu(ba, nm);
  return nm;
}


/** \brief Insert a separator into an existing QMenu.
    \deprecated Use QMenu::insertSeparator instead.
  */
QObject * ScriptToolbox::menuInsertSeparator(QObject * menu, QObject * before)
{
  QMenu * m = qobject_cast<QMenu*>(menu);
  QAction * ba = qobject_cast<QAction*>(before);
  if (!ba)
  {
    QMenu * bm = qobject_cast<QMenu*>(before);
    ba = bm->menuAction();
  }
  QAction * na = new QAction(m);
  if(m && ba && na)
  {
    na->setSeparator(true);
    m->insertAction(ba, na);
    return na;
  }
  return 0;
}

/** \brief Remove a QAction from a QMenu.
    \deprecated Use QMenu::removeAction instead.
  */
void ScriptToolbox::menuRemove(QObject * menu, QObject * action)
{
  QMenu * m = qobject_cast<QMenu*>(menu);
  QAction * act = qobject_cast<QAction*>(action);
  if (!act)
  {
    QMenu * ma = qobject_cast<QMenu*>(action);
    act = ma->menuAction();
  }
  if(m && act)
    m->removeAction(act);
}

/** \brief Find the number of QActions in a QMenu.
  \param menu The QMenu whose QAction children you want to count
  \return The number of QAction objects in the menu or 0 if \c menu isn't really a QMenu
 */
int ScriptToolbox::menuActionCount(QObject * menu)
{
  QMenu * m = qobject_cast<QMenu*>(menu);
  if (m)
    return m->actions().count();
  return 0;
}

/** \brief Find the number of tabs in the QTabWidget
    \deprecated Use QTabWidget::count directly.
 */
int ScriptToolbox::tabCount(QWidget * tab)
{
  QTabWidget *tw = qobject_cast<QTabWidget*>(tab);
  if(tw)
    return tw->count();
  return 0;
}

/** \brief Return the QWidget for a particular tab in a QTabWidget.
    \deprecated Use QTabWidget::widget directly.
  */
QWidget * ScriptToolbox::tabWidget(QWidget * tab, int idx)
{
  QTabWidget *tw = qobject_cast<QTabWidget*>(tab);
  QWidget * w = 0;
  if(tw)
    w = tw->widget(idx);
  return w;
}

/** \brief Insert a new tab into a QTabWidget.
    \deprecated Use QTabWidget::insertTab directly.
  */
int ScriptToolbox::tabInsertTab(QWidget * tab, int idx, QWidget * page, const QString & text)
{
  QTabWidget *tw = qobject_cast<QTabWidget*>(tab);
  int i = -1;
  if(tw)
    i = tw->insertTab(idx, page, text);
  return i;
}

/** \brief Get the index of a particular tab in a QTabWidget.
    \deprecated Use QTabWidget::indexOf directly.
  */
int ScriptToolbox::tabTabIndex(QWidget * tab, QWidget * page)
{
  QTabWidget *tw = qobject_cast<QTabWidget*>(tab);
  if(tw)
    return tw->indexOf(page);
  return 0;
}

/** \brief Remove a particular tab from a QTabWidget.
    \deprecated Use QTabWidget::removeTab directly.
  */
void ScriptToolbox::tabRemoveTab(QWidget * tab, int idx)
{
  QTabWidget *tw = qobject_cast<QTabWidget*>(tab);
  if(tw)
    tw->removeTab(idx);
}

/** \brief Enable or disable a particular tab on a QTabWidget.
    \deprecated Use QTabWidget::setTabEnabled directly.
  */
void ScriptToolbox::tabSetTabEnabled(QWidget * tab, int idx, bool enable)
{
  QTabWidget *tw = qobject_cast<QTabWidget*>(tab);
  if(tw)
    tw->setTabEnabled(idx, enable);
}

/** \brief Set the text for a particular tab on a QTabWidget.
    \deprecated Use QTabWidget::setTabText directly.
  */
void ScriptToolbox::tabSetTabText(QWidget * tab, int idx, const QString & text)
{
  QTabWidget *tw = qobject_cast<QTabWidget*>(tab);
  if(tw)
    tw->setTabText(idx, text);
}

/** \brief Get the text for a particular tab on a QTabWidget.
    \deprecated Use QTabWidget::tabText directly.
  */
QString ScriptToolbox::tabtabText(QWidget * tab, int idx)
{
  QTabWidget *tw = qobject_cast<QTabWidget*>(tab);
  QString str;
  if(tw)
    str = tw->tabText(idx);
  return str;
}

/** \brief Create a widget based on the name of the desired class.

  This is a wrapper around XUiLoader::createWidget.
  
  The following example creates a new QPushButton:
  \dontinclude addButton.js
  \skipline newbutton

  \param className The name of the class of the desired widget
  \param parent    The widget that should be set as the parent of the new widget
  \param name      The name to give the newly-created widget

  \return The newly-created widget
 */
QWidget * ScriptToolbox::createWidget(const QString & className, QWidget * parent, const QString & name)
{
  XUiLoader ui;
  return ui.createWidget(className, parent, name);
}

/** \brief Create a new layout based on the name of the desired class.

  This is a wrapper around XUiLoader::createLayout.

  \param className The name of the class of the desired layout
  \param parent    The widget that should be set as the parent of the new layout
  \param name      The name to give the newly-created layout

  \return The newly-created layout

  \see qboxlayoutproto.cpp
  \see qgridlayoutproto.cpp
  \see QBoxLayout
  \see QGridLayout
*/
QObject * ScriptToolbox::createLayout(const QString & className, QWidget * parent, const QString & name)
{
  XUiLoader ui;
  return ui.createLayout(className, parent, name);
}

/** \brief Load a .ui definition from the %uiform table.

  Instantiate a new %user interface using a .ui definition that has been stored
  in the %uiform table. If multiple rows in the %uiform match
  the given name, the enabled row with the highest \c uiform_order is used.

  \param screenName The name of the .ui to load (\c uiform_name)
  \param parent     The widget to set as the parent of the loaded .ui

  \return A widget representing the %user interface; 0 if no record matches or
          if there was an error
 */
QWidget * ScriptToolbox::loadUi(const QString & screenName, QWidget * parent)
{
  if(screenName.isEmpty())
    return 0;

  XSqlQuery qui;
  qui.prepare("SELECT *"
              "  FROM uiform"
              " WHERE((uiform_name=:uiform_name)"
              "   AND (uiform_enabled))"
              " ORDER BY uiform_order DESC"
              " LIMIT 1;");
  qui.bindValue(":uiform_name", screenName);
  qui.exec();
  if(!qui.first())
  {
    QMessageBox::critical(0, tr("Could Not Create Form"),
                              tr("<p>Could not create the '%1' form. Either an "
                                 "error occurred or the specified form does "
                                 "not exist.").arg(screenName) );
    return 0;
  }

  XUiLoader loader;
  QByteArray ba = qui.value("uiform_source").toByteArray();
  QBuffer uiFile(&ba);
  if(!uiFile.open(QIODevice::ReadOnly))
  {
    QMessageBox::critical(0, tr("Could not load file"),
        tr("There was an error loading the UI Form from the database."));
    return 0;
  }
  QWidget *ui = loader.load(&uiFile, parent);
  uiFile.close();

  return ui;
}

/** \brief Return the last window opened by this instance of the ScriptToolbox.
  */
QWidget * ScriptToolbox::lastWindow() const
{
  return _lastWindow;
}

/** \brief Use the embedded OpenRPT to print a named report.

    Print the report using the given parameters, optionally saving the
    output to a PDF %file.

    \param name        The name of the report definition to use
    \param params      The MetaSQL parameters used to control the report output
    \param pdfFilename The name of the PDF %file to create.
                       If empty the function uses the standard print methodology

    \return True if the report printed successfully; false if there was an apparent error printing
 */
bool ScriptToolbox::printReport(const QString & name, const ParameterList & params, const QString & pdfFilename)
{
  orReport report(name, params);
  if(report.isValid())
  {
      if(!pdfFilename.isEmpty()) 
      {
          report.exportToPDF(pdfFilename);
      }
      else
      {
          report.print();
      }
  }
  else
  {
    report.reportError(NULL);
    return false;
  } 
  return true;
}

/** \brief Use the embedded OpenRPT to print or preview a named report.

    Print or preview on-screen the report using the given parameters.

    \param name    The name of the report definition to use
    \param params  The MetaSQL parameters used to control the report output
    \param preview If \c preview is true, open a report preview window;
                   otherwise print using the standard print methodology

    \return True if the report printed successfully; false if there was an apparent error printing or previewing
 */
bool ScriptToolbox::printReport(const QString & name, const ParameterList & params, const bool preview)
{
  orReport report(name, params);
  if (report.isValid())
    report.print(0, true, preview);
  else
  {
    report.reportError(NULL);
    return false;
  } 
  return true;
}

/** \brief Print multiple copies of a single report.

  Print the same report multiple times using the same set of parameters as
  part of the same print job.
  This serves as a scriptable replacement for multi-part %forms. Note that
  all copies of the report are identical as only one set of parameters
  is passed in.

  \param name   The name of the report definition to use
  \param params The MetaSQL parameters used to control the report output
  \param copies The number of copies to print

  \return True if the report printed successfully; false if there was an error
          or the %user canceled printing
  */
bool ScriptToolbox::printReportCopies(const QString & name, const ParameterList & params, int copies)
{
  QPrinter printer(QPrinter::HighResolution);

  orReport report(name, params);
  bool userCanceled = false;
  if (orReport::beginMultiPrint(&printer, userCanceled) == false)
  {
    if(!userCanceled)
      systemError(NULL, tr("Could not initialize printing system for multiple reports."));
    return userCanceled;
  }
  userCanceled = false;
  if (report.isValid())
  {
    userCanceled = true;
    for (int counter = 0; counter < copies; counter++)
      if (!report.print(&printer, (counter == 0)))
      {
        report.reportError(NULL);
        userCanceled = false;
        break;
      }
  }
  orReport::endMultiPrint(&printer);
  return userCanceled;
}

/** \brief Break a signal/slot or signal/signal connection established in the C++ core

    Much of the behavior in the core application is controlled by
    %Qt signal/slot connections. To modify certain behaviors, %scripts
    need to break the connection between signals emitted by core
    objects and the core slots.

    Note that a single object can emit several signals at different times and
    that each signal can have multiple recipients. Make sure the
    script disconnects all of the required signal/slot pairs and only those
    required.

    The caller is responsible for ensuring that the original behavior gets
    restored or appropriately modified. For example, if a script disconnects
    a Save button's \c clicked signal from a core method to save the data,
    that script developer takes responsibility to either save the data in
    the script or call the core's save method at an appropriate time.

    \param sender   The QObject that has emits the signal
    \param signal   The particular signal that the script needs to intercept
    \param receiver The core object coded to receive the signal
    \param method   The slot (or signal) of the receiver to disconnect

    \return Returns true if the connection is successfully broken;
            otherwise false

    \see QObject::disconnect

    \todo Add an example to sample_scripts
 */
bool ScriptToolbox::coreDisconnect(QObject * sender, const QString & signal, QObject * receiver, const QString & method)
{
  return QObject::disconnect(sender, QString("2%1").arg(signal).toUtf8().data(), receiver, QString("1%1").arg(method).toUtf8().data());
}

/** \brief Open a QFileDialog window.

    This is a convenience wrapper around common uses for the QFileDialog class.
    It can be used to open files or directories or to save a %file.

    \param parent  The QWidget to set as the parent of the QFileDialog
    \param caption The caption to use on the dialog
    \param dir     Start at the given directory
    \param filter  A semicolon-separated list of strings to filter the
                   directory contents
    \param fileModeSel An integer limiting what the %user may select. Note that
                       the values accepted here are not the same as the
                       integer values of the named %Qt constants:
                <table width="50%">
                  <tr><td>1</td><td>QFileDialog::ExistingFile</td></tr>
                  <tr><td>2</td><td>QFileDialog::Directory</td></tr>
                  <tr><td>3</td><td>QFileDialog::DirectoryOnly</td></tr>
                  <tr><td>4</td><td>QFileDialog::ExistingFiles</td></tr>
                  <tr><td>anything else</td><td>QFileDialog::AnyFile</td></tr>
                </table>
    \param acceptModeSel An integer controlling whether the dialog is to be
                         opened to let the %user open a %file or save a %file
              <table width="50%">
                <tr><td>1</td><td>QFileDialog::AcceptSave</td></tr>
                <tr><td>anything else</td><td>QFileDialog::AcceptOpen</td></tr>
              </table>

    \see QFileDialog

 */
QString ScriptToolbox::fileDialog(QWidget * parent, const QString & caption, const QString & dir, const QString & filter, int fileModeSel, int acceptModeSel)
{
  QStringList filters;
  QString path;
  QFileDialog::FileMode	 _fileMode;
  QFileDialog::AcceptMode _acceptMode;

  switch (fileModeSel)
  {
  case 1:
   _fileMode = QFileDialog::ExistingFile;
   break;
  case 2:
   _fileMode = QFileDialog::Directory;
   break;
  case 3:
   _fileMode = QFileDialog::DirectoryOnly;
   break;
  case 4:
   _fileMode = QFileDialog::ExistingFiles;
   break;
  default:
   _fileMode = QFileDialog::AnyFile;
  }

  switch (acceptModeSel)
  {
  case 1:
   _acceptMode = QFileDialog::AcceptSave;
   break;
  default:
   _acceptMode = QFileDialog::AcceptOpen;
  }

  QFileDialog newdlg(parent, caption, dir);
  newdlg.setFileMode(_fileMode);
  newdlg.setAcceptMode(_acceptMode);
  if (! filter.isEmpty())
  {
    filters << filter << tr("Any Files (*)");
    newdlg.setFilters(filters);
  }
  if (newdlg.exec())  path = newdlg.selectedFiles().join(", ");
  else path = "";

  return path;
}

/** \brief Open a web page or open a file with an appropriate application.

    This is a wrapper around QDesktopServices::openUrl.

    \param fileUrl The file or URL to open.
 */
void ScriptToolbox::openUrl(const QString & fileUrl)
{
    //If url scheme is missing, we'll assume it is "file" for now.
    QUrl url(fileUrl);
    if (url.scheme().isEmpty())
      url.setScheme("file");
    QDesktopServices::openUrl(url);
}

/** \deprecated Use QFile.copyFile(oldName, newName) instead */
bool ScriptToolbox::copyFile(const QString & oldName, const QString & newName)
{
   return QFile::copy(oldName, newName);
}

/** \deprecated Use var file = new QFileInfo(path).fileName() instead */
QString ScriptToolbox::getFileName(const QString & path)
{
    QFileInfo fi(path);
    return fi.fileName();
}

/** \deprecated Use QFile.rename(oldName, newName) instead */
bool ScriptToolbox::renameFile(const QString & oldName, const QString & newName)
{
   return QFile::rename(oldName, newName);
}

/** \deprecated Use QFile.removeFile(oldName, newName) instead */
bool ScriptToolbox::removeFile(const QString & name)
{
   return QFile::remove(name);
}

/** \deprecated Use QFile.exists(name) instead */
bool ScriptToolbox::fileExists(const QString & name)
{
   return QFile::exists(name);
}

/** \brief Read the entire contents of a text %file.

    This method opens a text %file, reads its contents into a string, and
    closes the %file again.

    \param name The name of the %file to open

    \return The textual content of the %file, or "error" if the %file could not
            be opened

    \see QFile
    \see QTextStream
 */
QString ScriptToolbox::textStreamRead(const QString & name)
{
  QFile file(name);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return "error";
  QTextStream stream(&file);
  QString fileText = tr(file.readAll());
  file.close();
  return fileText;
}

/** \brief Write the given text to the named %file.

    Create or overwrite a %file with the given name, using the given contents.
    If the %file name passed does not contain a suffix, \c .txt is appended
    when the file is created.

    \param pName     The name of the %file to create
    \param WriteText The string to write to the %file

    \return True if successful; false if any errors were detected

    \see QFile
    \see QTextStream
 */
bool ScriptToolbox::textStreamWrite(const QString & pName, const QString & WriteText)
{
   QString name = pName;
   if(name.isEmpty() || WriteText.isEmpty()) return false;
   QFileInfo fi(name);
   if(fi.suffix().isEmpty())
      name += ".txt";
   QFile file(name);
   if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
   QTextStream out(&file);
   out << WriteText;
   file.close();
   return true;
}

/** \deprecated use QFile.exists(name) instead */
bool fileExists(const QString & name)
{
   QFile file(name);
   return file.exists();
}

/** \deprecated Use QDir.homePath() instead */
QString ScriptToolbox::getHomeDir()
{
   return QDir::homePath();
}

/** \deprecated Use QDir.currentPath() instead */
QString ScriptToolbox::getCurrentDir()
{
   return QDir::currentPath();
}

/** \deprecated Use QDir.tempPath() instead */
QString ScriptToolbox::getTempDir()
{
   return QDir::tempPath();
}

/** \deprecated Use QDir.rootPath() instead */
QString ScriptToolbox::rootPath()
{
   return QDir::rootPath();
}

/** \deprecated Use var dir = new QDir(rootPath) followed by dir.mkpath(mkPath) instead */
bool ScriptToolbox::makePath(const QString & mkPath, const QString & rootPath)
{
   QDir dir(rootPath);

   return dir.mkpath(mkPath);
}

/** \deprecated Use var dir = new QDir(rootPath) followed by dir.rmpath(rmPath) instead */
bool ScriptToolbox::removePath(const QString & rmPath, const QString & rootPath)
{
   QDir dir(rootPath);

   return dir.rmpath(rmPath);
}

/** \brief Show the properties of a QtScript object.

    This writes all of the properties of a QtScript object using qWarning.
    This is a debugging tool, most useful when not using the Qt script debugger.
    The property list is not sorted and is not generally recursive.
    This method does show prototype properties in a separately labeled list
    if the given object has a prototype.

    The output has the name of each property and the result of toString(),
    so properties which are functions show as something like "function()".
    The values which would be returned by these functions are not displayed.
    For example,
    \code
      toolbox.listProperties(_aWarehouseClusterWidget);
    \endcode
    will show a line similar to this:
    \code
      id = "function() returning int"
    \endcode
    rather than the usually-more-useful
    \code
      id = -1
    \endcode

    \param obj The object whose properties should be listed
  */
void ScriptToolbox::listProperties(const QScriptValue &obj) const
{
  qWarning("Properties of %s:", qPrintable(obj.toString()));
  QScriptValue tmp = obj;
  while (tmp.isObject())
  {
    QScriptValueIterator prop(tmp);
    while (prop.hasNext())
    {
      prop.next();
      qWarning("  %s\t= %s",
               qPrintable(prop.name()), qPrintable(prop.value().toString()));
    }
    tmp = tmp.prototype();
    if (! (tmp.isNull() || tmp.isUndefined()))
      qWarning(" Prototype %s of %s:",
               qPrintable(tmp.toString()), qPrintable(obj.toString()));
  }
  qWarning("End of %s", qPrintable(obj.toString()));
}

/** \brief A wrapper around the QMessageBox static methods for standard dialogs.

    \deprecated Use QMessageBox.critical, QMessageBox.information,
                QMessageBox.question, or QMessageBox.warning.
 */
int ScriptToolbox::messageBox(const QString & type, QWidget * parent, const QString & title, const QString & text, int buttons, int defaultButton)
{
  int btn;
  if(type == "critical")
    btn = QMessageBox::critical(parent, title, text, (QMessageBox::StandardButton)buttons, (QMessageBox::StandardButton)defaultButton);
  else if(type == "information")
    btn = QMessageBox::information(parent, title, text, (QMessageBox::StandardButton)buttons, (QMessageBox::StandardButton)defaultButton);
  else if(type == "question")
    btn = QMessageBox::question(parent, title, text, (QMessageBox::StandardButton)buttons, (QMessageBox::StandardButton)defaultButton);
  else //if(type == "warning")
    btn = QMessageBox::warning(parent, title, text, (QMessageBox::StandardButton)buttons, (QMessageBox::StandardButton)defaultButton);
  return btn;
}

/** \brief Override the last window opened by this toolbox instance.
    \internal
 */
void ScriptToolbox::setLastWindow(QWidget * lw)
{
  _lastWindow = lw;
}

/** \brief Open a new scripted or core application window.

    This method opens a new window on the display. It can be defined
    in its .ui as an XWidget, XDialog, or XMainWindow.
    If there is a record in the %uiform table, this .ui is instantiated;
    otherwise openWindow opens a core application window with the
    given name if one exists.

    Windows that inherit from QDialog are handled specially.
    If the modality passed is Qt::NonModal, the modality is changed to
    Qt::WindowModal. The flags parameter gets Qt::Dialog set
    even if the caller did not set it.
    Because of the way the new windows are created,
    a special \c mydialog object gets created to allow access to the
    QDialog properties and methods, such as QDialog::exec.

    \param pname  The name of the %uiform or core application class of the
                  window to open
    \param parent The widget to set as the parent of the newly-created window
    \param modality This can be any of the defined Qt::WindowModality values
    \param flags    This can be any Qt::WindowFlags value

    \return The newly-opened window; 0 if there was an error
  */
QWidget *ScriptToolbox::openWindow(QString pname, QWidget *parent, Qt::WindowModality modality, Qt::WindowFlags flags)
{
  QWidget *returnVal = xtGetScreen(pname, parent, flags, 0);

  if(returnVal)
  {
    if(!returnVal->inherits("QDialog"))
    {
      omfgThis->handleNewWindow(returnVal);
      returnVal->setWindowModality(modality);
    }
    _lastWindow = returnVal;
    return returnVal;
  }

  XSqlQuery screenq;
  screenq.prepare("SELECT * "
                  "  FROM uiform "
                  " WHERE((uiform_name=:uiform_name)"
                  "   AND (uiform_enabled))"
                  " ORDER BY uiform_order DESC"
                  " LIMIT 1;");
  screenq.bindValue(":uiform_name", pname);
  screenq.exec();
  if (screenq.first())
  {
    XUiLoader loader;
    QByteArray ba = screenq.value("uiform_source").toString().toUtf8();
    QBuffer uiFile(&ba);
    if (!uiFile.open(QIODevice::ReadOnly))
    {
      QMessageBox::critical(0, tr("Could not load UI"),
                            tr("<p>There was an error loading the UI Form "
                               "from the database."));
      return 0;
    }
    QWidget *ui = loader.load(&uiFile);
    if (! ui)
    {
      QMessageBox::critical(0, tr("Could not load UI"),
                            tr("<p>There was an error creating a window from "
                               "the UI Form. It may be empty or invalid."));
      return 0;
    }
    QSize size = ui->size();
    uiFile.close();

    if (ui->inherits("QDialog"))
    {
      flags |= Qt::Dialog;
      if (modality == Qt::NonModal)
        modality = Qt::WindowModal;
    }

    XMainWindow *window = new XMainWindow(parent,
                                          screenq.value("uiform_name").toString().toAscii().data(),
                                          flags);

    window->setCentralWidget(ui);
    window->setWindowTitle(ui->windowTitle());
    window->setWindowModality(modality);
    window->resize(size);

    if (ui->inherits("QDialog"))
    {
      QDialog *innerdlg = qobject_cast<QDialog*>(ui);

      connect(innerdlg, SIGNAL(finished(int)), window, SLOT(close()));

      // alternative to creating mydialog object:
      // for each property of mydialog (including functions)
      //   add a property to _engine's mywindow property
      if (engine(window))
      {
        QScriptValue mydialog = engine(window)->newQObject(innerdlg);
        engine(window)->globalObject().setProperty("mydialog", mydialog);
      }
      else
        qWarning("Could not find the script engine to embed a dialog inside "
               "a placeholder window");

      omfgThis->handleNewWindow(window);
      returnVal = ui;
    }
    else
    {
      omfgThis->handleNewWindow(window);
      returnVal = window;
    }
    _lastWindow = window;
  }
  else if (screenq.lastError().type() != QSqlError::NoError)
  {
    systemError(0, screenq.lastError().databaseText(), __FILE__, __LINE__);
    return 0;
  }

  return returnVal;
}

/** \brief Add a column to an existing XTreeWidget
    \deprecated Use XTreeWidget::addColumn directly.
  */
void ScriptToolbox::addColumnXTreeWidget(QWidget * tree, const QString & pString, int pWidth, int pAlignment, bool pVisible, const QString pEditColumn, const QString pDisplayColumn)
{
  XTreeWidget *xt = qobject_cast<XTreeWidget*>(tree);
  if(xt)
    xt->addColumn(pString, pWidth, pAlignment, pVisible, pEditColumn, pDisplayColumn);
}

/** \brief Populate an XTreeWidget with the results of the given query.
    \deprecated Use XTreeWidget::populate directly.
  */
void ScriptToolbox::populateXTreeWidget(QWidget * tree, XSqlQuery pSql, bool pUseAltId)
{
  XTreeWidget *xt = qobject_cast<XTreeWidget*>(tree);
  if(xt)
    xt->populate(pSql, pUseAltId);
}

/** \brief Load the given QWebView with the given URL. */
void ScriptToolbox::loadQWebView(QWidget * webView, const QString & url)
{
  QUrl p = QUrl(url);
  QWebView *wv = qobject_cast<QWebView*>(webView);
  if(wv)
    wv->load(p);
}

/** \internal */
QScriptValue SetResponsetoScriptValue(QScriptEngine *engine, const enum SetResponse &sr)
{
  return QScriptValue(engine, (int)sr);
}

/** \internal */
void SetResponsefromScriptValue(const QScriptValue &obj, enum SetResponse &sr)
{
  sr = (enum SetResponse)obj.toInt32();
}

/** \internal */
QScriptValue ParameterGroupStatestoScriptValue(QScriptEngine *engine, const enum ParameterGroup::ParameterGroupStates &en)
{
  return QScriptValue(engine, (int)en);
}

/** \internal */
void ParameterGroupStatesfromScriptValue(const QScriptValue &obj, enum ParameterGroup::ParameterGroupStates &en)
{
  en = (enum ParameterGroup::ParameterGroupStates)obj.toInt32();
}

/** \internal */
QScriptValue ParameterGroupTypestoScriptValue(QScriptEngine *engine, const enum ParameterGroup::ParameterGroupTypes &en)
{
  return QScriptValue(engine, (int)en);
}

/** \internal */
void ParameterGroupTypesfromScriptValue(const QScriptValue &obj, enum ParameterGroup::ParameterGroupTypes &en)
{
  en = (enum ParameterGroup::ParameterGroupTypes)obj.toInt32();
}

/** \internal */
QScriptValue QtWindowModalitytoScriptValue(QScriptEngine *engine, const enum Qt::WindowModality &en)
{
  return QScriptValue(engine, (int)en);
}

/** \internal */
void QtWindowModalityfromScriptValue(const QScriptValue &obj, enum Qt::WindowModality &en)
{
  en = (enum Qt::WindowModality)obj.toInt32();
}

/** \internal */
QScriptValue WindowSystemtoScriptValue(QScriptEngine *engine, const enum GUIClient::WindowSystem &en)
{
  return QScriptValue(engine, (int)en);
}

/** \internal */
void WindowSystemfromScriptValue(const QScriptValue &obj, enum GUIClient::WindowSystem &en)
{
  en = (enum GUIClient::WindowSystem)obj.toInt32();
}

/** \internal */
QScriptValue SaveFlagstoScriptValue(QScriptEngine *engine, const enum SaveFlags &en)
{
  return QScriptValue(engine, (int)en);
}

/** \internal */
void SaveFlagsfromScriptValue(const QScriptValue &obj, enum SaveFlags &en)
{
  if (obj.isNumber())
    en = (enum SaveFlags)obj.toInt32();
  else if (obj.isString())
  {
    if (obj.toString() == "CHECK")
      en = CHECK;
    else if (obj.toString() == "CHANGEONE")
      en = CHANGEONE;
    else if (obj.toString() == "CHANGEALL")
      en = CHANGEALL;
    else
      qWarning("string %s could not be converted to SaveFlags",
               qPrintable(obj.toString()));
  }
  else
    qWarning("object %s could not be converted to SaveFlags",
             qPrintable(obj.toString()));
}

/** \brief Save a credit card definition from a script.

  This is a wrapper around creditCard::saveCreditCard().

  \see creditCard::saveCreditCard
 */
int ScriptToolbox::saveCreditCard(QWidget *parent,
                                  int custId,
                                  QString ccName, 
                                  QString ccAddress1, 
                                  QString ccAddress2,
                                  QString ccCity, 
                                  QString ccState, 
                                  QString ccZip, 
                                  QString ccCountry,
                                  QString ccNumber,
                                  QString ccType,
                                  QString ccExpireMonth,
                                  QString ccExpireYear,
                                  int ccId,
                                  bool ccActive )
{
  return creditCard::saveCreditCard(parent,
                                    custId,
                                    ccName,
                                    ccAddress1,
                                    ccAddress2,
                                    ccCity,
                                    ccState,
                                    ccZip,
                                    ccCountry,
                                    ccNumber,
                                    ccType,
                                    ccExpireMonth,
                                    ccExpireYear,
                                    ccId,
                                    ccActive);
}


/** \brief Get the CreditCardProcessor object for the default service.

  This is a wrapper around CreditCardProcessor::getProcessor.
  There is no scriptable way to get a CreditCardProcessor
  object for a named service provider.

  \return A CreditCardProcessor object for the current service provider

  \see CreditCardProcessor::getProcessor
 */
QObject *ScriptToolbox::getCreditCardProcessor()
{
  return CreditCardProcessor::getProcessor();
}

/** \brief C++ implementation of the original script include mechanism.

  This is the first implementation of script inclusion. It should never be
  called.

  \deprecated See include.cpp for the proper way to include scripts. This
              function will be removed, as will all examples
              of its use.

  \see include
 */
QString scriptHandleIncludes(QString source)
{
  QString returnVal = source;
  if (returnVal.contains(QRegExp("#include")))
  {
    QStringList line = returnVal.split(QRegExp("\n\r?|\r\n?"));
    for (int i = 0; i < line.size(); i++)
    {
      if (line.at(i).startsWith("#include"))
      {
        QStringList words = line.at(i).split(QRegExp("\\s+"));
        if (words.size() <= 1)
          qWarning("scriptByName(%s): found #include with no script name",
                   qPrintable(line.at(i)));

        int order = -1;
        if (words.size() > 2)
          order = words.at(2).toInt();

        QString name;
        if (words.size() > 1)
          name = words.at(1);

        line.replace(i, "// " + line.at(i));
        XSqlQuery inclq;
        inclq.prepare("SELECT script_source "
                      "FROM script "
                      "WHERE ((script_name=:name)"
                      "  AND  ((script_order=:order) OR (:order = -1))"
                      "  AND  script_enabled) "
                      "ORDER BY script_order;");
        inclq.bindValue(":name",  name);
        inclq.bindValue(":order", order);
        inclq.exec();
        bool found = false;
        while (inclq.next())
        {
          found = true;
          line.replace(i,
                       line.at(i) + "\n" + scriptHandleIncludes(inclq.value("script_source").toString()));
        }
        if (found)
          line.replace(i,
                       line.at(i) + "\n// end include of " + name);
      }
    }
    returnVal = line.join("\n");
  }

  return returnVal;
}

/** \brief This is a wrapper around the core storedProcErrorLookup. */
QString ScriptToolbox::storedProcErrorLookup(const QString proc, const int result)
{
  return ::storedProcErrorLookup(proc, result);
}

/** \brief This functions takes a regexp string and creates and returns a QRegExpValidator. */
QObject * ScriptToolbox::customVal(const QString & ReqExp)
{
  QRegExp rx(ReqExp);
  QValidator * val = new QRegExpValidator(rx, omfgThis);
  return val;
}


/** \fn int     ScriptToolbox::decimalPlaces(QString p)	
    \brief This is a wrapper for the C++ core function decimalPlaces.
 */
/** \fn QString ScriptToolbox::formatNumber(double value, int decimals)
    \brief This is a wrapper for the C++ core function formatNumber.
 */
/** \fn QString ScriptToolbox::formatMoney(double val, int curr = -1, int extra = 0)	
    \brief This is a wrapper for the C++ core function formatMoney.
 */
/** \fn QString ScriptToolbox::formatCost(double val, int curr= -1)	
    \brief This is a wrapper for the C++ core function formatCost.
 */
/** \fn QString ScriptToolbox::formatExtPrice(double val, int curr=-1)	
    \brief This is a wrapper for the C++ core function formatExtPrice.
 */
/** \fn QString ScriptToolbox::formatWeight(double val)	
    \brief This is a wrapper for the C++ core function formatWeight.
 */
/** \fn QString ScriptToolbox::formatQty(double val)	
    \brief This is a wrapper for the C++ core function formatQty.
 */
/** \fn QString ScriptToolbox::formatQtyPer(double val)	
    \brief This is a wrapper for the C++ core function  formatQtyPer.
 */
/** \fn QString ScriptToolbox::formatSalesPrice(double val, int curr = -1)	
    \brief This is a wrapper for the C++ core function formatSalesPrice.
 */
/** \fn QString ScriptToolbox::formatPurchPrice(double val, int curr = -1)	
    \brief This is a wrapper for the C++ core function formatPurchPrice.
 */
/** \fn QString ScriptToolbox::formatUOMRatio(double val)	
    \brief This is a wrapper for the C++ core function formatUOMRatio.
 */
/** \fn QString ScriptToolbox::formatPercent(double val)	
    \brief This is a wrapper for the C++ core function formatPercent.
 */
/** \fn QColor  ScriptToolbox::namedColor(QString name)	
    \brief This is a wrapper for the C++ core function namedColor.
 */
/** \fn QString ScriptToolbox::formatDate(const QDate &pDate)
    \brief This is a wrapper for the C++ core function formatDate.
 */
