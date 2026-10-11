# Askir RPG V26 – Schritte 301–350

V26 erweitert das Kampfsystem um eine Ausdauer-basierte Blockmechanik. Die folgenden Punkte sind Implementierungs- und Integrationspunkte; Editor- und Spieltests sind **nicht** ausgeführt.

301. Guard component declaration
302. Guard component constructor
303. Guard tick lifecycle
304. Guard start callable
305. Guard stop callable
306. Stamina restoration
307. Stamina percentage getter
308. Guard state getter
309. Guard broken event
310. Maximum stamina property
311. Current stamina property
312. Stamina regeneration rate
313. Damage reduction tuning
314. Stamina damage cost tuning
315. Guard cooldown tuning
316. Incoming damage clamping
317. Zero damage handling
318. Unblocked damage passthrough
319. Block reduction calculation
320. Stamina cost calculation
321. Partial guard absorption
322. Guard break detection
323. Guard break event broadcast
324. Guard cooldown timestamp
325. Guard start cooldown check
326. Guard stamina check
327. Guard regeneration only when idle
328. Guard stamina clamping
329. Combat component include integration
330. Combat damage pipeline integration
331. Combat owner validity check
332. Guard component optional lookup
333. Character guard forward declaration
334. Character guard component property
335. Character guard subobject creation
336. Blueprint editable guard tuning
337. Blueprint callable guard control
338. Blueprint event hookup
339. Blueprint guard UI support
340. Enemy NPC inherited guard
341. Player character inherited guard
342. Guard component reusable design
343. No hard-coded UI dependency
344. No timer allocation required
345. Unreal editor setup guide
346. Guard tuning examples
347. Static regression checks
348. ZIP integrity validation
349. Release manifest
350. GitHub import readiness

## Einschränkungen
Keine Unreal Engine Kompilierung durchgeführt. Blocken wird über Blueprint oder Gameplay-Input ausgelöst; Animation, UI, KI-Nutzung und Savegame-Integration sind noch offen.
