from pathlib import Path
import unittest
import re
ROOT=Path(__file__).resolve().parents[1]
# Source-contract checks only: these do not execute Unreal gameplay.
def compact(text):
    return re.sub(r'\s+', '', text)
def code(path):
    return compact((ROOT/'Source/Askir'/path).read_text(encoding='utf8'))

SOURCE='\n'.join(p.read_text(encoding='utf8') for p in (ROOT/'Source/Askir').rglob('*.cpp'))
GLBS=sorted((ROOT/'ArtSource/Enhanced3D_V19').glob('SM_*.glb'))
OBJS=sorted((ROOT/'ArtSource/Enhanced3D_V19').glob('SM_*.obj'))
class AskirReleaseChecks(unittest.TestCase):
    pass
    def test_01(self):
        merchant = code('Economy/AskirMerchantComponent.cpp')
        arithmetic = code('Core/AskirCheckedMath.h')
        self.assertIn('!AskirCheckedMath::CanCredit(MerchantGold,Total)', merchant)
        self.assertIn('std::int64_t(Balance)+Amount<=std::numeric_limits<std::int32_t>::max()', arithmetic)
        self.assertIn('Balance>=0&&Amount>=0', arithmetic)
    def test_02(self):
        self.assertIn('!AskirCheckedMath::CanCredit(PlayerGold,Total)', code('Economy/AskirMerchantComponent.cpp'))
        self.assertIn('!AskirCheckedMath::CanCredit(PlayerGold,Total)', code('Economy/AskirTradeBridgeComponent.cpp'))
    def test_03(self):
        merchant = code('Economy/AskirMerchantComponent.cpp')
        restock = merchant.split('UAskirMerchantComponent::Restock(', 1)[1]
        self.assertIn('int64(FMath::Max(0,Item.Stock))+UnitsPerItem', restock)
        self.assertIn('FMath::Min(int64(FMath::Max(0,Item.MaxStock))', restock)
        self.assertIn('bTransactionInProgress||UnitsPerItem<=0', restock)
    def test_04(self):
        self.assertIn('IsFinite(Multiplier)', SOURCE)
    def test_05(self):
        self.assertIn('IsFinite(SellBackRatio)', SOURCE)
    def test_06(self):
        self.assertIn('IsFinite(PriceMultiplier)', SOURCE)
    def test_07(self):
        buy = code('Economy/AskirMerchantComponent.cpp').split('UAskirMerchantComponent::BuyFromMerchant(', 1)[1].split('UAskirMerchantComponent::SellToMerchant(', 1)[0]
        self.assertIn('Total>PlayerGold', buy)
        self.assertLess(buy.index('Total>PlayerGold'), buy.index('PlayerGold-=Total'))
        self.assertIn('TryPositiveProduct(', buy)
    def test_08(self):
        self.assertIn('IsValid(I)', SOURCE)
    def test_09(self):
        self.assertIn('IsValid(I)', SOURCE)
    def test_10(self):
        inventory = code('Inventory/AskirInventoryComponent.cpp')
        count = inventory.split('UAskirInventoryComponent::GetItemCount(', 1)[1].split('UAskirInventoryComponent::UseItem(', 1)[0]
        self.assertIn('int64Count=0', count)
        self.assertIn('FMath::Max(0,Entry.Quantity)', count)
        self.assertIn('FMath::Min(Count,int64(MAX_int32))', count)
    def test_11(self):
        self.assertIn('FindNode(Choice.NextNode)', SOURCE)
    def test_12(self):
        self.assertIn('if(bActive||!FindNode(StartNode))', SOURCE)
    def test_13(self):
        self.assertIn('FMath::Min(Hours,10000)', SOURCE)
    def test_14(self):
        self.assertIn('IsFinite(DeltaTime)', SOURCE)
    def test_15(self):
        self.assertIn('AdvanceHours(Hours)', SOURCE)
    def test_16(self):
        self.assertIn('EngineUtils.h', SOURCE)
    def test_17(self):
        self.assertIn('double(MAX_int32)', SOURCE)
    def test_18(self):
        buy = code('Economy/AskirTradeBridgeComponent.cpp').split('UAskirTradeBridgeComponent::BuyItem(', 1)[1].split('UAskirTradeBridgeComponent::SellItem(', 1)[0]
        self.assertIn('Total>PlayerGold', buy)
        self.assertLess(buy.index('Total>PlayerGold'), buy.index('PlayerInventory->Items=MoveTemp(Prepared)'))
        self.assertLess(buy.index('PrepareExchange('), buy.index('PlayerGold-=Total'))
        self.assertIn('TGuardValue<bool>MerchantGuard', buy)
        self.assertIn('TGuardValue<bool>InventoryGuard', buy)
    def test_19(self):
        self.assertIn('Quantity<=0', SOURCE)
    def test_20(self):
        self.assertIn('HasFlag(C.RequiredFlag)', SOURCE)
    def test_21(self):
        self.assertTrue((ROOT/'.gitignore').is_file())
    def test_22(self):
        self.assertTrue((ROOT/'.gitattributes').is_file())
    def test_23(self):
        self.assertTrue((ROOT/'LICENSE_NOTICE.md').is_file())
    def test_24(self):
        self.assertTrue((ROOT/'GITHUB_IMPORT.md').is_file())
    def test_25(self):
        self.assertTrue((ROOT/'.github/workflows/static-checks.yml').is_file())
    def test_26(self):
        self.assertTrue((ROOT/'Tools/validate_release.py').is_file())
    def test_27(self):
        self.assertTrue((ROOT/'Tests/test_release.py').is_file())
    def test_28(self):
        self.assertTrue((ROOT/'AskirRPG.uproject').is_file())
    def test_29(self):
        self.assertTrue((ROOT/'RELEASE_V20_50_SCHRITTE.md').is_file())
    def test_30(self):
        self.assertTrue((ROOT/'Source/Askir/Askir.Target.cs').is_file())
    def test_31(self):
        self.assertGreater(len(GLBS),0)
        self.assertEqual(GLBS[0].read_bytes()[:4],b"glTF")
    def test_32(self):
        self.assertGreater(len(GLBS),1)
        self.assertEqual(GLBS[1].read_bytes()[:4],b"glTF")
    def test_33(self):
        self.assertGreater(len(GLBS),2)
        self.assertEqual(GLBS[2].read_bytes()[:4],b"glTF")
    def test_34(self):
        self.assertGreater(len(GLBS),3)
        self.assertEqual(GLBS[3].read_bytes()[:4],b"glTF")
    def test_35(self):
        self.assertGreater(len(GLBS),4)
        self.assertEqual(GLBS[4].read_bytes()[:4],b"glTF")
    def test_36(self):
        self.assertGreater(len(GLBS),5)
        self.assertEqual(GLBS[5].read_bytes()[:4],b"glTF")
    def test_37(self):
        self.assertGreater(len(GLBS),6)
        self.assertEqual(GLBS[6].read_bytes()[:4],b"glTF")
    def test_38(self):
        self.assertGreater(len(GLBS),7)
        self.assertEqual(GLBS[7].read_bytes()[:4],b"glTF")
    def test_39(self):
        self.assertGreater(len(GLBS),8)
        self.assertEqual(GLBS[8].read_bytes()[:4],b"glTF")
    def test_40(self):
        self.assertGreater(len(GLBS),9)
        self.assertEqual(GLBS[9].read_bytes()[:4],b"glTF")
    def test_41(self):
        self.assertGreater(len(GLBS),10)
        self.assertEqual(GLBS[10].read_bytes()[:4],b"glTF")
    def test_42(self):
        self.assertGreater(len(GLBS),11)
        self.assertEqual(GLBS[11].read_bytes()[:4],b"glTF")
    def test_43(self):
        self.assertGreater(len(GLBS),12)
        self.assertEqual(GLBS[12].read_bytes()[:4],b"glTF")
    def test_44(self):
        self.assertGreater(len(OBJS),0)
        self.assertIn("\nv ", "\n"+OBJS[0].read_text(errors="replace"))
    def test_45(self):
        self.assertGreater(len(OBJS),1)
        self.assertIn("\nv ", "\n"+OBJS[1].read_text(errors="replace"))
    def test_46(self):
        self.assertGreater(len(OBJS),2)
        self.assertIn("\nv ", "\n"+OBJS[2].read_text(errors="replace"))
    def test_47(self):
        self.assertGreater(len(OBJS),3)
        self.assertIn("\nv ", "\n"+OBJS[3].read_text(errors="replace"))
    def test_48(self):
        self.assertGreater(len(OBJS),4)
        self.assertIn("\nv ", "\n"+OBJS[4].read_text(errors="replace"))
    def test_49(self):
        self.assertGreater(len(OBJS),5)
        self.assertIn("\nv ", "\n"+OBJS[5].read_text(errors="replace"))
    def test_50(self):
        self.assertGreater(len(OBJS),6)
        self.assertIn("\nv ", "\n"+OBJS[6].read_text(errors="replace"))
