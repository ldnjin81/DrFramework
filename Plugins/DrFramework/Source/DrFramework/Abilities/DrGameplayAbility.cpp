#include "Abilities/DrGameplayAbility.h"
#include "Abilities/DrAbilitySystemComponent.h"
UDrGameplayAbility::UDrGameplayAbility() { InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor; NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted; }
AActor* UDrGameplayAbility::GetDrActor() const { return GetAvatarActorFromActorInfo(); }
UDrAbilitySystemComponent* UDrGameplayAbility::GetDrASC() const { return Cast<UDrAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo()); }
float UDrGameplayAbility::GetCooldownRemaining() const { auto* ASC=GetDrASC(); return ASC ? ASC->GetCooldownRemaining(CooldownTag) : 0; }
void UDrGameplayAbility::CommitCooldown() { if(auto* ASC=GetDrASC()) ASC->StartCooldown(CooldownTag,CooldownDuration); }
bool UDrGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayTagContainer* SourceTags,const FGameplayTagContainer* TargetTags,FGameplayTagContainer* OptionalRelevantTags) const {
 if(CooldownTag.IsValid() && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid() && ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(CooldownTag)) return false;
 return Super::CanActivateAbility(Handle,ActorInfo,SourceTags,TargetTags,OptionalRelevantTags);
}
