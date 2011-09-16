/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef _crmaccttCluster_h
#define _crmaccttCluster_h

#include "crmcluster.h"
#include "xcheckbox.h"

class XComboBox;

class XTUPLEWIDGETS_EXPORT CRMAcctInfoAction
{
  public:
    virtual ~CRMAcctInfoAction() {};
    virtual void crmacctInformation(QWidget* parent, int pCustid) = 0;
};

class XTUPLEWIDGETS_EXPORT CRMAcctLineEdit : public CrmClusterLineEdit
{
    Q_OBJECT

    public:
	CRMAcctLineEdit(QWidget*, const char* = 0);

        enum CRMAcctSubtype { Crmacct, Competitor, Cust,     Employee,
                              Partner, Prospect,   SalesRep, Taxauth,
                              User,    Vend,       CustAndProspect };

	virtual void		setSubtype(const CRMAcctSubtype);
	virtual CRMAcctSubtype	subtype()	const;

    protected slots:
	VirtualList*	listFactory();
	VirtualSearch*	searchFactory();

    protected:
	CRMAcctSubtype _subtype;
};

class XTUPLEWIDGETS_EXPORT CRMAcctList : public VirtualList
{
    Q_OBJECT

    friend class CRMAcctCluster;
    friend class CRMAcctLineEdit;

    public:
	CRMAcctList(QWidget*, const char* = 0, bool = false, Qt::WFlags = 0);

    public slots:
	virtual void sFillList();
	virtual void setId(const int);
	virtual void setShowInactive(const bool);
	virtual void setSubtype(const CRMAcctLineEdit::CRMAcctSubtype);

    protected:
	QWidget* _parent;
	bool	 _showInactive;
	enum CRMAcctLineEdit::CRMAcctSubtype _subtype;
        ParameterList *_queryParams;
};

class XTUPLEWIDGETS_EXPORT CRMAcctSearch : public VirtualSearch
{
    Q_OBJECT

    friend class CRMAcctCluster;
    friend class CRMAcctLineEdit;

    public:
	CRMAcctSearch(QWidget*, Qt::WindowFlags = 0);
	virtual void setId(const int);
	virtual void setShowInactive(const bool);
	virtual void setSubtype(const CRMAcctLineEdit::CRMAcctSubtype);

    public slots:
	virtual void sFillList();

    protected:
	QLabel*		_addressLit;
	QWidget*	_parent;
	XCheckBox*	_searchContact;
	XCheckBox*	_searchPhone;
        XCheckBox*      _searchEmail;
	XCheckBox*	_searchStreet;
	XCheckBox*	_searchCity;
	XCheckBox*	_searchState;
	XCheckBox*	_searchPostalCode;
	XCheckBox*	_searchCountry;
	QCheckBox*	_showInactive;
        XCheckBox*      _searchCombo;
        XComboBox*      _comboCombo;

	enum CRMAcctLineEdit::CRMAcctSubtype _subtype;
        ParameterList *_queryParams;

    private:
};

class XTUPLEWIDGETS_EXPORT CRMAcctCluster : public VirtualCluster
{
    Q_OBJECT

    public:
	CRMAcctCluster(QWidget*, const char* = 0);
	virtual void				setSubtype(CRMAcctLineEdit::CRMAcctSubtype const);
	virtual CRMAcctLineEdit::CRMAcctSubtype subtype() const;

};

#endif
