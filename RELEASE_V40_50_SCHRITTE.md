# Askir RPG V40 – Entwicklungsthemen 951–1000

Status **Code bearbeitet** bedeutet ausdrücklich nicht in Unreal kompiliert oder spielgetestet.

951. **Code bearbeitet, UE ungetestet** – Persist equipped weapon stable ID
952. **Code bearbeitet, UE ungetestet** – Persist equipped armor stable ID
953. **Code bearbeitet, UE ungetestet** – Persist weapon rating
954. **Code bearbeitet, UE ungetestet** – Persist armor rating
955. **Code bearbeitet, UE ungetestet** – Persist gear durability
956. **Code bearbeitet, UE ungetestet** – Capture equipment in save snapshot
957. **Code bearbeitet, UE ungetestet** – Restore equipment after map travel
958. **Code bearbeitet, UE ungetestet** – Reject NaN weapon rating
959. **Code bearbeitet, UE ungetestet** – Reject NaN armor rating
960. **Code bearbeitet, UE ungetestet** – Reject NaN durability
961. **Code bearbeitet, UE ungetestet** – Reject negative weapon rating
962. **Code bearbeitet, UE ungetestet** – Reject negative armor rating
963. **Code bearbeitet, UE ungetestet** – Reject durability outside 0..100
964. **Code bearbeitet, UE ungetestet** – Bump save format to v2
965. **Code bearbeitet, UE ungetestet** – Allow legacy v1 save loading
966. **Code bearbeitet, UE ungetestet** – Keep equipment values Blueprint-accessible
967. **Code bearbeitet, UE ungetestet** – Expose weapon assignment helper
968. **Code bearbeitet, UE ungetestet** – Expose armor assignment helper
969. **Code bearbeitet, UE ungetestet** – Clear item IDs when unequipping
970. **Code bearbeitet, UE ungetestet** – Add offline equipment integration tests
971. **Offen** – Validate equipment IDs against inventory
972. **Offen** – Reject incompatible armor slots
973. **Offen** – Support multi-slot equipment
974. **Offen** – Persist spell cooldowns
975. **Offen** – Persist timed buffs
976. **Offen** – Persist opened doors by map
977. **Offen** – Persist defeated unique bosses
978. **Offen** – Persist NPC schedules by region
979. **Offen** – Persist quest-driven access restrictions
980. **Offen** – Persist discovered fast travel points
981. **Offen** – Implement safe save-slot backup
982. **Offen** – Implement transactional save replacement
983. **Offen** – Implement interrupted-save recovery
984. **Offen** – Implement load snapshot rollback
985. **Offen** – Validate item assets before save
986. **Offen** – Prevent duplicate quest rewards across maps
987. **Offen** – Restore NPC interaction state
988. **Offen** – Restore chest contents
989. **Offen** – Validate cooked map whitelist
990. **Offen** – Automate 30 map cook test
991. **Offen** – Generate real .umap levels in Unreal Editor
992. **Offen** – Integrate region streaming
993. **Offen** – Test portal return spawn
994. **Offen** – Test multi-map inventory consistency
995. **Offen** – Test equipment visual meshes
996. **Offen** – Add gear durability UI
997. **Offen** – Add save migration test in UE
998. **Offen** – Run Unreal C++ build
999. **Offen** – Run automation tests in UE
1000. **Offen** – Produce Windows packaged build

**Einschränkung:** Equipment-ID ist derzeit ein stabiler FName ohne nachgewiesene Zuordnung zu realen Item-Assets; visuelle Ausrüstung und Kompatibilitätsvalidierung fehlen.
