/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef contactWidget_h
#define contactWidget_h

#include "widgets.h"
#include "virtualCluster.h"
#include "xcombobox.h"
#include "addresscluster.h"
#include "crmacctcluster.h"
#include "usernamecluster.h"
#include "xcheckbox.h"

#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

#include <parameter.h>

class QGridLayout;
class ContactWidget;

class XTUPLEWIDGETS_EXPORT ContactList : public VirtualList
{
  Q_OBJECT

  friend class ContactWidget;
  friend class ContactInfo;
  friend class ContactSearch;

public:
  ContactList(QWidget*, const char* = 0, bool = false, Qt::WFlags = 0);
  virtual void set( ParameterList & pParams );

public slots:
  virtual void sFillList(const bool searchAcct);
  virtual void sSearch(const QString&);

protected:
  QCheckBox*      _searchAcct;

private:
  int _id;
  int _searchAcctId;
  QString _query;
  QString _extraClause;
};

class XTUPLEWIDGETS_EXPORT ContactSearch : public VirtualSearch
{
  Q_OBJECT

  friend class ContactWidget;

public:
  ContactSearch(QWidget*, Qt::WindowFlags = 0);
  virtual void set( ParameterList & pParams );

public slots:
  virtual void sFillList();

protected:
  XCheckBox* _searchFirst;
  XCheckBox* _searchLast;
  XCheckBox* _searchTitle;
  XCheckBox* _searchCRMAcct;
  XCheckBox* _searchPhones;
  XCheckBox* _searchEmail;
  XCheckBox* _searchWebAddr;
  XCheckBox* _searchInactive;
  QCheckBox* _searchAcct;
private:
  int _searchAcctId;
  QString _query;
  QString _extraClause;
};

class XTUPLEWIDGETS_EXPORT ContactWidget : public VirtualCluster
{
  Q_OBJECT
  Q_ENUMS   (Mode);
  Q_PROPERTY(bool     accountVisible        READ accountVisible       	WRITE setAccountVisible);
  Q_PROPERTY(bool     ownerVisible          READ ownerVisible           WRITE setOwnerVisible);
  Q_PROPERTY(bool     ownerEnabled          READ ownerEnabled           WRITE setOwnerEnabled);
  Q_PROPERTY(bool     activeVisible         READ activeVisible        	WRITE setActiveVisible);
  Q_PROPERTY(bool     addressVisible        READ addressVisible       	WRITE setAddressVisible);
  Q_PROPERTY(bool     emailVisible          READ emailVisible         	WRITE setEmailVisible);
  Q_PROPERTY(bool     initialsVisible       READ initialsVisible      	WRITE setInitialsVisible);
  Q_PROPERTY(bool     minimalLayout         READ minimalLayout        	WRITE setMinimalLayout);
  Q_PROPERTY(bool     phonesVisible         READ phonesVisible        	WRITE setPhonesVisible);
  Q_PROPERTY(bool     webaddrVisible        READ webaddrVisible       	WRITE setWebaddrVisible);
  Q_PROPERTY(QString  fieldNameChange       READ fieldNameChange      	WRITE setFieldNameChange);
  Q_PROPERTY(QString  fieldNameNumber       READ fieldNameNumber      	WRITE setFieldNameNumber);
  Q_PROPERTY(QString  fieldNameActive       READ fieldNameActive      	WRITE setFieldNameActive);
  Q_PROPERTY(QString  fieldNameCrmAccount   READ fieldNameCrmAccount    WRITE setFieldNameCrmAccount);
  Q_PROPERTY(QString  fieldNameHonorific    READ fieldNameHonorific   	WRITE setFieldNameHonorific);
  Q_PROPERTY(QString  fieldNameFirst        READ fieldNameFirst       	WRITE setFieldNameFirst);
  Q_PROPERTY(QString  fieldNameMiddle       READ fieldNameMiddle       	WRITE setFieldNameMiddle);
  Q_PROPERTY(QString  fieldNameLast         READ fieldNameLast        	WRITE setFieldNameLast);
  Q_PROPERTY(QString  fieldNameSuffix       READ fieldNameSuffix       	WRITE setFieldNameSuffix);
  Q_PROPERTY(QString  fieldNameInitials     READ fieldNameInitials    	WRITE setFieldNameInitials);
  Q_PROPERTY(QString  fieldNameTitle        READ fieldNameTitle       	WRITE setFieldNameTitle);
  Q_PROPERTY(QString  fieldNamePhone        READ fieldNamePhone       	WRITE setFieldNamePhone);
  Q_PROPERTY(QString  fieldNamePhone2       READ fieldNamePhone2      	WRITE setFieldNamePhone2);
  Q_PROPERTY(QString  fieldNameFax          READ fieldNameFax         	WRITE setFieldNameFax);
  Q_PROPERTY(QString  fieldNameEmailAddress READ fieldNameEmailAddress 	WRITE setFieldNameEmailAddress);
  Q_PROPERTY(QString  fieldNameWebAddress   READ fieldNameWebAddress  	WRITE setFieldNameWebAddress);
  Q_PROPERTY(QString  fieldNameAddressChange READ fieldNameAddrChange   WRITE setFieldNameAddrChange);
  Q_PROPERTY(QString  fieldNameAddressNumber READ fieldNameAddrNumber   WRITE setFieldNameAddrNumber);
  Q_PROPERTY(QString  fieldNameLine1        READ fieldNameLine1         WRITE setFieldNameLine1);
  Q_PROPERTY(QString  fieldNameLine2        READ fieldNameLine2	    	  WRITE setFieldNameLine2);
  Q_PROPERTY(QString  fieldNameLine3        READ fieldNameLine3         WRITE setFieldNameLine3);
  Q_PROPERTY(QString  fieldNameCity         READ fieldNameCity          WRITE setFieldNameCity);
  Q_PROPERTY(QString  fieldNameState        READ fieldNameState         WRITE setFieldNameState);
  Q_PROPERTY(QString  fieldNamePostalCode   READ fieldNamePostalCode    WRITE setFieldNamePostalCode);
  Q_PROPERTY(QString  fieldNameCountry      READ fieldNameCountry       WRITE setFieldNameCountry);
  Q_PROPERTY(Mode     mode                  READ mode                   WRITE setMode);
  Q_PROPERTY(QString	number                READ number                 WRITE setNumber               DESIGNABLE false);
  Q_PROPERTY(QString  defaultText           READ defaultText                                          DESIGNABLE false);

  friend class ContactInfo;
  friend class ContactList;
  friend class ContactSearch;

public:
  // AccountLimits may be ORed together
  enum AccountLimits { Employee = 1,	Customer =  2,	Vendor     = 4,
                       Partner  = 8,	Prospect = 16,	Competitor = 32};
  enum Mode          { Edit, View, Select };

  ContactWidget(QWidget*, const char* = 0);
  inline virtual AddressCluster* addressWidget() const { return _address; }
  inline virtual bool    ownerVisible()   const { return _owner->isVisible(); }
  inline virtual bool    ownerEnabled()   const { return _owner->isEnabled(); }
  inline virtual bool    numberVisible()  const { return _number->isVisible(); }
  inline virtual bool    activeVisible()  const { return _active->isVisible(); }
  inline virtual bool    accountVisible() const { return _crmAcct->isVisible(); }
  inline virtual bool    addressVisible() const { return _address->isVisible(); }
  inline virtual int     addressId()	  const { return _address->id(); }
  inline virtual QString change()         const { return _change->text(); }
  inline virtual int     crmAcctId()	  const { return _crmAcct->id(); }
  inline virtual int     ownerId()        const { return _owner->id(); }
  inline virtual QString description()    const { return ""; }
  inline virtual bool    emailVisible()   const { return _email->isVisible(); }
  inline virtual QString defaultText()    const { return QString(); }
  inline virtual bool    initialsVisible()const { return _initials->isVisible(); }
  inline virtual bool    isValid()        const { return _valid; }
  inline virtual QString label()	  const { return _label->text(); }
  inline virtual bool    minimalLayout()  const { return _minimalLayout; }
  Q_INVOKABLE virtual Mode    mode()      const { return _mode; }
  inline virtual QString notes()	  const { return _notes; }
  inline virtual bool    phonesVisible()  const { return _phone->isVisible(); }
  inline virtual int     searchAcct()	  const { return _searchAcctId; }
  inline virtual bool    webaddrVisible() const { return _webaddr->isVisible(); }

  Q_INVOKABLE virtual bool    active()		const { return _active->isChecked(); }
  Q_INVOKABLE virtual int     id()	        const { return _id; }
  Q_INVOKABLE virtual QString emailAddress()	const { return _email->text(); }
  Q_INVOKABLE virtual QString fax()		const { return _fax->text(); }
  Q_INVOKABLE virtual QString first()		const { return _first->text(); }
  Q_INVOKABLE virtual QString honorific()       const { return _honorific->currentText(); }
  Q_INVOKABLE virtual QString initials()        const { return _initials->text(); }
  Q_INVOKABLE virtual QString last()		const { return _last->text(); }
  Q_INVOKABLE virtual QString name()            const;
  Q_INVOKABLE virtual QString number()          const { return _number->text(); }
  Q_INVOKABLE virtual QString ownerUsername()   const { return _owner->username(); }
  Q_INVOKABLE virtual QString phone()		const { return _phone->text(); }
  Q_INVOKABLE virtual QString phone2()		const { return _phone2->text(); }
  Q_INVOKABLE virtual QString title()           const { return _title->text(); }
  Q_INVOKABLE virtual QString webAddress()	const { return _webaddr->text(); }
  Q_INVOKABLE virtual QString suffix()		const { return _suffix->text(); }
  Q_INVOKABLE virtual QString middle()		const { return _middle->text(); }
  Q_INVOKABLE virtual QString address1()        const { return _address->line1(); }
  Q_INVOKABLE virtual QString address2()        const { return _address->line2(); }
  Q_INVOKABLE virtual QString address3()        const { return _address->line3(); }
  Q_INVOKABLE virtual QString city()            const { return _address->city(); }
  Q_INVOKABLE virtual QString postalCode()      const { return _address->postalCode(); }
  Q_INVOKABLE virtual QString state()           const { return _address->state(); }
  Q_INVOKABLE virtual QString country()         const { return _address->country(); }
  Q_INVOKABLE virtual QString owner()           const { return _owner->username(); }

  //Return Data Mapping values
  virtual QString  fieldNameChange()        const { return _fieldNameChange; }
  virtual QString  fieldNameNumber()        const { return _fieldNameNumber; }
  virtual QString  fieldNameActive()        const { return _fieldNameActive; }
  virtual QString  fieldNameCrmAccount()    const { return _fieldNameCrmAccount; }
  virtual QString  fieldNameHonorific()     const { return _fieldNameHonorific; }
  virtual QString  fieldNameFirst()         const { return _fieldNameFirst; }
  virtual QString  fieldNameMiddle()        const { return _fieldNameMiddle; }
  virtual QString  fieldNameLast()          const { return _fieldNameLast; }
  virtual QString  fieldNameSuffix()        const { return _fieldNameSuffix; }
  virtual QString  fieldNameInitials()      const { return _fieldNameInitials; }
  virtual QString  fieldNameTitle()         const { return _fieldNameTitle; }
  virtual QString  fieldNamePhone()         const { return _fieldNamePhone; }
  virtual QString  fieldNamePhone2()        const { return _fieldNamePhone2; }
  virtual QString  fieldNameFax()           const { return _fieldNameFax; }
  virtual QString  fieldNameEmailAddress()  const { return _fieldNameEmailAddress; }
  virtual QString  fieldNameWebAddress()    const { return _fieldNameWebAddress; }
  virtual QString  fieldNameAddrChange()    const { return _fieldNameAddrChange; }
  virtual QString  fieldNameAddrNumber()    const { return _fieldNameAddrNumber; }
  virtual QString  fieldNameLine1() 	    const { return _fieldNameLine1; }
  virtual QString  fieldNameLine2()  	    const { return _fieldNameLine2; }
  virtual QString  fieldNameLine3()   	    const { return _fieldNameLine3; }
  virtual QString  fieldNameCity()   	    const { return _fieldNameCity; }
  virtual QString  fieldNameState()  	    const { return _fieldNameState; }
  virtual QString  fieldNamePostalCode()    const { return _fieldNamePostalCode; }
  virtual QString  fieldNameCountry() 	    const { return _fieldNameCountry; }

public slots:
         virtual void setListVisible(bool p);
         virtual void setNumber(QString p);
  inline virtual void clearExtraClause()	        { }
         virtual void findDuplicates();
  inline virtual void setExtraClause(const QString&)    { }
  inline virtual void setAddress(const int p)           { _address->setId(p); }
         virtual void setChange(QString p);
  inline virtual void setDescription(const QString&)    { }
  inline virtual void setEmailAddress(const QString& p) { _email->setText(p); }
  inline virtual void setFax(const QString& p)	        { _fax->setText(p); }
  inline virtual void setFirst(const QString& p)	{ _first->setText(p); }
  inline virtual void setHonorific(const QString& p)    { _honorific->setEditText(p); }
  inline virtual void setLabel(const QString& p)        { _label->setText(p); _label->setHidden(_label->text().isEmpty()); }
  inline virtual void setLast(const QString& p)         { _last->setText(p); }
  inline virtual void setMiddle(const QString& p)       { _middle->setText(p); }
         virtual void setMode(const Mode p);
  inline virtual void setNotes(const QString& p)        { _notes = p; }
  inline virtual void setPhone(const QString& p)	{ _phone->setText(p); }
  inline virtual void setPhone2(const QString& p)	{ _phone2->setText(p); }
  inline virtual void setSuffix(const QString& p)	{ _suffix->setText(p); }
  inline virtual void setTitle(const QString& p)	{ _title->setText(p); }
  inline virtual void setWebAddress(const QString& p)   { _webaddr->setText(p); }
  inline virtual void setOwnerUsername(const QString& p){ _owner->setUsername(p); }
  inline virtual void setOwnerId(const int p) { _owner->setId(p); }

  Q_INVOKABLE void setEmailSubjectText(const QString text);
  Q_INVOKABLE void setEmailBodyText(const QString text);
  
  virtual void	clear();
  virtual void	check();
  virtual bool  sChanged() { return _changed; }
  virtual void	sEllipses();
  virtual void	sInfo();
  virtual void	sList();
  virtual void  sLaunchEmail();
  virtual void  sLaunchWebaddr();
  virtual void	sSearch();
  virtual int	save(AddressCluster::SaveFlags = AddressCluster::CHECK);
  virtual void	setAccount(const int);
  virtual void  setOwnerVisible(const bool);
  virtual void  setOwnerEnabled(const bool);
  virtual void  setNumberVisible(const bool);
  virtual void	setAccountVisible(const bool);
  virtual void	setActiveVisible(const bool);
  virtual void	setAddressVisible(const bool);
  virtual void	setEmailVisible(const bool);
  virtual void	setId(const int);
  virtual void	setInitialsVisible(const bool);
  virtual void	setMinimalLayout(const bool);
  virtual void	setName(const QString& p);
  virtual void	setPhonesVisible(const bool);
  virtual void	setSearchAcct(const int);
  virtual void	setWebaddrVisible(const bool);

  //Set Data Mapping
  virtual void setDataWidgetMap(XDataWidgetMapper* m);
  virtual void setFieldNameChange(QString p)	    { _fieldNameChange = p ; }
  virtual void setFieldNameNumber(QString p)	    { _fieldNameNumber = p ; }
  virtual void setFieldNameActive(QString p)        { _fieldNameActive = p ; }
  virtual void setFieldNameCrmAccount(QString p)    { _fieldNameCrmAccount = p ; }
  virtual void setFieldNameHonorific(QString p)     { _fieldNameHonorific = p ; }
  virtual void setFieldNameFirst(QString p)         { _fieldNameFirst = p ; }
  virtual void setFieldNameMiddle(QString p)        { _fieldNameMiddle = p ; }
  virtual void setFieldNameLast(QString p)          { _fieldNameLast = p ; }
  virtual void setFieldNameSuffix(QString p)        { _fieldNameSuffix = p ; }
  virtual void setFieldNameInitials(QString p)      { _fieldNameInitials = p ; }
  virtual void setFieldNameTitle(QString p)         { _fieldNameTitle = p ; }
  virtual void setFieldNamePhone(QString p)         { _fieldNamePhone = p ; }
  virtual void setFieldNamePhone2(QString p)        { _fieldNamePhone2 = p ; }
  virtual void setFieldNameFax(QString p)           { _fieldNameFax = p ; }
  virtual void setFieldNameEmailAddress(QString p)  { _fieldNameEmailAddress = p ; }
  virtual void setFieldNameWebAddress(QString p)    { _fieldNameWebAddress = p ; }
  virtual void setFieldNameAddrChange(QString p)    { _fieldNameAddrChange = p ; }
  virtual void setFieldNameAddrNumber(QString p)    { _fieldNameAddrNumber = p ; }
  virtual void setFieldNameLine1(QString p)         { _fieldNameLine1 = p ; }
  virtual void setFieldNameLine2(QString p)         { _fieldNameLine2 = p ; }
  virtual void setFieldNameLine3(QString p)         { _fieldNameLine3 = p ; }
  virtual void setFieldNameCity(QString p)          { _fieldNameCity = p ; }
  virtual void setFieldNamePostalCode(QString p)    { _fieldNamePostalCode = p ; }
  virtual void setFieldNameState(QString p)         { _fieldNameState = p ; }
  virtual void setFieldNameCountry(QString p)       { _fieldNameCountry = p ; }

private slots:
  void sCheck();
  void setChanged();

signals:
  void changed();
  void newId(int);

protected:
  QHBoxLayout* _nameBox;
  QHBoxLayout* _initialsBox;
  QHBoxLayout* _titleBox;
  QGridLayout* _buttonBox;
  QVBoxLayout* _ownerBox;
  XLineEdit* _change;
  XLineEdit* _number;
  XComboBox* _honorific;
  Mode _mode;
  QLabel* _numberLit;
  QLabel* _nameLit;
  XLineEdit* _first;
  XLineEdit* _middle;
  XLineEdit* _last;
  XLineEdit* _suffix;
  QLabel* _initialsLit;
  XLineEdit* _initials;
  QLabel* _crmAcctLit;
  CRMAcctCluster* _crmAcct;
  QLabel* _titleLit;
  XLineEdit* _title;
  QLabel* _phoneLit;
  XLineEdit* _phone;
  QLabel* _phone2Lit;
  XLineEdit* _phone2;
  QLabel* _faxLit;
  XLineEdit* _fax;
  QLabel* _emailLit;
  XLineEdit* _email;
  QLabel* _webaddrLit;
  XLineEdit* _webaddr;
  QCheckBox* _active;
  QString _addressChange;
  AddressCluster* _address;
  UsernameCluster* _owner;

  QString _extraClause;
  QString _query;
  int _searchAcctId;
  QString _titleSingular;
  QString _titlePlural;

  bool _ignoreSignals;

  int _crmacctid;
  QString _crmacctname;

private:
  virtual void	init();
  virtual void	layout();
  virtual void	silentSetId(const int);
  XDataWidgetMapper* _mapper;

  QPushButton* _list;

  int	_id;
  bool _layoutDone;
  int	 _limits;
  bool _minimalLayout;
  QString	_notes;
  bool _valid;
  bool _changed;

  //Data Mapping Values
  QString  _fieldNameChange;
  QString  _fieldNameNumber;
  QString  _fieldNameActive;
  QString  _fieldNameCrmAccount;
  QString  _fieldNameHonorific;
  QString  _fieldNameFirst;
  QString  _fieldNameMiddle;
  QString  _fieldNameLast;
  QString  _fieldNameSuffix;
  QString  _fieldNameInitials;
  QString  _fieldNameTitle;
  QString  _fieldNamePhone;
  QString  _fieldNamePhone2;
  QString  _fieldNameFax;
  QString  _fieldNameEmailAddress;
  QString  _fieldNameWebAddress;
  QString  _fieldNameAddrChange;
  QString  _fieldNameAddrNumber;
  QString  _fieldNameLine1;
  QString  _fieldNameLine2;
  QString  _fieldNameLine3;
  QString  _fieldNameCity;
  QString  _fieldNamePostalCode;
  QString  _fieldNameState;
  QString  _fieldNameCountry;
  QString  _subjText;
  QString  _bodyText;
};

#endif
