from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parents[1]
class V40EquipmentTests(unittest.TestCase):
 def test_snapshot_fields(self):
  s=(ROOT/'Source/Askir/Save/AskirSaveGame.h').read_text()
  for f in ('SavedWeaponID','SavedArmorID','SavedWeaponRating','SavedArmorRating','SavedGearDurability','SaveFormatVersion = 2'):self.assertIn(f,s)
 def test_roundtrip_wiring(self):
  s=(ROOT/'Source/Askir/Game/AskirGameMode.cpp').read_text()
  for f in ('Gear->EquippedWeaponID','Gear->EquippedArmorID','Gear->RestoreEquipment(','Save->SaveFormatVersion > 2'):self.assertIn(f,s)
 def test_reject_invalid_values(self):
  s=(ROOT/'Source/Askir/Game/AskirGameMode.cpp').read_text()
  self.assertIn('FMath::IsFinite(Save->SavedGearDurability)',s)
  self.assertIn('Save->SavedGearDurability > 100.f',s)
 def test_unequip_clears_ids(self):
  s=(ROOT/'Source/Askir/Equipment/AskirEquipmentComponent.cpp').read_text()
  self.assertIn('EquippedWeaponID=NAME_None;EquippedArmorID=NAME_None',s)
if __name__=='__main__':unittest.main()
