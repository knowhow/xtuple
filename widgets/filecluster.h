/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef FILECLUSTER_H
#define FILECLUSTER_H

#include "widgets.h"
#include "parameter.h"
#include "xtreewidget.h"

#include <QFileDialog>
#include <QLineEdit>
#include <QKeyEvent>
#include <QPushButton>
#include <QWidget>

class FileLineEdit : public QLineEdit
{
  Q_OBJECT

  public:
    FileLineEdit(QWidget * parent = 0);

  protected:
    virtual void keyPressEvent(QKeyEvent *event);

  signals:
    void requestList();
    void requestSearch();
};

/*
  FileCluster allows the user to enter a string or browse the filesystem.
  The text() of the FileCluster should name a file or directory. The ellipsis
  button instantiates a QFileDialog, so the class has methods to control that
  QFileDialog.
 */

class XTUPLEWIDGETS_EXPORT FileCluster : public QWidget
{
  Q_OBJECT

  Q_PROPERTY(QString		    caption READ caption     WRITE setCaption )
  Q_PROPERTY(bool	        listVisible READ listVisible WRITE setListVisible )
  Q_PROPERTY(QFileDialog::FileMode fileMode READ fileMode    WRITE setFileMode )
  Q_PROPERTY(bool	           readOnly READ isReadOnly  WRITE setReadOnly )
  Q_PROPERTY(QString		       text READ text	     WRITE setText )

  public:
    FileCluster(QWidget*, const char* = 0);
    ~FileCluster();

    virtual inline QString caption()	 const	{ return _caption; };
    virtual inline void    clear()		{ _file->clear(); };
    virtual inline QString dir()	 const	{ return _dir; };
    virtual inline QFileDialog::FileMode fileMode() const { return _fileMode; };
    virtual inline bool	   isReadOnly()	 const	{ return _file->isReadOnly(); };
    virtual inline bool	   listVisible() const	{ return _list->isVisible(); };
    virtual inline void    setListVisible(bool b) { _list->setVisible(b); };
    virtual inline void    setReadOnly(bool b)	  { _file->setReadOnly(b);
						    setListVisible(! b); };
    virtual inline QString text()	 const	  { return _file->text(); };

  public slots:
    virtual void sEllipses();
    virtual void setCaption(const QString &s) { _caption = s; } ;
    virtual void setDir(const QString &s)     { _dir = s; } ;
    virtual void setEnabled(const bool b)     { _file->setEnabled(b);
						_list->setEnabled(! isReadOnly()); } ;
    virtual void setFileMode(const QFileDialog::FileMode &m) { _fileMode = m; };
    virtual void setFilter(const QString &s)  { _filter = s; } ;
    virtual void setText(const QString &s)    { _file->setText(s); };

  signals:
    void editingFinished();
    void textChanged(const QString&);

  protected:
    QString			_caption;
    QString			_dir;
    FileLineEdit		*_file;
    QString			_filter;
    QPushButton			*_list;
    QFileDialog::FileMode	_fileMode;
};

#endif
