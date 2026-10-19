#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/AskirAutomationWorldFixture.h"
#include "Characters/AskirCharacter.h"
#include "Combat/AskirCombatComponent.h"
#include "Crafting/AskirCraftingComponent.h"
#include "Crafting/AskirCraftingStationActor.h"
#include "Economy/AskirMerchantActor.h"
#include "Economy/AskirMerchantComponent.h"
#include "Interaction/AskirInteractable.h"
#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirVillageInteractionAuthorityTest,
    "Askir.Village.Interactions.RejectRemoteDeadAndNonPlayerCustomers",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirVillageInteractionAuthorityTest::RunTest(const FString& Parameters)
{
    FAskirAutomationTestWorld Fixture;
    if(!TestNotNull(TEXT("Initialized runtime world"),Fixture.World))return false;
    auto* Player=Fixture.Spawn<AAskirCharacter>(FVector(0.f,0.f,100.f));
    auto* Merchant=Fixture.Spawn<AAskirMerchantActor>(FVector(200.f,0.f,100.f));
    auto* Station=Fixture.Spawn<AAskirCraftingStationActor>(FVector(0.f,200.f,100.f));
    auto* OtherActor=Fixture.Spawn<AActor>(Player?Player->GetActorLocation():FVector::ZeroVector);
    if(!TestNotNull(TEXT("Spawned player"),Player)||
       !TestNotNull(TEXT("Spawned merchant"),Merchant)||
       !TestNotNull(TEXT("Spawned workbench"),Station)||
       !TestNotNull(TEXT("Spawned non-player actor"),OtherActor))return false;

    Merchant->InteractionRange=250.f;Station->InteractionRange=250.f;
    TestTrue(TEXT("Living nearby player can access merchant context"),Merchant->CanInteract(Player));
    TestTrue(TEXT("Living nearby player can access crafting context"),Station->CanInteract(Player));
    TestFalse(TEXT("Merchant rejects null customer"),Merchant->CanInteract(nullptr));
    TestFalse(TEXT("Workbench rejects null customer"),Station->CanInteract(nullptr));
    TestFalse(TEXT("Merchant rejects a generic actor"),Merchant->CanInteract(OtherActor));
    TestFalse(TEXT("Workbench rejects a generic actor"),Station->CanInteract(OtherActor));

    Player->SetActorLocation(FVector(1000.f,1000.f,100.f));
    TestFalse(TEXT("Merchant revokes context after customer moves away"),Merchant->CanInteract(Player));
    TestFalse(TEXT("Workbench revokes context after customer moves away"),Station->CanInteract(Player));
    TestFalse(TEXT("Remote interact cannot open a merchant"),IAskirInteractable::Execute_Interact(Merchant,Player));
    TestFalse(TEXT("Remote interact cannot open a workbench"),IAskirInteractable::Execute_Interact(Station,Player));
    Player->SetActorLocation(FVector(0.f,0.f,100.f));

    // Context validation remains available to the native menu, but Interact needs
    // an actual player controller/HUD. A missing UI must not report success.
    TestFalse(TEXT("Merchant cannot claim an opened UI without a controller"),IAskirInteractable::Execute_Interact(Merchant,Player));
    TestFalse(TEXT("Workbench cannot claim an opened UI without a controller"),IAskirInteractable::Execute_Interact(Station,Player));

    auto* SavedMerchant=Merchant->Merchant.Get();Merchant->Merchant=nullptr;
    TestFalse(TEXT("Missing merchant ledger disables interaction"),Merchant->CanInteract(Player));
    Merchant->Merchant=SavedMerchant;
    auto* SavedCrafting=Player->CraftingComponent.Get();Player->CraftingComponent=nullptr;
    TestFalse(TEXT("Missing crafting component disables interaction"),Station->CanInteract(Player));
    Player->CraftingComponent=SavedCrafting;

    Player->CombatComponent->Kill();
    TestFalse(TEXT("Death revokes an existing merchant context"),Merchant->CanInteract(Player));
    TestFalse(TEXT("Death revokes an existing workbench context"),Station->CanInteract(Player));
    TestFalse(TEXT("Dead player cannot reopen merchant UI"),IAskirInteractable::Execute_Interact(Merchant,Player));
    TestFalse(TEXT("Dead player cannot reopen crafting UI"),IAskirInteractable::Execute_Interact(Station,Player));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirVillageInteractionRangeTest,
    "Askir.Village.Interactions.ValidateConfiguredRangeAndBoundary",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirVillageInteractionRangeTest::RunTest(const FString& Parameters)
{
    FAskirAutomationTestWorld Fixture;
    if(!TestNotNull(TEXT("Initialized runtime world"),Fixture.World))return false;
    auto* Player=Fixture.Spawn<AAskirCharacter>(FVector(0.f,0.f,100.f));
    auto* Merchant=Fixture.Spawn<AAskirMerchantActor>(FVector(250.f,0.f,100.f));
    auto* Station=Fixture.Spawn<AAskirCraftingStationActor>(FVector(0.f,250.f,100.f));
    if(!TestNotNull(TEXT("Spawned player"),Player)||
       !TestNotNull(TEXT("Spawned merchant"),Merchant)||
       !TestNotNull(TEXT("Spawned workbench"),Station))return false;

    Merchant->InteractionRange=250.f;Station->InteractionRange=250.f;
    TestTrue(TEXT("Merchant range includes exact boundary"),Merchant->CanInteract(Player));
    TestTrue(TEXT("Workbench range includes exact boundary"),Station->CanInteract(Player));
    Merchant->InteractionRange=249.f;Station->InteractionRange=249.f;
    TestFalse(TEXT("Merchant rejects player immediately outside range"),Merchant->CanInteract(Player));
    TestFalse(TEXT("Workbench rejects player immediately outside range"),Station->CanInteract(Player));
    const float InvalidRanges[]={0.f,-250.f,std::numeric_limits<float>::quiet_NaN(),
        std::numeric_limits<float>::infinity(),-std::numeric_limits<float>::infinity()};
    for(float Range:InvalidRanges)
    {
        Merchant->InteractionRange=Range;Station->InteractionRange=Range;
        TestFalse(TEXT("Malformed merchant range disables interaction"),Merchant->CanInteract(Player));
        TestFalse(TEXT("Malformed workbench range disables interaction"),Station->CanInteract(Player));
        TestFalse(TEXT("Malformed merchant range cannot open UI"),IAskirInteractable::Execute_Interact(Merchant,Player));
        TestFalse(TEXT("Malformed workbench range cannot open UI"),IAskirInteractable::Execute_Interact(Station,Player));
    }
    return true;
}
#endif
