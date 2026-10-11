# Unreal-Editor-Testplan V27

1. Projekt mit Unreal Engine 5.6 und C++-Toolchain kompilieren. Fehler zuerst beheben.
2. In einem Character-Blueprint DodgeInvulnerabilitySeconds auf 0.22 setzen; Dodge auslösen, Treffer während und nach dem Fenster vergleichen.
3. Prüfen, dass Dodge ohne genügend Ausdauer keine Immunität auslöst.
4. Guard und Dodge separat sowie kombiniert testen; TrueDamage wird bei aktivem Dodge ebenfalls ignoriert.
5. Feind-Blueprint auf AAskirNPC basieren lassen; KillExperience=20 und GuaranteedDrop auf gültiges UAskirItemData setzen.
6. Feind durch Spieler töten: XP steigt, Item erscheint im Inventar.
7. DropChance=0 testen: keine Itemvergabe. DropChance=1 testen: garantierte Vergabe bei freiem Inventar.
8. NPC ohne Killer töten: keine Vergabe. Wiederholte Todesmeldung: keine doppelte Vergabe.
9. Quest-DefeatObjectiveID testen und auf doppelten Questfortschritt prüfen.
10. Speicherstand laden: erhaltene XP und Inventargegenstände müssen über bestehende Speicherpfade geprüft werden.

**Offen:** Build, Animationen, VFX, Loot-UI, Multiplayer-Autorität und tatsächlicher Editor-Test.
