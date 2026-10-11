# Askir RPG V22 – Entwicklungsblock 101–150

50 konkrete Entwicklungs- und Integrationsprüfpunkte. **Nicht** 50 neue Spielsysteme.

## Questdaten
101. QuestID ist gesetzt
102. Ziel-IDs sind gesetzt
103. Ziel-IDs sind eindeutig
104. Zielmenge ist positiv
105. Start setzt Zielstände zurück

## Queststart
106. Null-Quest wird abgewiesen
107. Leere Quest-ID wird abgewiesen
108. Leere Zielliste wird abgewiesen
109. Aktive Quest kann nicht erneut gestartet werden
110. Abgeschlossene Quest kann nicht erneut gestartet werden

## Mehrere Quests
111. Questziele sind je Quest gespeichert
112. Mehrere Quests können aktiv sein
113. Questfortschritt bleibt getrennt
114. Abschluss entfernt nur die betreffende Zielliste
115. Queststatus bleibt nach Abschluss erhalten

## Fortschritt
116. Negative Mengen werden abgewiesen
117. Null-Mengen werden abgewiesen
118. Leere Ziel-ID wird abgewiesen
119. Unbekannte Quest wird abgewiesen
120. Überlauf wird durch int64-Zwischenwert verhindert

## Abschluss
121. Abschluss prüft Aktivstatus
122. Abschluss prüft alle Ziele
123. Leere Zielliste kann nicht abgeschlossen werden
124. Abschluss setzt Status Completed
125. Abschluss löst Event aus

## Blueprint API
126. Queststart ist BlueprintCallable
127. Questfortschritt ist BlueprintCallable
128. Zielabfrage ist BlueprintPure
129. Queststatus ist BlueprintPure
130. Events sind BlueprintAssignable

## Questgeber
131. Questgeber ist Actor
132. Questgeber implementiert Interactable
133. Questgeber prüft Interactor
134. Questgeber startet Quest
135. Questgeber gibt Quest bei erfüllten Zielen ab

## Inventar-Events
136. Pickup prüft ItemData
137. Pickup prüft Interactor
138. Pickup prüft positive Menge
139. Pickup addiert vor Questfortschritt
140. Pickup meldet ItemID an Quests

## Speichern
141. Queststatus ist SaveGame
142. Ziellisten sind SaveGame
143. Legacy-Ziele bleiben enthalten
144. SaveGame speichert Ziellisten
145. LoadGame stellt Ziellisten wieder her

## Release
146. Version V22 dokumentiert
147. GitHub-Import erklärt
148. Testplan für Unreal enthalten
149. Statische Prüfungen enthalten
150. Vollprojekt-ZIP auf Integrität geprüft

## Inhalt und Grenzen
- Mehrere gleichzeitig aktive Quests mit separatem Fortschritt und Questgeber-Actor.
- Persistenz der neuen Ziellisten über SaveGame.
- Legacy-API bleibt für bestehende Pickup-Events verfügbar.
- **Keine** automatischen Gold-/XP-Belohnungen; dies erfordert noch transaktionale Integration.
- **Keine** in Unreal erzeugten Level/Blueprints und **keine** UE-Kompilierung oder PIE-Prüfung.
- Alte Speicherstände mit ausschließlich `SavedActiveObjectives` benötigen Migration; neue Questlisten sind dort noch nicht enthalten.
