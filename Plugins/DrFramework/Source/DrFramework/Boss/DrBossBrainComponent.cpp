#include "Boss/DrBossBrainComponent.h"
#include "Combat/DrCombatFXComponent.h"
#include "Audio/DrAudioSubsystem.h"

#include "Boss/DrBossCharacter.h"
#include "Boss/DrBossData.h"
#include "Combat/DrHitFeedbackSubsystem.h"
#include "Abilities/DrAbilitySystemComponent.h"
#include "Abilities/DrAttributeSet.h"
#include "Characters/DrCharacter.h"
#include "Core/DrGameplayTags.h"
#include "AbilitySystemGlobals.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/RootMotionSource.h"

namespace { const FName BossMoveName(TEXT("DrBossMove")); }
DEFINE_LOG_CATEGORY_STATIC(LogDrBossBrain, Log, All);
UDrBossBrainComponent::UDrBossBrainComponent() {
  PrimaryComponentTick.bCanEverTick = true;
  PrimaryComponentTick.bStartWithTickEnabled = true;
}
ADrBossCharacter* UDrBossBrainComponent::Boss() const { return Cast<ADrBossCharacter>(GetOwner()); }
void UDrBossBrainComponent::BeginPlay() {
  Super::BeginPlay();
  if (!bInjected) {
    TArray<FDrBossPhaseRow> LoadedPhases;
    TArray<FDrBossAttackRow> LoadedAttacks;
    for (const auto& Row : FDrBossPhaseRow::GetAll())
      if (Boss() && Row.Boss == Boss()->GetData()->BossName) LoadedPhases.Add(Row);
    for (const auto& Row : FDrBossAttackRow::GetAll()) LoadedAttacks.Add(Row);
    InjectTables(LoadedPhases, LoadedAttacks);
  }
  if (Boss()) Boss()->PlaySequence(Boss()->GetData()->IdleAnim, true);
}
void UDrBossBrainComponent::InjectTables(const TArray<FDrBossPhaseRow>& InPhases, const TArray<FDrBossAttackRow>& InAttacks) {
  StopMove();
  Phases = InPhases;
  Phases.Sort([](const auto& A, const auto& B) { return A.Phase < B.Phase; });
  Attacks = InAttacks;
  PhaseIndex = 0; PendingPhase = INDEX_NONE; AttackIndex = INDEX_NONE;
  CooldownUntil.Reset(); HitActors.Reset(); Clock = 0;
  bInjected = true;
  SetState(EDrBossState::Idle);
}
bool UDrBossBrainComponent::HasLivingTarget() const {
  if (!Target.IsValid()) return false;
  const UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target.Get());
  return !ASC || (!ASC->HasMatchingGameplayTag(DrTags::State_Dead) &&
      ASC->GetNumericAttribute(UDrAttributeSet::GetHealthAttribute()) > 0);
}
void UDrBossBrainComponent::ResetEncounter() {
  StopMove();
  if (auto* FX = Boss()->FindComponentByClass<UDrCombatFXComponent>()) FX->Reset();
  PhaseIndex = 0;
  PendingPhase = INDEX_NONE;
  AttackIndex = INDEX_NONE;
  CooldownUntil.Reset();
  HitActors.Reset();
  Clock = 0;
  StateElapsed = 0;
  LockedDirection = FVector::ForwardVector;
  bDirectionLocked = false;
  SetState(EDrBossState::Idle);
}

bool UDrBossBrainComponent::IsFacingTarget() const {
  if (!Target.IsValid() || !Boss())
    return false;
  const FVector Direction = (Target->GetActorLocation() - Boss()->GetActorLocation()).GetSafeNormal2D();
  return Direction.IsNearlyZero() || FVector::DotProduct(Boss()->GetActorForwardVector(), Direction) >=
      FMath::Cos(FMath::DegreesToRadians(Boss()->GetData()->AttackFacingAngle));
}

void UDrBossBrainComponent::WarnMissingTables() {
  if (bWarnedMissingTables)
    return;
  bool bHasPatterns = Phases.IsValidIndex(PhaseIndex);
  if (bHasPatterns)
    bHasPatterns = Attacks.ContainsByPredicate([this](const FDrBossAttackRow &Row) {
      return Row.PatternSet == Phases[PhaseIndex].PatternSet;
    });
  if (!bHasPatterns) {
    bWarnedMissingTables = true;
    UE_LOG(LogDrBossBrain, Warning, TEXT("보스 페이즈 또는 공격 행이 비어 있습니다. 테이블 로드를 확인하세요."));
  }
}
int32 UDrBossBrainComponent::PickAttack(float Distance) {
  WarnMissingTables();
  if (!Phases.IsValidIndex(PhaseIndex)) return INDEX_NONE;
  TArray<int32> Eligible;
  int32 Total = 0;
  for (int32 I=0; I<Attacks.Num(); ++I) {
    const auto& Row = Attacks[I];
    if (State == EDrBossState::Approach && !IsFacingTarget() &&
        Row.Kind != EDrBossAttackKind::Area && Row.Kind != EDrBossAttackKind::Siphon)
      continue;
    const double* Until = CooldownUntil.Find(Row.Id);
    if (Row.PatternSet != Phases[PhaseIndex].PatternSet || Row.Weight <= 0 ||
        Distance < Row.MinRange || Distance > Row.MaxRange || (Until && *Until > Clock)) continue;
    Eligible.Add(I); Total += Row.Weight;
  }
  if (Total <= 0) return INDEX_NONE;
  int32 Draw = Random.RandRange(1, Total);
  for (int32 I : Eligible) { Draw -= Attacks[I].Weight; if (Draw <= 0) return I; }
  return INDEX_NONE;
}
void UDrBossBrainComponent::CheckPhaseThreshold() {
  ADrBossCharacter* Owner = Boss();
  if (!Owner || State == EDrBossState::Dead || Owner->GetAttributes()->GetHealth() <= 0) return;
  const float Rate = Owner->GetAttributes()->GetHealth() / FMath::Max(1.f, Owner->GetAttributes()->GetMaxHealth());
  if (Phases.IsValidIndex(PhaseIndex+1) && Rate <=
      static_cast<float>(Phases[PhaseIndex+1].EnterHpRate) / FDrBossPhaseRow::EnterHpRateScale)
    PendingPhase = PhaseIndex+1;
}
void UDrBossBrainComponent::SetState(EDrBossState InState) {
  ADrBossCharacter* Owner = Boss();
  if (!Owner) return;
  StopMove();
  if (auto* FX = Owner->FindComponentByClass<UDrCombatFXComponent>()) {
    FX->EndTelegraph();
    if (InState == EDrBossState::PhaseTransition) FX->EnterPhaseTwo();
    if (InState == EDrBossState::Dead) FX->Reset();
  }
  State = InState; StateElapsed = 0;
  UDrAbilitySystemComponent* ASC = Owner->GetDrASC();
  ASC->SetLooseState(DrTags::State_Attacking, State == EDrBossState::Attack);
  ASC->SetLooseState(DrTags::State_Groggy, State == EDrBossState::Groggy);
  ASC->SetLooseState(DrTags::State_Invulnerable, State == EDrBossState::PhaseTransition);
  ASC->SetLooseState(DrTags::State_Dead, State == EDrBossState::Dead);
  FName Anim = Owner->GetData()->IdleAnim;
  bool bLoop = true;
  if (State == EDrBossState::Approach) Anim = Owner->GetData()->MoveAnim;
  if (State == EDrBossState::Groggy) { Anim = Owner->GetData()->GroggyAnim; bLoop = false; }
  if (State == EDrBossState::Dead) { Anim = Owner->GetData()->DeathAnim; bLoop = false; Owner->GetCharacterMovement()->DisableMovement(); }
  if (State != EDrBossState::Attack) { AttackIndex = INDEX_NONE; Owner->PlaySequence(Anim,bLoop); }
}
void UDrBossBrainComponent::EnterGroggy() {
  if (State == EDrBossState::Dead || State == EDrBossState::PhaseTransition || State == EDrBossState::Groggy) return;
  SetState(EDrBossState::Groggy);
  Boss()->GetDrASC()->SetNumericAttributeBase(UDrAttributeSet::GetPoiseAttribute(),0.f);
}
void UDrBossBrainComponent::EnterDead() {
  if (State == EDrBossState::Dead) return;
  PendingPhase = INDEX_NONE;
  SetState(EDrBossState::Dead);
  Boss()->GetDrASC()->CancelAllAbilities();
}
void UDrBossBrainComponent::TurnToTarget(float Seconds) {
  if (!Target.IsValid()) return;
  const FVector Direction = (Target->GetActorLocation() - Boss()->GetActorLocation()).GetSafeNormal2D();
  if (Direction.IsNearlyZero()) return;
  const float Yaw = FMath::FixedTurn(Boss()->GetActorRotation().Yaw,Direction.Rotation().Yaw,Boss()->GetData()->TurnRate*Seconds);
  Boss()->SetActorRotation(FRotator(0,Yaw,0));
}
void UDrBossBrainComponent::BeginAttack(int32 Index) {
  SetState(EDrBossState::Attack);
  AttackIndex = Index; HitActors.Reset(); bDirectionLocked = false;
  const auto& Row = Attacks[Index];
  CooldownUntil.Add(Row.Id, Clock + Row.Cooldown.GetTotalSeconds());
  Boss()->PlaySequence(Row.Anim,false);
  if (auto* FX = Boss()->FindComponentByClass<UDrCombatFXComponent>()) FX->BeginTelegraph(Row);
}
void UDrBossBrainComponent::StartMove(float Duration) {
  const auto* Row = GetCurrentAttack();
  if (!Row || Row->MoveDistance <= 0 || Duration <= 0) return;
  auto Move = MakeShared<FRootMotionSource_ConstantForce>();
  Move->InstanceName = BossMoveName;
  Move->AccumulateMode = ERootMotionAccumulateMode::Override;
  Move->Priority = 5; Move->Duration = Duration;
  Move->Force = LockedDirection * (Row->MoveDistance / Duration);
  Move->FinishVelocityParams.Mode = ERootMotionFinishVelocityMode::ClampVelocity;
  Move->FinishVelocityParams.ClampVelocity = 0;
  Boss()->GetCharacterMovement()->ApplyRootMotionSource(Move);
}
void UDrBossBrainComponent::StopMove() {
  if (Boss()) {
    auto* Movement = Boss()->GetCharacterMovement();
    Movement->RemoveRootMotionSource(BossMoveName);
    Movement->StopMovementImmediately();
    Boss()->ConsumeMovementInputVector();
  }
}
void UDrBossBrainComponent::ApplyHits() {
  const auto* Current = GetCurrentAttack();
  if (!Current || !GetWorld()) return;
  // 피해 델리게이트가 그로기·사망으로 재진입해도 이번 판정 데이터는 유지한다.
  const FDrBossAttackRow Row = *Current;
  TArray<FOverlapResult> Overlaps;
  const FVector Center = Boss()->GetActorLocation()+LockedDirection*Row.HitForwardOffset;
  GetWorld()->OverlapMultiByObjectType(Overlaps,Center,FQuat::Identity,FCollisionObjectQueryParams(ECC_Pawn),
      FCollisionShape::MakeSphere(Row.HitRadius),FCollisionQueryParams(SCENE_QUERY_STAT(DrBossHit),false,Boss()));
  for (const auto& Overlap : Overlaps) {
    if (State != EDrBossState::Attack) break;
    AActor* Actor = Overlap.GetActor();
    if (!Actor || Actor == Boss() || HitActors.Contains(Actor)) continue;
    auto* TargetASC = Cast<UDrAbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor));
    if (!TargetASC || TargetASC->HasMatchingGameplayTag(DrTags::State_Dead)) continue;
    HitActors.Add(Actor);
    const float Before = TargetASC->GetNumericAttribute(UDrAttributeSet::GetHealthAttribute());
    Boss()->GetDrASC()->ApplyDamage(TargetASC,Row.Damage,Row.PoiseDamage,Boss());
    // 쳐내기·무적으로 피해를 막은 대상에서는 흡수 회복과 부가 효과도 발생하지 않는다.
    if (TargetASC->GetNumericAttribute(UDrAttributeSet::GetHealthAttribute()) < Before) {
      GetWorld()->GetSubsystem<UDrHitFeedbackSubsystem>()->PlayHit(Boss(), Actor, EDrHitWeight::Boss);
      TargetASC->SpendStamina(Row.StaminaDamage);
      if (Row.Effect != 0) Boss()->GetDrASC()->ApplyEffectById(Row.Effect,TargetASC,Boss());
      if (State != EDrBossState::Dead) Boss()->GetDrASC()->ApplyHeal(Row.Heal);
    }
  }
}
void UDrBossBrainComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
  Super::TickComponent(DeltaTime,TickType,ThisTickFunction);
  ADrBossCharacter* Owner = Boss();
  if (!Owner || !Owner->HasAuthority() || State == EDrBossState::Dead || DeltaTime <= 0) return;
  Clock += DeltaTime;
  const double Previous = StateElapsed;
  StateElapsed += DeltaTime;
  if (!Target.IsValid()) if (TActorIterator<ADrCharacter> It(GetWorld()); It) Target = *It;
  CheckPhaseThreshold();
  if (PendingPhase != INDEX_NONE && State != EDrBossState::Attack && State != EDrBossState::Groggy && State != EDrBossState::PhaseTransition) {
    SetState(EDrBossState::PhaseTransition); return;
  }
  switch (State) {
  case EDrBossState::Idle:
    if (bEngageAllowed && HasLivingTarget() && FVector::Dist2D(Target->GetActorLocation(),Owner->GetActorLocation()) <= Owner->GetData()->AggroRadius)
      SetState(EDrBossState::Approach);
    break;
  case EDrBossState::Approach: {
    if (!HasLivingTarget()) { SetState(EDrBossState::Idle); break; }
    TurnToTarget(DeltaTime);
    const float Distance = FVector::Dist2D(Target->GetActorLocation(),Owner->GetActorLocation());
    const int32 Picked = PickAttack(Distance);
    if (Picked != INDEX_NONE) { BeginAttack(Picked); break; }
    if (!IsFacingTarget()) break;
    const float Rate = Phases.IsValidIndex(PhaseIndex) ? static_cast<float>(Phases[PhaseIndex].MoveSpeedRate)/FDrBossPhaseRow::MoveSpeedRateScale : 1.f;
    Owner->GetCharacterMovement()->MaxWalkSpeed = Owner->GetData()->BaseMoveSpeed*Rate*Owner->GetDrASC()->GetMoveSpeedMultiplier();
    Owner->AddMovementInput((Target->GetActorLocation()-Owner->GetActorLocation()).GetSafeNormal2D());
    break;
  }
  case EDrBossState::Attack: {
    if (!HasLivingTarget()) { SetState(EDrBossState::Recover); break; }
    const auto* Row = GetCurrentAttack();
    if (!Row) { SetState(EDrBossState::Recover); break; }
    const double Telegraph = Row->TelegraphTime.GetTotalSeconds();
    const double Start = Row->ActiveStart.GetTotalSeconds();
    if (auto* FX = Owner->FindComponentByClass<UDrCombatFXComponent>()) {
      FX->AdvanceTelegraph(StateElapsed, Telegraph);
      if (Previous <= Start && StateElapsed >= Start && (Previous < Start || Previous == 0.0)) FX->PlayAttackImpact(*Row);
    }
    if (!bDirectionLocked) {
      TurnToTarget(static_cast<float>(FMath::Max(0.0,FMath::Min(StateElapsed,Telegraph)-Previous)));
      if (StateElapsed >= Telegraph) {
        bDirectionLocked = true; LockedDirection = Owner->GetActorForwardVector();
        StartMove(static_cast<float>(FMath::Max(0.0,Start-Telegraph)));
      }
    }
    if (Previous <= Start && StateElapsed >= Start && (Previous < Start || Previous == 0.0))
      GetWorld()->GetSubsystem<UDrAudioSubsystem>()->PlayBossAttack(Owner, Row->Kind);
    if (Row->Kind == EDrBossAttackKind::Slam && Previous < Start && StateElapsed >= Start)
      GetWorld()->GetSubsystem<UDrHitFeedbackSubsystem>()->PlaySlam(Owner);
    const double Duration = Row->Duration.GetTotalSeconds();
    if (Previous < Row->ActiveEnd.GetTotalSeconds() && StateElapsed >= Start) ApplyHits();
    if (State == EDrBossState::Attack && StateElapsed >= Duration) SetState(EDrBossState::Recover);
    break;
  }
  case EDrBossState::Recover:
    if (StateElapsed >= Owner->GetData()->RecoverDuration) SetState(EDrBossState::Approach);
    break;
  case EDrBossState::Groggy:
    if (StateElapsed >= Owner->GetData()->GroggyDuration) {
      Owner->GetDrASC()->SetNumericAttributeBase(UDrAttributeSet::GetPoiseAttribute(),Owner->GetAttributes()->GetMaxPoise());
      SetState(EDrBossState::Approach);
    }
    break;
  case EDrBossState::PhaseTransition:
    if (StateElapsed >= Owner->GetData()->PhaseTransitionDuration) {
      PhaseIndex = PendingPhase; PendingPhase = INDEX_NONE;
      GetWorld()->GetSubsystem<UDrAudioSubsystem>()->SetPhase(2);
      SetState(EDrBossState::Approach);
    }
    break;
  case EDrBossState::Dead: break;
  }
}
void UDrBossBrainComponent::EndPlay(const EEndPlayReason::Type EndPlayReason) {
  StopMove(); Target.Reset(); HitActors.Reset();
  Super::EndPlay(EndPlayReason);
}
