# V84 – Prüfstatus und offene Abnahme

Maßgeblich ist `Reports/V84_verification.json` mit Befehlen, Ergebnissen und
unveränderten Quelldateihashes. Der Engine-Vorlauf steht unter
`Reports/V84/pipeline_environment.json`.

| Prüfung | Status |
| --- | --- |
| Python-Regressionen | Bestanden: 708 Tests |
| Portable C++-Produktionsregeln mit g++/UBSan | Bestanden: 457.277 Prüfungen in 15 Programmen |
| Dialogkatalog und Kräuterquellen | Bestanden: Katalog aktuell; echtes Modell ohne entartete Dreiecke |
| Native UHT-/UE-C++-Kompilierung | Blockiert: UE 5.6 fehlt |
| Native Askir-Automation | 80 Tests registriert, nicht ausgeführt |
| E-Interaktion, Dialog und HUD | Nicht ausgeführt |
| Reale Maps/Assets, Kollisions-/Navigationsprüfung | Blockiert: Engine fehlt |
| Win64-Paket und eigenständiger Spielstart | Blockiert: Engine und Windows-Host fehlen |
| Durchspielen und Performance | Nicht ausgeführt; keine FPS-Messung |

## Noch erforderliche Engine-Prüfung

1. V84-Buildrunner auf Windows mit UE 5.6 ausführen. Editor/Win64-Kompilierung,
   reale Import-/Kartenprüfung, native Automation und Paketstart müssen bestehen.
2. In Dornfeld mit Rurik sprechen. Dorfkunde und gelernte Brunnenkenntnis prüfen.
   Kräuterauftrag wahlweise als Hilfe oder Handel annehmen; genau eine Annahme
   und einmalige +2-Fraktion erwarten. Andere NPCs dürfen ihn nicht vergeben.
3. Regulär ins Umland reisen. Das markierte Kräutervorkommen untersuchen;
   blockierte Sicht, zu große Entfernung, falsche Quelle und Wiederholung dürfen
   keinen zusätzlichen Fortschritt erzeugen. Im Journal genau 1/1 erwarten.
4. Zwischen Annahme, Erkundung und Rückgabe jeweils speichern, Anwendung beenden
   und laden. Wiederholt die Region wechseln; keine verlorenen oder doppelten Belege.
5. Zu Rurik zurückkehren. Volles Inventar muss die Auszahlung verhindern. Nach
   Freimachen eines Platzes dieselbe Quest erfolgreich abschließen: genau 24 Gold,
   150 XP und ein `item_004`; keine Wiederholung nach erneutem Gespräch/Laden.
6. Einen älteren V83-Spielstand laden. Mara, Torben, Zielerfassung, Händler,
   Crafting, Menüs und Reiseablauf weiterhin vollständig prüfen.
7. Meshdarstellung, Materialfarben, Interaktionsbereich, Kameralesbarkeit und HUD
   manuell prüfen; anschließend FPS, Speicher und Ladezeiten auf dokumentierter
   Windows-Hardware messen.

Alle manuellen Punkte bleiben bis zu einem protokollierten Engine-Lauf
**Nicht ausgeführt**. Die Entwicklungs-ZIP ist kein spielbares Windows-Paket.
