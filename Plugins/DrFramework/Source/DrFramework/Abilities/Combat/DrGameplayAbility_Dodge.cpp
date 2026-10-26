// 구르기 회피의 방향 선택, 이동, 무적 구간을 구현한다.
#include "Abilities/Combat/DrGameplayAbility_Dodge.h"
#include "Combat/DrCombatFXComponent.h"

#include "Abilities/DrAbilitySystemComponent.h"
#include "Combat/DrPlayerCombatData.h"
#include "Animation/AnimMontage.h"
#include "Core/DrGameplayTags.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UDrGameplayAbility_Dodge::UDrGameplayAbility_Dodge() {
  SetAssetTags(FGameplayTagContainer(DrTags::Ability_Dodge));
  BlockDuringCombatStates();
}

float UDrGameplayAbility_Dodge::GetStaminaCost() const {
  return UDrPlayerCombatData::Get()->Dodge.StaminaCost;
}

int32 UDrGameplayAbility_Dodge::SelectRollIndex(const FVector &LocalDirection) {
  // 액터 공간은 X가 앞, Y가 오른쪽이라 각도가 시계 방향으로 커진다.
  const float Degrees = FMath::RadiansToDegrees(
      FMath::Atan2(LocalDirection.Y, LocalDirection.X));
  return (FMath::RoundToInt(Degrees / 45.f) + 8) % 8;
}

void UDrGameplayAbility_Dodge::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo *ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData *TriggerEventData) {
  Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
  if (!CommitAbility(Handle, ActorInfo, ActivationInfo) || !CommitStamina()) {
    EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
    return;
  }
  const FDrDodgeTuning &Tuning = UDrPlayerCombatData::Get()->Dodge;
  Elapsed = 0.f;
  ActiveDuration = Tuning.Duration;
  if (auto* FX = GetAvatarActorFromActorInfo()->FindComponentByClass<UDrCombatFXComponent>()) FX->SetDash(true);
  GetDrASC()->SetLooseState(DrTags::State_Dodging, true);
  if (const ACharacter *Character =
          Cast<ACharacter>(GetAvatarActorFromActorInfo())) {
    // 이동 입력이 있으면 그 방향, 없으면 뒤로 물러난다.
    FVector Direction =
        Character->GetCharacterMovement()->GetCurrentAcceleration();
    if (Direction.IsNearlyZero())
      Direction = Character->GetLastMovementInputVector();
    if (Direction.IsNearlyZero())
      Direction = -Character->GetActorForwardVector();
    const FVector Local = Character->GetActorTransform().InverseTransformVectorNoScale(
        Direction.GetSafeNormal2D());
    const int32 Index = SelectRollIndex(Local);
    // 구르기 모션은 이동이 뼈에 들어 있어 캐릭터가 모션을 따라 캡슐을 옮긴다.
    if (Tuning.RollMontages.IsValidIndex(Index) && Tuning.RollPlayRate > 0.f)
      if (const UAnimMontage *Played = PlayAbilityMontage(Tuning.RollMontages[Index], Tuning.RollPlayRate)) {
        ActiveDuration = Played->GetPlayLength() / Tuning.RollPlayRate;
        return;
      }
    ApplyConstantMove(Direction, Tuning.Distance, Tuning.Duration,
                      Character->GetCharacterMovement()->MaxWalkSpeed);
  }
}

void UDrGameplayAbility_Dodge::AbilityTick(float DeltaTime) {
  Elapsed += DeltaTime;
  const FDrDodgeTuning &Tuning = UDrPlayerCombatData::Get()->Dodge;
  SetInvulnerable(Elapsed >= Tuning.InvulnerableStart && Elapsed < Tuning.InvulnerableEnd);
  if (Elapsed >= ActiveDuration)
    EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(),
               GetCurrentActivationInfo(), true, false);
}

void UDrGameplayAbility_Dodge::SetInvulnerable(bool bEnabled) {
  UDrAbilitySystemComponent *ASC = GetDrASC();
  // 다른 출처가 건 무적을 지우지 않도록 자신이 건 것만 해제한다.
  if (!ASC || bGrantedInvulnerable == bEnabled)
    return;
  bGrantedInvulnerable = bEnabled;
  if (bEnabled)
    ASC->AddLooseGameplayTag(DrTags::State_Invulnerable);
  else
    ASC->RemoveLooseGameplayTag(DrTags::State_Invulnerable);
}

void UDrGameplayAbility_Dodge::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo *ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled) {
  if (auto* Avatar = GetAvatarActorFromActorInfo())
    if (auto* FX = Avatar->FindComponentByClass<UDrCombatFXComponent>()) FX->SetDash(false);
  SetInvulnerable(false);
  if (UDrAbilitySystemComponent *ASC = GetDrASC())
    ASC->SetLooseState(DrTags::State_Dodging, false);
  if (bWasCancelled) {
    ClearConstantMove();
    StopAbilityMontage();
  }
  Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility,
                    bWasCancelled);
}
