/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dictionaries.h"

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
#include <gunzip.h>
#include <tarfile.h>
#include <xtHelp.h>

dictionaries::dictionaries(QWidget* parent, const char* name, Qt::WFlags fl)
  : XWidget(parent, name, fl)
{
  setupUi(this);

  _state = Idle;
  nwam = new QNetworkAccessManager(this);
  _nwrep = 0;

  connect(nwam, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)));
  connect(_button, SIGNAL(clicked()), this, SLOT(sAction()));

  langext = QLocale().name().toLower();
}

dictionaries::~dictionaries()
{
  // no need to delete child widgets, Qt does it all for us
}

void dictionaries::languageChange()
{
  retranslateUi(this);
}

void dictionaries::finished(QNetworkReply * nwrep)
{
  if(nwrep->header(QNetworkRequest::LocationHeader).isValid())
  {
    QNetworkReply * newrep = nwam->get(QNetworkRequest(QUrl(nwrep->header(QNetworkRequest::LocationHeader).toString())));
    connect(newrep, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));
    _nwrep = newrep;
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
        QFile file(QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/spell." + langext + ".tar.gz");
        if(file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
          file.write(ba);
          file.close();
          QByteArray data = gunzipFile(file.fileName());
          if(!data.isEmpty())
          {
            TarFile *_files = new TarFile(data);
            if(_files->isValid())
            {
              bool error = false;
              QMapIterator<QString, QByteArray> i(_files->_list);
              while (i.hasNext())
              {
                i.next();
                //cout << i.key() << ": " << i.value() << endl;
                QFile ff(QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/" + i.key());
                if(ff.open(QIODevice::WriteOnly | QIODevice::Truncate))
                {
                  ff.write(i.value());
                  ff.close();
                }
                else
                {
                  error = true;
                }
              }
              if(error)
              {
                _label->setText(tr("Could not save one or more files."));
              }
              else
              {
                _label->setText(tr("Dictionaries downloaded."));
                xtHelp::reload();
              }
            }
            else
            {
              _label->setText(tr("Could not read archive format."));
            }
            delete _files;
          }
          else
          {
            _label->setText(tr("Could not uncompress file."));
          }
        }
        else
        {
          _label->setText(tr("Could not save file."));
        }
      }
      else
      {
        _label->setText(tr("No dictionary is currently available."));
      }
    }
    else
    {
      qDebug() << "Error: " << nwrep->error();
      _label->setText(tr("Could not retrieve dictionaries at this time."));
    }
    _progress->setRange(0, 100);
    _progress->setValue(100);
    _button->setText(tr("Start"));
    _state = Idle;
  }
  if(nwrep == _nwrep)
    _nwrep = 0;
  nwrep->deleteLater();
}

void dictionaries::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
  if(bytesTotal == -1)
    bytesTotal = 5000000; // chose some number that is reasonable
  _progress->setRange(0, bytesTotal);
  _progress->setValue(bytesReceived);
}

void dictionaries::sAction()
{
  if(_state == Busy)
  {
    if(_nwrep)
    {
      _nwrep->abort();
      _nwrep->deleteLater();
      _nwrep = 0;
    }
    _button->setText(tr("Start"));
    _state = Idle;
  }
  else
  {
    _nwrep = nwam->get(QNetworkRequest(QUrl("http://www.xtuple.org/xttranslate/guiexport/spell/" + langext + "/current")));
    connect(_nwrep, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));
    _label->setText(tr("Downloading..."));
    _button->setText(tr("Cancel"));
    _state = Busy;
  }
}

