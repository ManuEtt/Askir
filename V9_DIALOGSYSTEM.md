# Version V9 – Dialogsystem
Neue `UAskirDialogueComponent` mit Knoten, Sprechertext, auswählbaren Antworten, bedingten Antwortoptionen, Flags, Folge-Knoten und End-Dialog.
**Editor:** NPC-Blueprint um Komponente erweitern, `Nodes` pflegen; UMG-Widget liest `GetCurrentNode` und `GetAvailableChoices`, Button ruft `SelectChoice` auf.
**Grenze:** UMG-Widget und Lokalisierungstabellen werden im Unreal Editor erstellt. Dialogflags sind noch nicht in SaveGame persistiert.
