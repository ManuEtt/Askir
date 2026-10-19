#include "Economy/AskirMerchantComponent.h"
#include "Core/AskirCheckedMath.h"
#include "Economy/AskirEconomyRules.h"
#include "Economy/AskirMerchantStateRules.h"
#include "Game/AskirGameMode.h"
#include "Kismet/GameplayStatics.h"

UAskirMerchantComponent::UAskirMerchantComponent(){PrimaryComponentTick.bCanEverTick=false;}
bool UAskirMerchantComponent::CanAcceptTransactions() const
{
    if(bTransactionInProgress)return false;
    if(bHasInitialSnapshot && !CanRestoreSnapshot(InitialSnapshot))return false;
    const auto* Mode=GetWorld()?Cast<AAskirGameMode>(UGameplayStatics::GetGameMode(this)):nullptr;
    return !Mode || !Mode->IsPersistenceBusy();
}
bool UAskirMerchantComponent::OwnsLedgerReference(const int32& Value) const
{
    if(&Value==&MerchantGold)return true;
    for(const auto& Item:Inventory)
        if(&Value==&Item.Stock || &Value==&Item.BasePrice || &Value==&Item.MaxStock)return true;
    return false;
}
void UAskirMerchantComponent::BeginPlay()
{
    Super::BeginPlay();
    if(!CaptureInitialState())UE_LOG(LogTemp,Warning,TEXT("Askir merchant has invalid initial stock: %s"),*GetName());
}
bool UAskirMerchantComponent::HasValidCatalog() const
{
    if(static_cast<size_t>(Inventory.Num())>AskirMerchantStateRules::MaxItemsPerMerchant || !FMath::IsFinite(SellBackRatio) || SellBackRatio<=0.f)return false;
    TSet<FName> IDs;
    for(const auto& Item:Inventory)
    {
        if(Item.ItemID.IsNone() || IDs.Contains(Item.ItemID) || Item.BasePrice<=0 || Item.Stock<0 || Item.MaxStock<0)return false;
        IDs.Add(Item.ItemID);
    }
    return true;
}
bool UAskirMerchantComponent::ValidateSnapshotData(const FAskirMerchantSnapshot& Snapshot)
{
    if(!AskirMerchantStateRules::ValidSnapshot(Snapshot.Gold,static_cast<size_t>(Snapshot.StockByItem.Num())))return false;
    for(const auto& Pair:Snapshot.StockByItem)if(Pair.Key.IsNone() || !AskirMerchantStateRules::ValidAmount(Pair.Value))return false;
    return true;
}
bool UAskirMerchantComponent::CaptureSnapshot(FAskirMerchantSnapshot& OutSnapshot) const
{
    if(bTransactionInProgress || !HasValidCatalog())return false;
    FAskirMerchantSnapshot Prepared;Prepared.Gold=MerchantGold;
    for(const auto& Item:Inventory)Prepared.StockByItem.Add(Item.ItemID,Item.Stock);
    if(!ValidateSnapshotData(Prepared))return false;
    OutSnapshot=MoveTemp(Prepared);
    return true;
}
bool UAskirMerchantComponent::CaptureInitialState()
{
    if(bTransactionInProgress)return false;
    if(bHasInitialSnapshot)return CanRestoreSnapshot(InitialSnapshot);
    if(!CaptureSnapshot(InitialSnapshot))return false;
    bHasInitialSnapshot=true;
    return true;
}
bool UAskirMerchantComponent::GetInitialSnapshot(FAskirMerchantSnapshot& OutSnapshot) const
{
    if(!bHasInitialSnapshot || !CanRestoreSnapshot(InitialSnapshot))return false;
    OutSnapshot=InitialSnapshot;
    return true;
}
bool UAskirMerchantComponent::CanRestoreSnapshot(const FAskirMerchantSnapshot& Snapshot) const
{
    if(bTransactionInProgress || !HasValidCatalog() || !ValidateSnapshotData(Snapshot) || Snapshot.StockByItem.Num()!=Inventory.Num())return false;
    for(const auto& Item:Inventory)if(!Snapshot.StockByItem.Contains(Item.ItemID))return false;
    return true;
}
bool UAskirMerchantComponent::RestoreSnapshot(const FAskirMerchantSnapshot& Snapshot)
{
    if(!CanRestoreSnapshot(Snapshot))return false;
    MerchantGold=Snapshot.Gold;
    for(auto& Item:Inventory)Item.Stock=Snapshot.StockByItem.FindChecked(Item.ItemID);
    return true;
}
int32 UAskirMerchantComponent::GetUnitBuyPrice(FName ID,float Multiplier) const
{
    if(ID.IsNone()||!FMath::IsFinite(Multiplier)||!HasValidCatalog())return 0;
    for(const auto& Item:Inventory)if(Item.ItemID==ID)
        return AskirEconomyRules::UnitBuyPrice(Item.BasePrice,Multiplier);
    return 0;
}
int32 UAskirMerchantComponent::GetUnitSellPrice(FName ID,float Multiplier) const
{
    if(ID.IsNone()||!FMath::IsFinite(Multiplier)||!HasValidCatalog())return 0;
    for(const auto& Item:Inventory)if(Item.ItemID==ID)
        return AskirEconomyRules::UnitSellPrice(Item.BasePrice,Multiplier,SellBackRatio);
    return 0;
}
bool UAskirMerchantComponent::BuyFromMerchant(FName ID,int32 Quantity,int32& PlayerGold,float Multiplier)
{
    if(bTransactionInProgress||!CanAcceptTransactions()||OwnsLedgerReference(PlayerGold)||Quantity<=0||PlayerGold<0||MerchantGold<0||!CaptureInitialState())return false;
    for(auto& Item:Inventory)if(Item.ItemID==ID)
    {
        int32 Total=0;
        if(!AskirCheckedMath::TryPositiveProduct(GetUnitBuyPrice(ID,Multiplier),Quantity,Total)||Item.Stock<Quantity||Total>PlayerGold||!AskirCheckedMath::CanCredit(MerchantGold,Total))return false;
        TGuardValue<bool> Guard(bTransactionInProgress,true);
        Item.Stock-=Quantity;PlayerGold-=Total;MerchantGold+=Total;
        OnTradeCompleted.Broadcast(ID,Quantity,Total);
        return true;
    }
    return false;
}
bool UAskirMerchantComponent::SellToMerchant(FName ID,int32 Quantity,int32& PlayerGold,float Multiplier)
{
    if(bTransactionInProgress||!CanAcceptTransactions()||OwnsLedgerReference(PlayerGold)||Quantity<=0||PlayerGold<0||MerchantGold<0||!CaptureInitialState())return false;
    for(auto& Item:Inventory)if(Item.ItemID==ID)
    {
        int32 Total=0;
        if(Item.Stock<0||!AskirCheckedMath::TryPositiveProduct(GetUnitSellPrice(ID,Multiplier),Quantity,Total)||Total>MerchantGold||!AskirCheckedMath::CanCredit(PlayerGold,Total)||!AskirCheckedMath::CanCredit(Item.Stock,Quantity))return false;
        TGuardValue<bool> Guard(bTransactionInProgress,true);
        Item.Stock+=Quantity;MerchantGold-=Total;PlayerGold+=Total;
        OnTradeCompleted.Broadcast(ID,-Quantity,Total);
        return true;
    }
    return false;
}
void UAskirMerchantComponent::Restock(int32 UnitsPerItem)
{
    if(bTransactionInProgress||UnitsPerItem<=0||!CanAcceptTransactions()||!CaptureInitialState())return;
    // A restock target is not a disposal limit for goods the player sold back.
    for(auto& Item:Inventory)if(Item.Stock<Item.MaxStock)Item.Stock=int32(FMath::Min(int64(FMath::Max(0,Item.MaxStock)),int64(FMath::Max(0,Item.Stock))+UnitsPerItem));
}
