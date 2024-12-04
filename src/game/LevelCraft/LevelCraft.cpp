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

void LevelCraft::HandleMeleeOutcome(Unit* pVictim, CalcDamageInfo* damageInfo)
{
    // m_unit is attacking pVictim
    if (pVictim->IsPlayer())
    {
        Message(pVictim, "GUID: " + std::to_string(m_unit->GetGUID()));
    }
}
