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
    uint64 kills;
    uint64 deaths;
};

class LevelCraft
{
public:
    LevelCraft(Unit* unit);

    void SaveToDB();

    void HandleDamageReceived(Unit* pAttacker, CalcDamageInfo* damageInfo);
    void HandleDamageDealt(Unit* pVictim, CalcDamageInfo* damageInfo);

private:

    void LoadCombatExperienceFromDB(uint32 id, bool isAccount, uint32 target, bool isZone, std::map<uint32, CombatExperienceInfo>& map);

    Unit* m_unit;

    std::map<uint32, CombatExperienceInfo> m_zoneCombatExperience;
    std::map<uint32, CombatExperienceInfo> m_unitCombatExperience;
    std::map<uint32, CombatExperienceInfo> m_accountZoneCombatExperience;
    std::map<uint32, CombatExperienceInfo> m_accountUnitCombatExperience;
};

#endif
