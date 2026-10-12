# V77: Kampf, Schadenszuordnung und Dornfeld-Gegner

## Ausgangsbefund

Die bestehenden Kampfsysteme waren implementiert, aber in dieser Umgebung nicht mit Unreal ausgeführt. Der NPC verwendete einen eigenen unbegrenzten Schadensverursacher statt der zeitlich begrenzten Zuordnung des Combat-Components. Sein Questfortschritt lag außerhalb des Einmal-Reward-Schutzes. Projektile konnten nach ihrer Zerstörung die Zuordnung verlieren. Native NPC-Entscheidungen und ein spezialisierter Enemy-AI-Controller konnten denselben Gegner parallel angreifen lassen. Zurückkehrende Gegner konnten dasselbe entfernte Ziel sofort erneut verfolgen.

`AAskirNPCAdvanced` deklarierte außerdem die bereits geerbte reflektierte `NeedsComponent` erneut und erzeugte erneut ein Subobject mit dem Namen `Needs`. Dies war ein konkreter UHT-/Konstruktionsfehler. `FAskirMemory` deklarierte zwei reflektierte Float-Felder in einer gemeinsamen Deklaration.

## Implementierte Reparaturen

- `UAskirCombatComponent` löst beim positiven Treffer die verantwortliche Pawn über Instigator, Controller oder eine begrenzte Owner-Kette auf. Die Weak-Referenz bleibt damit unabhängig vom kurzlebigen Projektil. Direkte NPC-Treffer bleiben dem NPC zugeordnet. `GetLastDamageCauser()` berücksichtigt weiterhin das konfigurierbare Zeitfenster; `IsKillAttributedTo()` verlangt zusätzlich einen tatsächlichen Tod.
- Ein positiver Umwelttreffer ohne Verursacher löscht die Spielerzuordnung. Nullschaden, vollständiger Block, Dodge und ein deaktivierter Encounter erzeugen keine Schadensereignisse oder neue Zuordnung. Nicht endliche Schadens-/Heilwerte werden verworfen. Der Rückgabewert von `ApplyDamage()` ist der tatsächliche Gesundheitsverlust.
- Ein NPC setzt seinen Todes-Receipt vor Quest-, XP- und Inventarereignissen. Wiederholte Death-Delegates vergeben weder Questfortschritt noch XP erneut. Nur ein player-controlled Pawn erhält einen Spieler-Defeat-Questfortschritt. NPC-Kills können dem verantwortlichen NPC XP/Drop gewähren, falls er die entsprechenden Komponenten besitzt; sie geben keinem Spieler Questcredit.
- `SetEncounterActive()` versteckt und schützt inaktive Guardians, stoppt Bewegung und bricht beide Angriffspfade ab. `RestoreEncounterState()` stellt tot/lebendig ohne Death-Delegates her, setzt Reward-Receipt, Angriff, Stagger, Bewegung, Collision und Animation zurück und entfernt einen ablaufenden Lifespan. Der Missionsrelay kann damit gespeicherte Guardian-IDs korrekt wiederherstellen.
- NPCs mit `AAskirEnemyAIController` führen nicht zusätzlich den nativen Entscheidungsloop aus. Beide KI-Pfade prüfen den Spawn-Leash. Die native KI erreicht zunächst ihren Spawn, bevor sie erneut ein Ziel annimmt. Controller-Wechsel löscht Ziel-/Angriffszeit und erfasst den neuen Spawn.
- Zustandswechsel weg vom Angriff sowie Rückzug, Rückkehr und Tod brechen verzögerte Pawn-/Boss-Angriffe ab. Boss-Windups prüfen beim Ablauf Encounter, Sicht, Reichweite, Leash und ihren aktuellen KI-Zustand erneut. Stagger unterbricht auch den Angriffstimer eines spezialisierten Controllers.
- `UAskirAnimationStateComponent::ResetAfterLoad()` löscht transienten Block-/Angriffsstatus. Eine Trefferreaktion beim Blocken kehrt nach Ablauf in die Blockpose zurück. Gesundheits-Restore löscht Dodge-Immunität und Attribution.
- Advanced-NPCs verwenden die geerbte Needs-Komponente; Perception prüft World/Actor-Gültigkeit und entfernt zerstörte Erinnerungen. NPC-Interaktion verwendet die eindeutige umbenannte Klasse `UAskirNPCInteractionComponent`.

## Tests und tatsächlicher Status

| Prüfung | Status | Aussage |
|---|---|---|
| `git diff --check` | Bestanden | Keine Patch-Whitespace-Fehler zum Zeitpunkt der Prüfung. |
| `python3 Tools/validate_combat_pipeline_v55.py` | Bestanden | Bestehende JSON-Encounter-/Profilverträge (90 Encounters, 30 Regionen, 6 Profile); keine Laufzeitprüfung. |
| `python3 Tools/audit_build_readiness_v64.py` | Bestanden | JSON-/Python-/Datei-Preflight; keine Unreal-Kompilierung. |
| Neue Unreal-Automation `Askir.Combat.*` in `AskirCombatAttributionTests.cpp` | Blockiert | Unreal Engine, UHT und Unreal-Build-Werkzeuge fehlen hier. Testfälle umfassen zerstörte Projektile, Owner-Ketten, Umwelttod, Timeout, Dodge, Encounter-Gating, wiederholte Death-Delegates, player-only Questcredit, Encounter-Restore und Controller-Wiederverwendung. |
| Unreal-C++-Kompilierung / PIE-Kampf / Animations- und NavMesh-Prüfung | Blockiert | Benötigt Unreal Engine 5.6. |

Diese Änderungen sind Quellcode und vorbereitete echte Unreal-Automationstests. Sie gelten nicht als erfolgreich kompilierter oder spielgetesteter Kampf. Animation Blueprint, Montage, Charaktermodell, Audio, Bossarena und NavMesh bleiben getrennt in Unreal zu importieren/erzeugen und zu prüfen. Kein Asset oder spielbarer Level wird durch dieses Dokument behauptet.

## Nachprüfung der Automation und Missionsintegration

Die Tests verwenden jetzt `FAskirAutomationTestWorld`: ein einmalig initialisierter Runtime-World mit World-Context, Physics Scene und BeginPlay. Actors werden mit `SpawnActor` erzeugt. Der ursprüngliche Combat-Restore-Test erzeugte seinen Actor mit `NewObject`; die Mission-Fixture initialisierte einen bereits durch `CreateWorld` initialisierten World erneut. Beide Konstruktionen wurden korrigiert.

`Askir.Mission.Dornfeld.SpawnedPlayerCombatLootSaveAndMara` ergänzt einen Integrationstest mit einem besessenen nativen Spieler, Boden-Collision, Mara-Interaktion, drei physischen Scherben, zwei NPCs mit Death-Relays, echter verzögerter Melee-Trefferauflösung, Kellertruhe und Mara-Belohnung. Er serialisiert und restauriert den vollständigen `AAskirGameMode`-Snapshot nach dem ersten Guardian. Er prüft das Zurücksetzen eines später getöteten Guardians, Loot, Gold, XP und einen wiederholten Completed-Restore ohne doppelte Belohnung. Ein nur im Testprozess vorhandenes `/Game`-Item prüft die Soft-Asset-Referenz; dies ist kein geliefertes Unreal-Asset. Die Missionsbelohnung wird separat vom NPC-Kill-XP als genau 90 Gold und 180 XP geprüft.

Die drei Economy-Transaction-Tests wurden gelesen: Sie testen owner-unabhängige Komponentenfunktionen und benötigen dafür keine Actor-Spawn-Fixture. Ihre Actor-Konstruktion ist nicht betroffen. Auch die neue Integration und die korrigierten Fixtures sind **nicht ausgeführt / Engine-blockiert**.

Der sichtbare Cylinder-Fallback für NPCs wird von `AAskirCharacter` geerbt; es wird keine zweite reflektierte `ProxyVisual`-Property auf NPC angelegt. Er ist ein ausdrücklich temporäres primitives Modell ohne Skelett/Animation.
