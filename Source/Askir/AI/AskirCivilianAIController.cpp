#include "AI/AskirCivilianAIController.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
AAskirCivilianAIController::AAskirCivilianAIController(){PrimaryActorTick.bCanEverTick=false;}
void AAskirCivilianAIController::OnPossess(APawn* InPawn)
{
 Super::OnPossess(InPawn);
 Routine=InPawn?InPawn->FindComponentByClass<UAskirDailyRoutineComponent>():nullptr;
 if(Routine)
 {
  Routine->OnRoutineChanged.AddDynamic(this,&AAskirCivilianAIController::HandleRoutineChanged);
  RefreshDestination();
 }
}
void AAskirCivilianAIController::OnUnPossess()
{
 if(Routine)Routine->OnRoutineChanged.RemoveDynamic(this,&AAskirCivilianAIController::HandleRoutineChanged);
 Routine=nullptr;ActiveDestination=NAME_None;StopMovement();Super::OnUnPossess();
}
void AAskirCivilianAIController::HandleRoutineChanged(EAskirDailyActivity Activity,FName DestinationTag)
{
 (void)Activity;
 TravelToTag(DestinationTag);
}
bool AAskirCivilianAIController::RefreshDestination()
{
 return Routine && TravelToTag(Routine->GetDestinationTag());
}
bool AAskirCivilianAIController::TravelToTag(FName DestinationTag)
{
 if(!GetWorld()||!GetPawn()||DestinationTag.IsNone())return false;
 TArray<AActor*> Candidates;
 UGameplayStatics::GetAllActorsWithTag(this,DestinationTag,Candidates);
 AActor* Chosen=nullptr;
 float BestDistance=TNumericLimits<float>::Max();
 for(AActor* Candidate:Candidates)
 {
  if(!IsValid(Candidate)||Candidate==GetPawn())continue;
  const float Distance=FVector::DistSquared(GetPawn()->GetActorLocation(),Candidate->GetActorLocation());
  if(!Chosen||(bChooseNearestDestination&&Distance<BestDistance))
  {Chosen=Candidate;BestDistance=Distance;}
 }
 if(!Chosen)return false;
 const EPathFollowingRequestResult::Type Result=MoveToActor(Chosen,FMath::Max(20.f,AcceptanceRadius));
 if(Result==EPathFollowingRequestResult::Failed)return false;
 ActiveDestination=DestinationTag;
 return true;
}
