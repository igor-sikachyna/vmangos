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

void LevelCraft::HandleFlatAndPercentOutcomes(uint64 in_value, uint64 in_max, uint64& out_flat, uint64& out_pct, double& out_pctLow)
{
    out_flat += in_value;
    double temp_pct = (in_value * 100.0) / in_max;
    uint64 full_pct = temp_pct;
    out_pct += full_pct;
    out_pctLow += temp_pct - double(full_pct);
    if (out_pctLow >= 1.0)
    {
        uint64 diff = out_pctLow;
        out_pct += diff;
        out_pctLow -= diff;
    }
}

template <typename T>
T addVariance(T val)
{
    double rnd = double(rand()) / RAND_MAX;
    double variance = sWorld.getConfig(CONFIG_FLOAT_LEVELCRAFT_COMBAT_XP_VARIANCE);
    return val * (1.0 + 2.0 * (rnd - 0.5) * variance);
}

void LevelCraft::HandleMeleeOutcome(Unit* pVictim, CalcDamageInfo* damageInfo)
{
    // Ignore PvP
    if (m_unit->IsPlayer() && pVictim->IsPlayer())
        return;
    // Ignore EvE
    if (!m_unit->IsPlayer() && !pVictim->IsPlayer())
        return;

    // m_unit is attacking pVictim
    if (pVictim->IsPlayer())
    {
        uint32 entry = m_unit->GetEntry();
        m_unitCombatExperience[entry].damageReceived += addVariance(damageInfo->totalDamage);

        auto stats = ((Creature*)m_unit)->GetClassLevelStats();
        damageInfo->totalDamage /= (1.0 + double(m_unitCombatExperience[entry].damageReceived) / stats->health);
    }
    else if (m_unit->IsPlayer())
    {
        uint32 entry = pVictim->GetEntry();

        m_unitCombatExperience[entry].damageDealt += addVariance(damageInfo->totalDamage);

        auto stats = ((Creature*)pVictim)->GetClassLevelStats();
        damageInfo->totalDamage *= (1.0 + double(m_unitCombatExperience[entry].damageDealt) / stats->health);
    }

    Unit* creature = pVictim->IsPlayer() ? m_unit : pVictim;
    Unit* player = pVictim->IsPlayer() ? pVictim : m_unit;
    if (creature->IsCreature())
    {
        auto cinfo = ((Creature*)creature)->GetCreatureInfo();
        auto stats = ((Creature*)creature)->GetClassLevelStats();
        char buf[1024];
        if (cinfo && stats)
        {
            sprintf(buf, "Map: %d, Zone: %d, Area: %d, Entry: %d, Class: %d, Health multi: %f, Start health: %d, Max Health: %d", creature->GetMapId(), creature->GetZoneId(), creature->GetAreaId(), creature->GetEntry(), creature->GetClass(), cinfo->health_multiplier, stats->health, creature->GetMaxHealth());
        }
        else
        {
            sprintf(buf, "Map: %d, Zone: %d, Area: %d, Entry: %d, Class: %d, Max Health: %d", creature->GetMapId(), creature->GetZoneId(), creature->GetAreaId(), creature->GetZoneId(), creature->GetEntry(), creature->GetClass(), creature->GetMaxHealth());
        }
        Message(player, buf);
    }
}
