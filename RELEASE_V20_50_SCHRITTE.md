# Askir V20 – GitHub-Release nach 50 Arbeitsschritten

**Basis:** V19. **Ziel:** Robustheit, automatisierte Paketprüfung, GitHub-Import. Keine 50 neuen Gameplay-Features.

01. Kauf: Händlergold vor Überlauf schützen
02. Verkauf: Spielergold vor Überlauf schützen
03. Restock: Lageraddition in 64 Bit berechnen
04. Preis: nicht-endliche Multiplikatoren abfangen
05. Rückkauf: nicht-endliche Quoten abfangen
06. TradeBridge: ungültige Preisfaktoren zurückweisen
07. TradeBridge: Kaufbudget vor Commit prüfen
08. Inventar: ungültige Item-Referenzen bei AddItem ablehnen
09. Inventar: ungültige Item-Referenzen bei RemoveItem ablehnen
10. Inventar: Itemzählung vor int32-Überlauf schützen
11. Dialog: ungültige Folge-Knoten vor Flag-Commit prüfen
12. Dialog: aktives Gespräch nicht erneut starten
13. Weltuhr: Zeitsprung je Aufruf begrenzen
14. Weltuhr: nicht-endliche DeltaTime ignorieren
15. Weltuhr: Stundentick auf begrenzte Verarbeitung umstellen
16. GameMode: doppeltes Include entfernen
17. Händler: extrem große Preiswerte begrenzen
18. TradeBridge: Vorprüfung für Spielerbudget ergänzen
19. Handel: negative Mengen bleiben abgewiesen (Regression)
20. Dialog: Antworten behalten bedingte Sichtbarkeit (Regression)
21. GitHub: .gitignore für Unreal anlegen
22. GitHub: .gitattributes für Quellcode und Assets anlegen
23. GitHub: Lizenz- und Asset-Herkunftshinweis
24. GitHub: vollständige Importanleitung
25. GitHub Actions: Workflow für statische Prüfungen
26. Python: release validator ohne externe Dependencies
27. Python: Unit-Test-Harness für Paketchecks
28. Release: einheitliche Verzeichnisstruktur
29. Release: Changelog für 50 Schritte
30. Release: ZIP mit Repository-Root als oberster Ebene
31. Asset: Manifest-Datei prüfen
32. Asset: 13 GLB-Dateien zählen
33. Asset: 13 OBJ-Dateien zählen
34. Asset: Dateigröße aller GLB-Dateien prüfen
35. Asset: GLB-Magic-Header prüfen
36. Asset: OBJ-Geometrie auf Vertex-Einträge prüfen
37. Asset: GLB/OBJ-Namen paarweise prüfen
38. Asset: Blender-Exportskript beibehalten
39. Asset: Unreal-Importscript beibehalten
40. Asset: Dorf-Platzierungsskript beibehalten
41. Build: Unreal-Projektdatei prüfen
42. Build: Build.cs prüfen
43. Build: Target.cs prüfen
44. Build: Editor.Target.cs prüfen
45. Build: wichtige C++-Subsysteme prüfen
46. Build: Unreal-generated-header-Konvention prüfen
47. Build: leere C++-Dateien erkennen
48. Build: generierte Unreal-Ordner ausschließen
49. Test: 50 statische Testfälle in CI ausführen
50. Release: Integritätsprüfung und SHA256 für GitHub-ZIP

## Grenzen
- Statische Tests sind kein Ersatz für Unreal-5.6-Kompilierung.
- V19-3D-Modelle sind weiterhin Prototypen.
- Ein echter Unreal-Testlevel, animierte Charaktere und UI-Verdrahtung fehlen weiterhin.
- Der Code wurde nicht automatisch in GitHub gepusht.
