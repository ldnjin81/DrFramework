#pragma once
// 엔카운터·오디오 테스트가 함께 쓰는 주입 월드(플레이어·보스·체크포인트·문).
#if WITH_DEV_AUTOMATION_TESTS
#include "Encounter/DrEncounterSubsystem.h"
#include "Audio/DrAudioSubsystem.h"
#include "Components/AudioComponent.h"
#include "Abilities/DrAbilitySystemComponent.h"
#include "Abilities/DrAttributeSet.h"
#include "Abilities/Combat/DrGameplayAbility_AttackHeavy.h"
#include "Abilities/Combat/DrGameplayAbility_AttackLight.h"
#include "Abilities/Combat/DrGameplayAbility_MeleeAttack.h"
#include "Boss/DrBossBrainComponent.h"
#include "Boss/DrBossCharacter.h"
#include "Characters/DrCharacter.h"
#include "Core/DrGameplayTags.h"
#include "Engine/Engine.h"
#include "Engine/TargetPoint.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Misc/AutomationTest.h"
#include "Tests/DrTestFixture.h"

namespace DrEncounterTests {
struct FFixture {
  DrTestFixture::FWorldFixture Base{true};
  ADrCharacter *Player;
  ADrBossCharacter *Boss;
  UDrAbilitySystemComponent *ASC;
  UDrEncounterSubsystem *Encounter;
  AActor *Checkpoint;
  AActor *Door;
  AActor *Start;
  FFixture() {
    GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(Base.World);
    Base.World->InitializeActorsForPlay(FURL());
    Base.World->SetBegunPlay(true);
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    Player = Base.World->SpawnActor<ADrCharacter>(ADrCharacter::StaticClass(), FVector(-1000, 0, 100), FRotator::ZeroRotator, Params);
    Boss = Base.World->SpawnActor<ADrBossCharacter>(ADrBossCharacter::StaticClass(), FVector(2000, 0, 100), FRotator(0, 180, 0), Params);
    Checkpoint = Base.World->SpawnActor<ATargetPoint>(FVector(-500, 0, 100), FRotator::ZeroRotator);
    Door = Base.World->SpawnActor<AStaticMeshActor>(FVector(0, 0, 800), FRotator::ZeroRotator);
    Start = Base.World->SpawnActor<ATargetPoint>(FVector(-1000, 0, 100), FRotator::ZeroRotator);
    ASC = Cast<UDrAbilitySystemComponent>(Player->GetAbilitySystemComponent());
    ASC->GiveAbility(FGameplayAbilitySpec(UDrGameplayAbility_AttackLight::StaticClass(), 1));
    FDrBossPhaseRow P1;
    P1.Id = 1;
    P1.Phase = 1;
    P1.PatternSet = TEXT("P1");
    P1.MoveSpeedRate = 100;
    FDrBossPhaseRow P2 = P1;
    P2.Id = 2;
    P2.Phase = 2;
    P2.EnterHpRate = 50;
    FDrBossAttackRow Attack;
    Attack.Id = 1;
    Attack.PatternSet = TEXT("P1");
    Attack.Weight = 1;
    Attack.MaxRange = 3000;
    Attack.Cooldown = FTimespan::FromSeconds(60);
    Attack.Duration = FTimespan::FromSeconds(1);
    Boss->GetBrain()->InjectTables({P1, P2}, {Attack});
    Boss->GetBrain()->SetTarget(Player);
    Encounter = Base.World->GetSubsystem<UDrEncounterSubsystem>();
    Encounter->BindActors(Player, Boss, Checkpoint, Door, Start);
    Encounter->SetDoorClosedZ(250.f);
  }
  ~FFixture() {
    Base.World->EndPlay(EEndPlayReason::Quit);
    GEngine->DestroyWorldContext(Base.World);
  }
  void Activate() {
    Player->SetActorLocation(Checkpoint->GetActorLocation());
    Encounter->Advance(.01f);
  }
  void Enter() {
    Player->SetActorLocation(FVector(301, 0, 100));
    Encounter->Advance(.01f);
    Encounter->Advance(Encounter->DoorMoveDuration);
  }
  void KillPlayer() { Boss->GetDrASC()->ApplyDamage(ASC, 10000, 0, Boss); }
  void KillBoss() { ASC->ApplyDamage(Boss->GetDrASC(), 10000, 0, Player); }
  void Retry() { Encounter->Advance(Encounter->RespawnDelay); }
};
}
#endif
