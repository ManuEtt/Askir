# V84 – Persistenz von „Heilkräuter für Mara“

SQ_003 verwendet die bestehenden Format-5-Felder für Questzustände,
Zielfortschritt, eindeutige Ereignisse, einmalige Belohnungen, Dialogentscheidungen,
Fraktionswerte und Inventar. Ein neuer persistenter Gesprächs- oder Actorzeiger
wird nicht eingeführt. Die Gesprächsstufen sind virtuelle Bedingungen, keine
zweite gespeicherte Missionsautorität.

`ValidateSnapshot` prüft Annahme und genau eine Hilfe-/Handelsentscheidung,
Erkundungsbeleg und die feste Quelle `SQ003_Meadow_V84`. Unbekannte reservierte
Missionsbelege, widersprüchliche Belohnungen und vom Beleg abweichende aktive
Ziele werden vor Änderungen am laufenden Spiel abgelehnt. Vorhandene Spielstände
ohne SQ_003 bleiben gültig. Die zwei aktiven Ziele müssen Erkundung und Rückkehr
mit jeweils einem erforderlichen Fortschritt darstellen; die Rückkehr wird erst
in der atomaren Auszahlung abgeschlossen.

Während einer HerbMission-Transaktion sperrt der GameMode sowohl Aufnahme als
auch Wiederherstellung eines Spielstands. `IsPersistenceBusy` behält seine
bisherige Bedeutung, sodass die Mission ihre eigene Transaktion nicht blockiert.
Beim Laden werden Missionsspiegel und die Sicht-/Kollisionszustände der echten
Kräuter-Actors vor den abschließenden Restore-Benachrichtigungen aktualisiert.

Drei neue native Automationstests prüfen die reale Memory-Serialisierung,
Rücksprünge zwischen erkundetem und unerkanntem Weltzustand, Transaktionscallbacks,
gefälschte Belege und Ziellisten sowie mehrfaches Laden nach einmaliger Auszahlung.
Der Testgegenstand entsteht ausschließlich im Automationstest in einem benannten
Testpaket; die Laufzeitmission benötigt weiterhin das importierte echte Asset.

**Nicht ausgeführt:** Unreal Engine 5.6 ist in dieser Umgebung nicht verfügbar.
Die nativen Tests sind implementiert und registriert; dieser Bericht behauptet
keine Engine-Ausführung oder bestätigte Spielbarkeit.
