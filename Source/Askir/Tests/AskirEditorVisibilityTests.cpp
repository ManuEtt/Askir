#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/AskirAutomationWorldFixture.h"
#include "Production/AskirEditorLevelTools.h"
#include "Components/BoxComponent.h"
#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAskirEditorInteractionVisibilityTest,
    "Askir.Editor.V84.VisibilityValidationRequiresActualUnobstructedTargetHit",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAskirEditorInteractionVisibilityTest::RunTest(const FString& Parameters)
{
    FAskirAutomationTestWorld Fixture;
    if(!TestNotNull(TEXT("Initialized physics world"),Fixture.World))return false;
    auto* Target=Fixture.Spawn<AActor>();
    auto* Wall=Fixture.Spawn<AActor>();
    auto* Other=Fixture.Spawn<AActor>();
    if(!TestNotNull(TEXT("Target"),Target)||!TestNotNull(TEXT("Wall"),Wall)||!TestNotNull(TEXT("Other"),Other))return false;
    auto AddQueryBox=[&Fixture](AActor* Actor,const FVector& Position)
    {
        auto* Box=Fixture.AddComponent<UBoxComponent>(Actor);
        Actor->SetRootComponent(Box);Box->SetBoxExtent(FVector(20.,40.,40.));
        Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        Box->SetCollisionResponseToAllChannels(ECR_Ignore);Box->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
        Actor->SetActorLocation(Position);return Box;
    };
    AddQueryBox(Target,FVector(200.,0.,100.));
    AddQueryBox(Wall,FVector(100.,0.,100.));Wall->SetActorEnableCollision(false);
    const FVector Start(0.,0.,100.),End(200.,0.,100.);
    TestTrue(TEXT("Real first blocking hit belongs to intended target"),UAskirEditorLevelTools::HasVisibilityLineToActor(Fixture.World,Start,End,Target));
    TestFalse(TEXT("Another actor is not evidence for this target"),UAskirEditorLevelTools::HasVisibilityLineToActor(Fixture.World,Start,End,Other));
    Wall->SetActorEnableCollision(true);
    TestFalse(TEXT("A blocking wall cannot validate an interaction behind it"),UAskirEditorLevelTools::HasVisibilityLineToActor(Fixture.World,Start,End,Target));
    Wall->SetActorEnableCollision(false);Target->SetActorEnableCollision(false);
    TestFalse(TEXT("An empty line without a target hit is not success"),UAskirEditorLevelTools::HasVisibilityLineToActor(Fixture.World,Start,End,Target));
    TestFalse(TEXT("Null target"),UAskirEditorLevelTools::HasVisibilityLineToActor(Fixture.World,Start,End,nullptr));
    TestFalse(TEXT("Zero-length line"),UAskirEditorLevelTools::HasVisibilityLineToActor(Fixture.World,Start,Start,Target));
    TestFalse(TEXT("Nonfinite coordinates"),UAskirEditorLevelTools::HasVisibilityLineToActor(Fixture.World,FVector(std::numeric_limits<double>::infinity(),0.,100.),End,Target));
    FAskirAutomationTestWorld OtherFixture;
    if(TestNotNull(TEXT("Separate world"),OtherFixture.World))
        TestFalse(TEXT("A target from another world is invalid"),UAskirEditorLevelTools::HasVisibilityLineToActor(OtherFixture.World,Start,End,Target));
    return true;
}
#endif
