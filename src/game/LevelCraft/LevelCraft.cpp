// LevelCraft

#include "Database/DatabaseEnv.h"
#include "LevelCraft.h"
#include "Opcodes.h"
#include "WorldPacket.h"
#include "Chat.h"

const double XP_VARIANCE = 0.1;

void Message(Unit* player, std::string s)
{
    ChatHandler chat((Player*)player);
    chat.SendSysMessage(s.c_str());
}

LevelCraft::LevelCraft(Unit* unit) : m_unit(unit)
{

}

void LevelCraft::SaveToDB()
{
    
}

void LevelCraft::LoadCombatExperienceFromDB(uint32 id, bool isAccount, uint32 target, bool isZone, std::map<uint32, CombatExperienceInfo>& map)
{
    // Load `levelcraft_unit_experience`
    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "> Loading table `levelcraft_unit_experience` for %u(%u) %u(%u)", id, isAccount, target, isZone);
    uint32 count = 0;
    std::string condition = (isAccount ? "account=" : "character=") + std::to_string(id) + " and " + (isZone ? "zone=" : "unit=") + std::to_string(target);

    std::unique_ptr<QueryResult> result(WorldDatabase.Query("SELECT `damage_dealt`, `damage_received`, `crowd_controls`, `kills`, `deaths` FROM `levelcraft_unit_experience` WHERE " + condition));
    if (!result)
    {
        // Initialize empty data
        map[target] = CombatExperienceInfo();
    }
    else
    {
        int resultCount = (int)result->GetRowCount();
        // Should have only one result
        if (resultCount > 1)
        {
            sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "> Table `levelcraft_unit_experience` contains a duplicate entry for %u(%u) %u(%u)", id, isAccount, target, isZone);
        }
        else if (resultCount == 1)
        {
            Field* fields = result->Fetch();
            map[target].damageDealt = fields[0].GetUInt64();
            map[target].damageReceived = fields[0].GetUInt64();
            sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Loaded LevelCraft combat experience for %u(%u) %u(%u)", id, isAccount, target, isZone);
        }
    }
}

template <typename T>
T addVariance(T val)
{
    double rnd = double(rand()) / RAND_MAX;
    double variance = sWorld.getConfig(CONFIG_FLOAT_LEVELCRAFT_COMBAT_XP_VARIANCE);
    return val * (1.0 + 2.0 * (rnd - 0.5) * variance);
}

void PrintDebugInfo(Unit* player, Unit* creature) {
    auto cinfo = ((Creature*)creature)->GetCreatureInfo();
    auto stats = ((Creature*)creature)->GetClassLevelStats();

    if (cinfo && stats)
    {
        char buf[1024];
        sprintf(buf, "Zone: %d, Entry: %d, Class: %d, Health multi: %f, Start health: %d, Max Health: %d", creature->GetZoneId(), creature->GetEntry(), creature->GetClass(), cinfo->health_multiplier, stats->health, creature->GetMaxHealth());
        Message(player, buf);
    }
}

// LevelCraft TODO: Zone XP should be assigned to where the combat was initiated instead of the current location

void LevelCraft::HandleDamageReceived(Unit* pAttacker, CalcDamageInfo* damageInfo)
{
    // Only trigger in PvE
    if (!m_unit->IsPlayer() || !pAttacker->IsCreature())
        return;

    uint32 entry = pAttacker->GetEntry();
    uint32 zone = m_unit->GetZoneId();
    uint64 xpToAward = addVariance(damageInfo->totalDamage);

    // Ensure that the required data is loaded first if required
    Player* player = (Player*)m_unit;
    LoadCombatExperienceFromDB(player->GetGUIDLow(), false, entry, false, m_unitCombatExperience);
    LoadCombatExperienceFromDB(player->GetGUIDLow(), false, zone, true, m_unitCombatExperience);
    LoadCombatExperienceFromDB(player->GetSession()->GetAccountId(), true, entry, false, m_unitCombatExperience);
    LoadCombatExperienceFromDB(player->GetSession()->GetAccountId(), true, zone, true, m_unitCombatExperience);

    m_unitCombatExperience[entry].damageReceived += xpToAward;
    m_zoneCombatExperience[zone].damageReceived += xpToAward;
    m_accountUnitCombatExperience[entry].damageReceived += xpToAward;
    m_accountZoneCombatExperience[zone].damageReceived += xpToAward;

    auto stats = ((Creature*)pAttacker)->GetClassLevelStats();
    damageInfo->totalDamage /= (1.0 + double(m_unitCombatExperience[entry].damageReceived) / stats->health);

    PrintDebugInfo(m_unit, pAttacker);
}

void LevelCraft::HandleDamageDealt(Unit* pVictim, CalcDamageInfo* damageInfo)
{
    // Only trigger in PvE
    if (!m_unit->IsPlayer() || !pVictim->IsCreature())
        return;

    uint32 entry = pVictim->GetEntry();
    uint32 zone = pVictim->GetZoneId();
    uint64 xpToAward = addVariance(damageInfo->totalDamage);

    // Ensure that the required data is loaded first if required
    Player* player = (Player*)m_unit;
    LoadCombatExperienceFromDB(player->GetGUIDLow(), false, entry, false, m_unitCombatExperience);
    LoadCombatExperienceFromDB(player->GetGUIDLow(), false, zone, true, m_unitCombatExperience);
    LoadCombatExperienceFromDB(player->GetSession()->GetAccountId(), true, entry, false, m_unitCombatExperience);
    LoadCombatExperienceFromDB(player->GetSession()->GetAccountId(), true, zone, true, m_unitCombatExperience);

    m_unitCombatExperience[entry].damageDealt += xpToAward;
    m_zoneCombatExperience[zone].damageDealt += xpToAward;
    m_accountUnitCombatExperience[entry].damageDealt += xpToAward;
    m_accountZoneCombatExperience[zone].damageDealt += xpToAward;

    auto stats = ((Creature*)pVictim)->GetClassLevelStats();
    damageInfo->totalDamage *= (1.0 + double(m_unitCombatExperience[entry].damageDealt) / stats->health);

    PrintDebugInfo(m_unit, pVictim);
}
