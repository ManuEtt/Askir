import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

class DornfeldPersistenceTests(unittest.TestCase):
    def test_trigger_id_declared_savegame(self):
        text = (ROOT / 'Source/Askir/Quest/AskirQuestComponent.h').read_text()
        self.assertIn('SaveGame,Category="Askir|Quest") TSet<FName> ConsumedQuestTriggers', text)
    def test_repeated_overlap_cannot_increment(self):
        text = (ROOT / 'Source/Askir/Economy/AskirQuestTrigger.cpp').read_text()
        self.assertLess(text.index('HasConsumedQuestTrigger(PersistentTriggerID)'), text.index('AdvanceQuestObjective(QuestID,ObjectiveID,Amount)'))
        self.assertGreater(text.index('ConsumedQuestTriggers.Add(PersistentTriggerID)'), text.index('AdvanceQuestObjective(QuestID,ObjectiveID,Amount)'))
    def test_save_snapshot_roundtrip_wiring(self):
        save = (ROOT / "Source/Askir/Save/AskirSaveGame.h").read_text()
        mode = (ROOT / "Source/Askir/Game/AskirGameMode.cpp").read_text()
        self.assertIn("SavedConsumedQuestTriggers", save)
        self.assertIn("Save->SavedConsumedQuestTriggers = Player->QuestComponent->ConsumedQuestTriggers", mode)
        self.assertIn("Player->QuestComponent->ConsumedQuestTriggers = Save->SavedConsumedQuestTriggers", mode)

    def test_all_three_shards_have_stable_ids(self):
        text = (ROOT / 'Tools/build_dornfeld_playground_v74.py').read_text()
        self.assertIn("'persistent_trigger_id', 'Dornfeld_' + label", text)
        for shard in ('Shard_North', 'Shard_East', 'Shard_Well'):
            self.assertIn(shard, text)

if __name__ == '__main__': unittest.main()
