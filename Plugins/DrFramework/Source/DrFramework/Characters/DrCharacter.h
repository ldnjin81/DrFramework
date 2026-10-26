#pragma once
#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "DrCharacter.generated.h"
class UDrAbilitySystemComponent;
UCLASS(Blueprintable)
class DRFRAMEWORK_API ADrCharacter : public ACharacter, public IAbilitySystemInterface {
 GENERATED_BODY()
public:
 ADrCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
 virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
 virtual void PostInitializeComponents() override;
protected:
 virtual void InitializeAbilitySystem();
 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Dr|Abilities") TObjectPtr<UDrAbilitySystemComponent> AbilitySystem;
};
