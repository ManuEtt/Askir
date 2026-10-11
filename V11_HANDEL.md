# Version V11 – Handel und Händlerbestand
`UAskirMerchantComponent` mit Artikelpreisen, Händlergold, begrenztem Lagerbestand, Kauf/Verkauf, Rückkaufabschlag, Restock und Trade-Event.
**Editor:** Dem Händler die Komponente geben; Artikel-IDs mit bestehendem Inventarsystem verbinden; Preise optional mit `GetMerchantPriceMultiplier` aus V10 berechnen.
**Wichtig:** `BuyFromMerchant`/`SellToMerchant` ändern nur Gold und Händlerbestand. Der tatsächliche Transfer in das **Spielerinventar muss transaktional im Blueprint/C++-Spielsystem ergänzt** werden; Verkauf muss den Besitz des Artikels prüfen. Nicht direkt als vollständiges Handelssystem für Spieler freischalten.
