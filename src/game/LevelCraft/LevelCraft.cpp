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

bool LevelCraft::Create()
{
    return true;
}

void LevelCraft::SaveToDB()
{
    
}

bool LevelCraft::LoadFromDB()
{
    return true;
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
    uint64 xpToAward = addVariance(damageInfo->totalDamage);
    m_unitCombatExperience[entry].damageReceived += xpToAward;
    m_zoneCombatExperience[m_unit->GetZoneId()].damageReceived += xpToAward;

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
    uint64 xpToAward = addVariance(damageInfo->totalDamage);
    m_unitCombatExperience[entry].damageDealt += xpToAward;
    m_zoneCombatExperience[pVictim->GetZoneId()].damageDealt += xpToAward;

    auto stats = ((Creature*)pVictim)->GetClassLevelStats();
    damageInfo->totalDamage *= (1.0 + double(m_unitCombatExperience[entry].damageDealt) / stats->health);

    PrintDebugInfo(m_unit, pVictim);
}
