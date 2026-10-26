#include "Characters/DrCharacter.h"
#include "Abilities/DrAbilitySystemComponent.h"
ADrCharacter::ADrCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
 AbilitySystem = CreateDefaultSubobject<UDrAbilitySystemComponent>(TEXT("AbilitySystem"));
 AbilitySystem->SetIsReplicated(true);
 AbilitySystem->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}
UAbilitySystemComponent* ADrCharacter::GetAbilitySystemComponent() const { return AbilitySystem; }
void ADrCharacter::PostInitializeComponents() { Super::PostInitializeComponents(); InitializeAbilitySystem(); }
void ADrCharacter::InitializeAbilitySystem() { AbilitySystem->InitAbilityActorInfo(this, this); }
