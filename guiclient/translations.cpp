/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "translations.h"

#include <QDebug>
#include <QVariant>
#include <QMessageBox>
#include <QDesktopServices>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDir>
#include <QFile>
#include <QTranslator>

#include <parameter.h>

translations::translations(QWidget* parent, const char* name, Qt::WFlags fl)
  : XWidget(parent, name, fl)
{
  setupUi(this);

  nwam = new QNetworkAccessManager(this);

  connect(_locale, SIGNAL(newID(int)), this, SLOT(sLocaleChanged()));
  connect(_translations, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(sLanguageSelected(QTreeWidgetItem*, int)));
  connect(_translations, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*, QTreeWidgetItem*, int)));
  connect(nwam, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)));

  _translations->addColumn(tr("Package"),  _itemColumn, Qt::AlignLeft,   true, "package" );
  _translations->addColumn(tr("Found"),    _ynColumn,   Qt::AlignCenter, true, "found" );
  _translations->addColumn(tr("Location"), -1,          Qt::AlignLeft,   true, "location" );
  
  _locale->setId(_preferences->value("locale_id").toInt());
  _locale->setEnabled(false);
  sLocaleChanged();
}

translations::~translations()
{
  // no need to delete child widgets, Qt does it all for us
}

void translations::languageChange()
{
  retranslateUi(this);
}

void translations::sLocaleChanged()
{
  XSqlQuery langq;
  langq.prepare("SELECT * "
                "  FROM locale LEFT OUTER JOIN"
                "       lang ON (locale_lang_id=lang_id) LEFT OUTER JOIN"
                "       country ON (locale_country_id=country_id) "
                " WHERE(locale_id=:localeid);");
  langq.bindValue(":localeid", _locale->id());
  langq.exec();
  if(langq.first())
  {
    if (!langq.value("lang_abbr2").toString().isEmpty() &&
        !langq.value("country_abbr").toString().isEmpty())
    {
      langext = langq.value("lang_abbr2").toString() + "_" +
                langq.value("country_abbr").toString().toLower();
    }
    else if (!langq.value("lang_abbr2").toString().isEmpty())
    {
      langext = langq.value("lang_abbr2").toString();
    }

    _language->setText(langq.value("lang_name").toString());
    _country->setText(langq.value("country_name").toString());
  }
  else
  {
    langext = QLocale::system().name().toLower();
    _language->setText("Default");
    _country->setText("Default");
  }

  _translations->clear();

  QStringList files;
  files << "qt";
  files << "xTuple";
  files << "openrpt";
  files << "reports";

  XSqlQuery pkglist("SELECT pkghead_name"
                    "  FROM pkghead"
                    " WHERE packageIsEnabled(pkghead_name);");
  while(pkglist.next())
    files << pkglist.value("pkghead_name").toString();

  if(files.size() < 1)
    return;

  XTreeWidgetItem * item = 0;
  int i = 1;
  for (QStringList::Iterator fit = files.begin(); fit != files.end(); ++fit)
  {
    QString loc = translationFile(langext, *fit);
    item = new XTreeWidgetItem(_translations, item, i++, *fit, (loc.isEmpty() ? tr("No") : tr("Yes")), loc);
  }
}

void translations::sLanguageSelected(QTreeWidgetItem * item, int /*column*/)
{
  if(!item)
    return;

  QNetworkReply * nwrep = nwam->get(QNetworkRequest(QUrl("http://www.xtuple.org/xttranslate/guiexport/" + item->text(0) + "/" + langext + "/current")));
  nwrep->setProperty("package_name", item->text(0));
  connect(nwrep, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));
  item->setText(2, tr("Connecting..."));
}

void translations::finished(QNetworkReply * nwrep)
{
  QList<XTreeWidgetItem*> ilist = _translations->findItems(nwrep->property("package_name").toString(), Qt::MatchFixedString, 0);
  if(ilist.isEmpty())
  {
    qDebug() << "No items found matching replies package name";
    return;
  }
  QTreeWidgetItem * item = ilist.at(0);
  if(!item)
  {
    qDebug() << "Found item is not valid";
    return;
  }
  if(nwrep->header(QNetworkRequest::LocationHeader).isValid())
  {
    QNetworkReply * newrep = nwam->get(QNetworkRequest(QUrl(nwrep->header(QNetworkRequest::LocationHeader).toString())));
    newrep->setProperty("package_name", nwrep->property("package_name"));
    connect(newrep, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));
  }
  else
  {
    if(nwrep->error() == QNetworkReply::NoError)
    {
      QByteArray ba = nwrep->readAll();
      if(!ba.isEmpty())
      {
        QDir dir(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
        if(!dir.exists())
          dir.mkpath(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
        QFile file(QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/" + item->text(0) + "." + langext + ".qm");
        if(file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
          file.write(ba);
          file.close();
          QTranslator * translator = new QTranslator(qApp);
          if (translator->load(translationFile(langext, item->text(0))))
          {
            qApp->installTranslator(translator);
            qDebug("updated/installed %s", (item->text(0)).toAscii().data());
          }
          item->setText(1, tr("Yes"));
          item->setText(2, translationFile(langext, item->text(0)));
        }
        else
        {
          item->setText(2, tr("Could not save file."));
        }
      }
      else
      {
        item->setText(2, tr("No translation is currently available."));
      }
    }
    else
    {
      item->setText(2, tr("Could not retrieve translation at this time."));
    }
  }
  nwrep->deleteLater();
}

void translations::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
  if(bytesReceived == bytesTotal)
    return; // Don't do anything since the download is done and the finished function will do the rest.
  QNetworkReply * nwrep = qobject_cast<QNetworkReply*>(sender());
  if(!nwrep)
    return;
  QList<XTreeWidgetItem*> ilist = _translations->findItems(nwrep->property("package_name").toString(), Qt::MatchFixedString, 0);
  if(ilist.isEmpty())
  {
    qDebug() << "No items found matching replies package name";
    return;
  }
  QTreeWidgetItem * item = ilist.at(0);
  if(!item)
  {
    qDebug() << "Found item is not valid";
    return;
  }
  item->setText(2, tr("%1 bytes downloaded").arg(bytesReceived));
}

void translations::sPopulateMenu(QMenu * pMenu, QTreeWidgetItem *, int /*pColumn*/)
{
  QAction *menuItem;
  menuItem = pMenu->addAction(tr("Check for translation"), this, SLOT(sDownload()));
}

void translations::sDownload()
{
  QTreeWidgetItem * item = _translations->currentItem();
  if(item)
    sLanguageSelected(item, 0);
}

