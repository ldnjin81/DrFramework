// DrFramework 캐릭터의 전투 및 특성 속성 집합을 선언한다.
#pragma once
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "CoreMinimal.h"
#include "DrAttributeSet.generated.h"
/** 속성 하나에 대한 표준 Get/Set/Init 접근자 함수를 생성한다. */
#define DR_ATTRIBUTE_ACCESSORS(ClassName, PropertyName)                        \
  GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName)                   \
  GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName)                                 \
  GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName)                                 \
  GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
// 소유 액터가 바인딩하는 네이티브(비동적) 델리게이트.
DECLARE_MULTICAST_DELEGATE_ThreeParams(FDrDamageDelegate, float /*Damage*/,
                                       AActor * /*Instigator*/,
                                       AActor * /*Causer*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FDrOutOfHealthDelegate, AActor * /*Instigator*/, AActor * /*Causer*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FDrPoiseBrokenDelegate, AActor * /*Instigator*/, AActor * /*Causer*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FDrParriedDelegate, AActor * /*Instigator*/, AActor * /*Causer*/);

/** 특성(Trait)에서 전투 스탯을 파생하는 순수 공식 모음. */
struct FDrStatFormula {
  // 특성 1당 스탯 기여(튜닝값). 원본 한국어 명칭을 유지한다.
  static constexpr float HealthPerVitality = 12.f;  // 체력 → 최대 생명력
  static constexpr float DefensePerVitality = 1.f;  // 체력 → 물리 방어력
  static constexpr float StaminaPerSpirit = 4.f;    // 기 → 최대 기력
  static constexpr float RegenPerDexterity = .8f;   // 기예 → 기력 회복
  static constexpr float AttackPerStrength = 2.f;   // 힘 → 물리 공격력
  static constexpr float PoisePerStrength = 3.f;    // 힘 → 인내(포이즈)
  static constexpr float ResistPerWill = 2.f;       // 정신 → 속성 저항(%)
  static constexpr float MaxElementalResist = 80.f; // 속성 저항 상한(%)
  static float MaxHealth(float Base, float Vitality) {
    return Base + Vitality * HealthPerVitality;
  }
  static float Defense(float Base, float Vitality) {
    return Base + Vitality * DefensePerVitality;
  }
  static float MaxStamina(float Base, float Spirit) {
    return Base + Spirit * StaminaPerSpirit;
  }
  static float StaminaRegen(float Base, float Dexterity) {
    return Base + Dexterity * RegenPerDexterity;
  }
  static float AttackPower(float Base, float Strength) {
    return Base + Strength * AttackPerStrength;
  }
  static float MaxPoise(float Base, float Strength) {
    return Base + Strength * PoisePerStrength;
  }
  static float ElementalResist(float Will) {
    return FMath::Clamp(Will * ResistPerWill, 0.f, MaxElementalResist);
  }
};
/** 액터의 기본 스탯과 특성. 데이터 에셋에 두고 InitFromStats로 속성을 만든다. */
USTRUCT(BlueprintType)
struct FDrStatBlock {
  GENERATED_BODY()
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dr|Stats") float BaseMaxHealth = 100.f;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dr|Stats") float BaseMaxStamina = 100.f;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dr|Stats") float BaseStaminaRegen = 25.f;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dr|Stats") float BaseAttackPower = 20.f;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dr|Stats") float BaseDefense = 5.f;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dr|Stats") float BaseMaxPoise = 100.f;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dr|Stats") float PoiseRegenRate = 20.f;
  // 특성(힘·기예·체력·기·정신). FDrStatFormula로 위 기본값에 더해진다.
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dr|Trait") float Strength = 0.f;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dr|Trait") float Dexterity = 0.f;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dr|Trait") float Vitality = 0.f;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dr|Trait") float Spirit = 0.f;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dr|Trait") float Will = 0.f;
};

/**
 * 모든 전투 액터가 공유하는 핵심 속성 세트이다.
 * SetByCaller 메타 속성을 실제 생명력과 포이즈 변화로 변환한 뒤 이벤트를 알린다.
 */
UCLASS()
class DRFRAMEWORK_API UDrAttributeSet : public UAttributeSet {
  GENERATED_BODY()
public:
  UDrAttributeSet();
  virtual void PreAttributeChange(const FGameplayAttribute &Attribute,
                                  float &NewValue) override;
  virtual void PostGameplayEffectExecute(
      const FGameplayEffectModCallbackData &Data) override;
  /** 특성과 기본값으로 전투 스탯을 파생하고 현재치를 최대치로 채운다. */
  void InitFromStats(const FDrStatBlock &Stats);
  virtual void GetLifetimeReplicatedProps(
      TArray<FLifetimeProperty> &OutLifetimeProps) const override;
  // --- 바이탈 ---------------------------------------------------------
  UPROPERTY(BlueprintReadOnly, Category = "Dr|Vitals",
            ReplicatedUsing = OnRep_Health)
  FGameplayAttributeData Health;
  DR_ATTRIBUTE_ACCESSORS(UDrAttributeSet, Health)
  UPROPERTY(BlueprintReadOnly, Category = "Dr|Vitals",
            ReplicatedUsing = OnRep_MaxHealth)
  FGameplayAttributeData MaxHealth;
  DR_ATTRIBUTE_ACCESSORS(UDrAttributeSet, MaxHealth)
  UPROPERTY(BlueprintReadOnly, Category = "Dr|Vitals",
            ReplicatedUsing = OnRep_Stamina)
  FGameplayAttributeData Stamina;
  DR_ATTRIBUTE_ACCESSORS(UDrAttributeSet, Stamina)
  UPROPERTY(BlueprintReadOnly, Category = "Dr|Vitals",
            ReplicatedUsing = OnRep_MaxStamina)
  FGameplayAttributeData MaxStamina;
  DR_ATTRIBUTE_ACCESSORS(UDrAttributeSet, MaxStamina)
  UPROPERTY(BlueprintReadOnly, Category = "Dr|Vitals")
  FGameplayAttributeData StaminaRegenRate;
  DR_ATTRIBUTE_ACCESSORS(UDrAttributeSet, StaminaRegenRate)
  // --- 포이즈 (경직 저항력; 피격 시 감소, 시간이 지나면 회복) ----------
  UPROPERTY(BlueprintReadOnly, Category = "Dr|Poise",
            ReplicatedUsing = OnRep_Poise)
  FGameplayAttributeData Poise;
  DR_ATTRIBUTE_ACCESSORS(UDrAttributeSet, Poise)
  UPROPERTY(BlueprintReadOnly, Category = "Dr|Poise")
  FGameplayAttributeData MaxPoise;
  DR_ATTRIBUTE_ACCESSORS(UDrAttributeSet, MaxPoise)
  UPROPERTY(BlueprintReadOnly, Category = "Dr|Poise")
  FGameplayAttributeData PoiseRegenRate;
  DR_ATTRIBUTE_ACCESSORS(UDrAttributeSet, PoiseRegenRate)
  // --- 전투 스탯 ------------------------------------------------------
  UPROPERTY(BlueprintReadOnly, Category = "Dr|Combat")
  FGameplayAttributeData AttackPower;
  DR_ATTRIBUTE_ACCESSORS(UDrAttributeSet, AttackPower)
  UPROPERTY(BlueprintReadOnly, Category = "Dr|Combat")
  FGameplayAttributeData Defense;
  DR_ATTRIBUTE_ACCESSORS(UDrAttributeSet, Defense)
  UPROPERTY(BlueprintReadOnly, Category = "Dr|Combat")
  FGameplayAttributeData ElementalResist;
  DR_ATTRIBUTE_ACCESSORS(UDrAttributeSet, ElementalResist)
  // --- 특성(Trait): 힘·기예·체력·기·정신 ------------------------------
  UPROPERTY(BlueprintReadOnly, Category = "Dr|Trait")
  FGameplayAttributeData Strength;
  DR_ATTRIBUTE_ACCESSORS(UDrAttributeSet, Strength)
  UPROPERTY(BlueprintReadOnly, Category = "Dr|Trait")
  FGameplayAttributeData Dexterity;
  DR_ATTRIBUTE_ACCESSORS(UDrAttributeSet, Dexterity)
  UPROPERTY(BlueprintReadOnly, Category = "Dr|Trait")
  FGameplayAttributeData Vitality;
  DR_ATTRIBUTE_ACCESSORS(UDrAttributeSet, Vitality)
  UPROPERTY(BlueprintReadOnly, Category = "Dr|Trait")
  FGameplayAttributeData Spirit;
  DR_ATTRIBUTE_ACCESSORS(UDrAttributeSet, Spirit)
  UPROPERTY(BlueprintReadOnly, Category = "Dr|Trait")
  FGameplayAttributeData Will;
  DR_ATTRIBUTE_ACCESSORS(UDrAttributeSet, Will)
  // --- 메타 속성 (일시적, 리플리케이트하지 않음) ----------------------
  UPROPERTY(BlueprintReadOnly, Category = "Dr|Meta")
  FGameplayAttributeData IncomingDamage;
  DR_ATTRIBUTE_ACCESSORS(UDrAttributeSet, IncomingDamage)
  UPROPERTY(BlueprintReadOnly, Category = "Dr|Meta")
  FGameplayAttributeData IncomingPoiseDamage;
  DR_ATTRIBUTE_ACCESSORS(UDrAttributeSet, IncomingPoiseDamage)
  // --- 이벤트 (소유 액터가 바인딩; UPROPERTY 아님) --------------------
  mutable FDrDamageDelegate OnDamaged;
  mutable FDrOutOfHealthDelegate OnOutOfHealth;
  mutable FDrPoiseBrokenDelegate OnPoiseBroken;
  /** 쳐내기(패리)로 피해를 완전히 막았을 때 알린다. */
  mutable FDrParriedDelegate OnParried;

protected:
  /** 공격자가 소유 액터의 전방 반구에 있는지 확인한다. 위치를 모르면 false다. */
  bool IsAttackFromFront(const AActor *Instigator, const AActor *Causer) const;
  UFUNCTION() void OnRep_Health(const FGameplayAttributeData &OldValue);
  UFUNCTION() void OnRep_MaxHealth(const FGameplayAttributeData &OldValue);
  UFUNCTION() void OnRep_Stamina(const FGameplayAttributeData &OldValue);
  UFUNCTION() void OnRep_MaxStamina(const FGameplayAttributeData &OldValue);
  UFUNCTION() void OnRep_Poise(const FGameplayAttributeData &OldValue);
};
