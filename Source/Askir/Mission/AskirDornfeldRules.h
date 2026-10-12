#pragma once
#include <cstdint>
#include <limits>

// Production rules shared by the Unreal component and native tests.
// These tests validate decisions, not Unreal playability.
namespace AskirDornfeldRules
{
enum class Stage : std::uint8_t { NotStarted, CollectShards, DefeatGuardians, OpenChest, ReturnToElder, Completed };
constexpr int RequiredShards = 3;
constexpr int RequiredGuardians = 2;
constexpr int GoldReward = 90;
constexpr int ExperienceReward = 180;
constexpr bool IsValidStage(unsigned value) { return value <= static_cast<unsigned>(Stage::Completed); }
constexpr Stage NormalizeStage(unsigned savedStage, int shards, int guardians, bool chest, bool rewardClaimed)
{
    if (rewardClaimed) return Stage::Completed;
    if (!IsValidStage(savedStage) || savedStage == static_cast<unsigned>(Stage::NotStarted)) return Stage::NotStarted;
    if (shards < RequiredShards) return Stage::CollectShards;
    if (guardians < RequiredGuardians) return Stage::DefeatGuardians;
    return chest ? Stage::ReturnToElder : Stage::OpenChest;
}
constexpr bool CanCollectShard(Stage stage, bool validID, bool duplicate, int count)
{
    return stage == Stage::CollectShards && validID && !duplicate && count >= 0 && count < RequiredShards;
}
constexpr bool CanDefeatGuardian(Stage stage, bool validID, bool duplicate, int count)
{
    return stage == Stage::DefeatGuardians && validID && !duplicate && count >= 0 && count < RequiredGuardians;
}
constexpr bool CanOpenChest(Stage stage, bool validID) { return stage == Stage::OpenChest && validID; }
constexpr bool CanClaimReward(Stage stage, bool claimed, bool hasWallet, bool hasProgression, std::int64_t gold)
{
    return stage == Stage::ReturnToElder && !claimed && hasWallet && hasProgression
        && gold >= 0 && gold <= std::numeric_limits<std::int32_t>::max() - GoldReward;
}
}
