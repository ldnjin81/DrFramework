#include "Combat/DrLockOnTargetComponent.h"

#include "AbilitySystemGlobals.h"
#include "Abilities/DrAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/DrGameplayTags.h"

FVector UDrLockOnTargetComponent::GetTargetLocation() const {
  if (const auto* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
    if (!SocketName.IsNone() && Mesh->DoesSocketExist(SocketName))
      return Mesh->GetSocketLocation(SocketName);
  return GetOwner()->GetActorLocation() + ChestOffset;
}
bool UDrLockOnTargetComponent::IsAvailable() const {
  if (!bLockable || !IsValid(GetOwner()) || GetOwner()->IsActorBeingDestroyed()) return false;
  const auto* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
  return !ASC || (!ASC->HasMatchingGameplayTag(DrTags::State_Dead) &&
    ASC->GetNumericAttribute(UDrAttributeSet::GetHealthAttribute()) > 0.f);
}
