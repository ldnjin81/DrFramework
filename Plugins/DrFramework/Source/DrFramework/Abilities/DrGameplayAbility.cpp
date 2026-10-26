// 공통 어빌리티 비용과 재사용 대기시간 처리를 구현한다.
#include "Abilities/DrGameplayAbility.h"
#include "Abilities/DrAbilitySystemComponent.h"
#include "Animation/AnimMontage.h"
#include "Characters/DrCharacter.h"
#include "Core/DrGameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/RootMotionSource.h"
#include "Engine/World.h"
UDrGameplayAbility::UDrGameplayAbility() {
  InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
  NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}
bool UDrGameplayAbility::CanActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo *ActorInfo,
    const FGameplayTagContainer *SourceTags,
    const FGameplayTagContainer *TargetTags,
    FGameplayTagContainer *OptionalRelevantTags) const {
  // 쿨다운 태그가 ASC에 있으면 발동할 수 없다.
  if (CooldownTag.IsValid())
    if (const UDrAbilitySystemComponent *AbilitySystem = GetDrASC())
      if (AbilitySystem->HasMatchingGameplayTag(CooldownTag))
        return false;
  if (!HasEnoughStamina())
    return false;
  return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags,
                                   OptionalRelevantTags);
}
AActor *UDrGameplayAbility::GetDrActor() const {
  return GetAvatarActorFromActorInfo();
}
UDrAbilitySystemComponent *UDrGameplayAbility::GetDrASC() const {
  return Cast<UDrAbilitySystemComponent>(
      GetAbilitySystemComponentFromActorInfo());
}
bool UDrGameplayAbility::HasEnoughStamina() const {
  const float Cost = GetStaminaCost();
  return Cost <= 0 || (GetDrASC() && GetDrASC()->GetStamina() >= Cost);
}
bool UDrGameplayAbility::CommitStamina() {
  if (!HasEnoughStamina())
    return false;
  if (GetStaminaCost() > 0)
    GetDrASC()->SpendStamina(GetStaminaCost());
  return true;
}
void UDrGameplayAbility::CommitCooldown() {
  if (UDrAbilitySystemComponent *AbilitySystem = GetDrASC())
    AbilitySystem->StartCooldown(CooldownTag, CooldownDuration);
}
float UDrGameplayAbility::GetCooldownRemaining() const {
  const UDrAbilitySystemComponent *AbilitySystem = GetDrASC();
  return AbilitySystem ? AbilitySystem->GetCooldownRemaining(CooldownTag) : 0.f;
}
namespace {
const FName DrConstantMoveName(TEXT("DrConstantMove"));
}
void UDrGameplayAbility::ApplyConstantMove(const FVector &Direction,
                                           float Distance, float Duration,
                                           float EndSpeed) {
  ACharacter *Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
  UCharacterMovementComponent *Movement =
      Character ? Character->GetCharacterMovement() : nullptr;
  const FVector Dir = Direction.GetSafeNormal2D();
  if (!Movement || Distance <= 0 || Duration <= 0 || Dir.IsNearlyZero())
    return;
  ClearConstantMove();
  TSharedPtr<FRootMotionSource_ConstantForce> Move =
      MakeShared<FRootMotionSource_ConstantForce>();
  Move->InstanceName = DrConstantMoveName;
  Move->AccumulateMode = ERootMotionAccumulateMode::Override;
  Move->Priority = 5;
  Move->Force = Dir * (Distance / Duration);
  Move->Duration = Duration;
  Move->FinishVelocityParams.Mode =
      ERootMotionFinishVelocityMode::ClampVelocity;
  Move->FinishVelocityParams.ClampVelocity = EndSpeed;
  Movement->ApplyRootMotionSource(Move);
}
void UDrGameplayAbility::ClearConstantMove() {
  if (ACharacter *Character = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
    if (UCharacterMovementComponent *Movement = Character->GetCharacterMovement())
      Movement->RemoveRootMotionSource(DrConstantMoveName);
}
UAnimMontage *UDrGameplayAbility::PlayAbilityMontage(
    const TSoftObjectPtr<UAnimMontage> &Montage, float PlayRate) {
  ADrCharacter *Character = Cast<ADrCharacter>(GetAvatarActorFromActorInfo());
  if (!Character || !Character->CanPlayCombatMontage() || Montage.IsNull())
    return nullptr;
  PlayingMontage = Montage.LoadSynchronous();
  if (!PlayingMontage)
    return nullptr;
  Character->PlayCombatMontage(PlayingMontage, false, PlayRate);
  return PlayingMontage;
}
void UDrGameplayAbility::StopAbilityMontage() {
  if (ADrCharacter *Character = Cast<ADrCharacter>(GetAvatarActorFromActorInfo()))
    if (PlayingMontage)
      Character->StopCombatMontage(PlayingMontage);
  PlayingMontage = nullptr;
}
void UDrGameplayAbility::BlockDuringCombatStates() {
  ActivationBlockedTags.AddTag(DrTags::State_Dead);
  ActivationBlockedTags.AddTag(DrTags::State_Staggered);
  ActivationBlockedTags.AddTag(DrTags::State_Attacking);
  ActivationBlockedTags.AddTag(DrTags::State_Guarding);
  ActivationBlockedTags.AddTag(DrTags::State_Parrying);
  ActivationBlockedTags.AddTag(DrTags::State_Dodging);
}
