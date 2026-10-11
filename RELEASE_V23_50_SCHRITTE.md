# Askir V23 – Entwicklungsblock 151–200

50 dokumentierte Implementierungs- und Integrationspunkte. Diese sind **keine 50 neuen Spielfunktionen**.

151. **Questbelohnungen:** Belohnungsfunktion
152. **Questbelohnungen:** Questvalidierung
153. **Questbelohnungen:** Doppelbelohnungsschutz
154. **Questbelohnungen:** Queststatusprüfung
155. **Questbelohnungen:** Spielerbesitzerprüfung
156. **Questbelohnungen:** Goldkomponente prüfen
157. **Questbelohnungen:** Erfahrungskomponente prüfen
158. **Questbelohnungen:** Inventarkomponente prüfen
159. **Questbelohnungen:** Gold nicht negativ
160. **Questbelohnungen:** Erfahrung nicht negativ
161. **Questbelohnungen:** Gegenstandsmenge nicht negativ
162. **Questbelohnungen:** Goldüberlauf prüfen
163. **Questbelohnungen:** Gegenstandsreferenz prüfen
164. **Questbelohnungen:** Gegenstand vor Abschluss geben
165. **Questbelohnungen:** Rollback bei fehlgeschlagenem Abschluss
166. **Questbelohnungen:** Quest abschließen
167. **Questbelohnungen:** Belohnung markieren
168. **Questbelohnungen:** Gold gutschreiben
169. **Questbelohnungen:** Erfahrung vergeben
170. **Questbelohnungen:** Belohnungsstatus abfragen
171. **Händler:** Händler-Actor
172. **Händler:** Mesh-Komponente
173. **Händler:** Händler-Komponente
174. **Händler:** Interactable-Schnittstelle
175. **Händler:** Interaktionsdistanz
176. **Händler:** Ungültigen Spieler ablehnen
177. **Händler:** Interaktionsereignis
178. **Händler:** Blueprint-Bindung
179. **Händler:** Handelstext
180. **Händler:** Kein automatischer Kauf
181. **Questtrigger:** Trigger-Actor
182. **Questtrigger:** Box-Kollision
183. **Questtrigger:** Overlap-Delegate
184. **Questtrigger:** QuestID-Konfiguration
185. **Questtrigger:** ObjectiveID-Konfiguration
186. **Questtrigger:** Mengen-Konfiguration
187. **Questtrigger:** Spieler-Komponente finden
188. **Questtrigger:** Fortschritt melden
189. **Questtrigger:** Einmal-pro-Spieler-Schutz
190. **Questtrigger:** Nur bei Erfolg markieren
191. **Integration und QA:** Questgeber mit Belohnung verbinden
192. **Integration und QA:** Bestehendes Questsystem erhalten
193. **Integration und QA:** Quest-Datenasset erweitern
194. **Integration und QA:** Inventar-API wiederverwenden
195. **Integration und QA:** Wallet-API wiederverwenden
196. **Integration und QA:** Progression-API wiederverwenden
197. **Integration und QA:** V22 als Basis beibehalten
198. **Integration und QA:** Unreal-Testplan schreiben
199. **Integration und QA:** Release-Dokumentation erstellen
200. **Integration und QA:** ZIP-Struktur prüfen

## Spielstandintegration
- Einmalig ausgezahlte Questbelohnungen (`SavedClaimedRewards`) und Spieler-Gold (`SavedPlayerGold`) werden in GameMode gespeichert und geladen.

## Neue Quelldateien
- `Source/Askir/Economy/AskirMerchantActor.h/.cpp`
- `Source/Askir/Economy/AskirQuestTrigger.h/.cpp`
- Erweiterte Questdaten, Questkomponente und Questgeber.

## Einschränkungen
Die Quelltexte sind nicht in Unreal kompiliert. Es gibt keine fertige `.umap`, Händler-UI oder geriggten Charaktere. Der neue Trigger muss im Level platziert werden.
