// LevelCraft

#ifndef MANGOSSERVER_LEVELCRAFT_H
#define MANGOSSERVER_LEVELCRAFT_H

#include <map>

class Unit;
struct CalcDamageInfo;

struct CombatExperienceInfo
{
    uint64 damageDealt;
    uint64 damageReceived;
    uint64 crowdControls;
};

class LevelCraft
{
public:
    LevelCraft(Unit* unit);

    bool Create();

    void SaveToDB();
    bool LoadFromDB();

    void HandleDamageReceived(Unit* pAttacker, CalcDamageInfo* damageInfo);
    void HandleDamageDealt(Unit* pVictim, CalcDamageInfo* damageInfo);

private:

    Unit* m_unit;

    std::map<uint32, CombatExperienceInfo> m_zoneCombatExperience;
    std::map<uint32, CombatExperienceInfo> m_unitCombatExperience;
};

#endif
