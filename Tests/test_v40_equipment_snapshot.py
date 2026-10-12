"""Equipment persistence source contracts, migrated from V40 to V77.

These verify source wiring/validation placement; native runtime execution is
required to establish actual Unreal save/load behavior.
"""
from pathlib import Path
import re
import unittest

ROOT = Path(__file__).resolve().parents[1]

def code(relative):
    return re.sub(r'\s+', '', (ROOT / 'Source/Askir' / relative).read_text())

class V40EquipmentTests(unittest.TestCase):
    def test_snapshot_fields(self):
        save = code('Save/AskirSaveGame.h')
        for field in ('SavedWeaponID', 'SavedArmorID', 'SavedWeaponRating',
                      'SavedArmorRating', 'SavedGearDurability'):
            self.assertIn(field, save)
        self.assertIn('staticconstexprint32CurrentFormatVersion=3;', save)
        self.assertIn('SaveFormatVersion=CurrentFormatVersion;', save)

    def test_roundtrip_wiring(self):
        mode = code('Game/AskirGameMode.cpp')
        for assignment in ('Save->SavedWeaponID=Gear->EquippedWeaponID;',
                           'Save->SavedArmorID=Gear->EquippedArmorID;',
                           'Save->SavedWeaponRating=Gear->WeaponRating;',
                           'Save->SavedArmorRating=Gear->ArmorRating;',
                           'Save->SavedGearDurability=Gear->Durability;'):
            self.assertIn(assignment, mode)
        restore = mode.split('AAskirGameMode::RestoreSnapshot(', 1)[1]
        self.assertIn('RestoreEquipment(Save->SavedWeaponID,Save->SavedArmorID,Save->SavedWeaponRating,Save->SavedArmorRating,Save->SavedGearDurability)', restore)
        self.assertLess(restore.index('RestoreEquipment('), restore.index('ReconcileWithInventory()'))

    def test_reject_invalid_values(self):
        validate = code('Save/AskirSaveGame.cpp')
        # Validation moved into the shared snapshot object, used for disk and travel.
        for condition in ('!FMath::IsFinite(SavedWeaponRating)', 'SavedWeaponRating<0.f',
                          '!FMath::IsFinite(SavedArmorRating)', 'SavedArmorRating<0.f',
                          '!FMath::IsFinite(SavedGearDurability)', 'SavedGearDurability<0.f',
                          'SavedGearDurability>100.f'):
            self.assertIn(condition, validate)
        self.assertIn('SaveFormatVersion<1||SaveFormatVersion>CurrentFormatVersion', validate)
        restore = code('Game/AskirGameMode.cpp').split('AAskirGameMode::RestoreSnapshot(', 1)[1]
        self.assertLess(restore.index('Save->ValidateSnapshot(Reason)'),
                        restore.index('Player->InventoryComponent->Items=MoveTemp(RestoredInventory)'))

    def test_unequip_clears_ids(self):
        equipment = code('Equipment/AskirEquipmentComponent.cpp')
        self.assertIn('EquippedWeaponID=NAME_None;EquippedArmorID=NAME_None', equipment)

    def test_inventory_resolution_precedes_equipment_restore(self):
        restore = code('Game/AskirGameMode.cpp').split('AAskirGameMode::RestoreSnapshot(', 1)[1]
        self.assertLess(restore.index('SavedEntry.ItemAsset.TryLoad()'), restore.index('RestoreEquipment('))
        self.assertIn('if(!Item||Item->MaxStackSize<1)returnfalse;', restore)
        self.assertIn('if(Total>MAX_int32)returnfalse;', restore)

    def test_save_banks_validate_before_selection_and_commit(self):
        store = code('Save/AskirSaveSlotStore.cpp')
        self.assertIn('SlotName+TEXT("_BankA")', store)
        self.assertIn('SlotName+TEXT("_BankB")', store)
        self.assertLess(store.index('Save->ValidateSnapshot(Reason)'), store.index('Latest=Save'))
        write = store.split('boolWriteNext(', 1)[1]
        self.assertLess(write.index('Snapshot->ValidateSnapshot(Reason)'), write.index('SaveGameToSlot('))
        self.assertIn('Verified->SaveGeneration==Snapshot->SaveGeneration', write)
        self.assertIn('Verified->ValidateSnapshot(Reason)', write)

if __name__ == '__main__':
    unittest.main()
