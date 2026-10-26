#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Abilities/DrAttributeSet.h"
#include "DrBossData.generated.h"

/** 보스 한 종의 기본 수치·애니 이름·스탯. 공격 패턴은 BossPhase/BossAttack 테이블에 있다. 코드의 값은 에셋이 없을 때의 기본값이다. */
UCLASS(BlueprintType)
class DRFRAMEWORK_API UDrBossData : public UDataAsset {
  GENERATED_BODY()
public:
  UDrBossData();
  /** BossPhase.Boss와 맞춰 패턴을 찾는 이름. */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dr|Boss") FName BossName = TEXT("Boss");
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dr|Boss") FText DisplayName;
  /** 기본 스탯. 보스 포이즈는 그로기가 끝날 때 한 번에 채우므로 자동 재생은 0이다. */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dr|Stats") FDrStatBlock Stats;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dr|Boss") float BaseMoveSpeed = 350.f;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dr|Boss") float AggroRadius = 2500.f;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dr|Boss") float GroggyDuration = 2.67f;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dr|Boss") float PhaseTransitionDuration = 2.f;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dr|Boss") float RecoverDuration = .6f;
  /** 초당 회전 각도와, 정면으로 보는 반각(도). */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dr|Boss") float TurnRate = 180.f;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dr|Boss") float AttackFacingAngle = 60.f;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dr|Animation") FName IdleAnim = TEXT("Idle");
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dr|Animation") FName MoveAnim = TEXT("Jog_Fwd");
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dr|Animation") FName GroggyAnim = TEXT("Knock_back");
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dr|Animation") FName DeathAnim = TEXT("Death_front");
};
