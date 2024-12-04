// LevelCraft

#ifndef MANGOSSERVER_LEVELCRAFT_H
#define MANGOSSERVER_LEVELCRAFT_H

class Unit;
struct CalcDamageInfo;

class LevelCraft
{
public:
    LevelCraft(Unit* unit);

    virtual bool Create();

    virtual void SaveToDB();
    virtual bool LoadFromDB();

    virtual void HandleMeleeOutcome(Unit* pVictim, CalcDamageInfo* damageInfo);

private:
    Unit* m_unit;
};

#endif
