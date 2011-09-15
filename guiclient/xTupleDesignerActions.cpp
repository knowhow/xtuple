/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xTupleDesignerActions.h"

#include <QFile>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QtDesigner>

// TODO: can we live without this?
// copied from .../qt-mac-commercial-src-4.4.3/tools/designer/src/lib/shared/pluginmanager_p.h
#include "pluginmanager_p.h"

#include "xTupleDesigner.h"

#define DEBUG false

static QAction *separator(QObject *parent)
{
  QAction *sep = new QAction(parent);
  sep->setSeparator(true);
  return sep;
}

xTupleDesignerActions::xTupleDesignerActions(xTupleDesigner *parent)
  : QObject(parent)
{
  _designer = parent;

  _fileActions = new QActionGroup(parent);
  _fileActions->setObjectName("_fileActions");
  _fileActions->setExclusive(false);

  QAction *closeAct  = new QAction(tr("&Close"),   this);
  QAction *openAct   = new QAction(tr("&Open..."), this);
  QAction *revertAct = new QAction(tr("&Revert"),  this);
  QAction *saveAct   = new QAction(tr("&Save..."), this);

  closeAct->setShortcut(tr("CTRL+W"));
  openAct->setShortcut(tr("CTRL+O"));
  revertAct->setShortcut(tr("CTRL+R"));
  saveAct->setShortcut(tr("CTRL+S"));

  connect(closeAct,  SIGNAL(triggered()), this, SLOT(sClose()));
  connect(openAct,   SIGNAL(triggered()), this, SLOT(sOpen()));
  connect(revertAct, SIGNAL(triggered()), this, SLOT(sRevert()));
  connect(saveAct,   SIGNAL(triggered()), this, SLOT(sSave()));

  _fileActions->addAction(revertAct);
  _fileActions->addAction(separator(this));
  _fileActions->addAction(openAct);
  _fileActions->addAction(saveAct);
  _fileActions->addAction(separator(this));
  _fileActions->addAction(closeAct);

  _editActions = new QActionGroup(parent);
  _editActions->setObjectName("_editActions");
  _editActions->setExclusive(false);

  QDesignerFormWindowManagerInterface *formwm = parent->formeditor()->formWindowManager();

  //QAction *editBuddiesAct    = new QAction(tr("Edit Buddies"),   this);
  //QAction *editSignalSlotAct = new QAction(tr("Edit Signal/Slot Connections"), this);
  //QAction *editTabOrderAct   = new QAction(tr("Edit Tab Order"), this);
  //QAction *editWidgetsAct    = new QAction(tr("Edit Widgets"),   this);
  QAction *redoAct           = formwm->actionRedo();
  QAction *undoAct           = formwm->actionUndo();


  undoAct->setShortcut(tr("CTRL+Z"));
  redoAct->setShortcut(tr("CTRL+SHIFT+Z"));

  _editActions->addAction(undoAct);
  _editActions->addAction(redoAct);
  _editActions->addAction(separator(this));
  _editActions->addAction(formwm->actionCut());
  _editActions->addAction(formwm->actionCopy());
  _editActions->addAction(formwm->actionPaste());
  _editActions->addAction(formwm->actionDelete());
  _editActions->addAction(formwm->actionSelectAll());
  _editActions->addAction(separator(this));
  _editActions->addAction(formwm->actionLower());
  _editActions->addAction(formwm->actionRaise());

  _toolActions = new QActionGroup(parent);
  _toolActions->setObjectName("_toolActions");
  _toolActions->setExclusive(true);

  QAction *m_editWidgetsAction = new QAction(tr("Edit Widgets"),   this);
  m_editWidgetsAction->setCheckable(true);
  QList<QKeySequence> shortcuts;
  shortcuts.append(QKeySequence(Qt::Key_F3));
#if QT_VERSION >= 0x040900 // "ESC" switching to edit mode: Activate once item delegates handle shortcut overrides for ESC.
  shortcuts.append(QKeySequence(Qt::Key_Escape));
#endif
  m_editWidgetsAction->setShortcuts(shortcuts);
  m_editWidgetsAction->setIcon(QIcon(_designer->formeditor()->resourceLocation() + QLatin1String("/widgettool.png")));
  connect(m_editWidgetsAction, SIGNAL(triggered()), this, SLOT(sEditWidgets()));
  m_editWidgetsAction->setChecked(true);
  m_editWidgetsAction->setEnabled(false);
  _toolActions->addAction(m_editWidgetsAction);

  QList<QObject*> builtinPlugins = QPluginLoader::staticInstances();
  builtinPlugins += _designer->formeditor()->pluginManager()->instances();
  foreach (QObject *plugin, builtinPlugins)
  {
    if (QDesignerFormEditorPluginInterface *formEditorPlugin = qobject_cast<QDesignerFormEditorPluginInterface*>(plugin))
    {
      if (QAction *action = formEditorPlugin->action())
      {
        _toolActions->addAction(action);
        action->setCheckable(true);
      }
    }
  }

  _formActions = new QActionGroup(parent);
  _formActions->setObjectName("_formActions");
  _formActions->setExclusive(false);

  _formActions->addAction(formwm->actionHorizontalLayout());
  _formActions->addAction(formwm->actionVerticalLayout());
  _formActions->addAction(formwm->actionSplitHorizontal());
  _formActions->addAction(formwm->actionSplitVertical());
  _formActions->addAction(formwm->actionGridLayout());
  _formActions->addAction(formwm->actionFormLayout());
  _formActions->addAction(formwm->actionBreakLayout());
  _formActions->addAction(formwm->actionAdjustSize());
  _formActions->addAction(formwm->actionSimplifyLayout());

  connect(formwm, SIGNAL(activeFormWindowChanged(QDesignerFormWindowInterface*)),
          this,   SLOT(sActiveFormWindowChanged(QDesignerFormWindowInterface*)));
  sActiveFormWindowChanged(formwm->activeFormWindow());
}

void xTupleDesignerActions::sActiveFormWindowChanged(QDesignerFormWindowInterface *pwindow)
{
  if (DEBUG)
    qDebug("xTupleDesignerActions::sActiveFormWindowChanged(%p)",
           pwindow);

  QList<QActionGroup*> actionGroupList;
  actionGroupList.append(_fileActions);
  actionGroupList.append(_editActions);
  actionGroupList.append(_formActions);
  actionGroupList.append(_toolActions);

  for (int i = 0; i < actionGroupList.size(); i++)
    for (int j = 0; j < actionGroupList.at(i)->actions().size(); j++)
      actionGroupList.at(i)->actions().at(j)->setEnabled(pwindow != 0);
}

bool xTupleDesignerActions::sClose()
{
  if (! _designer->formwindow()->isDirty())
  {
    _designer->close();
    return true;
  }

  switch (QMessageBox::question(_designer, tr("Save changes?"),
                                tr("Do you want to save your changes before "
                                   "closing the window?"),
                          QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                          QMessageBox::Save))
  {
    case QMessageBox::Save:
      if (sSave())
        _designer->close();
      break;
    case QMessageBox::Discard:
      _designer->formwindow()->setDirty(false);
      _designer->close();
      break;
    case QMessageBox::Cancel:
    default:
      return false;
  }

  return true;
}

void xTupleDesignerActions::sOpen()
{
  QSettings settings("xTuple.com", "xTupleDesigner");
  QString path = settings.value("LastDirectory").toString();
  
  QString filename = QFileDialog::getOpenFileName(_designer, tr("Open File"),
                                                  path,
                                                  tr("UI (*.ui)"));
  if (! filename.isNull())
  {
    _designer->setSource(new QFile(filename), filename);
    _designer->formwindow()->setDirty(false);
  }
  
  QFileInfo fi(filename);
  settings.setValue("LastDirectory", fi.path());
}

void xTupleDesignerActions::sRevert()
{
  if (_designer->formwindow()->isDirty() &&
      QMessageBox::question(_designer, tr("Really revert?"),
                            tr("Are you sure you want to throw away "
                               "your changes since the last save?"),
                            QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
    _designer->sRevert();

  return;
}

bool xTupleDesignerActions::sSave()
{
  QMessageBox save;
  save.setText("How do you want to save your changes?");
  QPushButton *cancel= save.addButton(QMessageBox::Cancel);
  QPushButton *db    = save.addButton(tr("Database only"),    QMessageBox::AcceptRole);
  QPushButton *file  = save.addButton(tr("File only"),        QMessageBox::AcceptRole);
  QPushButton *both  = save.addButton(tr("Database and File"),QMessageBox::AcceptRole);

  save.setDefaultButton(_designer->formwindow()->fileName().isEmpty() ? db : both);
  save.setEscapeButton((QAbstractButton*)cancel);

  save.exec();
  if (save.clickedButton() == (QAbstractButton*)db)
    return sSaveToDB();
  else if (save.clickedButton() == (QAbstractButton*)file)
    return sSaveFile();
  else if (save.clickedButton() == (QAbstractButton*)both)
    return sSaveFile() && sSaveToDB();
  else if (save.clickedButton() == (QAbstractButton*)cancel)
    return false;
  else
  {
    qWarning("xTupleDesignerActions::sSave() bug - unknown button clicked");
    return false;
  }
}

bool xTupleDesignerActions::sSaveAs()
{
  bool returnValue = false;

  QString oldfilename = _designer->formwindow()->fileName();
  if (! oldfilename.isEmpty())
    _designer->formwindow()->setFileName(QString());

  returnValue = sSaveFile();

  if (! returnValue)
    _designer->formwindow()->setFileName(oldfilename);

  return returnValue;
}

bool xTupleDesignerActions::sSaveToDB()
{
  if (_designer->formId() == -1)
  {
    q.exec("SELECT NEXTVAL('uiform_uiform_id_seq') AS _uiform_id");
    if (q.first())
      _designer->setFormId(q.value("_uiform_id").toInt());
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(_designer, q.lastError().databaseText(), __FILE__, __LINE__);
      return false;
    }

    q.prepare( "INSERT INTO uiform "
               "(uiform_id, uiform_name, uiform_notes, uiform_order, uiform_enabled, uiform_source) "
               "VALUES "
               "(:uiform_id, :uiform_name, :uiform_notes, :uiform_order, :uiform_enabled, :uiform_source);" );

  }
  else
    q.prepare( "UPDATE uiform "
               "SET uiform_name=:uiform_name, uiform_notes=:uiform_notes,"
               "    uiform_order=:uiform_order, uiform_enabled=:uiform_enabled,"
               "    uiform_source=:uiform_source "
               "WHERE (uiform_id=:uiform_id);" );

  QString source = _designer->source();

  q.bindValue(":uiform_id",      _designer->formId());
  q.bindValue(":uiform_name",    _designer->name());
  q.bindValue(":uiform_order",   _designer->order());
  q.bindValue(":uiform_enabled", _designer->formEnabled());
  q.bindValue(":uiform_source",  source);
  q.bindValue(":uiform_notes",   _designer->notes());

  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(_designer, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  _designer->setSource(source); // otherwise the uiform window has the old source
  _designer->formwindow()->setDirty(false);
  return true;
}

bool xTupleDesignerActions::sSaveFile()
{
  QSettings settings("xTuple.com", "xTupleDesigner");
  QString path = settings.value("LastDirectory").toString();

  QString filename = _designer->formwindow()->fileName();
  if (_designer->formwindow()->fileName().isEmpty())
  {
    filename = QFileDialog::getSaveFileName(_designer, tr("Save File"),
                                            path + QDir::separator() + _designer->name() + ".ui",
                                            tr("UI (*.ui)"));
    if (filename.isNull())
      return false;
    _designer->formwindow()->setFileName(filename);
  }

  QFileInfo fi(filename);
  if (fi.suffix().isEmpty())
    filename += ".ui";

  QFile file(filename);
  if (!file.open(QIODevice::WriteOnly))
  {
    QMessageBox::critical(_designer,
                          tr("Could not export file"),
                          file.errorString());
    return false;
  }

  QString source = _designer->source();
  QTextStream ts(&file);
  ts.setCodec("UTF-8");
  ts << source;
  file.close();
  _designer->setSource(source);
  _designer->formwindow()->setDirty(false);

  settings.setValue("LastDirectory", fi.path());

  return true;
}

void xTupleDesignerActions::sEditBuddies()
{
  QMessageBox::information(_designer, tr("Not implemented yet."),
                           tr("Buddy editing is not implemented yet."));
}

void xTupleDesignerActions::sEditSignalSlot()
{
  QMessageBox::information(_designer, tr("Not implemented yet."),
                           tr("Visual Signal/Slot editing is not implemented yet."));
}

void xTupleDesignerActions::sEditTabOrder()
{
  QMessageBox::information(_designer, tr("Not implemented yet."),
                           tr("Tab Order editing is not implemented yet."));
}

void xTupleDesignerActions::sEditWidgets()
{
  _designer->formwindow()->editWidgets();
}
