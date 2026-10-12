#include "Inventory/AskirInventoryComponent.h"
#include "Inventory/AskirItemData.h"
#include "Characters/AskirCharacter.h"

UAskirInventoryComponent::UAskirInventoryComponent(){PrimaryComponentTick.bCanEverTick=false;}

bool UAskirInventoryComponent::PrepareExchange(const TArray<FAskirInventoryEntry>& RemovedItems,const TArray<FAskirInventoryEntry>& AddedItems,TArray<FAskirInventoryEntry>& PreparedItems) const
{
    if(bTransactionInProgress || (RemovedItems.IsEmpty() && AddedItems.IsEmpty()))return false;
    const int32 SlotLimit=FMath::Clamp(MaxInventorySlots,1,4096);
    if(Items.Num()>SlotLimit)return false;
    TMap<UAskirItemData*,int64> Counts,Removals,Additions;
    for(const auto& Entry:Items)
    {
        if(!IsValid(Entry.Item)||Entry.Quantity<=0||Entry.Quantity>FMath::Max(1,Entry.Item->MaxStackSize))return false;
        int64& Count=Counts.FindOrAdd(Entry.Item.Get());
        Count+=Entry.Quantity;
        if(Count>MAX_int32)return false;
    }
    auto Aggregate=[](const TArray<FAskirInventoryEntry>& Entries,TMap<UAskirItemData*,int64>& Totals)
    {
        for(const auto& Entry:Entries)
        {
            if(!IsValid(Entry.Item)||Entry.Quantity<=0)return false;
            int64& Count=Totals.FindOrAdd(Entry.Item.Get());
            Count+=Entry.Quantity;
            if(Count>MAX_int32)return false;
        }
        return true;
    };
    if(!Aggregate(RemovedItems,Removals)||!Aggregate(AddedItems,Additions))return false;
    for(const auto& Pair:Removals)
    {
        int64& Count=Counts.FindOrAdd(Pair.Key);
        if(Count<Pair.Value)return false;
        Count-=Pair.Value;
    }
    for(const auto& Pair:Additions)
    {
        int64& Count=Counts.FindOrAdd(Pair.Key);
        Count+=Pair.Value;
        if(Count>MAX_int32)return false;
    }
    PreparedItems=Items;
    for(const auto& Pair:Removals)
    {
        int64 Remaining=Pair.Value;
        for(int32 Index=PreparedItems.Num()-1;Index>=0&&Remaining>0;--Index)
        {
            auto& Entry=PreparedItems[Index];
            if(Entry.Item!=Pair.Key)continue;
            const int32 Taken=int32(FMath::Min(int64(Entry.Quantity),Remaining));
            Entry.Quantity-=Taken;Remaining-=Taken;
            if(Entry.Quantity==0)PreparedItems.RemoveAt(Index);
        }
    }
    for(const auto& Pair:Additions)
    {
        int64 Remaining=Pair.Value;
        const int32 StackLimit=FMath::Max(1,Pair.Key->MaxStackSize);
        for(auto& Entry:PreparedItems)
        {
            if(Entry.Item!=Pair.Key||Entry.Quantity>=StackLimit)continue;
            const int32 Added=int32(FMath::Min(int64(StackLimit-Entry.Quantity),Remaining));
            Entry.Quantity+=Added;Remaining-=Added;
            if(Remaining==0)break;
        }
        const int64 NewSlots=(Remaining+StackLimit-1)/StackLimit;
        if(NewSlots>SlotLimit-PreparedItems.Num())return false;
        while(Remaining>0)
        {
            FAskirInventoryEntry Entry;
            Entry.Item=Pair.Key;Entry.Quantity=int32(FMath::Min(int64(StackLimit),Remaining));
            PreparedItems.Add(Entry);Remaining-=Entry.Quantity;
        }
    }
    return true;
}

bool UAskirInventoryComponent::ExchangeItems(const TArray<FAskirInventoryEntry>& RemovedItems,const TArray<FAskirInventoryEntry>& AddedItems)
{
    TArray<FAskirInventoryEntry> Prepared;
    if(!PrepareExchange(RemovedItems,AddedItems,Prepared))return false;
    TGuardValue<bool> Guard(bTransactionInProgress,true);
    Items=MoveTemp(Prepared);
    OnInventoryChanged.Broadcast();
    return true;
}
bool UAskirInventoryComponent::CanExchangeItems(const TArray<FAskirInventoryEntry>& RemovedItems,const TArray<FAskirInventoryEntry>& AddedItems) const
{
    TArray<FAskirInventoryEntry> Prepared;
    return PrepareExchange(RemovedItems,AddedItems,Prepared);
}
bool UAskirInventoryComponent::AddItem(UAskirItemData* Item,int32 Quantity)
{
    FAskirInventoryEntry Entry;Entry.Item=Item;Entry.Quantity=Quantity;
    return ExchangeItems({}, {Entry});
}
bool UAskirInventoryComponent::RemoveItem(UAskirItemData* Item,int32 Quantity)
{
    FAskirInventoryEntry Entry;Entry.Item=Item;Entry.Quantity=Quantity;
    return ExchangeItems({Entry}, {});
}
bool UAskirInventoryComponent::HasItem(UAskirItemData* Item,int32 Quantity) const
{return IsValid(Item)&&Quantity>0&&GetItemCount(Item)>=Quantity;}
int32 UAskirInventoryComponent::GetItemCount(UAskirItemData* Item) const
{
    int64 Count=0;if(!IsValid(Item))return 0;
    for(const auto& Entry:Items)if(Entry.Item==Item)Count+=FMath::Max(0,Entry.Quantity);
    return int32(FMath::Min(Count,int64(MAX_int32)));
}
bool UAskirInventoryComponent::UseItem(UAskirItemData* Item)
{
    AAskirCharacter* Character=Cast<AAskirCharacter>(GetOwner());
    if(!Character||!IsValid(Item)||Item->ItemType!=EAskirItemType::Consumable||!FMath::IsFinite(Item->HealAmount)||Item->HealAmount<=0.f)return false;
    if(!RemoveItem(Item,1))return false;
    Character->Heal(Item->HealAmount);
    return true;
}
float UAskirInventoryComponent::GetTotalWeight()const
{
    double Weight=0;
    for(const auto& Entry:Items)if(IsValid(Entry.Item)&&FMath::IsFinite(Entry.Item->Weight))Weight+=FMath::Max(0.f,Entry.Item->Weight)*double(FMath::Max(0,Entry.Quantity));
    return float(FMath::Min(Weight,double(MAX_flt)));
}
