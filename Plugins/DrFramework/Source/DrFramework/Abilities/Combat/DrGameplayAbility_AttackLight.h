#pragma once

#include "CoreMinimal.h"
#include "Abilities/Combat/DrGameplayAbility_MeleeAttack.h"
#include "DrGameplayAbility_AttackLight.generated.h"

/** 약공격 3타 콤보(Greystone Primary A·B·C). */
UCLASS()
class DRFRAMEWORK_API UDrGameplayAbility_AttackLight
    : public UDrGameplayAbility_MeleeAttack {
  GENERATED_BODY()
public:
  UDrGameplayAbility_AttackLight();
  virtual const TArray<FDrMeleeStage> &GetStages() const override;
};
