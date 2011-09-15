/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "filemoveselector.h"
#include "shortcuts.h"

#include <QtScript>

#define DEBUG false

/** \class FileMoveSelector

    \brief The FileMoveSelector provides a standardized user interface for
           selecting what the application should do with a file in the
           host file system after it has been processed.

    The FileMoveSelector encodes a simple user interface paradigm for
    how the application should handle files after they have been processed.
    The application window that has a FileMoveSelector must store the user's
    selection. The application code that processes the files should check
    this stored selection and have its own means of handling files in the
    selected manner.

    \see configureIE
    \see ImportHelper
 */

FileMoveSelector::FileMoveSelector(QWidget* parent, Qt::WFlags fl)
  : QWidget(parent, fl)
{
  setupUi(this);

  setObjectName("fileMoveSelector");

  shortcuts::setStandardKeys(this);

  _status->append(Nothing,   tr("Do nothing"),             codeForOption(Nothing));
  _status->append(Suffix,    tr("Add a suffix"),           codeForOption(Suffix));
  _status->append(ChangeDir, tr("Move to a subdirectory"), codeForOption(ChangeDir));
  _status->append(Delete,    tr("Delete"),                 codeForOption(Delete));
}

FileMoveSelector::~FileMoveSelector()
{
  // no need to delete child widgets, Qt does it all for us
}

void FileMoveSelector::languageChange()
{
  retranslateUi(this);
}

/** \brief Return the text code value of the currently-selected return option.

    \see codeForOption(FileMoveOption) for possible return values
  */
QString FileMoveSelector::code() const
{
  return _status->code();
}

/** \brief Translate a FileMoveOption enumerated value to a text code value.

  The text code values are not translatable. They are intended to be used
  for storing the user's selection in a database or configuration file, and
  for applications to read from the database or configuration file
  when deciding how to handle a file after it has been processed.

  \return The text code value corresponding to the FileMoveOption enumerated
          value.
          Possible values are "None", "Rename", "Move", and "Delete",
          or QString::null if poption is invalid or not yet supported.
  */
QString FileMoveSelector::codeForOption(FileMoveOption poption) const
{
  QString returnValue = QString::null;
  switch (poption)
  {
    case Nothing:       returnValue = "None";   break;
    case Suffix:        returnValue = "Rename"; break;
    case ChangeDir:     returnValue = "Move";   break;
    case Delete:        returnValue = "Delete"; break;
  }

  if (DEBUG)
    qDebug("%s::codeForOption(%d) returning '%s'",
           qPrintable(objectName()), poption, qPrintable(returnValue));
  return returnValue;
}

/** \brief The destination directory for files which are to be moved.

  This value reflects the current contents of the destination directory
  field. The destination directory may not be visible, depending on the
  currently selected option.

  \returns The text entered in the destination directory field of the
           FileMoveSelector. This value should not be used to determine
           how the user would like files to be handled. Use code() or
           option() for this purpose.

  */
QString FileMoveSelector::destdir() const
{
  return _movedir->text();
}

/** \brief The enumerated FileMoveOption value of the current selection.

  \returns The currently selected option, or -1 if there is an error.
  */
FileMoveSelector::FileMoveOption FileMoveSelector::option() const
{
  return (FileMoveOption)_status->id();
}

/** \brief The suffix that should be appended to files that have been processed.

  This value reflects the current contents of the suffix field. This field
  may not be visible, depending on the currently selected option.

  \returns The text entered in the suffix field of the FileMoveSelector.
           This value should not be used to determine how the user would
           like files to be handled. Use code() or option() for this purpose.
  */
QString FileMoveSelector::suffix() const
{
  return _suffix->text();
}

/** \brief Programatically set the selection using a text code value.

    \return true if successful or false if the pcode does not exactly match
            a known value.
  */
bool FileMoveSelector::setCode(QString pcode)
{
  if (DEBUG)
    qDebug("%s::setCode(%s) entered",
           qPrintable(objectName()), qPrintable(pcode));
  _status->setCode(pcode);
  if (_status->id() < 0)
    _status->setItemText(0, _status->nullStr());

  if (DEBUG)
    qDebug("%s::setCode() returning %d with id %d",
           qPrintable(objectName()), _status->id() >= 0, _status->id());
  return (_status->id() >= 0);
}

/** \brief Programatically set the destination directory.

    This does not automatically set the selection to ChangeDir.

    \return true
  */
bool FileMoveSelector::setDestdir(QString pdir)
{
  _movedir->setText(pdir);
  return true;
}

/** \brief Programatically set the selection using one of the
           FileMoveOption enumerated values.

    \return true if successful or false if poption is not valid.
  */
bool FileMoveSelector::setOption(FileMoveOption poption)
{
  _status->setId(poption);
  return (_status->id() >= 0);
}

/** \brief Programatically set the suffix to append.

    This does not automatically set the selection to Suffix.

    \return true
  */
bool FileMoveSelector::setSuffix(QString psuffix)
{
  _suffix->setText(psuffix);
  return true;
}

/* script exposure *********************************************************/

QScriptValue constructFileMoveSelector(QScriptContext *context,
                                       QScriptEngine  *engine)
{
  FileMoveSelector *obj = 0;

  if (context->argumentCount() == 0)
    obj = new FileMoveSelector();
  else if (context->argumentCount() == 1 &&
           qscriptvalue_cast<QWidget*>(context->argument(0)))
    obj = new FileMoveSelector(qscriptvalue_cast<QWidget*>(context->argument(0)));
  else if (context->argumentCount() > 1 &&
           qscriptvalue_cast<QWidget*>(context->argument(0))
           && context->argument(1).isNumber())
    obj = new FileMoveSelector(qscriptvalue_cast<QWidget*>(context->argument(0)),
                               (Qt::WFlags)context->argument(1).toInteger());
  else
     context->throwError(QScriptContext::UnknownError,
                         "Could not find an appropriate FileMoveSelector constructor");

  return engine->toScriptValue(obj);
}

QScriptValue FileMoveSelectorToScriptValue(QScriptEngine *engine, FileMoveSelector *const &item)
{
  return engine->newQObject(item);
}

void FileMoveSelectorFromScriptValue(const QScriptValue &obj, FileMoveSelector * &item)
{
  item = qobject_cast<FileMoveSelector*>(obj.toQObject());
}

void setupFileMoveSelector(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, FileMoveSelectorToScriptValue, FileMoveSelectorFromScriptValue);

  QScriptValue constructor = engine->newFunction(constructFileMoveSelector);
  engine->globalObject().setProperty("FileMoveSelector", constructor,
                                     QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
