/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef virtCluster_h
#define virtCluster_h

#include "widgets.h"
#include "guiclientinterface.h"
#include "parameter.h"
#include "xlineedit.h"
#include "xtextedit.h"
#include "xtreewidget.h"
#include "xcheckbox.h"
#include "xdatawidgetmapper.h"

#include <QSqlQueryModel>
#include <QAction>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QCompleter>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QSqlQueryModel>
#include <QVBoxLayout>
#include <QWidget>

class QGridLayout;
class VirtualClusterLineEdit;

#define ID              1
#define NUMBER          2
#define DESCRIPTION     3
#define ACTIVE          4

class XTUPLEWIDGETS_EXPORT VirtualList : public QDialog
{
    Q_OBJECT

    friend class VirtualClusterLineEdit;

    public:
        VirtualList();
        VirtualList(QWidget*, Qt::WindowFlags = 0);

    public slots:
        virtual void sClose();
        virtual void sFillList();
        virtual void sSearch(const QString&);
        virtual void sSelect();

    protected:
        virtual void init();
        void showEvent(QShowEvent *);

        VirtualClusterLineEdit* _parent;
        QVBoxLayout* _dialogLyt;
        QLabel*      _searchLit;
        QLineEdit*   _search;
        QLabel*      _titleLit;
        QDialogButtonBox*  _buttonBox;
        QPushButton* _select;
        XTreeWidget* _listTab;
        int          _id;
};

class XTUPLEWIDGETS_EXPORT VirtualSearch : public QDialog
{
    Q_OBJECT

    friend class VirtualClusterLineEdit;

    public:
        VirtualSearch(QWidget*, Qt::WindowFlags = 0);

        Q_INVOKABLE virtual void setQuery(QSqlQuery query);
        Q_INVOKABLE virtual void setSearchText(const QString& text);

    public slots:
        virtual void sClose();
        virtual void sFillList();
        virtual void sSelect();

    protected:
        VirtualClusterLineEdit* _parent;
        QVBoxLayout* _dialogLyt;
        int  _id;
        QLabel*      _searchLit;
        QLineEdit*   _search;
        XCheckBox*   _searchNumber;
        XCheckBox*   _searchName;
        XCheckBox*   _searchDescrip;
        QLabel*      _titleLit;
        QDialogButtonBox*  _buttonBox;
        QPushButton* _select;
        XTreeWidget* _listTab;
        QLayout*     dialogLyt;
        QLayout*     searchLyt;
        QLayout*     searchStrLyt;
        QGridLayout* selectorsLyt;
        QLayout*     tableLyt;
        QLayout*     buttonsLyt;
};

class XTUPLEWIDGETS_EXPORT VirtualInfo : public QDialog
{
    Q_OBJECT

    friend class VirtualClusterLineEdit;

    public:
        VirtualInfo(QWidget*,  Qt::WindowFlags = 0);

    public slots:
        virtual void sPopulate();

    protected:
        VirtualClusterLineEdit* _parent;
        QLabel*         _titleLit;
        QLabel*         _numberLit;
        QLabel*         _number;
        QLabel*         _nameLit;
        QLabel*         _name;
        QLabel*         _descripLit;
        QLabel*         _descrip;
        QPushButton*    _close;

    private:
        int _id;
};

/*
    VirtualClusterLineEdit is an abstract class that encapsulates
    the basics of retrieving an ID given a NUMBER or a NUMBER given
    an ID.  This class and its subclasses are intended to be used
    by VirtualCluster objects, not directly by an application.

    Users of this class need to supply the names of the table, its
    id column, and its number column (numbers are user-readable
    strings such as customer abbreviations or purchase order numbers).

    Subclasses must supply implementations for the sInfo(), sList(),
    and sSearch() slots.  sInfo() is a distinct dialog that shows
    extended data from the record with table_id = id(). sList() and
    sSearch() are dialogs that let the user select a number/id pair
    value for the widget.
*/

class XTUPLEWIDGETS_EXPORT VirtualClusterLineEdit : public XLineEdit
{
    Q_OBJECT
    
    friend class VirtualCluster;
    friend class VirtualInfo;
    friend class VirtualList;
    friend class VirtualSearch;

    public:
        VirtualClusterLineEdit(QWidget*, const char*, const char*, const char*,
                               const char*, const char*, const char*,
                               const char* = 0, const char* = 0);

       void setMenu(QMenu *menu);
       QMenu *menu() const { return _menu; }

       static GuiClientInterface *_guiClientInterface;

       Q_INVOKABLE        virtual bool canOpen();
       Q_INVOKABLE inline virtual QString uiName()         const { return _uiName; }
       Q_INVOKABLE inline virtual QString editPriv()       const { return _editPriv; }
       Q_INVOKABLE inline virtual QString viewPriv()       const { return _viewPriv; }

       Q_INVOKABLE inline virtual QSqlQueryModel* model() const { return _model;}

       Q_INVOKABLE inline virtual QString name()        const { return _name; }
       Q_INVOKABLE inline virtual QString description() const { return _description; }

    public slots:
        virtual void clear();
        virtual QString extraClause() const { return _extraClause; }
        virtual void sEllipses();
        virtual void sInfo();
        virtual void sList();
        virtual void sOpen();
        virtual void sNew();
        virtual void sSearch();
        virtual void setId(const int);
        virtual void setNumber(const QString&);
        virtual void setShowInactive(const bool);

        virtual void setUiName(const QString& name);
        virtual void setEditPriv(const QString& priv);
        virtual void setNewPriv(const QString& priv);
        virtual void setViewPriv(const QString& priv);

    protected slots:
        inline virtual void clearExtraClause() { _extraClause = ""; }

        virtual VirtualList*    listFactory();
        virtual VirtualSearch*  searchFactory();
        virtual VirtualInfo*    infoFactory();

        virtual void setStrikeOut(bool enable = false);
        virtual void sHandleCompleter();
        virtual void sHandleNullStr();
        virtual void sParse();
        virtual void sUpdateMenu();
        virtual QWidget* sOpenWindow(const QString& uiName, ParameterList &params);

        virtual void setTitles(const QString&, const QString& = 0);
        inline virtual void setExtraClause(const QString& pExt) { _extraClause = pExt; }
        virtual void setTableAndColumnNames(const char* pTabName,
                                            const char* pIdColumn,
                                            const char* pNumberColumn,
                                            const char* pNameColumn,
                                            const char* pDescripColumn,
                                            const char* pActiveColumn);

        void setStrict(bool);
        bool isStrict() const { return _strict; }

    signals:
        void newId(int);
        void parsed();
        void valid(bool);

    protected:
        virtual bool eventFilter(QObject *obj, QEvent* event);
        virtual void focusInEvent(QFocusEvent * event);
        virtual void resizeEvent(QResizeEvent *e);

        QAction* _infoAct;
        QAction* _openAct;
        QAction* _newAct;
        QCompleter* _completer;
        QLabel* _menuLabel;
        QMenu* _menu;
        QString _titleSingular;
        QString _titlePlural;
        QString _query;
        QString _activeClause;
        QString _idClause;
        QString _numClause;
        QString _extraClause;
        QString _name;
        QString _description;
        QString _idColName;
        QString _numColName;
        QString _nameColName;
        QString _descripColName;
        QString _activeColName;
        QString _uiName;
        QString _editPriv;
        QString _newPriv;
        QString _viewPriv;
        bool _hasDescription;
        bool _hasName;
        bool _hasActive;
        bool _strict;
        bool _showInactive;

        virtual void silentSetId(const int);

    private:
        void positionMenuLabel();

        QString _cText;

        QSqlQueryModel* _model;
};

/*

    VirtualCluster is a widget that contains a VirtualClusterLineEdit
    and surrounds it with a label and two buttons.  One button lets
    the user invoke VirtualClusterLineEdit's sInfo slot while another
    button invokes either sSearch() or sList(), depending on the
    user's preferences.
    
    VirtualCluster provides a copy of much of the VirtualClusterLineEdit
    API to hide from the caller the fact that it contains one of these
    objects.  It also lets the calling code set it as read-only.

*/

class XTUPLEWIDGETS_EXPORT VirtualCluster : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QString label          READ label          WRITE setLabel)
    Q_PROPERTY(bool    nameVisible    READ nameVisible    WRITE setNameVisible)
    Q_PROPERTY(bool    descriptionVisible    READ descriptionVisible    WRITE setDescriptionVisible)
    Q_PROPERTY(bool    readOnly       READ readOnly       WRITE setReadOnly)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
    Q_PROPERTY(QString fieldName      READ fieldName      WRITE setFieldName)
    Q_PROPERTY(QString number         READ number         WRITE setNumber         DESIGNABLE false)
    Q_PROPERTY(QString defaultNumber  READ defaultNumber  WRITE setDefaultNumber  DESIGNABLE false)
    Q_PROPERTY(QString nullStr        READ nullStr        WRITE setNullStr)

    friend class VirtualClusterLineEdit;

    public:
        VirtualCluster(QWidget*, const char* = 0);
        VirtualCluster(QWidget*, VirtualClusterLineEdit* = 0, const char* = 0);

        Q_INVOKABLE inline virtual int     id()             const { return _number->id(); }
                    inline virtual QString label()          const { return _label->text(); }
                    inline virtual QString number()         const { return _number->text(); }
        Q_INVOKABLE inline virtual QString description()    const { return _description->text(); }
        Q_INVOKABLE inline virtual bool    isValid()        const { return _number->isValid(); }
        Q_INVOKABLE        virtual QString name()           const { return _name->text(); }
        Q_INVOKABLE inline virtual bool    isStrict()       const { return _number->isStrict(); }
                    inline virtual bool    readOnly()       const { return _readOnly; }
                           virtual QString defaultNumber()  const { return _default; }
                    inline virtual QString fieldName()      const { return _fieldName; }
        Q_INVOKABLE inline virtual QString extraClause()    const { return _number->extraClause(); }

        virtual Qt::Orientation orientation();
        virtual void setOrientation(Qt::Orientation orientation);

        virtual bool   nameVisible() const;
        virtual void   setNameVisible(const bool p);

        virtual bool   descriptionVisible() const;
        virtual void   setDescriptionVisible(const bool p);

        virtual QString nullStr() const;
        virtual void setNullStr(const QString &text);

    public slots:
        // most of the heavy lifting is done by VirtualClusterLineEdit _number
        virtual void clearExtraClause()                { _number->clearExtraClause(); }
        virtual void setDefaultNumber(const QString& p){ _default=p;}
        virtual void setDescription(const QString& p)  { _description->setText(p); }
        virtual void setExtraClause(const QString& p)  { _number->setExtraClause(p); }
        virtual void setFieldName(QString p)           { _fieldName = p; }
        virtual void setId(const int p)                { _number->setId(p); }
        virtual void setName(const QString& p)         { _name->setText(p); }
        virtual void setNumber(const int p)            { _number->setNumber(QString::number(p)); }
        virtual void setNumber(const QString& p)       { _number->setNumber(p); }

        virtual void clear();
        virtual void setDataWidgetMap(XDataWidgetMapper* m);
        virtual void setEnabled(const bool p);
        virtual void setLabel(const QString& p);
        virtual void setStrict(const bool b);
        virtual void setReadOnly(const bool b);
        virtual void setShowInactive(const bool);
        virtual void sRefresh();
        virtual void updateMapperData();

        virtual void sEllipses();
        virtual void sInfo();
        virtual void sList();
        virtual void sSearch();

    signals:
        void newId(int);
        void valid(bool);

    protected:
        virtual void addNumberWidget(VirtualClusterLineEdit* pNumberWidget);

        QGridLayout*            _grid;
        QLabel*                 _label;
        VirtualClusterLineEdit* _number;
        QLabel*                 _description;
        QLabel*                 _name;
        QSpacerItem*            _hspcr;
        bool                    _readOnly;
	QString                 _fieldName;
        QString                 _default;
        XDataWidgetMapper       *_mapper;
        Qt::Orientation         _orientation;

    private:
        virtual void init();
};

#endif
