#include "Mission/AskirDornfeldMissionComponent.h"
#include "Magic/AskirMagicComponent.h"
#include "Animation/AskirAnimationPlaybackComponent.h"
#include "Audio/AskirAudioLibrary.h"
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
#include "NPC/AskirNPCInteractionComponent.h"
#include "NPC/AskirDialogueComponent.h"
#include "Interaction/AskirInteractable.h"
#include "Game/AskirPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Engine/LocalPlayer.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
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
    MagicComponent=CreateDefaultSubobject<UAskirMagicComponent>(TEXT("Magic"));
    AnimationPlayback=CreateDefaultSubobject<UAskirAnimationPlaybackComponent>(TEXT("AnimationPlayback"));
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
    bUseControllerRotationYaw=false;
    GetCharacterMovement()->bOrientRotationToMovement=true;
    GetCharacterMovement()->RotationRate=FRotator(0.f,540.f,0.f);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
    ProxyVisual=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProxyVisual"));
    ProxyVisual->SetupAttachment(RootComponent);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Body(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if(Body.Succeeded())ProxyVisual->SetStaticMesh(Body.Object);
    ProxyVisual->SetRelativeScale3D(FVector(.55f,.55f,1.6f));
    ProxyVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
void AAskirCharacter::BeginPlay()
{
    Super::BeginPlay();
    Stamina=FMath::Max(0.f,MaxStamina);
    if(ProxyVisual)ProxyVisual->SetVisibility(!GetMesh()->GetSkeletalMeshAsset());
    if(CombatComponent)CombatComponent->OnDeath.AddDynamic(this,&AAskirCharacter::HandleDeath);
}
void AAskirCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if(bSprinting && !IsDead() && GetVelocity().SizeSquared2D()>1.f)
        if(!SpendStamina(FMath::Max(0.f,DeltaSeconds)*12.f))StopSprint();
    if(!IsDead() && GetWorld() && GetWorld()->GetTimeSeconds()-LastStaminaUseTime>=StaminaRegenDelay)
        Stamina=FMath::Min(FMath::Max(0.f,MaxStamina),Stamina+FMath::Max(0.f,StaminaRegenPerSecond)*DeltaSeconds);
}
void AAskirCharacter::SetupPlayerInputComponent(UInputComponent*P)
{
    Super::SetupPlayerInputComponent(P);
    UEnhancedInputComponent* Input=Cast<UEnhancedInputComponent>(P);
    if(!Input){UE_LOG(LogTemp,Error,TEXT("Askir requires EnhancedInputComponent; check DefaultInput.ini"));return;}
    InitializeEnhancedInput();
    if(RuntimeInputActions.Num()!=19)return;
    Input->BindAction(RuntimeInputActions[0],ETriggerEvent::Triggered,this,&AAskirCharacter::InputMoveForward);
    Input->BindAction(RuntimeInputActions[1],ETriggerEvent::Triggered,this,&AAskirCharacter::InputMoveRight);
    Input->BindAction(RuntimeInputActions[2],ETriggerEvent::Triggered,this,&AAskirCharacter::InputTurn);
    Input->BindAction(RuntimeInputActions[3],ETriggerEvent::Triggered,this,&AAskirCharacter::InputLookUp);
    Input->BindAction(RuntimeInputActions[4],ETriggerEvent::Started,this,&AAskirCharacter::InputJump);
    Input->BindAction(RuntimeInputActions[4],ETriggerEvent::Completed,this,&AAskirCharacter::InputStopJump);
    Input->BindAction(RuntimeInputActions[5],ETriggerEvent::Started,this,&AAskirCharacter::StartSprint);
    Input->BindAction(RuntimeInputActions[5],ETriggerEvent::Completed,this,&AAskirCharacter::StopSprint);
    Input->BindAction(RuntimeInputActions[5],ETriggerEvent::Canceled,this,&AAskirCharacter::StopSprint);
    Input->BindAction(RuntimeInputActions[6],ETriggerEvent::Started,this,&AAskirCharacter::Attack);
    Input->BindAction(RuntimeInputActions[7],ETriggerEvent::Started,this,&AAskirCharacter::HeavyAttack);
    Input->BindAction(RuntimeInputActions[8],ETriggerEvent::Started,this,&AAskirCharacter::Dodge);
    Input->BindAction(RuntimeInputActions[9],ETriggerEvent::Started,this,&AAskirCharacter::StartBlock);
    Input->BindAction(RuntimeInputActions[9],ETriggerEvent::Completed,this,&AAskirCharacter::StopBlock);
    Input->BindAction(RuntimeInputActions[9],ETriggerEvent::Canceled,this,&AAskirCharacter::StopBlock);
    Input->BindAction(RuntimeInputActions[10],ETriggerEvent::Started,this,&AAskirCharacter::DemoInteractInput);
    Input->BindAction(RuntimeInputActions[11],ETriggerEvent::Started,this,&AAskirCharacter::DemoSaveInput);
    Input->BindAction(RuntimeInputActions[12],ETriggerEvent::Started,this,&AAskirCharacter::DemoLoadInput);
    Input->BindAction(RuntimeInputActions[13],ETriggerEvent::Started,this,&AAskirCharacter::InputPause);
    Input->BindAction(RuntimeInputActions[14],ETriggerEvent::Started,this,&AAskirCharacter::InputInventory);
    Input->BindAction(RuntimeInputActions[15],ETriggerEvent::Started,this,&AAskirCharacter::InputJournal);
    Input->BindAction(RuntimeInputActions[16],ETriggerEvent::Started,this,&AAskirCharacter::InputCast);
    Input->BindAction(RuntimeInputActions[17],ETriggerEvent::Triggered,this,&AAskirCharacter::InputCycleSpell);
    Input->BindAction(RuntimeInputActions[18],ETriggerEvent::Started,this,&AAskirCharacter::InputMagicMenu);
}
void AAskirCharacter::InitializeEnhancedInput()
{
    if(!RuntimeInputMapping)
    {
        RuntimeInputMapping=NewObject<UInputMappingContext>(this);
        const FKey Keys[]={EKeys::W,EKeys::D,EKeys::MouseX,EKeys::MouseY,EKeys::SpaceBar,EKeys::LeftShift,EKeys::LeftMouseButton,EKeys::RightMouseButton,EKeys::LeftAlt,EKeys::Q,EKeys::E,EKeys::F5,EKeys::F9,EKeys::Escape,EKeys::Tab,EKeys::J,EKeys::F,EKeys::MouseWheelAxis,EKeys::K};
        for(int32 Index=0;Index<19;++Index)
        {
            UInputAction* Action=NewObject<UInputAction>(this);
            Action->ValueType=(Index<4||Index==17)?EInputActionValueType::Axis1D:EInputActionValueType::Boolean;
            Action->bTriggerWhenPaused=Index>=11&&Index<=15;
            RuntimeInputActions.Add(Action);
            FEnhancedActionKeyMapping& Mapping=RuntimeInputMapping->MapKey(Action,Keys[Index]);
            if(Index==3)Mapping.Modifiers.Add(NewObject<UInputModifierNegate>(RuntimeInputMapping));
        }
        RuntimeInputMapping->MapKey(RuntimeInputActions[0],EKeys::S).Modifiers.Add(NewObject<UInputModifierNegate>(RuntimeInputMapping));
        RuntimeInputMapping->MapKey(RuntimeInputActions[1],EKeys::A).Modifiers.Add(NewObject<UInputModifierNegate>(RuntimeInputMapping));
    }
    if(APlayerController* PC=Cast<APlayerController>(GetController()))
        if(ULocalPlayer* Local=PC->GetLocalPlayer())
            if(auto* Subsystem=Local->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())Subsystem->AddMappingContext(RuntimeInputMapping,0);
}
void AAskirCharacter::EndPlay(const EEndPlayReason::Type Reason)
{
    if(APlayerController* PC=Cast<APlayerController>(GetController()))
        if(ULocalPlayer* Local=PC->GetLocalPlayer())
            if(auto* Subsystem=Local->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())Subsystem->RemoveMappingContext(RuntimeInputMapping);
    CancelPendingAttack();
    Super::EndPlay(Reason);
}
void AAskirCharacter::InputMoveForward(const FInputActionValue& V){MoveForward(V.Get<float>());}
void AAskirCharacter::InputMoveRight(const FInputActionValue& V){MoveRight(V.Get<float>());}
void AAskirCharacter::InputTurn(const FInputActionValue& V){Turn(V.Get<float>());}
void AAskirCharacter::InputLookUp(const FInputActionValue& V){LookUp(V.Get<float>());}
void AAskirCharacter::InputJump(){if(CanUseGameplayInput())Jump();}
void AAskirCharacter::InputStopJump(){StopJumping();}
void AAskirCharacter::InputPause(){if(auto* PC=Cast<AAskirPlayerController>(Controller))PC->TogglePauseMenu();}
void AAskirCharacter::InputInventory(){if(auto* PC=Cast<AAskirPlayerController>(Controller))PC->ToggleInventory();}
void AAskirCharacter::InputJournal(){if(auto* PC=Cast<AAskirPlayerController>(Controller))PC->ToggleJournal();}
void AAskirCharacter::InputMagicMenu(){if(auto* PC=Cast<AAskirPlayerController>(Controller))PC->ToggleMagic();}
void AAskirCharacter::InputCast()
{
    if(!CanUseGameplayInput()||!MagicComponent)return;
    if(!MagicComponent->CastSelected())if(auto* PC=Cast<AAskirPlayerController>(Controller))PC->ShowNotification(MagicComponent->GetLastFailureText());
}
void AAskirCharacter::InputCycleSpell(const FInputActionValue& Value)
{
    if(CanUseGameplayInput()&&MagicComponent){const float Direction=Value.Get<float>();if(FMath::Abs(Direction)>.01f)MagicComponent->CycleSpell(Direction>0?1:-1);}
}
bool AAskirCharacter::CanUseGameplayInput() const
{
    if(IsDead()||!CombatComponent||!CombatComponent->IsDamageEnabled()||!GetWorld()||UGameplayStatics::IsGamePaused(this))return false;
    if(const auto* GM=Cast<AAskirGameMode>(UGameplayStatics::GetGameMode(this)))if(GM->IsPersistenceBusy())return false;
    const auto* PC=Cast<AAskirPlayerController>(Controller);return !PC||!PC->IsMenuOpen();
}
void AAskirCharacter::CancelGameplayActions(){AbortPendingAttack();StopBlock();StopSprint();StopJumping();if(MagicComponent)MagicComponent->CancelCasting();}
void AAskirCharacter::MoveForward(float V){if(V&&!IsDead()&&Controller)AddMovementInput(FRotationMatrix(FRotator(0,Controller->GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::X),V);}
void AAskirCharacter::MoveRight(float V){if(V&&!IsDead()&&Controller)AddMovementInput(FRotationMatrix(FRotator(0,Controller->GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::Y),V);}
void AAskirCharacter::Turn(float V){AddControllerYawInput(V);}
void AAskirCharacter::LookUp(float V){AddControllerPitchInput(V);}
void AAskirCharacter::StartSprint(){if(CanUseGameplayInput()&&Stamina>0.f&&!bBlocking){bSprinting=true;GetCharacterMovement()->MaxWalkSpeed=SprintSpeed;}}
void AAskirCharacter::StopSprint(){bSprinting=false;GetCharacterMovement()->MaxWalkSpeed=WalkSpeed;}
bool AAskirCharacter::SpendStamina(float Cost)
{
    Cost=FMath::Max(0.f,Cost);
    if(!FMath::IsFinite(Cost)||Stamina<Cost || !GetWorld())return false;
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
    if(!CanUseGameplayInput()||(MagicComponent&&MagicComponent->IsCasting()))return;
    if(const auto* PC=Cast<AAskirPlayerController>(Controller))if(PC->IsMenuOpen())return;
    const float Now=GetWorld()->GetTimeSeconds();
    if(Now<NextAttackAllowedTime||!SpendStamina(Cost))return;
    LastAttackTime=Now;
    NextAttackAllowedTime=Now+FMath::Max(0.f,Cooldown);
    StopSprint();
    bPendingAttack=true;
    PendingDamageMultiplier=DamageMultiplier;
    bPendingHeavyAttack=bHeavyAttack;
    if(AnimationState){if(bHeavyAttack)AnimationState->BeginHeavyAttack();else AnimationState->BeginLightAttack();}
    // Damage is no longer applied at button press. A notify or timer opens the hit window.
    GetWorldTimerManager().ClearTimer(PendingImpactTimer);
    // A missing AnimNotify must never leave bPendingAttack permanently true.
    // The watchdog is deliberately longer than the normal timed hit window.
    const float Delay=bUseAnimationNotifyForImpact
        ? FMath::Max(2.0f, (bHeavyAttack?HeavyImpactDelay:LightImpactDelay)*3.0f)
        : FMath::Max(0.01f,bHeavyAttack?HeavyImpactDelay:LightImpactDelay);
    GetWorldTimerManager().SetTimer(PendingImpactTimer,this,
        bUseAnimationNotifyForImpact ? &AAskirCharacter::ExpireUnresolvedAnimationAttack : &AAskirCharacter::ResolvePendingAttackImpact,Delay,false);
    UAskirAudioLibrary::PlayWorldCue(this,TEXT("MeleeSwing"));
    OnAttackStarted(bHeavyAttack);
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
            const float WeaponBonus=EquipmentComponent?EquipmentComponent->WeaponRating*FMath::Clamp(EquipmentComponent->Durability/100.f,0.f,1.f):0.f;
            FAskirDamageInfo Info;Info.Damage=FMath::Max(0.f,(BaseAttackDamage+WeaponBonus)*DamageMultiplier);
            Info.DamageCauser=this;Info.HitLocation=Hit.ImpactPoint;Info.HitDirection=Direction;
            if(AAskirCharacter* TargetCharacter=Cast<AAskirCharacter>(Target))TargetCharacter->ReceiveAskirDamage(Info);
            else TargetCombat->ApplyDamage(Info);
            UAskirAudioLibrary::PlayWorldCue(Target,TEXT("MeleeHit"));
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
    if(!CanUseGameplayInput()||bBlocking||!GetWorld())return;
    const float Now=GetWorld()->GetTimeSeconds();
    if(Now-LastDodgeTime<FMath::Max(0.f,DodgeCooldown)||!SpendStamina(DodgeCost))return;
    LastDodgeTime=Now;
    CancelPendingAttack();
    if(MagicComponent)MagicComponent->CancelCasting();
    if(CombatComponent)CombatComponent->GrantDodgeInvulnerability(DodgeInvulnerabilitySeconds);
    if(AnimationState)AnimationState->BeginDodge();
    OnDodgeStarted();
    FVector Direction=GetLastMovementInputVector().GetSafeNormal2D();
    if(Direction.IsNearlyZero())Direction=GetActorForwardVector().GetSafeNormal2D();
    LaunchCharacter(Direction*DodgeStrength,false,false);
}
void AAskirCharacter::StartBlock(){if(CanUseGameplayInput()&&!bBlocking){CancelPendingAttack();StopSprint();if(MagicComponent)MagicComponent->CancelCasting();bBlocking=true;if(AnimationState)AnimationState->SetBlocking(true);OnBlockStateChanged(true);}}
void AAskirCharacter::StopBlock(){if(bBlocking){bBlocking=false;if(AnimationState)AnimationState->SetBlocking(false);OnBlockStateChanged(false);}}
void AAskirCharacter::Heal(float A){if(CombatComponent)CombatComponent->Heal(A);}
void AAskirCharacter::ReceiveAskirDamage(const FAskirDamageInfo&I)
{
    if(!CombatComponent||CombatComponent->IsDead()||!CombatComponent->IsDamageEnabled()||CombatComponent->IsDodgeInvulnerable()||!FMath::IsFinite(I.Damage)||I.Damage<=0.f)return;
    FAskirDamageInfo Applied=I;
    if(bBlocking && I.DamageType!=EAskirDamageType::TrueDamage)
    {
        const float Cost=FMath::Max(0.f,I.Damage)*FMath::Max(0.f,BlockStaminaPerDamage);
        if(SpendStamina(Cost))Applied.Damage*=1.f-FMath::Clamp(BlockDamageReduction,0.f,1.f);
        else StopBlock();
    }
    if(EquipmentComponent && I.DamageType==EAskirDamageType::Physical)
    {
        const float Armor=FMath::Max(0.f,EquipmentComponent->ArmorRating)*FMath::Clamp(EquipmentComponent->Durability/100.f,0.f,1.f);
        Applied.Damage*=100.f/(100.f+Armor);
        EquipmentComponent->DamageGear(Applied.Damage*.05f);
    }
    const float DamageReceived=CombatComponent->ApplyDamage(Applied);
    if(AnimationState&&DamageReceived>0.f)AnimationState->RegisterHit();
}
bool AAskirCharacter::IsDead()const{return CombatComponent&&CombatComponent->IsDead();}
void AAskirCharacter::HandleDeath(){CancelPendingAttack();StopSprint();if(AnimationState)AnimationState->RegisterDeath();StopBlock();GetCharacterMovement()->DisableMovement();GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);}
void AAskirCharacter::RestoreAfterLoad()
{
    CancelGameplayActions();
    LastAttackTime=LastDodgeTime=LastStaminaUseTime=NextAttackAllowedTime=-1000.f;
    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->SetMovementMode(IsDead()?MOVE_None:MOVE_Walking);
    GetCapsuleComponent()->SetCollisionEnabled(IsDead()?ECollisionEnabled::NoCollision:ECollisionEnabled::QueryAndPhysics);
    if(AnimationState)AnimationState->ResetAfterLoad(IsDead());
    if(AnimationPlayback)AnimationPlayback->RestorePlaybackAfterLoad();
}
void AAskirCharacter::DemoInteractInput()
{
 if(!GetWorld()||!Camera||IsDead())return;
 if(const auto* PC=Cast<AAskirPlayerController>(Controller))if(PC->IsMenuOpen())return;
 const FVector Start=GetActorLocation()+FVector(0,0,50);
 const FVector End=Start+(Controller?Controller->GetControlRotation().Vector():GetActorForwardVector())*250.f;
 FHitResult Hit;
 FCollisionQueryParams Query(SCENE_QUERY_STAT(AskirInteract),false,this);
 if(GetWorld()->LineTraceSingleByChannel(Hit,Start,End,ECC_Visibility,Query))
 {
  AActor* Target=Hit.GetActor();
  if(IsValid(Target))
  {
   if(Target->GetClass()->ImplementsInterface(UAskirInteractable::StaticClass()))
   {
    const bool bOk=IAskirInteractable::Execute_Interact(Target,this);
    if(auto* PC=Cast<AAskirPlayerController>(Controller))PC->ShowNotification(bOk?IAskirInteractable::Execute_GetInteractionText(Target):NSLOCTEXT("AskirUI","Unavailable","Diese Interaktion ist derzeit nicht möglich."));
    return;
   }
   if(UAskirNPCInteractionComponent* Interaction=Target->FindComponentByClass<UAskirNPCInteractionComponent>())
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
