#pragma once
#include <cstdint>
#include <limits>

/** Production SQ_001 decisions, used by the runtime and portable boundary tests. */
namespace AskirDeliveryRules
{
enum class Stage : std::uint8_t { NotStarted, DefeatBandits, RecoverCargo, ReturnToTorben, Completed };
constexpr int RequiredBandits=2;
constexpr int GoldReward=16;
constexpr int ExperienceReward=100;
constexpr int ItemRewardQuantity=1;
constexpr Stage ResolveStage(bool active,bool completed,bool rewardClaimed,int creditedKills,bool recoveredCargo)
{
    if(completed || rewardClaimed)return Stage::Completed;
    if(!active)return Stage::NotStarted;
    if(creditedKills<RequiredBandits)return Stage::DefeatBandits;
    return recoveredCargo?Stage::ReturnToTorben:Stage::RecoverCargo;
}
constexpr bool CanAccept(Stage stage,bool validChoice,bool acceptedReceipt,bool canMutate)
{return stage==Stage::NotStarted && validChoice && !acceptedReceipt && canMutate;}
constexpr bool CanCreditBandit(Stage stage,bool validID,bool duplicate,int killCount,bool playerCredit,bool canMutate)
{return stage==Stage::DefeatBandits && validID && !duplicate && killCount>=0 && killCount<RequiredBandits && playerCredit && canMutate;}
constexpr bool CanRecoverCargo(Stage stage,bool validID,bool duplicate,bool canMutate)
{return stage==Stage::RecoverCargo && validID && !duplicate && canMutate;}
constexpr bool CanTurnIn(Stage stage,bool claimed,bool canMutate,bool hasRewardAsset,bool hasBag,bool hasWallet,bool hasProgression,std::int64_t gold)
{
    return stage==Stage::ReturnToTorben && !claimed && canMutate && hasRewardAsset && hasBag && hasWallet && hasProgression
        && gold>=0 && gold<=std::numeric_limits<std::int32_t>::max()-GoldReward;
}
constexpr bool ValidateReceipt(bool accepted,bool help,bool bargain,bool active,bool completed,bool rewardClaimed)
{
    if(help && bargain)return false;
    if(completed!=rewardClaimed || (active && completed))return false;
    if(accepted)return (help!=bargain) && (active || completed);
    return !help && !bargain && !active && !completed && !rewardClaimed;
}
constexpr bool ValidateEncounterReceipts(bool accepted,int kills,bool cargo,int cargoSources,bool completed)
{
    if(kills<0 || kills>RequiredBandits || cargoSources<0 || cargoSources>1)return false;
    if(!accepted && (kills!=0 || cargo || cargoSources!=0))return false;
    if(cargo && (kills!=RequiredBandits || cargoSources!=1))return false;
    if(!cargo && cargoSources!=0)return false;
    return !completed || cargo;
}
constexpr bool ValidateActiveObjectiveMirror(int killsRequired,int killsCurrent,int cargoRequired,int cargoCurrent,
    int returnRequired,int returnCurrent,int killsReceipts,bool cargoReceipt)
{
    return killsRequired==RequiredBandits && killsCurrent==killsReceipts && cargoRequired==1
        && cargoCurrent==(cargoReceipt?1:0) && returnRequired==1 && returnCurrent==0;
}
}
