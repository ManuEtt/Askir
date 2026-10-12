# ASKIR RPG – technischer Bestandsaudit der V76-Ausgangsbasis

Stand: 12. Oktober 2026. Dieser Bericht bewertet die unveränderte, vor Änderungen gesicherte Datei `backups/AskirRPG_V76_Original.zip`. Er beschreibt ausdrücklich die Ausgangsbasis V76. Parallel entstandene V77-Änderungen sind damit weder abgenommen noch als weiterhin fehlerhaft eingestuft; ihr Status gehört in den V77-Änderungs- und Testbericht.

## Ergebnis und Nachweisgrenze

V76 ist ein kleines C++-Quellprojekt mit Produktionsdaten, Rohgeometrie und Editor-Automationsskripten. Es enthält **keine einzige echte Unreal-Karte und kein einziges Unreal-Asset**. Das ZIP enthält 483 Dateien mit zusammen 2.704.104 unkomprimierten Byte. Ein tatsächlich spielbarer Unreal- oder Windows-Build kann aus dem vorhandenen Archiv nicht nachgewiesen werden.

Alle **431 vorhandenen Python-Unittests** bestanden in der isoliert entpackten Originalversion. Alle 83 Python-Dateien sind syntaxgültig; alle 44 JSON-Dateien sind parsebar. Diese Ergebnisse prüfen Daten, Python-Referenzmodelle und Quelltextverträge. Sie führen weder Unreal Header Tool noch Unreal Build Tool aus und beweisen keine Spielbarkeit. Die Tests erkennen insbesondere die unten belegten reflektierten Namenskollisionen und privaten C++-API-Aufrufe nicht.

Die aktuelle Umgebung läuft unter Linux. `UnrealEditor`, `UnrealEditor-Cmd`, `UnrealBuildTool`, `RunUAT.sh` und der Windows-Compiler `cl` sind im PATH nicht verfügbar. `g++`, Git und Blender stehen zur Verfügung. Ein normaler Linux-C++-Compiler kann ein Unreal-Modul ohne Engine-Header, generierte Reflection-Dateien, UBT und Engine-Bibliotheken nicht prüfen. Native `.umap`-/`.uasset`-Erstellung, UE-Kompilierung, PIE, Cook, Windows-Paketierung, Paketstart und FPS-Messungen sind deshalb blockiert. Blender-Rohasset-Erstellung bleibt unabhängig ausführbar.

Die vollständige Ausgangsdateiliste mit Größen und SHA-256, die GLB-Strukturprüfung und eine Einzeltestmatrix aller 30 vorgesehenen Levels stehen in `Reports/V77_inventory.json`. Der Audit beansprucht keine Prüfung nicht mitgelieferter Repository-Dateien.

## Kategorien

| Kategorie | Bedeutung |
|---|---|
| 1 | Vollständig implementiert und erfolgreich getestet; die konkrete Prüfart muss angegeben sein |
| 2 | Implementiert, aber ungetestet in Unreal beziehungsweise der Zielplattform |
| 3 | Teilweise implementiert; wesentliche Funktion oder Integration fehlt |
| 4 | Nur Daten, Plan, Konzept oder Generierungsanweisung vorhanden |
| 5 | Fehlerhaft; konkreter Quell-/Referenzbefund vorhanden |
| 6 | Vollständig fehlendes benötigtes Artefakt oder System |

**Kein vollständiges Gameplay-System erhält Kategorie 1.** Kategorie 1 gilt hier ausschließlich für die genannten erfolgreichen Offline-Prüfungen. Quellcodevorhandensein und grüne Texttests werden nicht zur Runtime-Abnahme umgedeutet.

## Ordner, Engine, Module und Build

| Bereich | Bestandsnachweis | Einstufung |
|---|---|---|
| Projekt | `AskirRPG.uproject`, EngineAssociation `5.6`, Runtime-Modul `Askir` | 2 |
| Build-Regeln | `Source/Askir/Askir.Build.cs`, Game-/Editor-Targets, BuildSettings V5, IncludeOrder Unreal5_6 | 2 |
| Modulabhängigkeiten | Core, CoreUObject, Engine, InputCore, EnhancedInput, AIModule, NavigationSystem, GameplayTasks, UMG, Slate, SlateCore | 2; Abhängigkeit allein ist keine Systemintegration |
| Plugins | Keine explizite Plugins-Liste im `.uproject`; Python-Editor-Tools sind vorhanden, Python-/Editor-Scripting-Aktivierung wird nicht deklariert | 3 |
| Startup-Konfiguration | `/Engine/Maps/Entry`, fehlendes `/Game/Askir/Blueprints/Core/BP_AskirGameMode.BP_AskirGameMode_C` | 5 |
| C++ | 67 Header, 65 Implementierungen, drei Build-/Target-Dateien | 2 mit belegten Befunden Kategorie 5 |
| Content | Ordner fehlt; 0 `.uasset`, 0 `.umap` | 6 |
| Production | 47 Dateien, überwiegend JSON-/CSV-Design- und Profilbeschreibungen | 4 |
| Tools | 50 Python-Dateien, vielfach Preview/Dry Run als Default | 2 als Skriptquellen; erzeugte UE-Ausgaben 6 |
| Tests | 33 Python-Testdateien und weitere Review-Berichte; zwei native Automation-Testquellen | Offline 1; native Ausführung blockiert |
| Berichte und Root-Dokumente | Viele historische Release- und Testplan-Dateien | 4; historische Behauptungen sind keine aktuelle Ausführungsevidenz |
| Versionierung | `.gitignore`, `.gitattributes`, GitHub-Workflow vorhanden | 3; `.git`-Historie fehlt im ZIP, Binärattribute sind keine Git-LFS-Filter |

Der GitHub-Workflow läuft auf Ubuntu und führt ausschließlich `validate_release.py` sowie Python-unittest aus. UE-Kompilierung, Blueprint-Kompilierung, MapCheck und Windows-Paketstart werden dadurch nicht abgedeckt. Die Target-Dateien liegen innerhalb des Modulordners; die Zielerkennung muss beim ersten UBT-Lauf zusätzlich geprüft werden.

## Belegte Fehler der Ausgangsbasis

Die folgenden Befunde sind anhand des Originalquelltexts reproduzierbar. Es wurde kein Unreal-Kompilierlauf ausgeführt; „Buildblocker“ bezeichnet hier die verletzte C++-/Reflection-Struktur, keine angebliche Compilerlog-Ausgabe.

| ID | Schwere | Befund und Evidenz | Erforderliche Reparatur |
|---|---|---|---|
| B01 | Buildblocker | Sowohl `Interaction/AskirInteractionComponent.h/.cpp` als auch `NPC/AskirInteractionComponent.h/.cpp` definieren `UAskirInteractionComponent`, einschließlich desselben Konstruktors, mit verschiedenen APIs. | Klassen eindeutig benennen, Includes und Call Sites migrieren. |
| B02 | Buildblocker | `FAskirScheduleEntry` wird in `Simulation/AskirScheduleComponent.h` und `NPC/AskirDailyRoutineComponent.h` als verschiedene reflektierte Strukturen definiert. | Eindeutiger USTRUCT-Name für das Simulationsmodell. |
| B03 | Buildblocker | `Characters/AskirCharacter.h:149–150` deklariert `ResolvePendingAttackImpact` und `HasPendingAttack` unter `private:`. Enemy-/Boss-Controller und `Animation/AskirAttackImpactNotify.cpp:10` rufen sie von außen auf. | Öffentliche, schmale Gameplay-API herstellen. |
| B04 | UHT-Blocker | `AskirCharacter.h:146–151`: vier private BlueprintReadWrite/-ReadOnly-Properties ohne `meta=(AllowPrivateAccess="true")`. | Zugriff/Metadaten konsistent machen. |
| B05 | UHT-Blocker | Mehrere Variablen pro einzelner UPROPERTY, z.B. QuestData `Title,Description`, `GoldReward,ExperienceReward`; Needs `Hunger,Thirst,Fatigue,Stress,Safety`; Schedule `StartMinute,EndMinute`; Relationship `Affinity,Trust,Fear,Respect,Suspicion`; Perception `Threat,Confidence`; Economy `BasePrice,CurrentPrice`. | Für jedes reflektierte Feld eine eigene UPROPERTY-Deklaration. |
| B06 | Startblocker | `Config/DefaultEngine.ini` verweist auf einen nicht existierenden Blueprint-GameMode. | Vorhandenen nativen GameMode einstellen; echten Startlevel erzeugen und validieren. |
| B07 | Laufzeitblocker | `Game/AskirGameModeDemo.cpp:62,103` verwendet `ConstructorHelpers::FObjectFinder` in `MakeBlock`/`BuildDemoScene` nach Konstruktion. ConstructorHelpers sind für UObject-Konstruktionskontext bestimmt. | Mesh im Konstruktor laden/cachen oder reguläres Laufzeit-Asset-Loading verwenden. |
| B08 | Interaktionsblocker | `Characters/AskirCharacter.cpp:205` prüft in `DemoInteractInput` nur NPC-Komponenten. Das `IAskirInteractable`-Interface wird dort nicht ausgeführt. LootPickup, LootChest, QuestGiver und DornfeldEventActor implementieren genau dieses Interface. | Einheitliche E-Interaktion mit Interface-Aufruf, Reichweiten-/Sichtprüfung und geeignetem Trace. |
| B09 | Questblocker | Shard-/Chest-Interaktionen verbrauchen das Objekt vor `OnCollected`/`OnOpened`. Der Relay ruft anschließend Mission-Code auf, der Ereignisse außerhalb der aktuellen Stage verwirft. Früh eingesammelte Scherben, früh geöffnete Truhe und früh getötete Wächter können Fortschritt dauerhaft verhindern. | Mission-Stages vor Verbrauch prüfen oder frühe Ereignisse sicher persistent übernehmen; Wiederaufnahme testen. |
| B10 | Belohnungsfehler | `Characters/AskirNPC.cpp:208,222,233` nutzt eine eigene zeitlich unbegrenzte `LastDamageCauser`-Referenz. Der CombatComponent besitzt dagegen das 8-Sekunden-Killfenster; Projectile Owner/Instigator werden bei NPC-Belohnungen nicht aufgelöst. | Gemeinsame Killzuordnung mit Ablauf, Spieler-/NPC-/Projektile-/Umweltschadenfällen. |
| B11 | Save-/Load-Fehler | `SavedMapName` wird gespeichert, aber `LoadGame` wechselt nicht zur gespeicherten Region und prüft deren Übereinstimmung nicht. Gespeicherte Position kann in falsche Karte gesetzt werden. | Mapwechsel und Snapshot-Ankunft koordiniert behandeln. |
| B12 | Wiederbelebungsfehler | `HandleDeath` deaktiviert Movement/Kapselkollision und setzt AnimationState auf tot. `LoadGame` restauriert lediglich Health/Stamina; die genannten Zustände werden nicht rückgängig gemacht. | Vollständiges Character-Lifecycle-Restore statt rein numerischem Health-Restore. |
| B13 | Fortschritts-/Overflow-Risiko | `Trade/AskirTradeComponent.cpp::EarnGold` addiert int32 ohne Überlaufgrenze; mehrere Hilfskomponenten verändern Zähler ohne gemeinsame Transaktionsprüfung. | Grenzen und atomare Transaktionen an zentraler Stelle. |
| B14 | Kampf-/Ausrüstungsintegration | Melee berechnet `BaseAttackDamage`, ohne `EquipmentComponent->WeaponRating` einzubeziehen. Inventory-Armor wird explizit mit Rating 0 ausgerüstet. | Reale Itemstatistiken, Damage-/Armor-Anbindung und Restore-Reconciliation. |
| B15 | KI-Integrationsrisiko | `AAskirNPC::Tick` enthält eigene Angriffsentscheidungen. Separat kann `AAskirEnemyAIController::Think` ebenfalls `Attack()` auslösen. Bei gemeinsamer Zuweisung drohen konkurrierende Entscheidungen. | Eine zuständige Kampf-KI pro Pawn konfigurieren und Doppelangriffe prüfen. |
| B16 | Dornfeld-Editorintegration | `build_dornfeld_playground_v74.py` erzeugt Mara als NPC, Scherben als QuestTrigger und Guardians, bindet aber keine DornfeldMissionRelays, keine Mara-Missionsinteraktion und keine reale Loot-Contents-/Itemzuweisung. | Vollständige Actor-/Mission-/UI-/Save-Integration; keine Abnahme des Skriptplans. |

## Gameplay-Systeme: Implementierung und Lücken

| System | Kategorie | Konkreter Bestand / fehlender Nachweis |
|---|---|---|
| Third-Person-Bewegung/Kamera | 3 | ACharacter, SpringArm, Kamera, WASD, Maus, Jump, Sprint vorhanden. Legacy-Input statt Enhanced Input; keine zugewiesenen Skeletal-/Animation-Assets, Sprintkosten/Target Lock/Weapon Switch fehlen. |
| Enhanced Input | 6 als Integration | Nur Build-Abhängigkeit; kein UEnhancedInputComponent, UInputAction, UInputMappingContext oder Subsystem-Binding. |
| Nahkampf/Stamina/Block/Dodge | 3 | Timer-/Notify-Trefferfenster, Sweeps, leichte/schwere Treffer, Blockkosten, Dodge-Invulnerability, Resistances und Death-Delegates vorhanden. Kombos, kritische Treffer, konsistente Ausrüstungsstats, Statussystem und Runtime-Belege fehlen. |
| Fernkampf/Projektilsystem | 6 | Keine Projectile-Actor- oder Ranged-Weapon-Implementierung in Source. Gegnertyp-Daten allein bewirken keinen Fernkampf. |
| Magie | 4 | 24 JSON-/CSV-Zauber und FAskirProductionSpellRow; kein Mana, Casting, Cooldown-Ledger, Runtime-Effekt, VFX, Audio oder Spell-UI. |
| Gegner-KI | 3 | Native EnemyAIController mit Patrouille, Sicht-/Hör-/Memory-/Chase-/Retreat-/Return-Logik; NPC eigene Windup-/Stagger-Logik. Kein NavMesh-Asset/Level, kein Gruppen-/Fern-/Zauberkampf-Nachweis. |
| Bosskampf | 3 | BossController mit drei HP-Phasen, Telegraph, Interrupt, Montage-Fallback; Profil-JSON. Arenen, Bossmodelle, Animationen, besondere Attack-Muster, VFX und echte Kämpfe fehlen. |
| Questkern | 3 | QuestData, mehrfache ObjectivesByQuest, Claim-Ledger, QuestGiver/Trigger, Encounter-Dedup. Keine importierten Questassets oder vollständigen Missionplatzierungen, kein Journal/HUD; Failure/Branch-Autorierung nicht abgeschlossen. |
| Dorfmission | 3, konkrete Blocker 5 | Eigene Dornfeld-Zustandsmaschine, 3 Unique Shards, 2 Guardians, Chest, 90 Gold/180 XP, Snapshot, Relay. Es fehlen echter Level und vollständige Interaktions-/Actor-Anbindung; B08/B09/B16 verhindern Abnahme. |
| Hauptgeschichte/Enden | 4 | 5 Kapitel, 20 Missions-IDs und drei Endentexte in JSON; kein zentraler Campaign-Director und kein durchspielbarer Ending-Pfad. |
| Nebenquests | 4 als Spielinhalt | 24 Design-Datensätze, teils generische `target_001`-Ziele, 24 Dialogentwürfe; keine 20 platzierten, vollständig spielbaren Nebenquests. |
| Inventar/Stacks/Consumables | 3 | Add/Remove/Count/Weight/Healing implementiert; ItemData-Rohklassen. Keine Item-Assets, Tooltips, UI, Ausrüstungsslots oder getestete Load-Migration. |
| Loot/Truhen | 3 | Interact-Interface, Contents, PersistentLootID, Collected/Opened-State, Slot-Ledger vorhanden; realer Item-/Mesh-/Actorbestand und Mission-Gates fehlen. |
| Ausrüstung | 3 | Weapon/Armor IDs, Rating/Durability, Inventory-Reconciliation; nur einfache zwei Ausrüstungsgruppen und ArmorRating=0. |
| Crafting | 3 Kern / 4 Rezepte | Native Komponente zählt `Materials`, `CraftedItems`, `CraftSkill`; die 20 JSON-Rezepte werden nicht geladen, Zutaten-/Output-/Station-/Unlock-System fehlt. |
| Handel | 3 | Merchant/Economy/TradeBridge mit Stock-/Gold-/Preisprüfungen, Buy/Sell und Rollback. Keine Händlerassets/Inventare/UI; verschiedene Wallet-/Marketmodelle müssen vereinheitlicht und persistent werden. |
| Dialoge | 3 | Node-/Choice-/Flag-Laufzeit mit Filter und Weiterleitung. JSON nicht geladen, kein Dialogwidget, keine Quest-/Faction-Effects für JSON-Choices; Flags liegen an NPC-Komponente, Save liest Player-Komponente. |
| Fraktionen/NPC-Beziehungen | 3 | Reputation/Faction/Relationship-Komponenten; Player erzeugt keine Reputation-/Dialogue-Komponente, obwohl Save diese optional sucht. Glaubwürdige Weltreaktionen nicht integriert. |
| Tagesabläufe/Patrouillen | 3 | DailyRoutine, Clock, CivilianAIController, tagged destinations vorhanden. Keine residents/schedules/navmesh/World-Actors in einem echten Level. |
| Tag/Nacht | 3 | Spieluhr/DaylightFactor/Events; keine dynamisch daran gebundene Sonne-/Sky-/Lighting-Welt. |
| Wetter/Zufallswelt/Wildtiere | 6 als vollständige Systeme | Keine entsprechende Wetter- oder Wildtier-Laufzeitimplementierung; generische Encounter-Quellen sind kein vollständiges Umweltsystem. |
| Reisen/Portale | 3 | Package-Existenzcheck, TravelSnapshot, reciprocal arrival. 30 Karten fehlen; questabhängige Zugänge, Schiff/Schnellreise und Abkunftstests fehlen. |
| Save/Load | 3, konkrete Fehler 5 | Version 2, SoftObjectPath-Inventory, Quests/Gold/Equipment/Dornfeld/Clock/Loot Ledger. Health-/Map-/Dialogue-Lücken siehe B11/B12; Mana/Spells/Crafting/Merchant-Stock und vollständige NPC-Zustände fehlen. |
| UMG/Oberflächen/Menüs | 6 | UMG/Slate sind Modulabhängigkeiten. Keine UUserWidget-Klasse, kein CreateWidget, kein Widget Blueprint. Debug-OnScreenMessages im Demo-GameMode sind kein UI. |
| Simulation/Survival/Stealth/Companions/Camp | 3 | Kleine Zahlen-/Zustandskomponenten und CharacterIntegration-Hilfsaufrufe. Keine vollständigen visuellen, KI-, Equipment- oder Gameplayabläufe; Zahleneffekte sind keine fertigen Spielsysteme. |

## Assets, Animation, Audio und Rechte

Das Archiv enthält zwei Varianten eines Rohasset-Satzes mit jeweils 13 Modellen (Hero, Guard, Merchant, Raider sowie neun Requisiten/Environment-Modelle), je als OBJ und GLB. Das sind **26 OBJ und 26 GLB**, keine 52 unterschiedlichen Spielobjekte. Alle 26 GLB haben gültige glTF-2-Header und passende Dateilängen. Zusammengenommen enthalten sie **0 Skins, 0 Animationen und 0 Texturen**. Materialfarben sind Rohmodellmaterialien, keine Unreal-Materialassets. Diese Strukturprüfung bewertet keine Deformations-/Animations-/Gameplayqualität.

`ArtSource/Realistic3D_V1/character_manifest.json` kennzeichnet die Charaktere zutreffend als `rigged:false`, `animated:false`. Das Blender-Rigging-Skript beschreibt ein 18-Bone-Rig, starre Regionsweights und neun Actions pro Rolle, aber V76 liefert **0 FBX, keine .blend-Datei und keine exportierten Animationsclips**. Jump, Sprint und Casting gehören nicht zu diesen neun geplanten Clips. Native AnimInstance/AnimationState/Notify sind Quellcode; Montage-/Skeleton-/AnimBlueprint-Assets fehlen vollständig.

| Artefakt | V76-Bestand | Kategorie |
|---|---:|---|
| OBJ-/GLB-Prototypgeometrie | 26 + 26 Dateien | 2; Strukturprüfung bestanden, UE-Integration ungetestet |
| Unreal Static-/Skeletal Mesh, Skeleton, AnimBlueprint, Montage | 0 | 6 |
| Texturen, Materialassets, Niagara-Systeme | 0 | 6 |
| Musik, Soundeffekte, Umgebungsgeräusche | 0 | 6 |
| Produktions-Assetmanifest | 436 Einträge, keine entsprechenden Dateien | 4 |
| Lizenzübersicht | LICENSE_NOTICE mit Projekterzeugungshinweis | 3 |

`LICENSE_NOTICE.md` bezeichnet aktuelle ArtSource-Modelle als projektgenerierte Prototypgeometrie und behauptet keine Drittanbieter-Lizenz. Es gibt keine mitgelieferten externen Texturen/Audios/Fonts. Für spätere externe Integration sind konkrete Quelle und Lizenz je Asset zu erfassen. Die Softwarelizenz ist nicht festgelegt. Eine uneingeschränkte rechtliche Freigabe des ganzen Projekts lässt sich aus diesem technischen Audit nicht ableiten.

## Levelbestand und Einzel-Abnahme

Die 30 Regionen sind ein JSON-Manifest mit Verbindungen, Kontinent, Kapitel, Position, POIs und Seeds. Alle 30 besitzen `implementation_status=editor_generation_pending`. V42-Generatoren beschreiben selbst „blockout maps“ und erzeugen primitive Würfel sowie Exit-Marker. Gameplay-Tools benötigen weitere native Klassen, Enemy-Blueprints und manuelle Quest-ID-/Objective-Zuordnung. Kein Generator ersetzt eine erfolgreich gespeicherte, geladene und getestete `.umap`.

In V76 scheitert für **jeden Level die Dateiexistenz-Prüfung**. Load-/Spawn-/Collision-/NPC-/Quest-/Transition-/Save-Tests sind blockiert, Performance nicht ausgeführt. Damit beträgt die Zahl erfüllter Levelabnahmen **0/30**. Die maschinenlesbare Einzelmatrix enthält auch jedem Level seine beabsichtigte Package-Adresse und die separat blockierten Tests.

| ID | Vorgesehener Level | Kontinent | Kapitel | .umap | Runtime-Tests |
|---|---|---|---:|---|---|
| L01 | L01_Dornfeld | Ardem | 1 | Fehlt | Blockiert |
| L02 | L02_Dornfelder_Umland | Ardem | 1 | Fehlt | Blockiert |
| L03 | L03_Grauwald | Ardem | 1 | Fehlt | Blockiert |
| L04 | L04_Alte_Brunnenkrypta | Ardem | 1 | Fehlt | Blockiert |
| L05 | L05_Verlassene_Silbermine | Ardem | 1 | Fehlt | Blockiert |
| L06 | L06_Falkenwacht | Ardem | 1 | Fehlt | Blockiert |
| L07 | L07_Nordpass | Ardem | 2 | Fehlt | Blockiert |
| L08 | L08_Ruinen_von_Eldrath | Ardem | 2 | Fehlt | Blockiert |
| L09 | L09_Nebelmoor | Ardem | 2 | Fehlt | Blockiert |
| L10 | L10_Krypta_der_Sieben | Ardem | 2 | Fehlt | Blockiert |
| L11 | L11_Verlorenes_Kloster | Ardem | 2 | Fehlt | Blockiert |
| L12 | L12_Kuestenhafen_Ardem | Ardem | 2 | Fehlt | Blockiert |
| L13 | L13_Hafen_von_Thalor | Thalor | 3 | Fehlt | Blockiert |
| L14 | L14_Sonnenebene | Thalor | 3 | Fehlt | Blockiert |
| L15 | L15_Kristallhoehlen | Thalor | 3 | Fehlt | Blockiert |
| L16 | L16_Vergessene_Bibliothek | Thalor | 3 | Fehlt | Blockiert |
| L17 | L17_Wuestenruinen_SarKel | Thalor | 3 | Fehlt | Blockiert |
| L18 | L18_Tempel_des_Zweiten_Siegels | Thalor | 3 | Fehlt | Blockiert |
| L19 | L19_Grenzfestung_Velis | Velis | 4 | Fehlt | Blockiert |
| L20 | L20_Verbrannte_Lande | Velis | 4 | Fehlt | Blockiert |
| L21 | L21_Schwarzwald_Velis | Velis | 4 | Fehlt | Blockiert |
| L22 | L22_Unterirdische_Zwergenstadt | Velis | 4 | Fehlt | Blockiert |
| L23 | L23_Blutsteinminen | Velis | 4 | Fehlt | Blockiert |
| L24 | L24_Festung_der_Schattenlegion | Velis | 4 | Fehlt | Blockiert |
| L25 | L25_Aschenfeste | Velis | 5 | Fehlt | Blockiert |
| L26 | L26_Tal_der_gefallenen_Koenige | Velis | 5 | Fehlt | Blockiert |
| L27 | L27_Tempel_des_Vierten_Siegels | Velis | 5 | Fehlt | Blockiert |
| L28 | L28_Verlorene_Koenigsstadt | Velis | 5 | Fehlt | Blockiert |
| L29 | L29_Heiligtum_der_Krone | Velis | 5 | Fehlt | Blockiert |
| L30 | L30_Thronsaal_von_Askir | Velis | 5 | Fehlt | Blockiert |

## Vollständiger Source-Bereichskatalog

Die folgenden Bereiche umfassen alle 132 C++-Header und Implementierungen der Ausgangsbasis. Keine davon wurde mit Unreal kompiliert. Die drei Build-Regeldateien sind separat oben erfasst.

| Bereich | Header / Implementierungen | Laufzeitstatus |
|---|---|---|
| AI | 4 / 4 | Blockiert; Quellbestand inventarisiert |
| Animation | 3 / 3 | Blockiert; Quellbestand inventarisiert |
| Camp | 1 / 1 | Blockiert; Quellbestand inventarisiert |
| Characters | 3 / 4 | Blockiert; Quellbestand inventarisiert |
| Combat | 3 / 2 | Blockiert; Quellbestand inventarisiert |
| Companion | 1 / 1 | Blockiert; Quellbestand inventarisiert |
| Core | 2 / 1 | Blockiert; Quellbestand inventarisiert |
| Crafting | 1 / 1 | Blockiert; Quellbestand inventarisiert |
| Demo | 1 / 1 | Blockiert; Quellbestand inventarisiert |
| Dialogue | 1 / 0 | Blockiert; Quellbestand inventarisiert |
| Economy | 5 / 5 | Blockiert; Quellbestand inventarisiert |
| Equipment | 1 / 1 | Blockiert; Quellbestand inventarisiert |
| Exploration | 1 / 1 | Blockiert; Quellbestand inventarisiert |
| Game | 1 / 2 | Blockiert; Quellbestand inventarisiert |
| Interaction | 2 / 2 | Blockiert; Quellbestand inventarisiert |
| Inventory | 2 / 1 | Blockiert; Quellbestand inventarisiert |
| Loot | 2 / 2 | Blockiert; Quellbestand inventarisiert |
| Mission | 3 / 3 | Blockiert; Quellbestand inventarisiert |
| Morale | 1 / 1 | Blockiert; Quellbestand inventarisiert |
| NPC | 3 / 3 | Blockiert; Quellbestand inventarisiert |
| Production | 2 / 1 | Blockiert; Quellbestand inventarisiert |
| Progression | 1 / 1 | Blockiert; Quellbestand inventarisiert |
| Quest | 3 / 2 | Blockiert; Quellbestand inventarisiert |
| Save | 1 / 1 | Blockiert; Quellbestand inventarisiert |
| Simulation | 3 / 3 | Blockiert; Quellbestand inventarisiert |
| Social | 3 / 3 | Blockiert; Quellbestand inventarisiert |
| Stealth | 1 / 1 | Blockiert; Quellbestand inventarisiert |
| Survival | 1 / 1 | Blockiert; Quellbestand inventarisiert |
| Tests | 0 / 2 | Blockiert; Quellbestand inventarisiert |
| Trade | 1 / 1 | Blockiert; Quellbestand inventarisiert |
| Travel | 3 / 3 | Blockiert; Quellbestand inventarisiert |
| World | 6 / 6 | Blockiert; Quellbestand inventarisiert |
| Modulroot | 1 / 1 | Blockiert; Modulinitialisierung vorhanden |

## Tests und konkrete Qualitätsgrenzen

| Prüfung | Ergebnis | Evidenz / Grenze |
|---|---|---|
| Original-Archivbestand | Bestanden | 483 Dateien, Einzel-SHA-256 in Inventory |
| Python unittest | Bestanden | `python3 -m unittest discover -s Tests -p 'test_*.py' -v`, Exit 0, 431 Tests |
| Python-Syntax | Bestanden | compile() auf allen 83 Dateien, keine Syntaxfehler |
| JSON-Parsing | Bestanden | 44 Dokumente, keine Parsefehler |
| GLB-Containerstruktur | Bestanden | 26 gültige Header; keine Skins/Animationen/Texturen |
| 30 `.umap`-Dateien vorhanden | Fehlgeschlagen | 0 gefunden |
| Benötigte Blueprint-/Content-Assets vorhanden | Fehlgeschlagen | 0 `.uasset` gefunden; Startreferenz fehlt |
| Native UE-Automation-Tests | Blockiert | Zwei Testquellen vorhanden, Engine-/Compilerlauf fehlt |
| C++-Kompilierung/UHT/UBT | Blockiert | Engine fehlt; strukturelle Blocker vorab erkannt |
| Engine öffnen/Projektstart | Blockiert | Engine fehlt und Content/Startreferenzen fehlen |
| Dorfmission E2E / alle Spielsysteme | Blockiert | Kein echter Level und keine Runtime verfügbar |
| Campaign/20 Sidequests/3 Enden | Blockiert | Inhalte bislang Datenentwürfe |
| Cook/Win64-Build/Paketstart | Blockiert | UE/UAT und Windows-Toolchain/Testumgebung fehlen |
| FPS, Ladezeit, VRAM/RAM | Nicht ausgeführt | Keine Messwerte; Ziel 60 FPS wird nicht als erreicht behauptet |

Die beiden nativen Tests decken nur Dornfeld-Zustandsprogression/Dedup/Snapshot und Combat-Health-Restore ab. Sie verwenden `NewObject<AActor>` statt einen in einer Testwelt gespawnten Actor; diesen Testaufbau und vollständige Component-Registration muss der erste Engine-Lauf überprüfen. Sie testen keine reale Eingabe, Questbelohnung mit Gameplaykomponenten, Karte, Physik, Renderer, NPC-/Bosskampf, Slot-Datei, Travel oder Wiederaufnahme nach Tod.

Ein Großteil der Python-Suiten prüft Zeichenketten im C++-Quelltext und Profildaten. Die 431 grünen Tests dürfen daher nicht zur Aussage „431 Gameplaytests bestanden“ werden. Ein gesonderter Engine-Testplan muss Interaktionen mit Objekten ausführen und beobachtbare Zustände sowie real erzeugte Save-Dateien prüfen.

## Abnahmekriterien des Master-Prompts

Die Projektabnahme bleibt offen. Es wird kein Häkchen für ein nicht tatsächlich verifiziertes Kriterium gesetzt.

- [ ] UE5.6 öffnet das Projekt ohne blockierende Fehler.
- [ ] C++ und alle benötigten Blueprints kompilieren.
- [ ] Mindestens 30 echte Levels erfüllen jeweils ihre technische und spielerische Abnahme.
- [ ] Zusammenhängende Erkundung und sämtliche Übergänge funktionieren.
- [ ] Dornfeld-Mission einschließlich 90 Gold, 180 XP und Save/Load ist end-to-end gespielt.
- [ ] Hauptgeschichte, mindestens 20 Nebenquests und drei Enden sind regulär erreichbar.
- [ ] Kampf, 24 Zauber, Gegner und Bosskämpfe sind mit Animation, Effekten und Audio spielbar.
- [ ] Inventar, Ausrüstung, Loot, Crafting, Handel, NPCs und verzweigte Dialoge funktionieren.
- [ ] Vollständige UMG-Menüs und Gameplayoberflächen sind verbunden.
- [ ] Persistenz einschließlich Regionen, Entscheidungen und Weltzuständen ist wiederholt geprüft.
- [ ] Erforderliche native Integrationstests bestanden.
- [ ] Windows-64-Bit-Build erstellt, außerhalb des Editors gestartet und getestet.
- [ ] Keine bekannten blockierenden Fehler verbleiben.

## Nächster überprüfbarer Entwicklungsschritt

Die Prioritäten in `Docs/V77_DEVELOPMENT_PRIORITIES.md` beginnen mit Quellenreparatur und der vollständigen Dornfeld-Sequenz. Unabhängig von der Engine können konkrete C++-Reparaturen, Blender-Exports, Datenvalidierung, reproduzierbare Editor-/Build-Skripte, Sicherungen und präzise Tests implementiert werden. Ihre Unreal-Ausgaben und die Zielplattform-Abnahme müssen solange blockiert bleiben, bis echte Engine-Tools verfügbar sind. Es werden keine künstlichen `.umap`- oder `.uasset`-Attrappen angelegt.
