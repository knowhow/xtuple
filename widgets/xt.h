#ifndef XT_H
#define XT_H

#include <QScriptEngine>

void setupXt(QScriptEngine *engine);

class Xt
{

  public:

  enum ItemDataRole {
    RawRole = (Qt::UserRole + 1),
    ScaleRole,
    IdRole,
    RunningSetRole,
    RunningInitRole,
    TotalSetRole,
    TotalInitRole,
    IndentRole,
    DeletedRole
  };

  enum StandardModules
  {
    AllModules        = 0x00000000,
    AccountingModule  = 0x00000200,
    SalesModule       = 0x00000400,
    CRMModule         = 0x00000800,
    ManufactureModule = 0x00001000,
    PurchaseModule    = 0x00002000,
    ScheduleModule    = 0x00004000,
    InventoryModule   = 0x00008000,
    ProductsModule    = 0x00010000,
    SystemModule      = 0x00020000
  };

};

#endif // XT_H
