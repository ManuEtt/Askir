#include "Game/AskirGameMode.h"
#include "Demo/AskirDemoInteractable.h"
#include "Characters/AskirCharacter.h"
#include "Characters/AskirNPC.h"
#include "Combat/AskirCombatComponent.h"
#include "Progression/AskirProgressionComponent.h"
#include "Camp/AskirCampComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Game/AskirPlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Travel/AskirWorldTravelSubsystem.h"
#include "Travel/AskirWorldPortal.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"

void AAskirGameMode::BeginPlay()
{
 Super::BeginPlay();
 if(bEnableLegacyDemo)BuildDemoScene();
 // Wait for spawned pawns and actor BeginPlay hooks, rather than consuming the
 // pending request before a player exists in the destination world.
 GetWorldTimerManager().SetTimerForNextTick(this,&AAskirGameMode::RestorePendingArrival);
 GetWorldTimerManager().SetTimerForNextTick(this,&AAskirGameMode::LogRuntimeReadiness);
}
void AAskirGameMode::LogRuntimeReadiness()
{
 auto* Player=Cast<AAskirCharacter>(UGameplayStatics::GetPlayerPawn(this,0));
 if(Player&&Player->IsPlayerControlled())
 {
  const FString Map=UGameplayStatics::GetCurrentLevelName(this,true);
  const TCHAR* Marker=Map==TEXT("Dornfeld_V78")?TEXT("ASKIR_V78_RUNTIME_READY"):TEXT("ASKIR_V77_RUNTIME_READY");
  UE_LOG(LogTemp,Display,TEXT("%s map=%s player=%s"),Marker,*Map,*Player->GetName());
 }
 else
 {
  UE_LOG(LogTemp,Error,TEXT("ASKIR_V77_RUNTIME_BLOCKED: player or native controller not initialized"));
  UE_LOG(LogTemp,Error,TEXT("ASKIR_V78_RUNTIME_BLOCKED: player or native controller not initialized"));
 }
}

void AAskirGameMode::RestorePendingArrival()
{
 UGameInstance* Instance=GetGameInstance();
 auto* State=Instance?Instance->GetSubsystem<UAskirWorldTravelSubsystem>():nullptr;
 const FName Map(*UGameplayStatics::GetCurrentLevelName(this,true));
 if(!State || State->PendingDestination!=Map || !State->PendingSnapshot)return;
 auto* Player=Cast<AAskirCharacter>(UGameplayStatics::GetPlayerPawn(this,0));
 if(!Player)
 {
  if(++ArrivalRestoreAttempts<120)GetWorldTimerManager().SetTimerForNextTick(this,&AAskirGameMode::RestorePendingArrival);
  else UE_LOG(LogTemp,Error,TEXT("Askir arrival blocked: player never spawned in %s"),*Map.ToString());
  return;
 }
 FVector Arrival=Player->GetActorLocation();
 FRotator Rotation=Player->GetActorRotation();
 const bool bRestorePosition=State->bPendingRestorePosition;
 if(!bRestorePosition)
 {
  const float Radius=Player->GetCapsuleComponent()->GetScaledCapsuleRadius();
  const float Height=Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
  const auto IsOutsidePortals=[&](const FVector& Candidate)
  {
   for(TActorIterator<AAskirWorldPortal> It(GetWorld());It;++It)
    if(It->Trigger && It->Trigger->Bounds.GetBox().ExpandBy(FVector(Radius,Radius,Height)).IsInsideOrOn(Candidate))return false;
   return true;
  };
  bool bFound=false;
  for(TActorIterator<AAskirWorldPortal> It(GetWorld());It;++It)
  {
   const bool bMatches=State->PendingArrivalPortalID.IsNone()?It->DestinationMap==State->PendingArrivalFrom:It->PortalID==State->PendingArrivalPortalID;
   if(!bMatches || It->ArrivalOffset.ContainsNaN())continue;
   FVector Candidate=It->GetActorLocation()+It->GetActorRotation().RotateVector(It->ArrivalOffset);
   const FRotator CandidateRotation=It->GetActorRotation();
   if(GetWorld()->FindTeleportSpot(Player,Candidate,CandidateRotation) && IsOutsidePortals(Candidate))
   {Arrival=Candidate;Rotation=CandidateRotation;bFound=true;break;}
  }
  // A map without a reciprocal portal can arrive at its authored PlayerStart.
  if(!bFound)
   for(TActorIterator<APlayerStart> It(GetWorld());It;++It)
   {
    FVector Candidate=It->GetActorLocation();
    if(GetWorld()->FindTeleportSpot(Player,Candidate,It->GetActorRotation()) && IsOutsidePortals(Candidate))
    {Arrival=Candidate;Rotation=It->GetActorRotation();bFound=true;break;}
   }
  if(!bFound)
  {
   UE_LOG(LogTemp,Error,TEXT("Askir arrival blocked: no collision-free portal or PlayerStart in %s"),*Map.ToString());
   return;
  }
 }
 if(!RestoreSnapshot(State->PendingSnapshot,bRestorePosition))
 {
  UE_LOG(LogTemp,Error,TEXT("Askir arrival snapshot restore failed in %s; pending snapshot retained"),*Map.ToString());
  return;
 }
 if(!bRestorePosition)
 {
  Player->SetActorLocationAndRotation(Arrival,Rotation,false,nullptr,ETeleportType::TeleportPhysics);
  if(auto* Controller=Cast<APlayerController>(Player->GetController()))Controller->SetControlRotation(Rotation);
 }
 State->ClearPendingTravel();
 State->ArrivalGraceUntilSeconds=GetWorld()->GetTimeSeconds()+2.0;
}
void AAskirGameMode::MakeBlock(FVector Position,FVector Size)
{
 if(!GetWorld())return;
 AStaticMeshActor* Block=GetWorld()->SpawnActor<AStaticMeshActor>(Position,FRotator::ZeroRotator);
 if(!Block)return;
 UStaticMesh* Cube=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube"));
 UStaticMeshComponent* Mesh=Block->GetStaticMeshComponent();
 if(Cube)Mesh->SetStaticMesh(Cube);
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
     UStaticMesh* Cube=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube"));
     if(Cube)Shape->SetStaticMesh(Cube);
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
  if(DemoStage==4 && !bDemoFinished){bDemoFinished=true;Nearest->SetUsed(true);if(P->ProgressionComponent)P->ProgressionComponent->AwardExperience(100);}
  break;
 }
}
void AAskirGameMode::DemoQuickSave()
{
 const bool bOk=SaveGame(TEXT("AskirQuick"));
 if(auto* PC=Cast<AAskirPlayerController>(UGameplayStatics::GetPlayerController(this,0)))PC->ShowNotification(FText::FromString(bOk?TEXT("Spiel gespeichert."):TEXT("Speichern fehlgeschlagen.")));
 if(GEngine)GEngine->AddOnScreenDebugMessage(5004,3.f,bOk?FColor::Green:FColor::Red,bOk?TEXT("Spiel gespeichert"):TEXT("Speichern fehlgeschlagen"));
}
void AAskirGameMode::DemoQuickLoad()
{
 const bool bOk=LoadGame(TEXT("AskirQuick"));
 if(auto* PC=Cast<AAskirPlayerController>(UGameplayStatics::GetPlayerController(this,0)))PC->ShowNotification(FText::FromString(bOk?TEXT("Spiel geladen."):TEXT("Laden fehlgeschlagen.")));
 if(GEngine)GEngine->AddOnScreenDebugMessage(5004,3.f,bOk?FColor::Green:FColor::Red,bOk?TEXT("Spiel geladen"):TEXT("Laden fehlgeschlagen"));
}
