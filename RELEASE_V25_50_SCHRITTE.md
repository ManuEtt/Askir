# Askir RPG V25 – Entwicklungspaket Schritte 251–300

Die folgenden 50 Punkte sind ein **Arbeits- und Prüfplan**. Einträge mit „Test“ sind Testfälle, **keine ausgeführten Unreal-Tests**.

251. Encounter trigger integration
252. Spawn wave configuration
253. Enemy class validation
254. Player validation
255. Collision-aware spawning
256. Spawn ring positions
257. Enemy combat component checks
258. Death delegate tracking
259. Dead enemy cleanup
260. Multi-wave transition
261. Encounter completion event
262. Quest objective completion
263. Repeat protection
264. Spawner reset policy
265. Blueprint configuration
266. Unreal Editor placement
267. Enemy Blueprint setup
268. NavMesh preparation
269. Collision channel setup
270. Wave count bounds
271. Enemy count bounds
272. Radius bounds
273. Missing asset handling
274. Missing component handling
275. Partial spawn handling
276. Empty wave handling
277. No-player handling
278. No-world handling
279. Repeated activation handling
280. Dead combat tracking
281. Quest missing handling
282. Quest ID handling
283. Objective ID handling
284. Delegate binding lifecycle
285. Static component ownership
286. Actor lifecycle
287. World placement checklist
288. PIE test plan
289. Multi-wave test
290. Quest progress test
291. Enemy death test
292. Encounter replay test
293. No enemy class test
294. Bad enemy blueprint test
295. Collision blocked test
296. Invalid player test
297. Performance spawn cap
298. Repository packaging
299. Source validation
300. Release documentation

## Neu implementiert
`AAskirEncounterSpawner` und `UAskirEncounterEnemyTracker`: optionaler Enemy-Wave-Spawner, Todesereignis-Anbindung und Questabschluss nach der letzten Welle.

## Status
Statische Prüfungen sind ausführbar. Unreal 5.6, NavMesh, Gegner-Blueprints und PIE sind hier nicht verfügbar; keine Kompilierungs- oder Spielbarkeitszusage.
