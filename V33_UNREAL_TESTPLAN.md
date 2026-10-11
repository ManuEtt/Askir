# Unreal-Testplan V33

1. Projekt mit UE 5.6 als Development Editor Win64 kompilieren.
2. Automations-Test `Askir.Mission.Dornfeld.ProgressionAndDedup` starten.
3. Gegner mit direktem Spielertreffer töten: +1 Wächter.
4. Gegner mit korrekt instigiertem Projektil töten: +1 Wächter.
5. Gegner durch anderen NPC töten: kein Questfortschritt.
6. Gegner durch Umwelt töten: kein Questfortschritt, sofern keine positive Spieler-Schadensquelle zuletzt gespeichert ist.
7. Prüfen, ob ein früherer Spielerhit vor einem Umwelttod irrtümlich angerechnet wird (bekannte Einschränkung).
8. Quest-HUD mit `OnObjectiveChanged` verbinden, Scherbenzähler 0/3 → 3/3 prüfen.
9. Wächterzähler 0/2 → 2/2 prüfen.
10. Nach Save/Load sofortiges ObjectiveChanged-Update prüfen.
11. Questbelohnung einmalig: 90 Gold, 180 XP.
12. Nach Levelwechsel und erneuter Bindung keine doppelten Ereignisse.

## Hinweis
Im Editor jeden Missions-Relay-Actor mit genau einer Quelle und eindeutiger EventID konfigurieren.
