#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/AskirAutomationWorldFixture.h"
#include "Characters/AskirCharacter.h"
#include "Characters/AskirNPC.h"
#include "Dialogue/AskirConversationComponent.h"
#include "Game/AskirGameMode.h"
#include "Game/AskirPlayerController.h"
#include "NPC/AskirDialogueComponent.h"
#include "Save/AskirSaveGame.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/StrongObjectPtr.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirConversationLorePersistenceTest,
    "Askir.Dialogue.V83.LoreSurvivesSerializedSaveWhileCursorAndButtonsExpire",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirConversationLorePersistenceTest::RunTest(const FString& Parameters)
{
    FAskirAutomationTestWorld Fixture;
    if(!TestNotNull(TEXT("Initialized runtime world"),Fixture.World))return false;
    auto* Player=Fixture.Spawn<AAskirCharacter>(FVector(0,0,96));
    auto* Controller=Fixture.Spawn<AAskirPlayerController>();
    auto* Rurik=Fixture.Spawn<AAskirNPC>(FVector(150,0,96));
    auto* Mode=Fixture.Spawn<AAskirGameMode>();
    if(!TestNotNull(TEXT("Player"),Player)||!TestNotNull(TEXT("Controller"),Controller)||
       !TestNotNull(TEXT("Rurik"),Rurik)||!TestNotNull(TEXT("Game mode"),Mode))return false;
    Controller->Possess(Player);Player->GetCharacterMovement()->DisableMovement();Rurik->GetCharacterMovement()->DisableMovement();
    Rurik->bHostile=false;Rurik->ConversationID=TEXT("D_DORNFELD_RURIK");
    auto* Conversation=Player->ConversationComponent.Get();
    TestTrue(TEXT("Compiled Rurik catalog exists and validates"),UAskirConversationComponent::IsKnownDialogue(Rurik->ConversationID));
    if(!TestTrue(TEXT("Configured native NPC opens authored graph"),Conversation->BeginConversation(Rurik)))return false;
    TestEqual(TEXT("First greeting has not learned the well story"),Conversation->GetCurrentNode().NodeID,FName(TEXT("greeting")));
    TestFalse(TEXT("An active conversation owns gameplay input even without a HUD"),Player->CanUseGameplayInput());
    if(!TestTrue(TEXT("Stable well choice advances actual graph"),Conversation->SelectChoice(TEXT("well"),Conversation->GetRevision())))return false;
    TestTrue(TEXT("Knowledge is recorded in the existing saved flag ledger"),Player->DialogueComponent->HasFlag(TEXT("RURIK_WELL_LORE")));
    const int64 OldRevision=Conversation->GetRevision();
    TStrongObjectPtr<UAskirSaveGame> Save(Mode->CaptureSnapshot());
    if(!TestNotNull(TEXT("Completed conversation action permits consistent capture"),Save.Get()))return false;
    TArray<uint8> Bytes;
    if(!TestTrue(TEXT("Actual SaveGame memory serialization"),UGameplayStatics::SaveGameToMemory(Save.Get(),Bytes)))return false;
    TStrongObjectPtr<UAskirSaveGame> Loaded(Cast<UAskirSaveGame>(UGameplayStatics::LoadGameFromMemory(Bytes)));
    if(!TestNotNull(TEXT("Actual deserialized save"),Loaded.Get()))return false;
    Conversation->EndConversation();Player->DialogueComponent->RestoreFlags({});
    TestTrue(TEXT("A changed unsaved timeline can reopen the old greeting"),Conversation->BeginConversation(Rurik));
    if(!TestTrue(TEXT("Restore complete native player snapshot"),Mode->RestoreSnapshot(Loaded.Get(),false)))return false;
    TestFalse(TEXT("Load never restores an old actor/cursor session"),Conversation->IsActive());
    TestTrue(TEXT("Load restores the learned lore flag"),Player->DialogueComponent->HasFlag(TEXT("RURIK_WELL_LORE")));
    TestTrue(TEXT("Restored input can open another conversation"),Player->CanUseGameplayInput());
    if(!TestTrue(TEXT("Reopen with restored knowledge"),Conversation->BeginConversation(Rurik)))return false;
    TestEqual(TEXT("Saved lore changes the next greeting"),Conversation->GetCurrentNode().NodeID,FName(TEXT("remembered")));
    TestFalse(TEXT("A pre-load button revision cannot advance the new session"),Conversation->SelectChoice(TEXT("well_again"),OldRevision));
    TestEqual(TEXT("Stale action preserves the current node"),Conversation->GetCurrentNode().NodeID,FName(TEXT("remembered")));
    Controller->CloseAnyMenu();
    TestFalse(TEXT("Headless close also ends the backend session"),Conversation->IsActive());
    TestTrue(TEXT("Closing restores gameplay ownership"),Player->CanUseGameplayInput());
    return true;
}
#endif
