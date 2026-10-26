#pragma once
#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "DrGameplayAbility.generated.h"
class UDrAbilitySystemComponent;
UCLASS(Abstract)
class DRFRAMEWORK_API UDrGameplayAbility : public UGameplayAbility {
 GENERATED_BODY()
public:
 UDrGameplayAbility();
 UPROPERTY(EditDefaultsOnly, Category="Dr|Cooldown") float CooldownDuration = 0;
 UPROPERTY(EditDefaultsOnly, Category="Dr|Cooldown") FGameplayTag CooldownTag;
 UFUNCTION(BlueprintPure, Category="Dr|Ability") AActor* GetDrActor() const;
 UFUNCTION(BlueprintPure, Category="Dr|Ability") UDrAbilitySystemComponent* GetDrASC() const;
 UFUNCTION(BlueprintPure, Category="Dr|Cooldown") float GetCooldownRemaining() const;
 virtual void AbilityTick(float DeltaTime) {}
 virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags=nullptr, const FGameplayTagContainer* TargetTags=nullptr, FGameplayTagContainer* OptionalRelevantTags=nullptr) const override;
protected:
 void CommitCooldown();
};
