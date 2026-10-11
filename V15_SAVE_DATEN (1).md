# V15 – Speicherdaten für die lebendige Welt
`UAskirSaveGame` besitzt zusätzliche serialisierbare Felder für Tag, Stunde, Fraktionsruf, Dialogflags und bereits ausgelöste einmalige Weltereignisse. `UAskirDialogueComponent` stellt `GetFlags` und `RestoreFlags` bereit.
**Offen:** GameMode muss diese Felder beim Speichern befüllen und beim Laden in die Komponenten zurückschreiben. Bis dahin ist dies eine SaveGame-Schema-Erweiterung, **keine funktionierende Persistenz**.
