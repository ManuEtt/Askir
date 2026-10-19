#include "AskirEquipmentComponent.h"
#include "Inventory/AskirInventoryComponent.h"
#include "Inventory/AskirItemData.h"
#include "GameFramework/Actor.h"

void UAskirEquipmentComponent::EquipArmor(float Rating) { ArmorRating=FMath::Max(0.f,Rating); }
void UAskirEquipmentComponent::EquipWeapon(float Rating) { WeaponRating=FMath::Max(0.f,Rating); }
void UAskirEquipmentComponent::DamageGear(float Amount) { Durability=FMath::Max(0.f,Durability-FMath::Max(0.f,Amount)); }
void UAskirEquipmentComponent::RepairGear(float Amount) { Durability=FMath::Min(100.f,Durability+FMath::Max(0.f,Amount)); }
void UAskirEquipmentComponent::UnequipAll() { ArmorRating=0.f;WeaponRating=0.f;EquippedWeaponID=NAME_None;EquippedArmorID=NAME_None; }
void UAskirEquipmentComponent::SetEquippedWeapon(FName ItemID,float Rating) { EquippedWeaponID=ItemID;WeaponRating=ItemID.IsNone()?0.f:FMath::Max(0.f,Rating); }
void UAskirEquipmentComponent::SetEquippedArmor(FName ItemID,float Rating) { EquippedArmorID=ItemID;ArmorRating=ItemID.IsNone()?0.f:FMath::Max(0.f,Rating); }
void UAskirEquipmentComponent::RestoreEquipment(FName WeaponID,FName ArmorID,float Weapon,float Armor,float Condition)
{
 EquippedWeaponID=WeaponID;EquippedArmorID=ArmorID;
 WeaponRating=FMath::Max(0.f,Weapon);ArmorRating=FMath::Max(0.f,Armor);
 Durability=FMath::Clamp(Condition,0.f,100.f);
}


bool UAskirEquipmentComponent::TryEquipInventoryItem(UAskirItemData* Item)
{
 const UAskirInventoryComponent* Inventory = GetOwner() ? GetOwner()->FindComponentByClass<UAskirInventoryComponent>() : nullptr;
 if (!IsValid(Item) || !Inventory || !Inventory->HasItem(Item, 1) || Item->ItemID.IsNone()) return false;
 if (Item->ItemType == EAskirItemType::Weapon)
 {
  if (!FMath::IsFinite(Item->Damage) || Item->Damage < 0.f) return false;
  SetEquippedWeapon(Item->ItemID, Item->Damage);
  return true;
 }
 if (Item->ItemType == EAskirItemType::Armor)
 {
  // Armor assets currently have no dedicated armor stat. Keep rating at zero
  // until an explicit ArmorRating field is introduced; never infer it from price.
  SetEquippedArmor(Item->ItemID, 0.f);
  return true;
 }
 return false;
}

void UAskirEquipmentComponent::ReconcileWithInventory()
{
 const UAskirInventoryComponent* Inventory = GetOwner() ? GetOwner()->FindComponentByClass<UAskirInventoryComponent>() : nullptr;
 if (!Inventory) { UnequipAll(); return; }
 bool bWeaponFound = false, bArmorFound = false;
 for (const FAskirInventoryEntry& Entry : Inventory->Items)
 {
  const UAskirItemData* Item = Entry.Item.Get();
  if (!IsValid(Item) || Entry.Quantity <= 0) continue;
  if (!EquippedWeaponID.IsNone() && Item->ItemID == EquippedWeaponID && Item->ItemType == EAskirItemType::Weapon) bWeaponFound = true;
  if (!EquippedArmorID.IsNone() && Item->ItemID == EquippedArmorID && Item->ItemType == EAskirItemType::Armor) bArmorFound = true;
 }
 if (!bWeaponFound) SetEquippedWeapon(NAME_None, 0.f);
 if (!bArmorFound) SetEquippedArmor(NAME_None, 0.f);
}
