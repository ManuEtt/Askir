import unittest
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

class DornfeldPersistenceTests(unittest.TestCase):
    def test_trigger_id_declared_savegame(self):
        text = (ROOT / 'Source/Askir/Quest/AskirQuestComponent.h').read_text()
        self.assertIn('SaveGame,Category="Askir|Quest") TSet<FName> ConsumedQuestTriggers', text)
    def test_repeated_overlap_cannot_increment(self):
        text = (ROOT / 'Source/Askir/Economy/AskirQuestTrigger.cpp').read_text()
        self.assertLess(text.index('HasConsumedQuestTrigger(PersistentTriggerID)'), text.index('AdvanceQuestObjective(QuestID,ObjectiveID,Amount)'))
        # The claim now precedes callbacks so a reentrant overlap is rejected.
        claim = text.index('ConsumedQuestTriggers.Add(PersistentTriggerID)')
        advance = text.index('AdvanceQuestObjective(QuestID,ObjectiveID,Amount)')
        self.assertLess(claim, advance)
        self.assertIn('if(!Quests->AdvanceQuestObjective(QuestID,ObjectiveID,Amount))', re.sub(r'\s+', '', text))
        self.assertGreater(text.index('ConsumedQuestTriggers.Remove(PersistentTriggerID)'), advance)
        self.assertGreater(text.index('TriggeredActors.Remove(OtherActor)'), advance)
    def test_save_snapshot_roundtrip_wiring(self):
        save = (ROOT / "Source/Askir/Save/AskirSaveGame.h").read_text()
        mode = (ROOT / "Source/Askir/Game/AskirGameMode.cpp").read_text()
        self.assertIn("SavedConsumedQuestTriggers", save)
        self.assertIn("Save->SavedConsumedQuestTriggers=Player->QuestComponent->ConsumedQuestTriggers", re.sub(r"\s+", "", mode))
        self.assertIn("Player->QuestComponent->ConsumedQuestTriggers=Save->SavedConsumedQuestTriggers", re.sub(r"\s+", "", mode))

    def test_all_three_shards_have_stable_ids(self):
        text = (ROOT / 'Tools/build_dornfeld_playground_v74.py').read_text()
        self.assertIn("'persistent_trigger_id', 'Dornfeld_' + label", text)
        for shard in ('Shard_North', 'Shard_East', 'Shard_Well'):
            self.assertIn(shard, text)

    def test_trigger_rolls_back_uncommitted_claims(self):
        text = re.sub(r'\s+', '', (ROOT / 'Source/Askir/Economy/AskirQuestTrigger.cpp').read_text())
        # Both transient and persistent claims are taken before progress delegates,
        # and both are released if the objective cannot advance.
        self.assertLess(text.index('TriggeredActors.Add(OtherActor)'), text.index('AdvanceQuestObjective('))
        self.assertIn('if(!Quests->AdvanceQuestObjective(QuestID,ObjectiveID,Amount)){', text)
        self.assertIn('if(!PersistentTriggerID.IsNone())Quests->ConsumedQuestTriggers.Remove(PersistentTriggerID);', text)
        self.assertIn('if(bOneShotPerActor)TriggeredActors.Remove(OtherActor);', text)

if __name__ == '__main__': unittest.main()
