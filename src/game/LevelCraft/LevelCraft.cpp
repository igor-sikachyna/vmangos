// LevelCraft

#include "Database/DatabaseEnv.h"
#include "LevelCraft.h"
#include "Opcodes.h"
#include "WorldPacket.h"
#include "Chat.h"

void Message(Unit* player, std::string s)
{
    ChatHandler chat((Player*)player);
    chat.SendSysMessage(s.c_str());
}

LevelCraft::LevelCraft(Unit* unit) : m_unit(unit)
{

}

void LevelCraft::LoadFromDB()
{
    // TBA
}

void LevelCraft::InsertOrUpdateCombatExperience(CombatExperienceInfo* info, uint32 id, CombatExperienceKeyType keyType, uint32 target, CombatExperienceTargetType targetType)
{
    try
    {
        bool pResult;

        if (info->dbEntryExists)
        {
            std::string statement = std::string("UPDATE `levelcraft_unit_experience` SET ") +
                "`damage_dealt` = %u, " +
                "`damage_received` = %u " +
                "`kills` = %u " +
                "`deaths` = %u " +
                "WHERE " +
                (keyType == CombatExperienceKeyType::ACCOUNT ? "`account`" : "`character`") + " = %u, " +
                (targetType == CombatExperienceTargetType::UNIT ? "`unit`" : "`zone`") + " = %u ;";

            pResult = CharacterDatabase.PExecute(statement.c_str(), info->damageDealt, info->damageReceived, info->kills, info->deaths, id, target);
        }
        else
        {
            std::string statement = std::string("INSERT INTO `levelcraft_unit_experience` SET ") +
                (keyType == CombatExperienceKeyType::ACCOUNT ? "`account`" : "`character`") + " = %u " +
                (targetType == CombatExperienceTargetType::UNIT ? "`unit`" : "`zone`") + " = %u " +
                "`damage_dealt` = %u " +
                "`damage_received` = %u " +
                "`kills` = %u " +
                "`deaths` = %u ;";

            pResult = CharacterDatabase.PExecute(statement.c_str(), id, target, info->damageDealt, info->damageReceived, info->kills, info->deaths);

            info->dbEntryExists = true;
        }

        if (!pResult)
        {
            sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "> Failed to write to `levelcraft_unit_experience` for %u(%u) %u(%u)", id, keyType, target, targetType);
        }
    }
    catch (std::runtime_error& err)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "> Exception thrown during an SQL update in `levelcraft_unit_experience` for %u(%u) %u(%u) with %s", id, keyType, target, targetType, err.what());
    }
}

void LevelCraft::SaveToDB()
{
    for (auto& m : m_modified)
    {
        // Save to DB
        auto& q = m.second;
        CombatExperienceInfo* characterInfo;
        CombatExperienceInfo* accountInfo;
        if (q.targetType == CombatExperienceTargetType::UNIT)
        {
            characterInfo = &m_unitCombatExperience[q.target];
            accountInfo = &m_accountUnitCombatExperience[q.target];
        }
        else
        {
            characterInfo = &m_zoneCombatExperience[q.target];
            accountInfo = &m_accountZoneCombatExperience[q.target];
        }

        Player* player = (Player*)m_unit;
        InsertOrUpdateCombatExperience(characterInfo, player->GetGUIDLow(), CombatExperienceKeyType::CHARACTER, q.target, q.targetType);
        InsertOrUpdateCombatExperience(accountInfo, player->GetSession()->GetAccountId(), CombatExperienceKeyType::ACCOUNT, q.target, q.targetType);
    }

    m_modified.clear();
}

void LevelCraft::LoadCombatExperienceFromDB(uint32 id, CombatExperienceKeyType keyType, uint32 target, CombatExperienceTargetType targetType, std::map<uint32, CombatExperienceInfo>& map)
{
    if (map.count(target) > 0)
        return;

    try
    {
        // Load `levelcraft_unit_experience`
        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "> Loading table `levelcraft_unit_experience` for %u(%u) %u(%u)", id, keyType, target, targetType);
        uint32 count = 0;
        std::string condition = (keyType == CombatExperienceKeyType::ACCOUNT ? "`account` = " : "`character` = ") + std::to_string(id) + " and " + (targetType == CombatExperienceTargetType::ZONE ? "`zone` = " : "`unit` = ") + std::to_string(target);

        std::unique_ptr<QueryResult> result(CharacterDatabase.Query("SELECT `damage_dealt`, `damage_received`, `crowd_controls`, `kills`, `deaths` FROM `levelcraft_unit_experience` WHERE " + condition));
        if (!result || result->GetRowCount() == 0)
        {
            // Initialize empty data
            map[target] = CombatExperienceInfo();
            map[target].dbEntryExists = false;
        }
        else
        {
            // Should have only one result
            if (result->GetRowCount() > 1)
            {
                sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "> Table `levelcraft_unit_experience` contains a duplicate entry for %u(%u) %u(%u)", id, keyType, target, targetType);
            }

            Field* fields = result->Fetch();
            map[target].damageDealt = fields[0].GetUInt64();
            map[target].damageReceived = fields[1].GetUInt64();
            map[target].dbEntryExists = true;
            sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Loaded LevelCraft combat experience for %u(%u) %u(%u)", id, keyType, target, targetType);
        }
    }
    catch (std::runtime_error& err)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "> Exception thrown during an SQL query in `levelcraft_unit_experience` for %u(%u) %u(%u) with %s", id, keyType, target, targetType, err.what());
    }
}

void LevelCraft::AddModifiedEntry(uint32 target, CombatExperienceTargetType targetType)
{
    ModifiedEntryQueueElement m;
    m.target = target;
    m.targetType = targetType;
    uint64 id = m.ToUint64();
    if (m_modified.count(id) == 0)
    {
        m_modified[id] = m;
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
// LevelCraft TODO: Periodically save data to DB
// LevelCraft TODO: Handle spell and pet damage

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
    LoadCombatExperienceFromDB(player->GetGUIDLow(), CombatExperienceKeyType::CHARACTER, entry, CombatExperienceTargetType::UNIT, m_unitCombatExperience);
    LoadCombatExperienceFromDB(player->GetGUIDLow(), CombatExperienceKeyType::CHARACTER, zone, CombatExperienceTargetType::ZONE, m_zoneCombatExperience);
    LoadCombatExperienceFromDB(player->GetSession()->GetAccountId(), CombatExperienceKeyType::ACCOUNT, entry, CombatExperienceTargetType::UNIT, m_accountUnitCombatExperience);
    LoadCombatExperienceFromDB(player->GetSession()->GetAccountId(), CombatExperienceKeyType::ACCOUNT, zone, CombatExperienceTargetType::ZONE, m_accountZoneCombatExperience);

    m_unitCombatExperience[entry].damageReceived += xpToAward;
    m_zoneCombatExperience[zone].damageReceived += xpToAward;
    m_accountUnitCombatExperience[entry].damageReceived += xpToAward;
    m_accountZoneCombatExperience[zone].damageReceived += xpToAward;

    AddModifiedEntry(entry, CombatExperienceTargetType::UNIT);
    AddModifiedEntry(zone, CombatExperienceTargetType::ZONE);

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
    LoadCombatExperienceFromDB(player->GetGUIDLow(), CombatExperienceKeyType::CHARACTER, entry, CombatExperienceTargetType::UNIT, m_unitCombatExperience);
    LoadCombatExperienceFromDB(player->GetGUIDLow(), CombatExperienceKeyType::CHARACTER, zone, CombatExperienceTargetType::ZONE, m_zoneCombatExperience);
    LoadCombatExperienceFromDB(player->GetSession()->GetAccountId(), CombatExperienceKeyType::ACCOUNT, entry, CombatExperienceTargetType::UNIT, m_accountUnitCombatExperience);
    LoadCombatExperienceFromDB(player->GetSession()->GetAccountId(), CombatExperienceKeyType::ACCOUNT, zone, CombatExperienceTargetType::ZONE, m_accountZoneCombatExperience);

    m_unitCombatExperience[entry].damageDealt += xpToAward;
    m_zoneCombatExperience[zone].damageDealt += xpToAward;
    m_accountUnitCombatExperience[entry].damageDealt += xpToAward;
    m_accountZoneCombatExperience[zone].damageDealt += xpToAward;

    AddModifiedEntry(entry, CombatExperienceTargetType::UNIT);
    AddModifiedEntry(zone, CombatExperienceTargetType::ZONE);

    auto stats = ((Creature*)pVictim)->GetClassLevelStats();
    damageInfo->totalDamage *= (1.0 + double(m_unitCombatExperience[entry].damageDealt) / stats->health);

    PrintDebugInfo(m_unit, pVictim);
}

void LevelCraft::HandleKill(Unit* pVictim)
{
    // Only trigger in PvE
    if (!m_unit->IsPlayer() || !pVictim->IsCreature())
        return;

    uint32 entry = pVictim->GetEntry();
    uint32 zone = pVictim->GetZoneId();

    m_unitCombatExperience[entry].kills += 1;
    m_zoneCombatExperience[zone].kills += 1;
    m_accountUnitCombatExperience[entry].kills += 1;
    m_accountZoneCombatExperience[zone].kills += 1;

    AddModifiedEntry(entry, CombatExperienceTargetType::UNIT);
    AddModifiedEntry(zone, CombatExperienceTargetType::ZONE);

    PrintDebugInfo(m_unit, pVictim);
}

void LevelCraft::HandleDeath(Unit* pAttacker)
{
    // Only trigger in PvE
    if (!m_unit->IsPlayer() || !pAttacker->IsCreature())
        return;

    uint32 entry = pAttacker->GetEntry();
    uint32 zone = m_unit->GetZoneId();

    m_unitCombatExperience[entry].deaths += 1;
    m_zoneCombatExperience[zone].deaths += 1;
    m_accountUnitCombatExperience[entry].deaths += 1;
    m_accountZoneCombatExperience[zone].deaths += 1;

    AddModifiedEntry(entry, CombatExperienceTargetType::UNIT);
    AddModifiedEntry(zone, CombatExperienceTargetType::ZONE);

    PrintDebugInfo(m_unit, pAttacker);
}
