import unittest,json
from pathlib import Path
ROOT=Path(__file__).resolve().parent.parent
class EncounterPersistenceV66(unittest.TestCase):
 def test_savegame_field(self):
  self.assertIn('SavedCompletedEncounters',(ROOT/'Source/Askir/Save/AskirSaveGame.h').read_text())
 def test_save_restore(self):
  code=(ROOT/'Source/Askir/Game/AskirGameMode.cpp').read_text()
  self.assertIn('Save->SavedCompletedEncounters = Player->QuestComponent->CompletedEncounters;',code)
  self.assertIn('Player->QuestComponent->CompletedEncounters = Save->SavedCompletedEncounters;',code)
 def test_idempotent_quest_progress(self):
  code=(ROOT/'Source/Askir/World/AskirEncounterSpawner.cpp').read_text()
  self.assertIn('Quests->MarkEncounterCompleted(ID)',code)
  self.assertIn('if(bNewCompletion && !CompletionQuestID.IsNone()',code)
 def test_restart_guard(self):
  self.assertIn('Quests->HasCompletedEncounter(Zone->EncounterID)',(ROOT/'Source/Askir/World/AskirEncounterSpawner.cpp').read_text())
 def test_50_scenarios(self):
  data=json.loads((ROOT/'Production/Bosses/encounter_persistence_scenarios_v66.json').read_text())['scenarios']
  self.assertEqual(len(data),50)
  self.assertEqual([r['topic'] for r in data],list(range(2051,2101)))
  self.assertTrue(all(r['execution']=='pending_unreal_pie' for r in data))
