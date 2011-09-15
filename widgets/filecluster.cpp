/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "filecluster.h"
#include <QHBoxLayout>

FileCluster::FileCluster(QWidget* pParent, const char* pName) :
    QWidget(pParent)
{
  setObjectName(pName);
  _caption = tr("Open File");
  _dir     = "";
  _fileMode = QFileDialog::AnyFile;
  _filter  = "";

  QHBoxLayout *hbox = new QHBoxLayout(this);
  hbox->setContentsMargins(0, 0, 0, 0); // prevent double margins

  _file = new FileLineEdit();

  _list = new QPushButton(tr("..."), this);
  _list->setObjectName("_list");
  _list->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
#ifndef Q_WS_MAC
  _list->setMaximumWidth(25);
#endif

  hbox->addWidget(_file);
  hbox->addWidget(_list);

  connect(_file, SIGNAL(editingFinished()),           this, SIGNAL(editingFinished()));
  connect(_file, SIGNAL(requestList()),               this, SLOT(sEllipses()));
  connect(_file, SIGNAL(requestSearch()),             this, SLOT(sEllipses()));
  connect(_file, SIGNAL(textChanged(const QString&)), this, SIGNAL(textChanged(const QString&)));
  connect(_list, SIGNAL(clicked()),                   this, SLOT(sEllipses()));
}

FileCluster::~FileCluster()
{
}

void FileCluster::sEllipses()
{
  QFileDialog newdlg(this, _caption, _dir);
  newdlg.setFileMode(_fileMode);
  if (! _filter.isEmpty())
  {
    QStringList filters;
    filters << _filter << tr("Any Files (*)");
    newdlg.setFilters(filters);
  }
  if (newdlg.exec())
  {
    _file->setText(newdlg.selectedFiles().join(", "));
    emit editingFinished();
  }
}

FileLineEdit::FileLineEdit(QWidget *parent)
  : QLineEdit(parent)
{
}

void FileLineEdit::keyPressEvent(QKeyEvent *event)
{
  bool handled = false;
  if (event->modifiers() & Qt::ControlModifier)
  {
    if (event->key() == Qt::Key_L)
    {
      emit (requestList());
      handled = true;
    }
    else if (event->key() == Qt::Key_S)
    {
      emit (requestSearch());
      handled = true;
    }
  }

  if (handled)
    event->accept();
  else
    QLineEdit::keyPressEvent(event);
}
