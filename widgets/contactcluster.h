/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef _contactCluster_h

#define _contactCluster_h

#include "virtualCluster.h"
#include "addresscluster.h"
#include "xurllabel.h"
#include "contactwidget.h"


class XTUPLEWIDGETS_EXPORT ContactClusterLineEdit : public VirtualClusterLineEdit
{
    Q_OBJECT

    friend class ContactCluster;
    public:
      ContactClusterLineEdit(QWidget*, const char* = 0);

      Q_INVOKABLE int searchAcctId() { return _searchAcctId; }

    public slots:
      void setNewAddr(QString line1, QString line2, QString line3,
                      QString city, QString state, QString postalcode,
                      QString country);
      void setSearchAcct(int crmAcctId);
      void sList();
      void sNew();
      void sSearch();

    protected slots:
      ContactList* listFactory();
      ContactSearch* searchFactory();
      void silentSetId(const int);

    private:
      QStringList _newAddr;
      int _searchAcctId;
};

class XTUPLEWIDGETS_EXPORT ContactCluster : public VirtualCluster
{
    Q_OBJECT
    Q_PROPERTY(bool minimalLayout READ minimalLayout WRITE setMinimalLayout);

    public:
      ContactCluster(QWidget*, const char* = 0);
      Q_INVOKABLE int searchAcctId() { return _searchAcctId; }

      Q_INVOKABLE int crmAcctId() { return _crmAcctId; }
      Q_INVOKABLE bool minimalLayout() { return _minLayout; }

      Q_INVOKABLE void setHonorific(const QString honorifc);
      Q_INVOKABLE void setFirst(const QString first);
      Q_INVOKABLE void setMiddle(const QString middle);
      Q_INVOKABLE void setLast(const QString last);
      Q_INVOKABLE void setSuffix(const QString suffix);
      Q_INVOKABLE void setPhone(const QString phone);
      Q_INVOKABLE void setTitle(const QString title);
      Q_INVOKABLE void setFax(const QString fax);
      Q_INVOKABLE void setEmailAddress(const QString email);

      Q_INVOKABLE void setEmailSubjectText(const QString text);
      Q_INVOKABLE void setEmailBodyText(const QString text);

      Q_INVOKABLE QString name() const;
      Q_INVOKABLE QString honorific() const { return _fname->at(0); }
      Q_INVOKABLE QString first() const { return _fname->at(1); }
      Q_INVOKABLE QString middle() const { return _fname->at(2); }
      Q_INVOKABLE QString last() const { return _fname->at(3); }
      Q_INVOKABLE QString suffix() const { return _fname->at(4); }
      Q_INVOKABLE QString title() const { return _description->text(); }
      Q_INVOKABLE QString phone() const { return _phone->text(); }
      Q_INVOKABLE QString fax() const { return _fax->text(); }
      Q_INVOKABLE QString emailAddress() const { return _email->text(); }

    public slots:
      void launchEmail(QString url);
      void openUrl(QString url);
      void setDescriptionVisible(const bool p);
      void setMinimalLayout(bool);
      void setNameVisible(const bool) { }
      void setNewAddr(QString line1, QString line2, QString line3,
                      QString city, QString state, QString postalcode,
                      QString country);
      void setSearchAcct(int crmAcctId);

    private slots:
      void populate();

    signals:
      void changed();

    protected:
      void addNumberWidget(ContactClusterLineEdit* pNumberWidget);
      void setName(int segment, const QString name);

    private:
      bool _minLayout;
      int _crmAcctId;
      int _searchAcctId;
      QVBoxLayout* _addrLayout;
      QLabel* _titleLit;
      QLabel* _phoneLit;
      QLabel* _phone;
      QLabel* _phone2Lit;
      QLabel* _phone2;
      QLabel* _faxLit;
      QLabel* _fax;
      QLabel* _emailLit;
      QLabel* _webaddrLit;
      QLabel* _addr;
      QStringList* _fname;
      XURLLabel* _email;
      XURLLabel* _webaddr;
      QString _subjText;
      QString _bodyText;
};

#endif
