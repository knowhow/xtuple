/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "xwidget.h"

class QTreeWidgetItem;
class XTreeWidget;
class displayPrivate;
class ParameterWidget;

class display : public XWidget
{
    Q_OBJECT

    friend class displayPrivate;

public:
    display(QWidget* = 0, const char* = 0, Qt::WindowFlags = 0);
    ~display();

    Q_INVOKABLE virtual bool setParams(ParameterList &);

    Q_INVOKABLE void setReportName(const QString &);
    Q_INVOKABLE QString reportName() const;
    Q_INVOKABLE void setMetaSQLOptions(const QString &, const QString &);
    Q_INVOKABLE void setListLabel(const QString &);

    Q_INVOKABLE void setUseAltId(bool);
    Q_INVOKABLE bool useAltId() const;

    Q_INVOKABLE void setNewVisible(bool);
    Q_INVOKABLE bool newVisible() const;

    Q_INVOKABLE void setCloseVisible(bool);
    Q_INVOKABLE bool closeVisible() const;

    Q_INVOKABLE void setParameterWidgetVisible(bool);
    Q_INVOKABLE bool parameterWidgetVisible() const;

    Q_INVOKABLE void setSearchVisible(bool);
    Q_INVOKABLE bool searchVisible() const;

    Q_INVOKABLE void setQueryOnStartEnabled(bool);
    Q_INVOKABLE bool queryOnStartEnabled() const;

    Q_INVOKABLE void setAutoUpdateEnabled(bool);
    Q_INVOKABLE bool autoUpdateEnabled() const;

    Q_INVOKABLE XTreeWidget * list();
    Q_INVOKABLE ParameterWidget * parameterWidget();
    Q_INVOKABLE QWidget * optionsWidget();
    Q_INVOKABLE QToolBar * toolBar();
    Q_INVOKABLE QAction * newAction();
    Q_INVOKABLE QAction * closeAction();
    Q_INVOKABLE QAction * filterSeparator();
    Q_INVOKABLE QAction * printSeparator();
    Q_INVOKABLE QAction * printAction();
    Q_INVOKABLE QAction * previewAction();
    Q_INVOKABLE QAction * querySeparator();
    Q_INVOKABLE QAction * searchAction();
    Q_INVOKABLE QAction * queryAction();

    Q_INVOKABLE QString   searchText();

    Q_INVOKABLE void      setupCharacteristics(unsigned int uses);

public slots:
    virtual void sNew();
    virtual void sPrint();
    virtual void sPrint(ParameterList, bool = false);
    virtual void sPreview();
    virtual void sPreview(ParameterList, bool = false);
    virtual void sFillList();
    virtual void sFillList(ParameterList, bool = false);
    virtual void sPopulateMenu(QMenu *, QTreeWidgetItem *, int);

protected:
    Q_INVOKABLE ParameterList getParams();
    virtual void showEvent(QShowEvent*);

protected slots:
    virtual void languageChange();
    virtual void sAutoUpdateToggled();

signals:
    void fillList();
    void fillListBefore();
    void fillListAfter();

private:
    displayPrivate * _data;
};

#endif // __DISPLAY_H__
