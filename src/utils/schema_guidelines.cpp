#include "source2mod/ISource2ModApi.h"
#include "source2mod/ISource2ModPlugin.h"
#include "source2mod/sdk/schemasystem_helper.h"

#include <cstring>

SOURCE2MOD_GLOBALVARS();

namespace schema
{
    bool IsGuidelineField(const char* memberName)
    {
        // What SourceMod and CounterStrikeSharp refuse under the same option.
        static const char* const kFields[] = {
            "m_bIsValveDS", "m_bIsQuestEligible", "m_iItemDefinitionIndex", "m_iEntityLevel", "m_iItemIDHigh",
            "m_iItemIDLow", "m_iAccountID", "m_iEntityQuality", "m_bInitialized", "m_szCustomName",
            "m_iAttributeDefinitionIndex", "m_iRawValue32", "m_iRawInitialValue32", "m_flValue", "m_flInitialValue",
            "m_bSetBonus", "m_nRefundableCurrency", "m_OriginalOwnerXuidLow", "m_OriginalOwnerXuidHigh",
            "m_nFallbackPaintKit", "m_nFallbackSeed", "m_flFallbackWear", "m_nFallbackStatTrak", "m_iCompetitiveWins",
            "m_iCompetitiveRanking", "m_iCompetitiveRankType", "m_iCompetitiveRankingPredicted_Win",
            "m_iCompetitiveRankingPredicted_Loss", "m_iCompetitiveRankingPredicted_Tie", "m_nActiveCoinRank", "m_nMusicID",
        };
        for (const char* field : kFields)
        {
            if (!strcmp(field, memberName))
                return true;
        }
        return false;
    }

    bool GuidelinesBlockWrite(const char* className, const char* memberName)
    {
        if (!g_pSource2ModAPI || !g_pSource2ModAPI->FollowsServerGuidelines())
            return false;
        g_pSource2ModAPI->Log("Cannot set %s::%s with \"FollowCS2ServerGuidelines\" enabled.\n", className, memberName);
        return true;
    }
}
