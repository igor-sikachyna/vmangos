// LevelCraft

#ifndef MANGOSSERVER_LEVELCRAFT_H
#define MANGOSSERVER_LEVELCRAFT_H

#include <map>

class Unit;
struct CalcDamageInfo;

struct CombatExperienceInfo
{
    uint64 damageDealt;
    uint64 damageDealtPct;
    uint64 damageReceived;
    uint64 damageReceivedPct;

    double temp_damageDealtPctLow;
    double temp_damageReceivedPctLow;
};

class LevelCraft
{
public:
    LevelCraft(Unit* unit);

    bool Create();

    void SaveToDB();
    bool LoadFromDB();

    void HandleMeleeOutcome(Unit* pVictim, CalcDamageInfo* damageInfo);

private:
    void HandleFlatAndPercentOutcomes(uint64 in_value, uint64 in_max, uint64& out_flat, uint64& out_pct, double& out_pctLow);

    Unit* m_unit;

    std::map<uint32, CombatExperienceInfo> m_combatExperience;
};

#endif
