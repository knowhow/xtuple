/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>

#include "guiclient.h"

#include "project.h"
#include "projects.h"
#include "dspOrderActivityByProject.h"

#include "contact.h"
#include "contacts.h"
#include "contactMerge.h"
#include "crmaccountMerge.h"
#include "address.h"
#include "addresses.h"
#include "crmaccount.h"
#include "crmaccounts.h"
#include "incidentWorkbench.h"
#include "incident.h"
#include "opportunityList.h"
#include "todoList.h"
#include "todoListCalendar.h"
#include "todoItem.h"

#include "opportunity.h"

#include "editOwners.h"
#include "createRecurringItems.h"

#include "setup.h"

#include "menuCRM.h"

menuCRM::menuCRM(GUIClient *Pparent) :
  QObject(Pparent)
{
  setObjectName("crmModule");
  parent = Pparent;
  
  toolBar = new QToolBar(tr("CRM Tools"));
  toolBar->setObjectName("CRM Tools");
  toolBar->setIconSize(QSize(32, 32));
  parent->addToolBar(toolBar);

  // Menus
  crmMenu           = new QMenu(parent);
  projectsMenu      = new QMenu(parent);
  incidentMenu      = new QMenu(parent);
  todoMenu          = new QMenu(parent);
  reportsMenu       = new QMenu(parent);
  accountsMenu      = new QMenu(parent);
  contactsMenu      = new QMenu(parent);
  addressMenu       = new QMenu(parent);
  utilitiesMenu     = new QMenu(parent);
  opportunityMenu   = new QMenu(parent);

  crmMenu->setObjectName("menu.crm");
  projectsMenu->setObjectName("menu.crm.projects");
  incidentMenu->setObjectName("menu.crm.incident");
  todoMenu->setObjectName("menu.crm.todo");
  reportsMenu->setObjectName("menu.crm.reports");
  accountsMenu->setObjectName("menu.crm.accounts");
  contactsMenu->setObjectName("menu.crm.contacts");
  addressMenu->setObjectName("menu.crm.address");
  utilitiesMenu->setObjectName("menu.crm.utilities");
  opportunityMenu->setObjectName("menu.crm.opportunity");

  actionProperties acts[] = {
    // CRM | Incident
    { "menu",			tr("&Incident"),	(char*)incidentMenu,		crmMenu,	"true", NULL, NULL, true	, NULL },
    { "crm.incident",		tr("&New..."),		SLOT(sIncident()),		incidentMenu,	"MaintainPersonalIncidents MaintainAllIncidents", NULL, NULL, true , NULL },
    { "crm.incidentList",	tr("&List..."),	SLOT(sIncidentWorkbench()),	incidentMenu,	"ViewPersonalIncidents MaintainPersonalIncidents ViewAllIncidents MaintainAllIncidents", QPixmap(":/images/incidents.png"), toolBar, true , tr("Incident List") },

    // CRM / To Do
    { "menu",			tr("&To-Do"),	(char*)todoMenu,	crmMenu,	"true", NULL, NULL, true	, NULL },
    { "crm.todoItem",		tr("&New..."),	SLOT(sTodoItem()),	todoMenu,	"MaintainPersonalToDoItems MaintainAllToDoItems", NULL, NULL, true	, NULL },
    { "crm.todoList",		tr("&List..."),		SLOT(sTodoList()),	todoMenu,	"MaintainPersonalToDoItems ViewPersonalToDoItems MaintainAllToDoItems ViewAllToDoItems", QPixmap(":/images/toDoList.png"), toolBar, true	, tr("To-Do List") },
    { "crm.todoListCalendar",		tr("&Calendar List..."),		SLOT(sTodoListCalendar()),	todoMenu,	"MaintainPersonalToDoItems ViewPersonalToDoItems MaintainAllToDoItems ViewAllToDoItems", NULL, NULL, true, NULL},

    //  Project
    { "menu", tr("Pro&ject"), (char*)projectsMenu, crmMenu, "true", NULL, NULL, true	, NULL },
    { "pm.newProject", tr("&New..."), SLOT(sNewProject()), projectsMenu, "MaintainPersonalProjects MaintainAllProjects", NULL, NULL, true , NULL },
    { "pm.projects", tr("&List..."), SLOT(sProjects()), projectsMenu, "ViewPersonalProjects MaintainPersonalProjects ViewAllProjects MaintainAllProjects", QPixmap(":/images/projects.png"), toolBar, true , tr("List Projects") },
    
    // Opportunity
    { "menu",		tr("&Opportunity"),	(char*)opportunityMenu,	crmMenu,		"true", NULL, NULL, true	, NULL },
    { "crm.newOpportunity", tr("&New..."), SLOT(sNewOpportunity()), opportunityMenu, "MaintainPersonalOpportunities MaintainAllOpportunities", NULL, NULL, true , NULL },
    { "crm.listOpportunity", tr("&List..."), SLOT(sOpportunities()), opportunityMenu, "MaintainPersonalOpportunities ViewPersonalOpportunities MaintainAllOpportunities ViewAllOpportunities", NULL, NULL, true , NULL },

    { "separator",		NULL,				NULL,			crmMenu,	"true", NULL, NULL, true	, NULL },

    // Reports
    { "menu",				tr("&Reports"),		(char*)reportsMenu,			crmMenu,	"true", NULL, NULL, true	, NULL },

    { "pm.dspOrderActivityByProject", tr("Order &Activity by Project..."), SLOT(sDspOrderActivityByProject()), reportsMenu, "ViewAllProjects ViewPersonalProjects", NULL, NULL, true , NULL },
    { "separator",		NULL,				NULL,			crmMenu,	"true", NULL, NULL, true	, NULL },
    
    // CRM | Account
    { "menu",		tr("&Account"),		(char*)accountsMenu,	crmMenu,		"true", NULL, NULL, true	, NULL },
    { "crm.crmaccount",		tr("&New..."),	SLOT(sCRMAccount()),	accountsMenu,	"MaintainPersonalCRMAccounts MaintainAllCRMAccounts", NULL, NULL, true , NULL },
    { "crm.crmaccounts",	tr("&List..."),	SLOT(sCRMAccounts()),	accountsMenu,	"MaintainPersonalCRMAccounts ViewPersonalCRMAccounts MaintainAllCRMAccounts ViewAllCRMAccounts", QPixmap(":/images/accounts.png"), toolBar, true , tr("List Accounts") },
      
    // CRM | Contact
    { "menu",		tr("&Contact"),		(char*)contactsMenu,	crmMenu,		"true", NULL, NULL, true	, NULL },
    { "crm.contact",	tr("&New..."),		SLOT(sContact()),	contactsMenu,	"MaintainPersonalContacts MaintainAllContacts", NULL, NULL, true	, NULL },
    { "crm.contacts",	tr("&List..."),		SLOT(sContacts()),	contactsMenu,	"MaintainPersonalContacts ViewPersonalContacts MaintainAllContacts ViewAllContacts", QPixmap(":/images/contacts.png"), toolBar, true , tr("List Contacts") },
    
    // CRM | Address
    { "menu",		tr("A&ddress"),		(char*)addressMenu,	crmMenu,		"true", NULL, NULL, true	, NULL },
    { "crm.address",	tr("&New..."),		SLOT(sAddress()),	addressMenu,	"MaintainAddresses", NULL, NULL, true	, NULL },
    { "crm.addresses",	tr("&List..."),	SLOT(sAddresses()),	addressMenu,	"MaintainAddresses ViewAddresses", NULL, NULL, true , NULL },

    { "separator",		NULL,				NULL,			crmMenu,	"true", NULL, NULL, true	, NULL },

    //Utilities
    { "menu",			tr("&Utilities"),		(char*)utilitiesMenu,		crmMenu,	"true", NULL, NULL, true	, NULL },
    { "crm.replaceOwner",	tr("Edit O&wners"),		SLOT(sEditOwners()),	utilitiesMenu,	"EditOwner", NULL, NULL, true, NULL },
    { "crm.createRecurringItems",tr("Create &Recurring Items..."), SLOT(sCreateRecurringItems()),utilitiesMenu, "MaintainPersonalIncidents MaintainPersonalIncidents MaintainAllIncidents MaintainAllProjects MaintainPersonalToDoItems MaintainAllToDoItems", NULL, NULL, true, NULL },
    { "crm.contactMerge",                tr("&Merge Contacts..."), SLOT(sContactMerge()),        utilitiesMenu, "MergeContacts",          NULL, NULL, true, NULL },
    { "crm.crmaccountMerge",         tr("Merge &CRM Accounts..."), SLOT(sCrmaccountMerge()),     utilitiesMenu, "MaintainAllCRMAccounts", NULL, NULL, true, NULL },

    { "crm.setup",	tr("&Setup..."),	SLOT(sSetup()),	crmMenu,	NULL,	NULL,	NULL,	true, NULL}

  };

  addActionsToMenu(acts, sizeof(acts) / sizeof(acts[0]));

  parent->populateCustomMenu(crmMenu, "CRM");
  QAction * m = parent->menuBar()->addMenu(crmMenu);
  if(m)
    m->setText(tr("C&RM"));
}
  
void menuCRM::addActionsToMenu(actionProperties acts[], unsigned int numElems)
{
  QAction * m = 0;
  for (unsigned int i = 0; i < numElems; i++)
  {
    if (! acts[i].visible)
    {
      continue;
    }
    else if (acts[i].actionName == QString("menu"))
    {
      m = acts[i].menu->addMenu((QMenu*)(acts[i].slot));
      if(m)
        m->setText(acts[i].actionTitle);
    }
    else if (acts[i].actionName == QString("separator"))
    {
      acts[i].menu->addSeparator();
    }
    else if ((acts[i].toolBar != NULL) && (!acts[i].toolTip.isEmpty()))
    {
      new Action( parent,
                  acts[i].actionName,
                  acts[i].actionTitle,
                  this,
                  acts[i].slot,
                  acts[i].menu,
                  acts[i].priv,
                  (acts[i].pixmap),
                  acts[i].toolBar,
                  acts[i].toolTip) ;
    }
    else if (acts[i].toolBar != NULL)
    {
      new Action( parent,
                  acts[i].actionName,
                  acts[i].actionTitle,
                  this,
                  acts[i].slot,
                  acts[i].menu,
                  acts[i].priv,
                  (acts[i].pixmap),
                  acts[i].toolBar,
                  acts[i].actionTitle) ;
    }
    else
    {
      new Action( parent,
                  acts[i].actionName,
                  acts[i].actionTitle,
                  this,
                  acts[i].slot,
                  acts[i].menu,
                  acts[i].priv ) ;
    }
  }
}

void menuCRM::sNewProject()
{
  ParameterList params;
  params.append("mode", "new");

  project newdlg(omfgThis, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuCRM::sProjects()
{
  omfgThis->handleNewWindow(new projects());
}

void menuCRM::sDspOrderActivityByProject()
{
  omfgThis->handleNewWindow(new dspOrderActivityByProject());
}

void menuCRM::sCRMAccount()
{
  ParameterList params;
  params.append("mode", "new");
  crmaccount* newdlg = new crmaccount();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuCRM::sCRMAccounts()
{
  omfgThis->handleNewWindow(new crmaccounts());
}

void menuCRM::sEditOwners()
{
  omfgThis->handleNewWindow(new editOwners());
}

void menuCRM::sCreateRecurringItems()
{
  omfgThis->handleNewWindow(new createRecurringItems());
}

void menuCRM::sContact()
{
  ParameterList params;
  params.append("mode", "new");
  contact* newdlg = new contact();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuCRM::sContacts()
{
  omfgThis->handleNewWindow(new contacts());
}

void menuCRM::sContactMerge()
{
  omfgThis->handleNewWindow(new contactMerge());
}

void menuCRM::sCrmaccountMerge()
{
  omfgThis->handleNewWindow(new crmaccountMerge(omfgThis, "crmaccountMerge", Qt::Dialog));
}

void menuCRM::sAddress()
{
  ParameterList params;
  params.append("mode", "new");
  address* newdlg = new address();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuCRM::sAddresses()
{
  omfgThis->handleNewWindow(new addresses());
}

void menuCRM::sIncidentWorkbench()
{
  omfgThis->handleNewWindow(new incidentWorkbench());
}

void menuCRM::sIncident()
{
  ParameterList params;
  params.append("mode", "new");
  incident* newdlg = new incident();
  newdlg->set(params);
  newdlg->exec();
}

void menuCRM::sTodoList()
{
  ParameterList params;
  params.append("run");
  todoList* win = new todoList();
  win->set(params);
  omfgThis->handleNewWindow(win);
}

void menuCRM::sTodoListCalendar()
{
  omfgThis->handleNewWindow(new todoListCalendar());
}

void menuCRM::sTodoItem()
{
  ParameterList params;
  params.append("mode", "new");
  todoItem* newdlg = new todoItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuCRM::sNewOpportunity()
{
  ParameterList params;
  params.append("mode", "new");

  opportunity newdlg(omfgThis, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void menuCRM::sOpportunities()
{
  ParameterList params;
  params.append("run");
  opportunityList* win = new opportunityList();
  win->set(params);
  omfgThis->handleNewWindow(win);
}

void menuCRM::sSetup()
{
  ParameterList params;
  params.append("module", Xt::CRMModule);

  setup newdlg(parent);
  newdlg.set(params);
  newdlg.exec();
}
