#include "Inventory/AskirInventoryComponent.h"
#include "Inventory/AskirItemData.h"
#include "Characters/AskirCharacter.h"
UAskirInventoryComponent::UAskirInventoryComponent(){PrimaryComponentTick.bCanEverTick=false;}
bool UAskirInventoryComponent::AddItem(UAskirItemData* I,int32 Q)
{
    if(!IsValid(I)||Q<=0)return false; int32 R=Q; int32 Max=FMath::Max(1,I->MaxStackSize);
    for(auto& E:Items)if(E.Item==I&&E.Quantity<Max){int32 A=FMath::Min(Max-E.Quantity,R);E.Quantity+=A;R-=A;if(R<=0)break;}
    while(R>0){FAskirInventoryEntry E;E.Item=I;E.Quantity=FMath::Min(Max,R);R-=E.Quantity;Items.Add(E);}
    OnInventoryChanged.Broadcast();return true;
}
bool UAskirInventoryComponent::RemoveItem(UAskirItemData* I,int32 Q)
{
    if(!IsValid(I)||Q<=0||!HasItem(I,Q))return false;int32 R=Q;
    for(int32 n=Items.Num()-1;n>=0&&R>0;--n)if(Items[n].Item==I){int32 A=FMath::Min(Items[n].Quantity,R);Items[n].Quantity-=A;R-=A;if(Items[n].Quantity<=0)Items.RemoveAt(n);}
    OnInventoryChanged.Broadcast();return true;
}
bool UAskirInventoryComponent::HasItem(UAskirItemData* I,int32 Q)const{return GetItemCount(I)>=Q;}
int32 UAskirInventoryComponent::GetItemCount(UAskirItemData* I)const{int64 C=0;if(!IsValid(I))return 0;for(const auto&E:Items)if(E.Item==I)C+=FMath::Max(0,E.Quantity);return int32(FMath::Min(C,int64(MAX_int32)));}
bool UAskirInventoryComponent::UseItem(UAskirItemData* I)
{
    if(!I||!HasItem(I))return false; AAskirCharacter*C=Cast<AAskirCharacter>(GetOwner());if(!C)return false;
    if(I->ItemType==EAskirItemType::Consumable){C->Heal(I->HealAmount);return RemoveItem(I,1);}return false;
}
float UAskirInventoryComponent::GetTotalWeight()const{float W=0;for(const auto&E:Items)if(E.Item)W+=E.Item->Weight*E.Quantity;return W;}