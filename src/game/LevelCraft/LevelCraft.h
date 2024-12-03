// LevelCraft

#ifndef MANGOSSERVER_LEVELCRAFT_H
#define MANGOSSERVER_LEVELCRAFT_H

#include "Unit.h"

class LevelCraft
{
public:
    LevelCraft(Unit* unit);

    virtual bool Create();

    virtual void SaveToDB();
    virtual bool LoadFromDB();

    virtual void HandleMeleeOutcome();

private:
    Unit* m_unit;
};

#endif
