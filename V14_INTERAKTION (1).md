# V14 – NPC-Interaktion
Neue `UAskirInteractionComponent`: einheitlicher Interaktionsabstand, Interaktionstext und `OnInteracted`-Event. NPC-Blueprints können damit V9-Dialoge starten, Händler öffnen oder Quests anbieten.
**Editor:** Interaktion per Spieler-LineTrace auf NPC auslösen, `CanInteract` prüfen, dann `Interact` aufrufen; `OnInteracted` im NPC an Dialog/Shop binden.
**Sicherheit:** Der Reichweitencheck ist nur lokale Spiellogik, keine serverautoritativ geprüfte Mehrspieleraktion.
