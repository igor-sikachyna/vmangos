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

    bool dbEntryExists;
};

class LevelCraft
{
public:
    LevelCraft(Unit* unit);

    void LoadFromDB();
    void SaveToDB();

    void HandleDamageReceived(Unit* pAttacker, CalcDamageInfo* damageInfo);
    void HandleDamageDealt(Unit* pVictim, CalcDamageInfo* damageInfo);

private:

    enum CombatExperienceKeyType
    {
        ACCOUNT,
        CHARACTER
    };

    enum CombatExperienceTargetType
    {
        ZONE,
        UNIT
    };

    struct ModifiedEntryQueueElement
    {
        CombatExperienceTargetType targetType;
        uint32 target;

        uint64 ToUint64() { return (uint64)target + (static_cast<uint64>(targetType) << 32); }
    };
    std::map<uint64, ModifiedEntryQueueElement> m_modified;
    void AddModifiedEntry(uint32 target, CombatExperienceTargetType targetType);

    void LoadCombatExperienceFromDB(uint32 id, CombatExperienceKeyType keyType, uint32 target, CombatExperienceTargetType targetType, std::map<uint32, CombatExperienceInfo>& map);

    void InsertOrUpdateCombatExperience(CombatExperienceInfo* info, uint32 id, CombatExperienceKeyType keyType, uint32 target, CombatExperienceTargetType targetType);

    Unit* m_unit;

    std::map<uint32, CombatExperienceInfo> m_zoneCombatExperience;
    std::map<uint32, CombatExperienceInfo> m_unitCombatExperience;
    std::map<uint32, CombatExperienceInfo> m_accountZoneCombatExperience;
    std::map<uint32, CombatExperienceInfo> m_accountUnitCombatExperience;
};

#endif
