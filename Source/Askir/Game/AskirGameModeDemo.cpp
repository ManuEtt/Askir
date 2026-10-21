#include "Game/AskirGameMode.h"
#include "Demo/AskirDemoInteractable.h"
#include "Characters/AskirCharacter.h"
#include "Characters/AskirNPC.h"
#include "Combat/AskirCombatComponent.h"
#include "Combat/AskirTargetLockComponent.h"
#include "Dialogue/AskirConversationComponent.h"
#include "Progression/AskirProgressionComponent.h"
#include "Camp/AskirCampComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Game/AskirPlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Travel/AskirWorldTravelSubsystem.h"
#include "Travel/AskirWorldPortal.h"
#include "Travel/AskirTravelRules.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/PackageName.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"

namespace
{
AskirTravelRules::Point TravelPoint(const FVector& V){return {V.X,V.Y,V.Z};}
}
void AAskirGameMode::BeginPlay()
{
 Super::BeginPlay();
 if(bEnableLegacyDemo)BuildDemoScene();
 // This callback owns readiness too: startup is not ready before travel restoration.
 GetWorldTimerManager().SetTimerForNextTick(this,&AAskirGameMode::RestorePendingArrival);
}
void AAskirGameMode::LogRuntimeReadiness()
{
 const auto* Instance=GetGameInstance();
 const auto* State=Instance?Instance->GetSubsystem<UAskirWorldTravelSubsystem>():nullptr;
 if(State&&State->HasPendingTravel())return;
 auto* Player=Cast<AAskirCharacter>(UGameplayStatics::GetPlayerPawn(this,0));
 if(Player&&Player->IsPlayerControlled())
 {
  // Frontend polling may pause this world only after arrival restoration and
  // possession have completed. Blocked/missing-player paths never set this.
  bRuntimeReadyForFrontend=true;
  const FString Map=UGameplayStatics::GetCurrentLevelName(this,true);
  const TCHAR* Marker=(Map==TEXT("Dornfeld_V80")||Map==TEXT("Dornfelder_Umland_V80"))?TEXT("ASKIR_V80_RUNTIME_READY"):
    (Map==TEXT("Dornfeld_V79")?TEXT("ASKIR_V79_RUNTIME_READY"):
    (Map==TEXT("Dornfeld_V78")?TEXT("ASKIR_V78_RUNTIME_READY"):
    (Map==TEXT("Dornfeld_V77")?TEXT("ASKIR_V77_RUNTIME_READY"):TEXT("ASKIR_RUNTIME_READY"))));
  UE_LOG(LogTemp,Display,TEXT("%s map=%s player=%s"),Marker,*Map,*Player->GetName());
  if(IsValid(Player->TargetLockComponent)&&Player->TargetLockComponent->IsRegistered()&&Player->TargetLockComponent->GetOwner()==Player)
   UE_LOG(LogTemp,Display,TEXT("ASKIR_V82_TARGETING_READY map=%s player=%s component=AskirTargetLockComponent"),*Map,*Player->GetName());
  else UE_LOG(LogTemp,Error,TEXT("ASKIR_V82_TARGETING_BLOCKED: configured player target-lock component missing"));
  if(IsValid(Player->ConversationComponent)&&Player->ConversationComponent->IsRegistered()&&Player->ConversationComponent->GetOwner()==Player)
   UE_LOG(LogTemp,Display,TEXT("ASKIR_V83_CONVERSATION_READY map=%s player=%s component=AskirConversationComponent"),*Map,*Player->GetName());
  else UE_LOG(LogTemp,Error,TEXT("ASKIR_V83_CONVERSATION_BLOCKED: configured player conversation component missing"));
 }
 else UE_LOG(LogTemp,Error,TEXT("ASKIR_V80_RUNTIME_BLOCKED: player or native controller not initialized"));
}
bool AAskirGameMode::RetryPendingArrival()
{
 auto* State=GetGameInstance()?GetGameInstance()->GetSubsystem<UAskirWorldTravelSubsystem>():nullptr;
 const FName Map(*UGameplayStatics::GetCurrentLevelName(this,true));
 if(!State||!State->PendingSnapshot||!State->bArrivalFailed||IsPersistenceBusy())return false;
 if(State->PendingDestination!=Map)
 {
  // A failed Browse can leave a replacement world with the source map's name.
  // Its new actors must receive the captured timeline before gameplay resumes.
  if(State->PendingSnapshot->SavedMapName!=Map)return false;
  State->PendingDestination=Map;
  State->PendingArrivalFrom=NAME_None;
  State->PendingArrivalPortalID=NAME_None;
  State->PendingDeparturePortalID=NAME_None;
  State->bPendingRestorePosition=true;
 }
 ArrivalRestoreAttempts=0;
 State->ResumeArrival();
 GetWorldTimerManager().SetTimerForNextTick(this,&AAskirGameMode::RestorePendingArrival);
 return true;
}
bool AAskirGameMode::CanReturnToSavedWorld() const
{
 const auto* State=GetGameInstance()?GetGameInstance()->GetSubsystem<UAskirWorldTravelSubsystem>():nullptr;
 if(!State||!State->PendingSnapshot||!State->bArrivalFailed||IsPersistenceBusy())return false;
 const FName Map(*UGameplayStatics::GetCurrentLevelName(this,true));
 return State->PendingSnapshot->SavedMapName!=Map;
}
bool AAskirGameMode::ReturnToSavedWorld()
{
 if(!CanReturnToSavedWorld())return false;
 auto* State=GetGameInstance()->GetSubsystem<UAskirWorldTravelSubsystem>();
 FString Package;
 if(!State->RedirectToSavedWorld(Package))return false;
 UGameplayStatics::OpenLevel(this,FName(*Package));
 return true;
}
void AAskirGameMode::RestorePendingArrival()
{
 UGameInstance* Instance=GetGameInstance();
 auto* State=Instance?Instance->GetSubsystem<UAskirWorldTravelSubsystem>():nullptr;
 const FName Map(*UGameplayStatics::GetCurrentLevelName(this,true));
 if(!State||!State->HasPendingTravel())
 {
  if(!UGameplayStatics::GetPlayerPawn(this,0)&&AskirTravelRules::RetrySpawn(++ArrivalRestoreAttempts))
   GetWorldTimerManager().SetTimerForNextTick(this,&AAskirGameMode::RestorePendingArrival);
  else LogRuntimeReadiness();
  return;
 }
 if(State->bArrivalFailed)return;
 if(State->PendingDestination==Map&&!State->PendingSnapshot)
 {
  // Legacy arrival flags contain no timeline to restore. Acknowledge them so
  // old Blueprint requests cannot leave movement permanently locked.
  FName FromMap;
  State->ConsumeArrival(Map,FromMap);
  LogRuntimeReadiness();
  return;
 }
 const auto Fail=[&](const FText& Reason)
 {
  State->MarkArrivalFailed(Reason);
  if(auto* PC=Cast<AAskirPlayerController>(UGameplayStatics::GetPlayerController(this,0)))PC->ShowNotification(Reason);
 };
 if(State->PendingDestination!=Map||!State->PendingSnapshot)
 {
  Fail(NSLOCTEXT("Askir","ArrivalWrongWorld","Die Reise wurde unterbrochen. Bitte kehre zurück und versuche es erneut."));
  return;
 }
 auto* Player=Cast<AAskirCharacter>(UGameplayStatics::GetPlayerPawn(this,0));
 if(!Player||!Player->GetController()||!Player->GetCapsuleComponent())
 {
  if(AskirTravelRules::RetrySpawn(++ArrivalRestoreAttempts))GetWorldTimerManager().SetTimerForNextTick(this,&AAskirGameMode::RestorePendingArrival);
  else Fail(NSLOCTEXT("Askir","ArrivalPlayerMissing","Die Ankunft ist noch nicht möglich. Versuche es erneut oder kehre zurück."));
  return;
 }
 Player->SuspendForWorldTravel();
 const float Radius=Player->GetCapsuleComponent()->GetScaledCapsuleRadius();
 const float Height=Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
 const auto IsOutsidePortals=[&](const FVector& Candidate)
 {
  for(TActorIterator<AAskirWorldPortal> It(GetWorld());It;++It)
  {
   if(!It->Trigger)continue;
   const FBoxSphereBounds& Bounds=It->Trigger->Bounds;
   if(!AskirTravelRules::OutsideExpandedTrigger(TravelPoint(Candidate),TravelPoint(Bounds.Origin),TravelPoint(Bounds.BoxExtent),Radius,Height))return false;
  }
  return true;
 };
 const auto ValidateLanding=[&](FVector& Candidate,const FRotator& Facing)
 {
  if(Candidate.ContainsNaN()||Facing.ContainsNaN()||!GetWorld()->FindTeleportSpot(Player,Candidate,Facing)||!IsOutsidePortals(Candidate))return false;
  FCollisionQueryParams Query(SCENE_QUERY_STAT(AskirTravelGround),false,Player);
  for(TActorIterator<AAskirWorldPortal> It(GetWorld());It;++It)Query.AddIgnoredActor(*It);
  FHitResult Ground;
  if(!GetWorld()->LineTraceSingleByChannel(Ground,Candidate,Candidate-FVector(0,0,Height+80.f),ECC_Visibility,Query))return false;
  if(!Ground.Component.IsValid()||Ground.Component->GetCollisionResponseToChannel(ECC_Pawn)!=ECR_Block)return false;
  const float WalkableZ=Player->GetCharacterMovement()?Player->GetCharacterMovement()->GetWalkableFloorZ():.71f;
  if(!AskirTravelRules::GroundSupports(Candidate.Z-Height,Ground.ImpactPoint.Z,Ground.ImpactNormal.Z,WalkableZ))return false;
  Candidate.Z=Ground.ImpactPoint.Z+Height+2.f;
  // Grounding may change collision or trigger intersection; validate the final capsule.
  if(!GetWorld()->FindTeleportSpot(Player,Candidate,Facing)||!IsOutsidePortals(Candidate))return false;
  return AskirTravelRules::GroundSupports(Candidate.Z-Height,Ground.ImpactPoint.Z,Ground.ImpactNormal.Z,WalkableZ);
 };
 FVector Arrival=State->PendingSnapshot->PlayerLocation;
 FRotator Rotation=State->PendingSnapshot->PlayerRotation;
 bool bFound=false;
 if(State->bPendingRestorePosition)bFound=ValidateLanding(Arrival,Rotation);
 else
 {
  TSet<FName> SeenPortalIDs;
  AAskirWorldPortal* ArrivalPortal=nullptr;
  int32 Matches=0;
  bool bDuplicate=false;
  for(TActorIterator<AAskirWorldPortal> It(GetWorld());It;++It)
  {
   if(!It->PortalID.IsNone())
   {
    if(SeenPortalIDs.Contains(It->PortalID))bDuplicate=true;
    SeenPortalIDs.Add(It->PortalID);
   }
   const bool bMatches=State->PendingArrivalPortalID.IsNone()?It->DestinationMap==State->PendingArrivalFrom:It->PortalID==State->PendingArrivalPortalID;
   if(bMatches){++Matches;ArrivalPortal=*It;}
  }
  const bool bExplicit=!State->PendingArrivalPortalID.IsNone();
  const bool bReciprocal=ArrivalPortal&&ArrivalPortal->DestinationMap==State->PendingArrivalFrom
   &&(State->PendingDeparturePortalID.IsNone()||ArrivalPortal->DestinationPortalID==State->PendingDeparturePortalID)
   &&UAskirWorldTravelSubsystem::IsUsableWorldPackage(ArrivalPortal->DestinationPackagePath.IsEmpty()?
    FString::Printf(TEXT("/Game/Askir/WorldV42/%s"),*ArrivalPortal->DestinationMap.ToString()):ArrivalPortal->DestinationPackagePath,State->PendingArrivalFrom);
  if(!bDuplicate&&AskirTravelRules::ArrivalPortalMatches(bExplicit,Matches,bReciprocal)&&ArrivalPortal)
  {
   Arrival=ArrivalPortal->GetActorLocation()+ArrivalPortal->GetActorRotation().RotateVector(ArrivalPortal->ArrivalOffset);
   Rotation=ArrivalPortal->GetActorRotation();
   bFound=ValidateLanding(Arrival,Rotation);
  }
  // Only a legacy request without an explicit portal may use a PlayerStart.
  if(!bFound&&!bExplicit&&!bDuplicate&&Matches==0)
   for(TActorIterator<APlayerStart> It(GetWorld());It;++It)
   {
    FVector Candidate=It->GetActorLocation();
    if(ValidateLanding(Candidate,It->GetActorRotation())){Arrival=Candidate;Rotation=It->GetActorRotation();bFound=true;break;}
   }
 }
 if(!bFound)
 {
  Fail(NSLOCTEXT("Askir","ArrivalNoSafeGround","Der Ankunftsort ist nicht sicher. Versuche es erneut oder kehre zurück."));
  return;
 }
 // The same validated landing is used for portals and cross-map save loads.
 // RestoreSnapshot(false) cannot reposition into a trigger after our validation.
 if(!RestoreSnapshot(State->PendingSnapshot,false))
 {
  Fail(NSLOCTEXT("Askir","ArrivalRestoreFailed","Dein Spielstand konnte noch nicht übernommen werden. Versuche es erneut oder kehre zurück."));
  return;
 }
 if(!Player->SetActorLocationAndRotation(Arrival,Rotation,false,nullptr,ETeleportType::TeleportPhysics))
 {
  Fail(NSLOCTEXT("Askir","ArrivalPlacementFailed","Die Ankunft ist noch nicht möglich. Versuche es erneut oder kehre zurück."));
  return;
 }
 if(auto* Controller=Cast<APlayerController>(Player->GetController()))Controller->SetControlRotation(Rotation);
 State->ClearPendingTravel();
 State->ArrivalGraceUntilSeconds=GetWorld()->GetTimeSeconds()+2.0;
 LogRuntimeReadiness();
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
