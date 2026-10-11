# V17 – Dorf-Testszene: Integrations- und Abnahmepaket

## Entwicklungsumfang
- Konsolidiertes Projekt mit V13–V16.
- Ausführbares statisches Prüfskript `Tools/validate_v17_project.py`.
- Reproduzierbarer PIE-Abnahmeplan für das Dorf und NPC-Systeme.

## Unreal-Editor-Aufbau (noch manuell)
1. `L_AskirVillage_Test` als neues Level anlegen.
2. Dorf-Props (Hütte, Brunnen, Lagerfeuer, Kisten) aus `Realistic3D_V1` importieren und platzieren.
3. `NavMeshBoundsVolume` über das Dorf legen; Navigation prüfen.
4. `AAskirVillageDirector` platzieren.
5. Drei NPC-Blueprints erstellen: Dorfbewohner, Wache, Händler.
6. Tagesablauf-Komponenten hinzufügen und Schedule-Vorlagen initialisieren.
7. NPCs im Director-Array `Residents` eintragen.
8. Händler bekommt Merchant-, Interaction- und Dialogue-Komponente.
9. Spieler bekommt Reputation-Komponente und Dialog-UI.
10. Dialog- und Shop-Widgets im Editor mit Komponenten-Events verbinden.

## Manuelle Abnahmefälle (PIE)
- 08:00: Dorfbewohner haben Activity Work.
- 18:00: Dorfbewohner haben Activity Leisure.
- Tageswechsel 23 -> 00 erhöht Day.
- Interaktion über 250 cm wird abgewiesen.
- Dialog mit fehlender RequiredFlag blendet Choice aus.
- Rufänderung von 0 auf +40 wird als Friendly erkannt.
- Kauf über verfügbare Goldmenge wird abgewiesen.
- Speichern/Laden stellt Ruf, Weltzeit und Dialogflags wieder her.
- Einmaliges Weltereignis wird nach Laden nicht erneut ausgelöst.
- Gegner-KI verfolgt nur bei vorhandenem NavMesh und erfüllter Sichtprüfung.

## Grenzen
- Keine `.umap`, `.uasset` oder vollständige fertige Dorf-Szene erzeugt.
- Testskript prüft Dateistruktur, nicht C++-Kompilierung oder Laufzeit.
- V11-Handelsfunktionen übertragen noch keine Spieleritems; daher Handel nicht als fertig integrierten UI-Shop freigeben.
- Charaktere, Rigging und Animations-Blueprints benötigen Blender-/Unreal-Ausführung.
- Keine Unreal Engine 5.6 Installation im Ausführungsumfeld.
