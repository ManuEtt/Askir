#include "Mission/AskirDornfeldMissionComponent.h"
#include "Animation/AskirAnimationStateComponent.h"
#include "Simulation/AskirNeedsComponent.h"
#include "Game/AskirGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Crafting/AskirCraftingComponent.h"
#include "Exploration/AskirExplorationComponent.h"
#include "Companion/AskirCompanionComponent.h"
#include "Camp/AskirCampComponent.h"
#include "Trade/AskirTradeComponent.h"
#include "Travel/AskirTravelComponent.h"
#include "Morale/AskirMoraleComponent.h"
#include "Stealth/AskirStealthComponent.h"
#include "Equipment/AskirEquipmentComponent.h"
#include "Survival/AskirSurvivalComponent.h"
#include "Progression/AskirProgressionComponent.h"
#include "Characters/AskirCharacter.h"
#include "Combat/AskirGuardComponent.h"
#include "Camera/CameraComponent.h"
#include "NPC/AskirInteractionComponent.h"
#include "NPC/AskirDialogueComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "Combat/AskirCombatComponent.h"
#include "Inventory/AskirInventoryComponent.h"
#include "Quest/AskirQuestComponent.h"
AAskirCharacter::AAskirCharacter()
{
    ProgressionComponent=CreateDefaultSubobject<UAskirProgressionComponent>(TEXT("Progression"));
    SurvivalComponent=CreateDefaultSubobject<UAskirSurvivalComponent>(TEXT("Survival"));
    EquipmentComponent=CreateDefaultSubobject<UAskirEquipmentComponent>(TEXT("Equipment"));
    StealthComponent=CreateDefaultSubobject<UAskirStealthComponent>(TEXT("Stealth"));
    MoraleComponent=CreateDefaultSubobject<UAskirMoraleComponent>(TEXT("Morale"));
    TravelComponent=CreateDefaultSubobject<UAskirTravelComponent>(TEXT("Travel"));
    TradeComponent=CreateDefaultSubobject<UAskirTradeComponent>(TEXT("Trade"));
    CampComponent=CreateDefaultSubobject<UAskirCampComponent>(TEXT("Camp"));
    CompanionComponent=CreateDefaultSubobject<UAskirCompanionComponent>(TEXT("Companion"));
    ExplorationComponent=CreateDefaultSubobject<UAskirExplorationComponent>(TEXT("Exploration"));
    CraftingComponent=CreateDefaultSubobject<UAskirCraftingComponent>(TEXT("Crafting"));
    NeedsComponent=CreateDefaultSubobject<UAskirNeedsComponent>(TEXT("Needs"));
    AnimationState=CreateDefaultSubobject<UAskirAnimationStateComponent>(TEXT("AnimationState"));
    PrimaryActorTick.bCanEverTick=true;
    GetCharacterMovement()->MaxWalkSpeed=WalkSpeed;
    GetCharacterMovement()->JumpZVelocity=600;
    CameraBoom=CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));CameraBoom->SetupAttachment(RootComponent);CameraBoom->TargetArmLength=330.f;CameraBoom->bUsePawnControlRotation=true;
    Camera=CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));Camera->SetupAttachment(CameraBoom);Camera->bUsePawnControlRotation=false;
    CombatComponent=CreateDefaultSubobject<UAskirCombatComponent>(TEXT("Combat"));
    GuardComponent=CreateDefaultSubobject<UAskirGuardComponent>(TEXT("Guard"));
    InventoryComponent=CreateDefaultSubobject<UAskirInventoryComponent>(TEXT("InventoryComponent"));
    QuestComponent=CreateDefaultSubobject<UAskirQuestComponent>(TEXT("QuestComponent"));
    DornfeldMission=CreateDefaultSubobject<UAskirDornfeldMissionComponent>(TEXT("DornfeldMission"));
}
void AAskirCharacter::BeginPlay(){Super::BeginPlay();Stamina=FMath::Max(0.f,MaxStamina);if(CombatComponent)CombatComponent->OnDeath.AddDynamic(this,&AAskirCharacter::HandleDeath);}
void AAskirCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if(!IsDead() && GetWorld() && GetWorld()->GetTimeSeconds()-LastStaminaUseTime>=StaminaRegenDelay)
        Stamina=FMath::Min(FMath::Max(0.f,MaxStamina),Stamina+FMath::Max(0.f,StaminaRegenPerSecond)*DeltaSeconds);
}
void AAskirCharacter::SetupPlayerInputComponent(UInputComponent*P)
{
    Super::SetupPlayerInputComponent(P);
    P->BindAxis("MoveForward",this,&AAskirCharacter::MoveForward);P->BindAxis("MoveRight",this,&AAskirCharacter::MoveRight);P->BindAxis("Turn",this,&AAskirCharacter::Turn);P->BindAxis("LookUp",this,&AAskirCharacter::LookUp);
    P->BindAction("Jump",IE_Pressed,this,&ACharacter::Jump);P->BindAction("Sprint",IE_Pressed,this,&AAskirCharacter::StartSprint);P->BindAction("Sprint",IE_Released,this,&AAskirCharacter::StopSprint);P->BindAction("Attack",IE_Pressed,this,&AAskirCharacter::Attack);
    P->BindAction("HeavyAttack",IE_Pressed,this,&AAskirCharacter::HeavyAttack);
    P->BindAction("Dodge",IE_Pressed,this,&AAskirCharacter::Dodge);
    P->BindAction("Block",IE_Pressed,this,&AAskirCharacter::StartBlock);
    P->BindAction("Block",IE_Released,this,&AAskirCharacter::StopBlock);
    P->BindAction("Interact",IE_Pressed,this,&AAskirCharacter::DemoInteractInput);
    P->BindAction("QuickSave",IE_Pressed,this,&AAskirCharacter::DemoSaveInput);
    P->BindAction("QuickLoad",IE_Pressed,this,&AAskirCharacter::DemoLoadInput);
}
void AAskirCharacter::MoveForward(float V){if(V)AddMovementInput(GetActorForwardVector(),V);}
void AAskirCharacter::MoveRight(float V){if(V)AddMovementInput(GetActorRightVector(),V);}
void AAskirCharacter::Turn(float V){AddControllerYawInput(V);}
void AAskirCharacter::LookUp(float V){AddControllerPitchInput(V);}
void AAskirCharacter::StartSprint(){GetCharacterMovement()->MaxWalkSpeed=SprintSpeed;}
void AAskirCharacter::StopSprint(){GetCharacterMovement()->MaxWalkSpeed=WalkSpeed;}
bool AAskirCharacter::SpendStamina(float Cost)
{
    Cost=FMath::Max(0.f,Cost);
    if(Stamina<Cost || !GetWorld())return false;
    Stamina=FMath::Max(0.f,Stamina-Cost);
    LastStaminaUseTime=GetWorld()->GetTimeSeconds();
    return true;
}
void AAskirCharacter::SetImpactFromAnimationNotify(bool bEnabled){bUseAnimationNotifyForImpact=bEnabled;}
void AAskirCharacter::AbortPendingAttack(){CancelPendingAttack();}
void AAskirCharacter::FallbackPendingAttackToTimer()
{
    if(!bPendingAttack||!GetWorld())return;
    bUseAnimationNotifyForImpact=false;
    GetWorldTimerManager().ClearTimer(PendingImpactTimer);
    const float Delay=bPendingHeavyAttack?HeavyImpactDelay:LightImpactDelay;
    GetWorldTimerManager().SetTimer(PendingImpactTimer,this,&AAskirCharacter::ResolvePendingAttackImpact,
        FMath::Max(0.01f,Delay),false);
}
void AAskirCharacter::Attack(){PerformAttack(1.f,LightAttackCost,AttackCooldown,false);}
void AAskirCharacter::HeavyAttack(){PerformAttack(HeavyDamageMultiplier,HeavyAttackCost,HeavyAttackCooldown,true);}
void AAskirCharacter::PerformAttack(float DamageMultiplier,float Cost,float Cooldown,bool bHeavyAttack)
{
    if(!CombatComponent||CombatComponent->IsDead()||bBlocking||bPendingAttack||!GetWorld())return;
    const float Now=GetWorld()->GetTimeSeconds();
    if(Now-LastAttackTime<FMath::Max(0.f,Cooldown)||!SpendStamina(Cost))return;
    LastAttackTime=Now;
    if(AnimationState){if(bHeavyAttack)AnimationState->BeginHeavyAttack();else AnimationState->BeginLightAttack();}
    OnAttackStarted(bHeavyAttack);
    // Damage is no longer applied at button press. A notify or timer opens the hit window.
    bPendingAttack=true;
    PendingDamageMultiplier=DamageMultiplier;
    bPendingHeavyAttack=bHeavyAttack;
    GetWorldTimerManager().ClearTimer(PendingImpactTimer);
    // A missing AnimNotify must never leave bPendingAttack permanently true.
    // The watchdog is deliberately longer than the normal timed hit window.
    const float Delay=bUseAnimationNotifyForImpact
        ? FMath::Max(2.0f, (bHeavyAttack?HeavyImpactDelay:LightImpactDelay)*3.0f)
        : FMath::Max(0.01f,bHeavyAttack?HeavyImpactDelay:LightImpactDelay);
    GetWorldTimerManager().SetTimer(PendingImpactTimer,this,
        bUseAnimationNotifyForImpact ? &AAskirCharacter::ExpireUnresolvedAnimationAttack : &AAskirCharacter::ResolvePendingAttackImpact,Delay,false);
}
void AAskirCharacter::ExpireUnresolvedAnimationAttack()
{
    // A missing montage notify is a missed hit, never delayed unavoidable damage.
    if (!bPendingAttack) return;
    const bool bHeavy = bPendingHeavyAttack;
    CancelPendingAttack();
    OnAttackMissed(bHeavy);
}
void AAskirCharacter::ResolvePendingAttackImpact()
{
    if(!bPendingAttack)return; // one damage resolution per attack, even if notify fires twice
    bPendingAttack=false;
    GetWorldTimerManager().ClearTimer(PendingImpactTimer);
    if(IsDead()||!GetWorld())return;
    const float DamageMultiplier=PendingDamageMultiplier;
    const bool bHeavyAttack=bPendingHeavyAttack;
    // Begin in front of the player, not at the camera behind the character.
    const FRotator ViewRotation=Controller?Controller->GetControlRotation():GetActorRotation();
    FVector Direction=ViewRotation.Vector().GetSafeNormal2D();
    if(Direction.IsNearlyZero())Direction=GetActorForwardVector().GetSafeNormal2D();
    const FVector Start=GetActorLocation()+FVector(0.f,0.f,GetCapsuleComponent()->GetScaledCapsuleHalfHeight()*0.55f);
    const FVector End=Start+Direction*FMath::Max(0.f,AttackRange);
    FCollisionQueryParams Params(SCENE_QUERY_STAT(AskirMelee),false,this);
    FHitResult Hit;
    // Visibility collision stops the attack at world geometry.
    if(!GetWorld()->SweepSingleByChannel(Hit,Start,End,FQuat::Identity,ECC_Visibility,FCollisionShape::MakeSphere(FMath::Max(1.f,AttackRadius)),Params))
    {
        OnAttackMissed(bHeavyAttack);
        return;
    }
    if(AActor* Target=Hit.GetActor())
    {
        if(UAskirCombatComponent* TargetCombat=Target->FindComponentByClass<UAskirCombatComponent>())
        {
            if(TargetCombat->IsDead()) { OnAttackMissed(bHeavyAttack); return; }
            FAskirDamageInfo Info;Info.Damage=FMath::Max(0.f,BaseAttackDamage*DamageMultiplier);
            Info.DamageCauser=this;Info.HitLocation=Hit.ImpactPoint;Info.HitDirection=Direction;
            if(AAskirCharacter* TargetCharacter=Cast<AAskirCharacter>(Target))TargetCharacter->ReceiveAskirDamage(Info);
            else TargetCombat->ApplyDamage(Info);
            OnAttackHit(Target,Hit.ImpactPoint,bHeavyAttack);
        }
        else OnAttackMissed(bHeavyAttack);
    }
    else OnAttackMissed(bHeavyAttack);
}
void AAskirCharacter::CancelPendingAttack()
{
    bPendingAttack=false;
    bUseAnimationNotifyForImpact=false;
    GetWorldTimerManager().ClearTimer(PendingImpactTimer);
}
void AAskirCharacter::Dodge()
{
    if(IsDead()||bBlocking||!GetWorld())return;
    const float Now=GetWorld()->GetTimeSeconds();
    if(Now-LastDodgeTime<FMath::Max(0.f,DodgeCooldown)||!SpendStamina(DodgeCost))return;
    LastDodgeTime=Now;
    if(CombatComponent)CombatComponent->GrantDodgeInvulnerability(DodgeInvulnerabilitySeconds);
    if(AnimationState)AnimationState->BeginDodge();
    OnDodgeStarted();
    FVector Direction=GetLastMovementInputVector().GetSafeNormal2D();
    if(Direction.IsNearlyZero())Direction=GetActorForwardVector().GetSafeNormal2D();
    LaunchCharacter(Direction*DodgeStrength,false,false);
}
void AAskirCharacter::StartBlock(){if(!IsDead()&&!bBlocking){bBlocking=true;if(AnimationState)AnimationState->SetBlocking(true);OnBlockStateChanged(true);}}
void AAskirCharacter::StopBlock(){if(bBlocking){bBlocking=false;if(AnimationState)AnimationState->SetBlocking(false);OnBlockStateChanged(false);}}
void AAskirCharacter::Heal(float A){if(CombatComponent)CombatComponent->Heal(A);}
void AAskirCharacter::ReceiveAskirDamage(const FAskirDamageInfo&I)
{
    if(!CombatComponent||CombatComponent->IsDead())return;
    FAskirDamageInfo Applied=I;
    if(bBlocking && I.DamageType!=EAskirDamageType::TrueDamage)
    {
        const float Cost=FMath::Max(0.f,I.Damage)*FMath::Max(0.f,BlockStaminaPerDamage);
        if(SpendStamina(Cost))Applied.Damage*=1.f-FMath::Clamp(BlockDamageReduction,0.f,1.f);
        else StopBlock();
    }
    if(AnimationState&&Applied.Damage>0.f && !CombatComponent->IsDodgeInvulnerable())AnimationState->RegisterHit();
    CombatComponent->ApplyDamage(Applied);
}
bool AAskirCharacter::IsDead()const{return CombatComponent&&CombatComponent->IsDead();}
void AAskirCharacter::HandleDeath(){CancelPendingAttack();if(AnimationState)AnimationState->RegisterDeath();StopBlock();GetCharacterMovement()->DisableMovement();GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);}
void AAskirCharacter::DemoInteractInput()
{
 if(!GetWorld()||!Camera||IsDead())return;
 const FVector Start=Camera->GetComponentLocation();
 const FVector End=Start+Camera->GetForwardVector()*400.f;
 FHitResult Hit;
 FCollisionQueryParams Query(SCENE_QUERY_STAT(AskirInteract),false,this);
 if(GetWorld()->LineTraceSingleByChannel(Hit,Start,End,ECC_Visibility,Query))
 {
  AActor* Target=Hit.GetActor();
  if(IsValid(Target))
  {
   if(UAskirInteractionComponent* Interaction=Target->FindComponentByClass<UAskirInteractionComponent>())
   {
    if(Interaction->Interact(this))
    {
     if(UAskirDialogueComponent* Dialogue=Target->FindComponentByClass<UAskirDialogueComponent>())Dialogue->BeginDialogue();
     return;
    }
   }
  }
 }
 if(AAskirGameMode* GM=Cast<AAskirGameMode>(UGameplayStatics::GetGameMode(this)))GM->DemoInteract();
}
void AAskirCharacter::DemoSaveInput(){if(AAskirGameMode* GM=Cast<AAskirGameMode>(UGameplayStatics::GetGameMode(this)))GM->DemoQuickSave();}
void AAskirCharacter::DemoLoadInput(){if(AAskirGameMode* GM=Cast<AAskirGameMode>(UGameplayStatics::GetGameMode(this)))GM->DemoQuickLoad();}
