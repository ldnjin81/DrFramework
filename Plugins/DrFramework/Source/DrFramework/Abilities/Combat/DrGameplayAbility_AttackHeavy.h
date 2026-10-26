#pragma once

#include "CoreMinimal.h"
#include "Abilities/Combat/DrGameplayAbility_MeleeAttack.h"
#include "DrGameplayAbility_AttackHeavy.generated.h"

/** 긴 선딜의 강공격 1타(Greystone RMB). */
UCLASS()
class DRFRAMEWORK_API UDrGameplayAbility_AttackHeavy
    : public UDrGameplayAbility_MeleeAttack {
  GENERATED_BODY()
public:
  UDrGameplayAbility_AttackHeavy();
  virtual const TArray<FDrMeleeStage> &GetStages() const override;
};
