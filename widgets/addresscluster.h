/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef addressCluster_h
#define addressCluster_h

#include "widgets.h"
#include "virtualCluster.h"
#include "xcheckbox.h"
#include "xcombobox.h"

#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

class QGridLayout;
class AddressCluster;

class XTUPLEWIDGETS_EXPORT AddressList : public VirtualList
{
    Q_OBJECT

    friend class AddressCluster;
    friend class AddressInfo;
    friend class AddressSearch;

    public:
	AddressList(QWidget*, const char* = 0, bool = false, Qt::WFlags = 0);

    public slots:
	virtual void sFillList();
	virtual void sSearch(const QString& pTarget);

    protected:
	AddressCluster*	_parent;
	QLabel*		_searchLit;
	QLineEdit*	_search;
	QLabel*		_titleLit;
	QPushButton*	_close;
	QPushButton*	_select;
	XTreeWidget*	_list;
        QCheckBox*      _searchAcct;
};

class XTUPLEWIDGETS_EXPORT AddressSearch : public VirtualSearch
{
    Q_OBJECT

    friend class AddressCluster;

    public:
	AddressSearch(QWidget*, Qt::WindowFlags = 0);

    public slots:
	virtual void sFillList();

    protected:
	AddressCluster*	_parent;
	XCheckBox*	_searchStreet;
	XCheckBox*	_searchCity;
	XCheckBox*	_searchState;
	XCheckBox*	_searchCountry;
	XCheckBox*	_searchPostalCode;
	XCheckBox*	_searchInactive;
        QCheckBox*      _searchAcct;

};

class XTUPLEWIDGETS_EXPORT AddressCluster : public VirtualCluster
{
    Q_OBJECT
    Q_ENUMS   (Mode)
    Q_PROPERTY(bool     activeVisible 	      READ activeVisible            WRITE setActiveVisible)
    Q_PROPERTY(QString  fieldNameAddressChange READ fieldNameAddrChange     WRITE setFieldNameAddrChange)
    Q_PROPERTY(QString  fieldNameNumber        READ fieldNameNumber         WRITE setFieldNameNumber)
    Q_PROPERTY(QString  fieldNameActive        READ fieldNameActive         WRITE setFieldNameActive)
    Q_PROPERTY(QString  fieldNameLine1         READ fieldNameLine1          WRITE setFieldNameLine1)
    Q_PROPERTY(QString  fieldNameLine2         READ fieldNameLine2	    WRITE setFieldNameLine2)
    Q_PROPERTY(QString  fieldNameLine3         READ fieldNameLine3          WRITE setFieldNameLine3)
    Q_PROPERTY(QString  fieldNameCity          READ fieldNameCity           WRITE setFieldNameCity)
    Q_PROPERTY(QString  fieldNameState         READ fieldNameState          WRITE setFieldNameState)
    Q_PROPERTY(QString  fieldNamePostalCode    READ fieldNamePostalCode     WRITE setFieldNamePostalCode)
    Q_PROPERTY(QString  fieldNameCountry       READ fieldNameCountry        WRITE setFieldNameCountry)
    Q_PROPERTY(Mode     mode                   READ mode                    WRITE setMode)

    friend class AddressInfo;
    friend class AddressList;
    friend class AddressSearch;

    public:
	enum SaveFlags { CHECK = 0, CHANGEONE = 1, CHANGEALL = 2 }; 
        enum Mode      { Edit, View, Select };

	AddressCluster(QWidget*, const char* = 0);
	
	inline virtual bool    activeVisible() const { return _active->isVisible(); };
        inline virtual QString addrChange()   const { return _addrChange->text(); };
	Q_INVOKABLE virtual QString city()    const { return _city->text(); }
	Q_INVOKABLE virtual QString country() const { return _country->currentText(); }
	Q_INVOKABLE virtual QString description() const { return ""; }
	Q_INVOKABLE virtual bool    isValid() const { return _valid; }
	inline virtual QString label()	      const { return _label->text(); };
	Q_INVOKABLE virtual QString line1()   const { return _addr1->text(); }
	Q_INVOKABLE virtual QString line2()   const { return _addr2->text(); }
	Q_INVOKABLE virtual QString line3()   const { return _addr3->text(); }
	Q_INVOKABLE virtual int     id()      const { return _id; }
        Q_INVOKABLE virtual Mode    mode()    const { return _mode; };
	inline virtual QString notes()	      const { return _notes; };
        Q_INVOKABLE virtual QString number()       const { return _number->text(); };
	Q_INVOKABLE virtual QString postalCode()  const { return _postalcode->text(); }
	Q_INVOKABLE virtual QString state()   const { return _state->currentText(); }
        Q_INVOKABLE virtual int searchAcctId() { return _searchAcctId; }
	
	// Return data map values
	virtual QString  fieldNameAddrChange()  const { return _fieldNameAddrChange; };
	virtual QString  fieldNameNumber()	const { return _fieldNameNumber; };
	virtual QString  fieldNameActive()	const { return _fieldNameActive; };
	virtual QString  fieldNameLine1() 	const { return _fieldNameLine1; };
	virtual QString  fieldNameLine2()  	const { return _fieldNameLine2; };
	virtual QString  fieldNameLine3()   	const { return _fieldNameLine3; };
	virtual QString  fieldNameCity()   	const { return _fieldNameCity; };
	virtual QString  fieldNamePostalCode()  const { return _fieldNamePostalCode; };
	virtual QString  fieldNameState()  	const { return _fieldNameState; };
	virtual QString  fieldNameCountry() 	const { return _fieldNameCountry; };

    public slots:
	inline virtual void clearExtraClause()	{ };
	inline virtual void setExtraClause(const QString&)  { };
	virtual void        setActiveVisible(const bool p);
        virtual void        setAddrChange(QString p);
	inline virtual void setCity(const QString& p)	{ _city->setText(p); };
	virtual void        setCountry(const QString& p);
	inline virtual void setDescription(const QString&) { };
	inline virtual void setLabel(const QString& p)  { _label->setText(p); _label->setHidden(_label->text().isEmpty()); };
	inline virtual void setLine1(const QString& p)	{ _addr1->setText(p); };
	inline virtual void setLine2(const QString& p)	{ _addr2->setText(p); };
	inline virtual void setLine3(const QString& p)	{ _addr3->setText(p); };
	       virtual void setMode(const Mode p);
	inline virtual void setNotes(const QString& p)  { _notes = p; };
	       virtual void setNumber(QString p);
               virtual void setNumber(const int)     {};
	inline virtual void setPostalCode(const QString& p) { _postalcode->setText(p); };
	       virtual void setState(const QString& p);
               virtual void setListVisible(bool p) { _list->setVisible(p); }
	virtual void	clear();
	virtual void	sEllipses();
	virtual void	sInfo();
	virtual void	sList();
	virtual void	sSearch();
	virtual void	setId(const int);
        virtual void	setSearchAcct(const int crmAcctId) { _searchAcctId = crmAcctId; }
        virtual int	save(enum SaveFlags);
        virtual void    check();
	virtual void    populateStateComboBox();

	// Set data map values      
        virtual void 	setDataWidgetMap(XDataWidgetMapper* m);
	virtual void  	setFieldNameAddrChange(QString p)   { _fieldNameAddrChange = p ; };
	virtual void  	setFieldNameNumber(QString p)       { _fieldNameNumber = p ; };
	virtual void  	setFieldNameActive(QString p)       { _fieldNameActive = p ; };
	virtual void  	setFieldNameLine1(QString p)        { _fieldNameLine1 = p ; };
	virtual void  	setFieldNameLine2(QString p)        { _fieldNameLine2 = p ; };
	virtual void  	setFieldNameLine3(QString p)        { _fieldNameLine3 = p ; };
	virtual void  	setFieldNameCity(QString p)         { _fieldNameCity = p ; };
	virtual void  	setFieldNamePostalCode(QString p)   { _fieldNamePostalCode = p ; };
	virtual void  	setFieldNameState(QString p)        { _fieldNameState = p ; };
	virtual void  	setFieldNameCountry(QString p)      { _fieldNameCountry = p ; };

    private slots:
        void emitAddressChanged();

    signals:
        void addressChanged(QString, QString, QString, QString, QString, QString, QString);
	void newId(int);
	void changed();

    protected:
	QString		_query;	
	QString		_extraClause;
        XLineEdit*      _addrChange;
	XLineEdit*      _number;
	QLabel*		_addrLit;
	XLineEdit*	_addr1;
	XLineEdit*	_addr2;
	XLineEdit*	_addr3;
	QLabel*		_cityLit;
	XLineEdit*	_city;
        Mode            _mode;
	QLabel*		_stateLit;
	XComboBox*	_state;
	QLabel*		_postalcodeLit;
	XLineEdit*	_postalcode;
	QLabel*		_countryLit;
	XComboBox*	_country;
	QCheckBox*	_active;

    private:
	virtual void	init();
	virtual void	populateCountryComboBox();
	virtual void	silentSetId(const int);
	int		_id;
	QString		_notes;
        int             _searchAcctId;
	bool		_selected;
	QString		_titlePlural;
	QString		_titleSingular;
	bool		_valid;
	XDataWidgetMapper* _mapper;
        QPushButton*    _list;

	// cached values
	QString         c_number;
	QString		c_addr1;
	QString		c_addr2;
	QString		c_addr3;
	QString		c_city;
	QString		c_state;
	QString		c_postalcode;
	QString		c_country;
	bool		c_active;
	QString		c_notes;

	// data map values
	QString  _fieldNameAddrChange;
	QString  _fieldNameNumber;
	QString  _fieldNameActive;
	QString  _fieldNameLine1;
	QString  _fieldNameLine2;
	QString  _fieldNameLine3;
	QString  _fieldNameCity;
	QString  _fieldNamePostalCode;
	QString  _fieldNameState;
	QString  _fieldNameCountry;
};

#endif
