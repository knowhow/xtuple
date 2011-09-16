/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlRecord>
#include <QVBoxLayout>

#include "xlineedit.h"
#include "xcheckbox.h"
#include "xsqlquery.h"
#include "xsqltablemodel.h"
#include "shortcuts.h"

#include "virtualCluster.h"

#define DEBUG false

void VirtualCluster::init()
{
    _number = 0;

    setFocusPolicy(Qt::StrongFocus);
    _label = new QLabel(this);
    _label->setObjectName("_label");
    _label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    _name = new QLabel(this);
    _name->setObjectName("_name");
    _name->setVisible(false);

    _description = new QLabel(this);
    _description->setObjectName("_description");
    _description->setVisible(false);
    _description->setMaximumWidth(300);
    if(!(_x_metrics && _x_metrics->boolean("VirtualClusterDisableMultiLineDesc")))
      _description->setWordWrap(true);

    _grid = new QGridLayout(this);
    _grid->setMargin(0);
    _grid->setSpacing(6);
    if (!_label->text().isEmpty())
      _grid->addWidget(_label,  0, 0);
    _hspcr = new QSpacerItem(0, 0, QSizePolicy::Expanding);
    _grid->addItem(_hspcr, 0, _grid->columnCount() + 1);
    _orientation = Qt::Horizontal;
    setOrientation(Qt::Vertical);
    
    _mapper = new XDataWidgetMapper(this);
}

VirtualCluster::VirtualCluster(QWidget* pParent, const char* pName) :
    QWidget(pParent)
{
  setObjectName(pName ? pName : "VirtualCluster");
  init();
}

VirtualCluster::VirtualCluster(QWidget* pParent,
			       VirtualClusterLineEdit* pNumberWidget,
			       const char* pName) :
    QWidget(pParent)
{
  setObjectName(pName);

  init();
  if (pNumberWidget)
    addNumberWidget(pNumberWidget);
}

void VirtualCluster::clear()
{
  if (DEBUG)
    qDebug("VC %s::clear()", qPrintable(objectName()));

  setId(-1);
  _number->clear();
  _name->clear();
  _description->clear();
}

QString VirtualCluster::nullStr() const
{
  if (_number)
    return _number->nullStr();
  return QString();
}

void VirtualCluster::setNullStr(const QString &text)
{
  if (_number)
    _number->setNullStr(text);
}

void VirtualClusterLineEdit::sEllipses()
{
    if (_x_preferences && _x_preferences->value("DefaultEllipsesAction") == "search")
        sSearch();
    else
        sList();
}

void VirtualCluster::setLabel(const QString& p)
{
  _label->setText(p);
  if (p.isEmpty())
    _grid->removeWidget(_label);
  else
    _grid->addWidget(_label,0,0);
}

void VirtualCluster::setDataWidgetMap(XDataWidgetMapper* m)
{
  disconnect(_number, SIGNAL(newId(int)), this, SLOT(updateMapperData()));
  m->addMapping(this, _fieldName, "number", "defaultNumber");
  _mapper=m;
  connect(_number, SIGNAL(newId(int)), this, SLOT(updateMapperData()));
}

void VirtualCluster::setEnabled(const bool p)
{
  QList<QWidget*> child = findChildren<QWidget*>();
  for (int i = 0; i < child.size(); i++)
  {
    if (child[i]->inherits("VirtualCluster"))
      ((VirtualCluster*)(child[i]))->setEnabled(p);
    else
      child[i]->setEnabled(p || child[i]->inherits("QLabel"));
  }
}

void VirtualCluster::setStrict(const bool b)
{
  _number->setStrict(b);
}

void VirtualCluster::setShowInactive(const bool b)
{
  _number->setShowInactive(b);
}

void VirtualCluster::addNumberWidget(VirtualClusterLineEdit* pNumberWidget)
{
  _number = pNumberWidget;
  if (! _number)
    return;

  _grid->addWidget(_number, 0, 1);
  setFocusProxy(pNumberWidget);

  connect(_number,	SIGNAL(newId(int)),	this,	 SIGNAL(newId(int)));
  connect(_number,	SIGNAL(parsed()), 	this, 	 SLOT(sRefresh()));
  connect(_number,	SIGNAL(valid(bool)),	this,	 SIGNAL(valid(bool)));
}

void VirtualCluster::sInfo()
{
  _number->sInfo();
}

void VirtualCluster::sList()
{
  _number->sList();
}

void VirtualCluster::sSearch()
{
  _number->sSearch();
}

void VirtualCluster::sRefresh()
{
  if (DEBUG)
    qDebug("VC %s::sRefresh() with id %d",
           qPrintable(objectName()), _number ? _number->_id : -1);
  _name->setText(_number->_name);
  _description->setText(_number->_description);
}

void VirtualCluster::setReadOnly(const bool b)
{
  _readOnly = b;
  if (b)
    _number->setEnabled(false);
  else
    _number->setEnabled(true);
}

void VirtualCluster::updateMapperData()
{
  if (_mapper->model() &&
      _mapper->model()->data(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(this))).toString() != number())
    _mapper->model()->setData(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(this)), number());
}

Qt::Orientation VirtualCluster::orientation()
{
  return _orientation;
}

void VirtualCluster::setOrientation(Qt::Orientation orientation)
{
  if (orientation == _orientation)
    return;

  _grid->removeWidget(_name);
  _grid->removeWidget(_description);
  _grid->removeItem(_hspcr);

  if (orientation == Qt::Vertical)
  {
    _grid->addWidget(_name,   1, 1, 1, -1);
    _grid->addWidget(_description, 2, 1, 1, -1);
  }
  else
  {
    _grid->addWidget(_name, 0, _grid->columnCount());
    _grid->addWidget(_description, 0, _grid->columnCount());
  }
  _grid->addItem(_hspcr,0, _grid->columnCount());
  _orientation = orientation;
}

bool VirtualCluster::nameVisible() const
{
  return _name && _name->isVisible();
}

void VirtualCluster::setNameVisible(const bool p)
{
  if (_name)
    _name->setHidden(!p);
}

bool VirtualCluster::descriptionVisible() const
{
  return _description && _description->isVisible();
}

void VirtualCluster::setDescriptionVisible(const bool p)
{
  if (_description)
    _description->setHidden(!p);
}

///////////////////////////////////////////////////////////////////////////

GuiClientInterface* VirtualClusterLineEdit::_guiClientInterface = 0;

VirtualClusterLineEdit::VirtualClusterLineEdit(QWidget* pParent,
					       const char* pTabName,
					       const char* pIdColumn,
					       const char* pNumberColumn,
					       const char* pNameColumn,
					       const char* pDescripColumn,
					       const char* pExtra,
                                               const char* pName,
                                               const char* pActiveColumn) :
    XLineEdit(pParent, pName)
{
    if (DEBUG)
      qDebug("VirtualClusterLineEdit(%p, %s, %s, %s, %s, %s, %s, %s, %s)",
             pParent ? pParent : 0,         pTabName ? pTabName : "",
             pIdColumn ? pIdColumn : "",     pNumberColumn ? pNumberColumn : "",
             pNameColumn ? pNameColumn : "",
             pDescripColumn ? pDescripColumn : "",
             pExtra ? pExtra : "",           pName ? pName : "",
             pActiveColumn ? pActiveColumn : "");

    setObjectName(pName ? pName : "VirtualClusterLineEdit");

    _valid  = false;
    _parsed = true;
    _strict = true;
    _completer = 0;
    _showInactive = false;

    setTableAndColumnNames(pTabName, pIdColumn, pNumberColumn, pNameColumn, pDescripColumn, pActiveColumn);

    if (pExtra && QString(pExtra).trimmed().length())
	_extraClause = pExtra;

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    clear();
    _titleSingular = tr("Object");
    _titlePlural = tr("Objects");

    // Completer set up
    if (_x_metrics)
    {
      if (!_x_metrics->boolean("DisableAutoComplete"))
      {
        QSqlQueryModel* hints = new QSqlQueryModel(this);
        _completer = new QCompleter(hints,this);
        _completer->setWidget(this);
        QTreeView* view = new QTreeView(this);
        view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view->setHeaderHidden(true);
        view->setRootIsDecorated(false);
        _completer->setPopup(view);
        _completer->setCaseSensitivity(Qt::CaseInsensitive);
        _completer->setCompletionColumn(1);
        _completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
        connect(this, SIGNAL(textEdited(QString)), this, SLOT(sHandleCompleter()));
        connect(_completer, SIGNAL(highlighted(QString)), this, SLOT(setText(QString)));
      }
    }

    // Set up actions
    connect(_listAct, SIGNAL(triggered()), this, SLOT(sList()));
    connect(_searchAct, SIGNAL(triggered()), this, SLOT(sSearch()));

    _infoAct = new QAction(tr("Info..."), this);
    _infoAct->setShortcut(QKeySequence(tr("Ctrl+Shift+I")));
    _infoAct->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    _infoAct->setToolTip(tr("View record information"));
    _infoAct->setEnabled(false);
    connect(_infoAct, SIGNAL(triggered()), this, SLOT(sInfo()));
    addAction(_infoAct);

    _openAct = new QAction(tr("Open..."), this);
    _openAct->setShortcut(QKeySequence(tr("Ctrl+Shift+O")));
    _openAct->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    _openAct->setToolTip(tr("Open record detail"));
    _openAct->setEnabled(false);
    connect(_openAct, SIGNAL(triggered()), this, SLOT(sOpen()));
    addAction(_openAct);

    _newAct = new QAction(tr("New..."), this);
    _newAct->setShortcut(QKeySequence(tr("Ctrl+Shift+N")));
    _newAct->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    _newAct->setToolTip(tr("Create new record"));
    _newAct->setEnabled(false);
    connect(_newAct, SIGNAL(triggered()), this, SLOT(sNew()));
    addAction(_newAct);

    connect(this, SIGNAL(valid(bool)), _infoAct, SLOT(setEnabled(bool)));

    _menuLabel = new QLabel(this);
    // Menu set up

    _menu = 0;
    _menuLabel->setPixmap(QPixmap(":/widgets/images/magnifier.png"));
    _menuLabel->installEventFilter(this);

    int height = minimumSizeHint().height();
    QString sheet = QLatin1String("QLineEdit{ padding-right: ");
    sheet += QString::number(_menuLabel->pixmap()->width() + 6);
    sheet += QLatin1String(";}");
    setStyleSheet(sheet);
    // Little hack. Somehow style sheet makes widget short. Put back height.
    setMinimumHeight(height);

    // Set default menu with standard actions
    QMenu* menu = new QMenu;
    menu->addAction(_listAct);
    menu->addAction(_searchAct);
    menu->addSeparator();
    menu->addAction(_infoAct);
    setMenu(menu);

    connect(this, SIGNAL(valid(bool)), this, SLOT(sUpdateMenu()));
    connect(menu, SIGNAL(aboutToShow()), this, SLOT(sUpdateMenu()));
  }

bool VirtualClusterLineEdit::eventFilter(QObject *obj, QEvent *event)
{
    if (!_menu || obj != _menuLabel)
        return QObject::eventFilter(obj, event);

    switch (event->type()) {
    case QEvent::MouseButtonPress: {
        const QMouseEvent *me = static_cast<QMouseEvent *>(event);
        _menu->exec(me->globalPos());
        return true;
    }
    default:
        break;
    }
    return QObject::eventFilter(obj, event);
}

void VirtualClusterLineEdit::focusInEvent(QFocusEvent * event)
{
  sUpdateMenu();
  XLineEdit::focusInEvent(event);
}

void VirtualClusterLineEdit::resizeEvent(QResizeEvent *)
{
  positionMenuLabel();
}

void VirtualClusterLineEdit::positionMenuLabel()
{
  _menuLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  _menuLabel->setStyleSheet("QLabel { margin-right:6}");

  _menuLabel->setGeometry(width() - _menuLabel->pixmap()->width() - 6, 0,
                          _menuLabel->pixmap()->width() + 6, height());
}

void VirtualClusterLineEdit::setShowInactive(const bool p)
{
  _showInactive = p;
}

void VirtualClusterLineEdit::setUiName(const QString& name)
{
  _uiName = name;
  sUpdateMenu();
}

void VirtualClusterLineEdit::setEditPriv(const QString& priv)
{
  _editPriv = priv;
  sUpdateMenu();
}

void VirtualClusterLineEdit::setNewPriv(const QString& priv)
{
  _newPriv = priv;
  sUpdateMenu();
}

void VirtualClusterLineEdit::setViewPriv(const QString& priv)
{
  _viewPriv = priv;
  sUpdateMenu();
}

void VirtualClusterLineEdit::sUpdateMenu()
{
  if (! _x_privileges)
    return;

  _listAct->setEnabled(isEnabled());
  _searchAct->setEnabled(isEnabled());
  _aliasAct->setEnabled(isEnabled());
  _infoAct->setEnabled(_id != -1);
  _openAct->setEnabled((_x_privileges->check(_editPriv) ||
                        _x_privileges->check(_viewPriv)) &&
                       _id != -1);
  _newAct->setEnabled(_x_privileges->check(_newPriv) &&
                      isEnabled());

  if (canOpen())
  {
    if (!menu()->actions().contains(_openAct))
      menu()->addAction(_openAct);

    if (!menu()->actions().contains(_newAct) &&
        !_newPriv.isEmpty())
      menu()->addAction(_newAct);
  }
  else
  {
    if (menu()->actions().contains(_openAct))
      menu()->removeAction(_openAct);

    if (menu()->actions().contains(_newAct))
      menu()->removeAction(_newAct);
  }
}

void VirtualClusterLineEdit::setMenu(QMenu *menu)
{
  _menu = menu;
}

void VirtualClusterLineEdit::sHandleCompleter()
{
  if (!hasFocus())
    return;

  QString stripped = text().trimmed().toUpper();
  if (stripped.isEmpty())
    return;

  int width = 0;
  QSqlQueryModel* model = static_cast<QSqlQueryModel *>(_completer->model());
  QTreeView * view = static_cast<QTreeView *>(_completer->popup());
  _parsed = true;
  XSqlQuery numQ;
  numQ.prepare(_query + _numClause +
               (_extraClause.isEmpty() || !_strict ? "" : " AND " + _extraClause) +
               ((_hasActive && ! _showInactive) ? _activeClause : "") +
               QString(" ORDER BY %1 LIMIT 10;").arg(_numColName));
  numQ.bindValue(":number", "^" + stripped);
  numQ.exec();
  if (numQ.first())
  {
    int numberCol = numQ.record().indexOf("number");
    int nameCol = numQ.record().indexOf("name");
    int descripCol = numQ.record().indexOf("description");
    model->setQuery(numQ);
    _completer->setCompletionPrefix(stripped);

    for (int i = 0; i < model->columnCount(); i++)
    {
      if ( (i != numberCol) &&
           (!_hasName || i != nameCol ) &&
           (!_hasDescription || i != descripCol) )
      {
        view->hideColumn(i);
      }
    }

    view->resizeColumnToContents(numberCol);
    width += view->columnWidth(numberCol);
    if (_hasName)
    {
      view->resizeColumnToContents(nameCol);
      width += view->columnWidth(nameCol);
    }
    if (_hasDescription)
    {
      view->resizeColumnToContents(descripCol);
      width += view->columnWidth(descripCol);
    }
  }
  else
    model->setQuery(QSqlQuery());

  if (width > 350)
    width = 350;

  QRect rect;
  rect.setHeight(height());
  rect.setWidth(width);
  rect.setBottomLeft(QPoint(0, height() - 2));
  _completer->complete(rect);
  _parsed = false;
}

void VirtualClusterLineEdit::sHandleNullStr()
{
  if (_nullStr.isEmpty() ||
      !_parsed)
    return;

  QString sheet = styleSheet();
  QString nullStyle = " QLineEdit{ color: Grey; "
                      "            font: italic; "
                      "            font-size: 12px}";

  if (!hasFocus() &&
      text().isEmpty() &&
      !_valid)
  {
    setText(_nullStr);
    sheet.append(nullStyle);
  }
  else if (hasFocus() && !_valid)
  {
    clear();
    sheet.remove(nullStyle);
  }
  else if (_valid)
    sheet.remove(nullStyle);

  setStyleSheet(sheet);
}

void VirtualClusterLineEdit::setStrikeOut(bool enable)
{
  if (font().strikeOut() == enable)
    return;

  QFont font = this->font();
  font.setStrikeOut(enable);
  setFont(font);

  if (enable)
    connect(this, SIGNAL(textEdited(QString)), this, SLOT(setStrikeOut()));
  else
    disconnect(this, SIGNAL(textEdited(QString)), this, SLOT(setStrikeOut()));
}

void VirtualClusterLineEdit::setTableAndColumnNames(const char* pTabName,
						    const char* pIdColumn,
						    const char* pNumberColumn,
						    const char* pNameColumn,
                                                    const char* pDescripColumn,
                                                    const char* pActiveColumn)
{
  _idColName = QString(pIdColumn);
  _numColName = QString(pNumberColumn);
  _nameColName = QString(pNameColumn);
  _descripColName = QString(pDescripColumn);
  _activeColName = QString(pActiveColumn);

  _query = QString("SELECT %1 AS id, %2 AS number ")
		  .arg(pIdColumn).arg(pNumberColumn);

  _hasName = (pNameColumn && QString(pNameColumn).trimmed().length());
  if (_hasName)
    _query += QString(", %1 AS name ").arg(pNameColumn);

  _hasDescription = (pDescripColumn &&
                     QString(pDescripColumn).trimmed().length());
  if (_hasDescription)
    _query += QString(", %1 AS description ").arg(pDescripColumn);

  _hasActive = (pActiveColumn && QString(pActiveColumn).trimmed().length());
  if (_hasActive)
    _query += QString(", %1 AS active ").arg(pActiveColumn);

  _query += QString("FROM %1 WHERE (TRUE) ").arg(pTabName);

  _idClause = QString(" AND (%1=:id) ").arg(pIdColumn);
  _numClause = QString(" AND (%1 ~* :number) ").arg(pNumberColumn);

  if (_hasActive)
    _activeClause = QString(" AND (%1) ").arg(pActiveColumn);
  else
    _activeClause = "";

  _extraClause = "";
  _model = new QSqlQueryModel(this);
}

void VirtualClusterLineEdit::setTitles(const QString& s, const QString& p)
{
    _titleSingular = s;
    _titlePlural = !p.isEmpty() ? p : s;
}

bool VirtualClusterLineEdit::canOpen()
{
  if (!_uiName.isEmpty() && _guiClientInterface)
    return true;
  return false;
}

void VirtualClusterLineEdit::clear()
{
    if (DEBUG)
      qDebug("VCLE %s::clear()", qPrintable(objectName()));

    int oldid = _id;
    bool oldvalid = _valid;

    XLineEdit::clear();
    _description = "";
    _name = "";
    _id = -1;	// calling setId() or silentSetId() is recursive
    _valid = false;
    _model = new QSqlQueryModel(this);
    if (oldvalid != _valid)
      emit valid(_valid);
    if (oldid != _id)
      emit newId(_id);
}

void VirtualClusterLineEdit::setId(const int pId)
{
  if (DEBUG)
    qDebug("VCLE %s::setId(%d)", qPrintable(objectName()), pId);

    if (pId == -1 || pId == 0)
    {
	clear();
        emit parsed();
    }
    else
    {
      bool changed = (pId != _id);
      silentSetId(pId);
      if (changed)
      {
        emit newId(pId);
        emit valid(_valid);
      }
    }
}

void VirtualClusterLineEdit::silentSetId(const int pId)
{
  if (DEBUG)
    qDebug("VCLE %s::silentSetId(%d)", qPrintable(objectName()), pId);

  if (pId == -1)
  {
    XLineEdit::clear();
    _model = new QSqlQueryModel(this);
  }
  else
  {
    XSqlQuery idQ;
    idQ.prepare(_query + _idClause + QString(";"));
    idQ.bindValue(":id", pId);
    idQ.exec();
    if (idQ.first())
    {
      if (_completer)
        static_cast<QSqlQueryModel* >(_completer->model())->setQuery(QSqlQuery());

      _id = pId;
      _valid = true;

      _model->setQuery(idQ);

      setText(idQ.value("number").toString());
      if (_hasName)
        _name = (idQ.value("name").toString());
      if (_hasDescription)
        _description = idQ.value("description").toString();
      if (_hasActive)
        setStrikeOut(!idQ.value("active").toBool());
    }
    else if (idQ.lastError().type() != QSqlError::NoError)
      QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
                            .arg(__FILE__)
                            .arg(__LINE__),
                            idQ.lastError().databaseText());
  }

  _parsed = true;
  sHandleNullStr();
  emit parsed();
}

void VirtualClusterLineEdit::setNumber(const QString& pNumber)
{
  _parsed = false;
  setText(pNumber);
  sParse();
}

void VirtualClusterLineEdit::sParse()
{
  if (DEBUG)
    qDebug("VCLE %s::sParse() entered with _parsed %d and text() %s",
           qPrintable(objectName()), _parsed, qPrintable(text()));

    if (! _parsed)
    {
      QString stripped = text().trimmed().toUpper();
      if (stripped.length() == 0)
      {
        _parsed = true;
	setId(-1);
      }
      else
      {
        XSqlQuery numQ;
        numQ.prepare(_query + _numClause +
		    (_extraClause.isEmpty() || !_strict ? "" : " AND " + _extraClause) +
                    ((_hasActive && ! _showInactive) ? _activeClause : "" ) +
                    QString("ORDER BY %1 LIMIT 1;").arg(_numColName));
        numQ.bindValue(":number", "^" + stripped);
        numQ.exec();
        if (numQ.first())
	{
	    _valid = true;
            setId(numQ.value("id").toInt());
	    if (_hasName)
              _name = (numQ.value("name").toString());
	    if (_hasDescription)
              _description = numQ.value("description").toString();
	}
	else
	{
            setId(-1);
            if (numQ.lastError().type() != QSqlError::NoError)
		QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
					      .arg(__FILE__)
					      .arg(__LINE__),
                        numQ.lastError().databaseText());
	}
      }
      emit valid(_valid);
      emit parsed();
    }
    _parsed = true;
    sHandleNullStr();
}

void VirtualClusterLineEdit::sList()
{
  disconnect(this, SIGNAL(editingFinished()), this, SLOT(sParse()));

  VirtualList* newdlg = listFactory();
  if (newdlg)
  {
    int id = newdlg->exec();
    setId(id);
  }
  else
    QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__),
                          tr("%1::sList() not yet defined").arg(objectName()));

  connect(this, SIGNAL(editingFinished()), this, SLOT(sParse()));
}

void VirtualClusterLineEdit::sSearch()
{
  disconnect(this, SIGNAL(editingFinished()), this, SLOT(sParse()));

  VirtualSearch* newdlg = searchFactory();
  if (newdlg)
  {
    QString stripped = text().trimmed();
    if(stripped.length())
    {
      XSqlQuery numQ;
      numQ.prepare(_query + _numClause +
                   (_extraClause.isEmpty() || !_strict ? "" : " AND " + _extraClause) +
                   ((_hasActive && ! _showInactive) ? _activeClause : "" ) +
                   QString("ORDER BY %1;").arg(_numColName));
      numQ.bindValue(":number", "^" + stripped);
      numQ.exec();
      if (numQ.first())
        newdlg->setQuery(numQ);
    }
    newdlg->setSearchText(text());
    int id = newdlg->exec();
    setId(id);
  }
  else
    QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__),
                          tr("%1::sSearch() not yet defined").arg(objectName()));

  connect(this, SIGNAL(editingFinished()), this, SLOT(sParse()));
}

void VirtualCluster::sEllipses()
{
  _number->sEllipses();
}

void VirtualClusterLineEdit::sInfo()
{
  VirtualInfo* newdlg = infoFactory();
  if (newdlg)
    newdlg->exec();
  else
    QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__),
                          tr("%1::sInfo() not yet defined").arg(objectName()));
}

void VirtualClusterLineEdit::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("captive", true);

  sOpenWindow(_uiName, params);
}

void VirtualClusterLineEdit::sOpen()
{
  ParameterList params;
  if (_x_privileges->check(_editPriv))
    params.append("mode", "edit");
  else
    params.append("mode", "view");
  params.append(_idColName, id());

  sOpenWindow(_uiName, params);
}

QWidget* VirtualClusterLineEdit::sOpenWindow(const QString &uiName, ParameterList &params)
{
  QWidget* w = 0;
  if (parentWidget()->window())
    w = _guiClientInterface->openWindow(uiName, params, parentWidget()->window() , Qt::WindowModal, Qt::Dialog);

  if (w->inherits("QDialog"))
  {
    QDialog* newdlg = qobject_cast<QDialog*>(w);
    int id = newdlg->exec();
    if (id != QDialog::Rejected)
    {
      silentSetId(id);
      emit newId(_id);
      emit valid(_id != -1);
    }
  }
  else
    connect(w, SIGNAL(saved(int)), this, SLOT(setId(int)));

  return w;
}

void VirtualClusterLineEdit::setStrict(const bool b)
{
  _strict = b;
}

VirtualInfo* VirtualClusterLineEdit::infoFactory()
{
    return new VirtualInfo(this);
}

VirtualList* VirtualClusterLineEdit::listFactory()
{
    return new VirtualList(this);
}

VirtualSearch* VirtualClusterLineEdit::searchFactory()
{
  return new VirtualSearch(this);
}

///////////////////////////////////////////////////////////////////////////////

void VirtualList::init()
{
    setWindowModality(Qt::ApplicationModal);
    setAttribute(Qt::WA_DeleteOnClose);
    _search	= new QLineEdit(this);
    _search->setObjectName("_search");
    _searchLit	= new QLabel(tr("S&earch for:"), this);
    _searchLit->setBuddy(_search);
    _searchLit->setObjectName("_searchLit");
#ifdef Q_WS_MAC
    _search->setMinimumHeight(22);
#endif
    _buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                      Qt::Vertical, this);
    _select	= _buttonBox->button(QDialogButtonBox::Ok);
    _listTab	= new XTreeWidget(this);
    _titleLit	= new QLabel("", this);
    _titleLit->setBuddy(_listTab);
    _titleLit->setObjectName("_titleLit");

    _listTab->setObjectName("_listTab");
    _listTab->setPopulateLinear(false);

    _searchLit->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    _select->setEnabled(false);
    _listTab->setMinimumHeight(250);
    _titleLit->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

    _dialogLyt                = new QVBoxLayout(this);
    _dialogLyt->setContentsMargins(5, 5, 5, 5);

    QHBoxLayout* topLyt	      = new QHBoxLayout();
    QVBoxLayout* searchLyt    = new QVBoxLayout();
    QVBoxLayout* buttonsLyt   = new QVBoxLayout();
    QHBoxLayout* searchStrLyt = new QHBoxLayout();
    QVBoxLayout* tableLyt     = new QVBoxLayout();

    topLyt->setObjectName("topLyt");
    searchLyt->setObjectName("searchLyt");
    buttonsLyt->setObjectName("buttonsLyt");
    searchStrLyt->setObjectName("searchStrLyt");
    tableLyt->setObjectName("tableLyt");

    _dialogLyt->addLayout(topLyt);
    topLyt->addLayout(searchLyt);
    topLyt->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding,
					    QSizePolicy::Minimum));
    topLyt->addLayout(buttonsLyt);
    searchLyt->addLayout(searchStrLyt);
    _dialogLyt->addLayout(tableLyt);

    searchStrLyt->addWidget(_searchLit);
    searchStrLyt->addWidget(_search);
    buttonsLyt->addWidget(_buttonBox);

    tableLyt->addWidget(_titleLit);
    tableLyt->addWidget(_listTab);

    topLyt->setStretchFactor(searchLyt, 0);
    _dialogLyt->setStretchFactor(topLyt, 0);
    _dialogLyt->setStretchFactor(tableLyt, 1);

    connect(_buttonBox,   SIGNAL(rejected()),	 this,   SLOT(sClose()));
    connect(_listTab, SIGNAL(itemSelected(int)), this,	 SLOT(sSelect()));
    connect(_listTab, SIGNAL(valid(bool)),     _select, SLOT(setEnabled(bool)));
    connect(_search,  SIGNAL(textChanged(const QString&)), this, SLOT(sSearch(const QString&)));
    connect(_buttonBox,  SIGNAL(accepted()),         this,	 SLOT(sSelect()));
}

VirtualList::VirtualList() : QDialog()
{
  init();
}

VirtualList::VirtualList(QWidget* pParent, Qt::WindowFlags pFlags ) :
    QDialog(pParent, pFlags)
{
    init();

    setObjectName("virtualList");

    if(pParent->inherits("ExpenseLineEdit"))
      _listTab->addColumn(tr("Category"), -1, Qt::AlignLeft, true, "number");
    else
      _listTab->addColumn(tr("Number"), -1, Qt::AlignLeft, true, "number");

    if (pParent->inherits("VirtualClusterLineEdit"))
    {
      _parent = (VirtualClusterLineEdit*)(pParent);
      setWindowTitle(_parent->_titlePlural);
      if (_parent->_hasName)
      {
	_listTab->addColumn(tr("Name"),	 -1, Qt::AlignLeft, true, "name");
	_listTab->setColumnWidth(_listTab->columnCount() - 1, 100);
      }

      if (_parent->_hasDescription)
      {
	  _listTab->addColumn(tr("Description"),  -1, Qt::AlignLeft, true, "description");
	  _listTab->setColumnWidth(_listTab->columnCount() - 1, 100);
      }

      _id = _parent->_id;
      _titleLit->setText(_parent->_titlePlural);
    }
    else
    {
      _parent = 0;
      _id = -1;
    }

    shortcuts::setStandardKeys(this);
    sFillList();
}

void VirtualList::sClose()
{
    done(_id);
}

void VirtualList::sSelect()
{
    done(_listTab->id());
}

void VirtualList::sSearch(const QString& pTarget)
{
  for (int i = 0; i < _listTab->columnCount(); i++)
  {
    QList<XTreeWidgetItem*> matches = _listTab->findItems(pTarget, Qt::MatchStartsWith, i);

    if (matches.size() > 0)
    {
      _listTab->setCurrentItem(matches[0]);
      _listTab->scrollToItem(matches[0]);
      return;
    }
  }
}

void VirtualList::sFillList()
{
    if (! _parent)
      return;

    _listTab->clear();
    XSqlQuery query(_parent->_query +
		    (_parent->_extraClause.isEmpty() ? "" :
					    " AND " + _parent->_extraClause) +
                    ((_parent->_hasActive && ! _parent->_showInactive) ? _parent->_activeClause : "") +
		    QString(" ORDER BY ") +
		    QString((_parent->_hasName) ? "name" : "number"));
    _listTab->populate(query);
}

void VirtualList::showEvent(QShowEvent* e)
{
  sFillList();
  QDialog::showEvent(e);
}

///////////////////////////////////////////////////////////////////////////////

VirtualSearch::VirtualSearch(QWidget* pParent, Qt::WindowFlags pFlags) :
    QDialog(pParent, pFlags)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::ApplicationModal);
    setObjectName("virtualSearch");

    _search = new QLineEdit(this);
    _search->setObjectName("_search");
    _searchLit = new QLabel(tr("S&earch for:"), this);
    _searchLit->setBuddy(_search);
    _searchLit->setObjectName("_searchLit");

    _searchNumber = new XCheckBox(tr("Search through Numbers"), this);
    _searchNumber->setObjectName("_searchNumber");
    _searchName = new XCheckBox(tr("Search through Names"), this);
    _searchName->setObjectName("_searchName");
    _searchDescrip = new XCheckBox(tr("Search through Descriptions"), this);
    _searchDescrip->setObjectName("_searchDescrip");
    _buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                      Qt::Vertical, this);
    _select = _buttonBox->button(QDialogButtonBox::Ok);
    _listTab = new XTreeWidget(this);
    _titleLit = new QLabel("", this);
    _titleLit->setBuddy(_listTab);
    _titleLit->setObjectName("_titleLit");

    _listTab->setObjectName("_listTab");
    _listTab->setPopulateLinear(false);

    _searchLit->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    _select->setEnabled(false);
    _listTab->setMinimumHeight(250);
    _titleLit->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

    _dialogLyt   = new QVBoxLayout(this);
    _dialogLyt->setContentsMargins(5, 5, 5, 5);

    QHBoxLayout* topLyt = new QHBoxLayout();
    searchLyt    = new QVBoxLayout();
    buttonsLyt   = new QVBoxLayout();
    searchStrLyt = new QHBoxLayout();
    selectorsLyt = new QGridLayout();
    tableLyt     = new QVBoxLayout();

    topLyt->setObjectName("topLyt");
    searchLyt->setObjectName("searchLyt");
    buttonsLyt->setObjectName("buttonsLyt");
    searchStrLyt->setObjectName("searchStrLyt");
    selectorsLyt->setObjectName("selectorsLyt");
    tableLyt->setObjectName("tableLyt");

    _dialogLyt->addLayout(topLyt);
    _dialogLyt->addLayout(tableLyt);
    topLyt->addLayout(searchLyt);
    topLyt->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding,
					    QSizePolicy::Minimum));
    topLyt->addLayout(buttonsLyt);
    searchLyt->addItem(searchStrLyt);
    searchLyt->addItem(selectorsLyt);
    searchLyt->setObjectName("searchLyt");

    searchStrLyt->addWidget(_searchLit);
    searchStrLyt->addWidget(_search);
#ifdef Q_WS_MAC
    _search->setMinimumHeight(22);
    selectorsLyt->setVerticalSpacing(6);
#else
    _search->setMinimumHeight(20);
#endif

    selectorsLyt->addWidget(_searchNumber,  0, 0);
    selectorsLyt->addWidget(_searchName,    1, 0);
    selectorsLyt->addWidget(_searchDescrip, 2, 0);
    buttonsLyt->addWidget(_buttonBox);
    buttonsLyt->addItem(new QSpacerItem(20, 0, QSizePolicy::Minimum,
						QSizePolicy::Expanding));
    tableLyt->addWidget(_titleLit);
    tableLyt->addWidget(_listTab);

    ((QBoxLayout*)topLyt)->setStretchFactor(searchLyt, 0);
    ((QBoxLayout*)_dialogLyt)->setStretchFactor(topLyt, 0);
    ((QBoxLayout*)_dialogLyt)->setStretchFactor(tableLyt, 1);

    connect(_listTab,	    SIGNAL(itemSelected(int)),	this, SLOT(sSelect()));
    connect(_buttonBox,	    SIGNAL(accepted()),		this, SLOT(sSelect()));
    connect(_buttonBox,	    SIGNAL(rejected()),		this, SLOT(sClose()));
    connect(_searchNumber,  SIGNAL(clicked()),	        this, SLOT(sFillList()));
    connect(_searchDescrip, SIGNAL(clicked()),  	this, SLOT(sFillList()));
    connect(_search,	    SIGNAL(lostFocus()),	this, SLOT(sFillList()));
    connect(_listTab,	    SIGNAL(valid(bool)),	_select, SLOT(setEnabled(bool)));

    _listTab->addColumn(tr("Number"), -1, Qt::AlignLeft, true, "number");

    if (pParent->inherits("VirtualClusterLineEdit"))
    {
      _parent = (VirtualClusterLineEdit*)(pParent);
      setWindowTitle(_parent->_titlePlural);
      if (_parent->_hasName)
      {
	_listTab->addColumn(tr("Name"),	 -1, Qt::AlignLeft, true, "name");
	_listTab->setColumnWidth(_listTab->columnCount() - 1, 100);
      }

      if (_parent->_hasDescription)
      {
	  _listTab->addColumn(tr("Description"),  -1, Qt::AlignLeft, true, "description" );
	  _listTab->setColumnWidth(_listTab->columnCount() - 1, 100);
      }

      _searchName->setHidden(! _parent->_hasName);
      _searchDescrip->setHidden(! _parent->_hasDescription);
      _id = _parent->_id;
      _titleLit->setText(_parent->_titlePlural);
    }
    else
    {
      _parent = 0;
      _id = -1;
    }
    shortcuts::setStandardKeys(this);
}

void VirtualSearch::sClose()
{
    done(_id);
}

void VirtualSearch::sSelect()
{
    done(_listTab->id());
}

void VirtualSearch::setQuery(QSqlQuery query)
{
  _listTab->setFocus();
  _listTab->populate(query);
}

void VirtualSearch::setSearchText(const QString& text)
{
  _search->setText(text);
}

void VirtualSearch::sFillList()
{
    if (! _parent)
      return;

    _listTab->clear();

    _search->setText(_search->text().trimmed());
    if (_search->text().length() == 0)
	return;

    QString search;
    if (_searchNumber->isChecked())
        search += QString("%1~*'%2'").arg(_parent->_numColName).arg(_search->text());
    if (_parent->_hasName &&
        (_searchName->isChecked()))
        search += (search.isEmpty() ?  QString("%1~*'%2'").arg(_parent->_nameColName).arg(_search->text()) :  
        " OR " +  QString("%1~'%2'").arg(_parent->_nameColName).arg(_search->text()));
    if (_parent->_hasDescription &&
        (_searchDescrip->isChecked()))
        search += (search.isEmpty() ?  QString("%1~*'%2'").arg(_parent->_descripColName).arg(_search->text()) :  
        " OR " +  QString("%1~*'%2'").arg(_parent->_descripColName).arg(_search->text()));
    if (!search.isEmpty())
    {
      search.prepend("(");
      search.append(")");
    }


    XSqlQuery qry(_parent->_query +
		    (search.isEmpty() ? "" :  " AND " + search) +
		    (_parent->_extraClause.isEmpty() ? "" :
					    " AND " + _parent->_extraClause) +
                    ((_parent->_hasActive && ! _parent->_showInactive) ? _parent->_activeClause : "") +
		    QString(" ORDER BY ") +
		    QString((_parent->_hasName) ? "name" : "number"));
                    
    _listTab->populate(qry);
}

///////////////////////////////////////////////////////////////////////////////

VirtualInfo::VirtualInfo(QWidget* pParent, Qt::WindowFlags pFlags) :
    QDialog(pParent, pFlags)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::WindowModal);
    _parent = (VirtualClusterLineEdit*)(pParent);
    setObjectName("virtualInfo");
    setWindowTitle(_parent->_titleSingular);
    _id = _parent->_id;
    
    _titleLit	= new QLabel(_parent->_titleSingular, this);
    _titleLit->setObjectName("_titleLit");
    _numberLit	= new QLabel(tr("Number:"), this);
    _numberLit->setObjectName("_numberLit");
    _number	= new QLabel(this);
    _number->setObjectName("_number");
    _nameLit	= new QLabel(tr("Name:"), this);
    _nameLit->setObjectName("_nameLit");
    _name	= new QLabel(this);
    _name->setObjectName("_name");
    _descripLit	= new QLabel(tr("Description:"), this);
    _descripLit->setObjectName("_descripLit");
    _descrip	= new QLabel(this);
    _descrip->setObjectName("_descrip");

    _titleLit->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    _numberLit->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    _nameLit->setAlignment(Qt::AlignVCenter | Qt::AlignRight);

    _close	= new QPushButton(tr("&Close"), this);
    _close->setObjectName("_close");
    _close->setDefault(true);

    QHBoxLayout* dialogLyt = new QHBoxLayout(this);
    dialogLyt->setContentsMargins(5, 5, 5, 5);

    QVBoxLayout* titleLyt  = new QVBoxLayout();
    dialogLyt->addLayout(titleLyt);
    titleLyt->addWidget(_titleLit);
    QHBoxLayout* dataLyt   = new QHBoxLayout();
    titleLyt->addLayout(dataLyt);
    QVBoxLayout* litLyt	   = new QVBoxLayout();
    dataLyt->addLayout(litLyt);
    litLyt->addWidget(_numberLit);
    litLyt->addWidget(_nameLit);
    litLyt->addWidget(_descripLit);
    QVBoxLayout* infoLyt   = new QVBoxLayout();
    dataLyt->addLayout(infoLyt);
    infoLyt->addWidget(_number);
    infoLyt->addWidget(_name);
    infoLyt->addWidget(_descrip);
    QSpacerItem* dataHtSpacer = new QSpacerItem(0, 0, QSizePolicy::Minimum,
						QSizePolicy::Expanding);
    titleLyt->addItem(dataHtSpacer);
    QHBoxLayout* buttonLyt = new QHBoxLayout();
    dialogLyt->addLayout(buttonLyt);
    QSpacerItem* wdSpacer  = new QSpacerItem(20, 20, QSizePolicy::Minimum,
					     QSizePolicy::Expanding);
    buttonLyt->addItem(wdSpacer);
    QVBoxLayout* buttonColLyt = new QVBoxLayout();
    buttonLyt->addLayout(buttonColLyt);

    buttonColLyt->addWidget(_close);
    QSpacerItem* htSpacer  = new QSpacerItem(20, 20, QSizePolicy::Minimum,
					     QSizePolicy::Expanding);
    buttonColLyt->addItem(htSpacer);

    connect(_close, SIGNAL(clicked()), this, SLOT(close()));

    _nameLit->setHidden(!_parent->_hasName);
    _name->setHidden(!_parent->_hasName);
    _descripLit->setHidden(!_parent->_hasDescription);
    _descrip->setHidden(!_parent->_hasDescription);

    sPopulate();
}

void VirtualInfo::sPopulate()
{
    XSqlQuery qry;
    qry.prepare(_parent->_query + _parent->_idClause + ";");
    qry.bindValue(":id", _parent->_id);
    qry.exec();
    if (qry.first())
    {
	_number->setText(qry.value("number").toString());
	if (_parent->_hasName)
	  _name->setText(qry.value("name").toString());
	if (_parent->_hasDescription)
	    _descrip->setText(qry.value("description").toString());
    }
    else if (qry.lastError().type() != QSqlError::NoError)
	QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
					  .arg(__FILE__)
					  .arg(__LINE__),
				  qry.lastError().databaseText());
}

