#include "Game/AskirGameMode.h"
#include "Demo/AskirDemoInteractable.h"
#include "Characters/AskirCharacter.h"
#include "Characters/AskirNPC.h"
#include "Combat/AskirCombatComponent.h"
#include "Progression/AskirProgressionComponent.h"
#include "Camp/AskirCampComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "EngineUtils.h"
#include "Travel/AskirWorldTravelSubsystem.h"
#include "Travel/AskirWorldPortal.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"

void AAskirGameMode::BeginPlay()
{
 Super::BeginPlay();
 const FString Map=UGameplayStatics::GetCurrentLevelName(this,true);
 if(!Map.StartsWith(TEXT("L")))BuildDemoScene();
 if(UGameInstance* Instance=GetGameInstance())
 {
  if(UAskirWorldTravelSubsystem* State=Instance->GetSubsystem<UAskirWorldTravelSubsystem>())
  {
   FName FromMap;
   if(State->ConsumeArrival(FName(*Map),FromMap))
   {
    // Restore inventory, quest, progression and world state before setting portal arrival.
    // Position is deliberately excluded: source-map coordinates are invalid here.
    if(State->bRestoreTravelSnapshot)
    {
     if(!LoadGame(TEXT("AskirTravelSnapshot"), false))
     {
      UE_LOG(LogTemp, Error, TEXT("Askir travel snapshot restore failed in %s"), *Map);
     }
     State->bRestoreTravelSnapshot=false;
    }
    // Spawn the player next to the reciprocal portal, never inside its overlap trigger.
    for(TActorIterator<AAskirWorldPortal> It(GetWorld());It;++It)
    {
     if(It->DestinationMap==FromMap)
     {
      if(APawn* Pawn=UGameplayStatics::GetPlayerPawn(this,0))
       Pawn->SetActorLocation(It->GetActorLocation()+FVector(0.f,420.f,40.f),false);
      break;
     }
    }
   }
  }
 }
}
void AAskirGameMode::MakeBlock(FVector Position,FVector Size)
{
 if(!GetWorld())return;
 AStaticMeshActor* Block=GetWorld()->SpawnActor<AStaticMeshActor>(Position,FRotator::ZeroRotator);
 if(!Block)return;
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
 UStaticMeshComponent* Mesh=Block->GetStaticMeshComponent();
 if(Cube.Succeeded())Mesh->SetStaticMesh(Cube.Object);
 Mesh->SetMobility(EComponentMobility::Movable);
 Mesh->SetWorldScale3D(Size/100.f);
 Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}
AAskirDemoInteractable* AAskirGameMode::MakeInteractable(FVector Position,int32 Kind,const TCHAR* Label)
{
 if(!GetWorld())return nullptr;
 AAskirDemoInteractable* Obj=GetWorld()->SpawnActor<AAskirDemoInteractable>(Position,FRotator::ZeroRotator);
 if(Obj){Obj->Kind=static_cast<EAskirDemoObject>(Kind);Obj->DisplayName=FText::FromString(Label);DemoObjects.Add(Obj);}
 return Obj;
}
void AAskirGameMode::BuildDemoScene()
{
 if(!GetWorld()||bDemoSceneReady)return;
 bDemoSceneReady=true;
 // Primitive test arena: no external marketplace assets required.
 MakeBlock(FVector(0,0,-60),FVector(4600,3600,100));
 MakeBlock(FVector(0,-1800,130),FVector(4600,100,350));
 MakeBlock(FVector(0,1800,130),FVector(4600,100,350));
 MakeBlock(FVector(-2300,0,130),FVector(100,3600,350));
 MakeBlock(FVector(2300,0,130),FVector(100,3600,350));
 MakeBlock(FVector(-650,650,40),FVector(260,240,80));
 MakeBlock(FVector(800,-500,60),FVector(350,120,120));
 MakeBlock(FVector(900,-650,60),FVector(120,350,120));
 MakeInteractable(FVector(-1100,250,75),0,TEXT("Lagerwaechter"));
 MakeInteractable(FVector(-100,400,75),1,TEXT("Vorratskiste"));
 MakeInteractable(FVector(400,300,75),2,TEXT("Lagerfeuer"));
 MakeInteractable(FVector(1700,100,75),3,TEXT("Lagerausgang"));
 DemoEnemy=GetWorld()->SpawnActor<AAskirNPC>(FVector(1250,-650,110),FRotator(0,180,0));
 if(DemoEnemy)
 {
   DemoEnemy->bHostile=true;
   DemoEnemy->NPCName=FText::FromString(TEXT("Pluenderer"));
   // Visible primitive representation; combat capsule remains the hit target.
   UStaticMeshComponent* Shape=NewObject<UStaticMeshComponent>(DemoEnemy,TEXT("DemoEnemyShape"));
   if(Shape)
   {
     Shape->SetupAttachment(DemoEnemy->GetRootComponent());
     static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
     if(Cube.Succeeded())Shape->SetStaticMesh(Cube.Object);
     Shape->SetRelativeScale3D(FVector(.65f,.65f,1.5f));
     Shape->SetCollisionEnabled(ECollisionEnabled::NoCollision);
     Shape->RegisterComponent();
   }
 }
 // The default engine entry map has no player start; place the pawn on the floor.
 if(AAskirCharacter* Player=Cast<AAskirCharacter>(UGameplayStatics::GetPlayerPawn(this,0)))
   Player->SetActorLocationAndRotation(FVector(-1750,0,120),FRotator::ZeroRotator);
}
FString AAskirGameMode::GetDemoObjective() const
{
 if(bDemoFinished)return TEXT("Auftrag abgeschlossen! Testszene geschafft.");
 switch(DemoStage)
 {
 case 0:return TEXT("Sprich mit dem Lagerwaechter (E)");
 case 1:return TEXT("Sammle die Vorraete aus der Kiste (E)");
 case 2:return TEXT("Besiege den Pluenderer (Linksklick)");
 case 3:return TEXT("Raste am Lagerfeuer (E)");
 default:return TEXT("Erreiche den Lagerausgang (E)");
 }
}
void AAskirGameMode::Tick(float DeltaSeconds)
{
 Super::Tick(DeltaSeconds);
 if(!bDemoSceneReady)return;
 if(DemoStage==2&&DemoEnemy&&DemoEnemy->IsDead())
 {
   DemoStage=3;
   if(AAskirCharacter* P=Cast<AAskirCharacter>(UGameplayStatics::GetPlayerPawn(this,0)))
     if(P->ProgressionComponent)P->ProgressionComponent->AwardExperience(50);
 }
 HudAccumulator+=DeltaSeconds;
 if(HudAccumulator<.3f)return;
 HudAccumulator=0.f;
 if(GEngine)
 {
  GEngine->AddOnScreenDebugMessage(5001,.45f,FColor::Yellow,TEXT("ASKIR | Das verlassene Lager | E Interaktion | F5 Speichern | F9 Laden"));
  GEngine->AddOnScreenDebugMessage(5002,.45f,FColor::White,GetDemoObjective());
  if(AAskirCharacter* P=Cast<AAskirCharacter>(UGameplayStatics::GetPlayerPawn(this,0)))
   if(P->CombatComponent)
    GEngine->AddOnScreenDebugMessage(5003,.45f,FColor::Green,FString::Printf(TEXT("LP %.0f | Ausdauer %.0f | Vorraete %d"),P->CombatComponent->GetHealth(),P->Stamina,DemoSupplies));
 }
}
void AAskirGameMode::DemoInteract()
{
 AAskirCharacter* P=Cast<AAskirCharacter>(UGameplayStatics::GetPlayerPawn(this,0));
 if(!P||P->IsDead())return;
 AAskirDemoInteractable* Nearest=nullptr;float BestDist=260.f;
 for(AAskirDemoInteractable* Obj:DemoObjects)
 {
  if(!Obj||Obj->bUsed)continue;
  const float D=FVector::Dist2D(P->GetActorLocation(),Obj->GetActorLocation());
  if(D<BestDist){BestDist=D;Nearest=Obj;}
 }
 if(!Nearest)return;
 switch(Nearest->Kind)
 {
 case EAskirDemoObject::Guide:
  if(DemoStage==0)DemoStage=1;
  break;
 case EAskirDemoObject::Supplies:
  if(DemoStage==1){DemoSupplies=3;Nearest->SetUsed(true);DemoStage=2;if(P->CampComponent)P->CampComponent->StockFood(3.f);}
  break;
 case EAskirDemoObject::Campfire:
  if(DemoStage==3){P->Heal(40.f);P->Stamina=P->MaxStamina;DemoStage=4;}
  break;
 case EAskirDemoObject::Exit:
  if(DemoStage==4){bDemoFinished=true;if(P->ProgressionComponent)P->ProgressionComponent->AwardExperience(100);}
  break;
 }
}
void AAskirGameMode::DemoQuickSave()
{
 const bool bOk=SaveGame(TEXT("AskirDemoQuick"));
 if(GEngine)GEngine->AddOnScreenDebugMessage(5004,3.f,bOk?FColor::Green:FColor::Red,bOk?TEXT("Spiel gespeichert"):TEXT("Speichern fehlgeschlagen"));
}
void AAskirGameMode::DemoQuickLoad()
{
 const bool bOk=LoadGame(TEXT("AskirDemoQuick"));
 if(GEngine)GEngine->AddOnScreenDebugMessage(5004,3.f,bOk?FColor::Green:FColor::Red,bOk?TEXT("Spiel geladen"):TEXT("Laden fehlgeschlagen"));
}
