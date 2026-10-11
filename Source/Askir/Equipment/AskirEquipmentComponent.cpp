#include "AskirEquipmentComponent.h"

void UAskirEquipmentComponent::EquipArmor(float Rating) { ArmorRating=FMath::Max(0.f,Rating); }
void UAskirEquipmentComponent::EquipWeapon(float Rating) { WeaponRating=FMath::Max(0.f,Rating); }
void UAskirEquipmentComponent::DamageGear(float Amount) { Durability=FMath::Max(0.f,Durability-FMath::Max(0.f,Amount)); }
void UAskirEquipmentComponent::RepairGear(float Amount) { Durability=FMath::Min(100.f,Durability+FMath::Max(0.f,Amount)); }
void UAskirEquipmentComponent::UnequipAll() { ArmorRating=0.f;WeaponRating=0.f; }
