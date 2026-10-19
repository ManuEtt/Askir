#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/AskirAutomationWorldFixture.h"
#include "Travel/AskirWorldPortal.h"
#include "Travel/AskirWorldTravelSubsystem.h"
#include "Components/BoxComponent.h"
#include "Engine/GameInstance.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/GarbageCollection.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirTravelHandoffTest,"Askir.Travel.PendingSnapshotCannotBeOverwrittenOrConsumedEarly",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirTravelHandoffTest::RunTest(const FString& Parameters)
{
 TStrongObjectPtr<UGameInstance> Instance(NewObject<UGameInstance>());
 TStrongObjectPtr<UAskirWorldTravelSubsystem> State(NewObject<UAskirWorldTravelSubsystem>(Instance.Get()));
 auto* First=NewObject<UAskirSaveGame>();First->SavedMapName=TEXT("Dornfeld_V80");First->SavedPlayerGold=137;
 TestTrue(TEXT("Queue validated source snapshot"),State->QueueSnapshot(TEXT("Dornfelder_Umland_V80"),TEXT("Dornfeld_V80"),First,false));
 auto* Second=NewObject<UAskirSaveGame>();Second->SavedMapName=TEXT("Dornfelder_Umland_V80");Second->SavedPlayerGold=1;
 TestFalse(TEXT("Repeated handoff cannot overwrite player timeline"),State->QueueSnapshot(TEXT("Dornfeld_V80"),TEXT("Dornfelder_Umland_V80"),Second,false));
 State->QueueArrival(TEXT("OtherMap"),TEXT("OtherSource"));
 TestEqual(TEXT("Legacy request preserves strong snapshot"),State->PendingSnapshot.Get(),First);
 TestEqual(TEXT("Original destination remains"),State->PendingDestination,FName(TEXT("Dornfelder_Umland_V80")));
 TWeakObjectPtr<UAskirSaveGame> WeakSnapshot(First);
 CollectGarbage(RF_NoFlags);
 TestTrue(TEXT("GI handoff retains save through garbage collection"),WeakSnapshot.IsValid());
 FName Source;
 TestFalse(TEXT("Consume cannot bypass snapshot restoration"),State->ConsumeArrival(TEXT("Dornfelder_Umland_V80"),Source));
 AddExpectedError(TEXT("Askir arrival blocked:"),EAutomationExpectedErrorFlags::Contains,2);
 State->MarkArrivalFailed(FText::FromString(TEXT("Test interruption")));
 TestTrue(TEXT("Failure retains captured player state"),State->PendingSnapshot==First&&State->IsGameplayBlocked());
 State->ResumeArrival();
 TestFalse(TEXT("Retry clears failure indicator"),State->bArrivalFailed);
 TestTrue(TEXT("Retry still protects pending state"),State->HasPendingTravel());
 State->ClearPendingTravel();
 TestFalse(TEXT("Acknowledgement unblocks gameplay"),State->IsGameplayBlocked());
 TestTrue(TEXT("Snapshot flags cleared"),!State->bRestoreTravelSnapshot&&!State->bPendingRestorePosition&&State->PendingArrivalPortalID.IsNone()&&State->PendingDeparturePortalID.IsNone());
 First->PlayerHealth=-1;
 TestFalse(TEXT("Corrupt snapshot rejected"),State->QueueSnapshot(TEXT("Dornfelder_Umland_V80"),TEXT("Dornfeld_V80"),First,false));
 TestFalse(TEXT("Rejected request leaves no pending state"),State->HasPendingTravel());
 First->PlayerHealth=100;
 TestFalse(TEXT("Portal snapshot must originate in declared source map"),State->QueueSnapshot(TEXT("OtherDestination"),TEXT("WrongSource"),First,false));
 TestFalse(TEXT("Disk load snapshot must name its destination"),State->QueueSnapshot(TEXT("OtherDestination"),NAME_None,First,true));
 TestFalse(TEXT("Portal route cannot travel to same source map"),State->QueueSnapshot(TEXT("Dornfeld_V80"),TEXT("Dornfeld_V80"),First,false));
 TestFalse(TEXT("Null snapshot cannot establish a handoff"),State->QueueSnapshot(TEXT("OtherDestination"),TEXT("Dornfeld_V80"),nullptr,false));
 TestTrue(TEXT("Disk-load handoff accepts its own saved map"),State->QueueSnapshot(TEXT("Dornfeld_V80"),NAME_None,First,true));
 TestTrue(TEXT("Disk-load handoff preserves position intent"),State->bPendingRestorePosition);
 FString Package=TEXT("untouched");
 State->MarkArrivalFailed(FText::FromString(TEXT("No recovery package")));
 First->SavedMapPackageName=TEXT("/Game/Askir/Tests/NotTheCapturedMap");
 const FName DestinationBefore=State->PendingDestination;
 TestFalse(TEXT("Missing saved package cannot start rollback"),State->RedirectToSavedWorld(Package));
 TestEqual(TEXT("Rejected rollback does not write output"),Package,FString(TEXT("untouched")));
 TestTrue(TEXT("Rejected rollback retains failure and strong snapshot"),State->bArrivalFailed&&State->PendingSnapshot==First);
 TestEqual(TEXT("Rejected rollback retains destination"),State->PendingDestination,DestinationBefore);
 State->ClearPendingTravel();
 State->QueueArrival(TEXT("Dornfelder_Umland_V80"),TEXT("Dornfeld_V80"));
 TestTrue(TEXT("Legacy arrival can be queued without inventing a save"),State->HasPendingTravel()&&!State->PendingSnapshot);
 TestFalse(TEXT("Wrong map cannot acknowledge legacy arrival"),State->ConsumeArrival(TEXT("WrongDestination"),Source));
 TestTrue(TEXT("Destination acknowledges legacy arrival"),State->ConsumeArrival(TEXT("Dornfelder_Umland_V80"),Source));
 TestEqual(TEXT("Compatibility source remains available to caller"),Source,FName(TEXT("Dornfeld_V80")));
 TestFalse(TEXT("Legacy acknowledgement unblocks gameplay"),State->HasPendingTravel());
 return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirPortalInteractionTest,"Askir.Travel.ExplicitInteractionDoesNotBlockWalkingAndRejectsInvalidTravelers",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirPortalInteractionTest::RunTest(const FString& Parameters)
{
 FAskirAutomationTestWorld Fixture;if(!TestNotNull(TEXT("World"),Fixture.World))return false;
 auto* Portal=Fixture.Spawn<AAskirWorldPortal>();
 TestTrue(TEXT("Portal exposes actual interaction interface"),Portal->GetClass()->ImplementsInterface(UAskirInteractable::StaticClass()));
 TestEqual(TEXT("Query collision only"),Portal->Trigger->GetCollisionEnabled(),ECollisionEnabled::QueryOnly);
 TestEqual(TEXT("Walking overlaps portal"),Portal->Trigger->GetCollisionResponseToChannel(ECC_Pawn),ECR_Overlap);
 TestEqual(TEXT("Interaction sweep sees portal"),Portal->Trigger->GetCollisionResponseToChannel(ECC_Visibility),ECR_Block);
 TestFalse(TEXT("Null traveler rejected"),Portal->CanTravel(nullptr));
 TestFalse(TEXT("Unpossessed arbitrary actor rejected"),Portal->Travel(Fixture.Spawn<AActor>()));
 TestFalse(TEXT("Arbitrary asset is not a destination world"),UAskirWorldTravelSubsystem::IsUsableWorldPackage(TEXT("/Engine/BasicShapes/Cube"),TEXT("Cube")));
 TestFalse(TEXT("Missing authored world rejected"),UAskirWorldTravelSubsystem::IsUsableWorldPackage(TEXT("/Game/Askir/Maps/Missing_TravelTest"),TEXT("Missing_TravelTest")));
 return true;
}
#endif
