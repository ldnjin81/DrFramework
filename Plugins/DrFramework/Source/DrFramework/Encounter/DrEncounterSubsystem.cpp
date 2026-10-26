#include "Encounter/DrEncounterSubsystem.h"

#include "Abilities/DrAttributeSet.h"
#include "Boss/DrBossCharacter.h"
#include "Boss/DrBossBrainComponent.h"
#include "Characters/DrCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"

ADrBossCharacter *UDrEncounterSubsystem::GetBoss() const { return Boss.Get(); }

bool UDrEncounterSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const {
  return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

TStatId UDrEncounterSubsystem::GetStatId() const {
  RETURN_QUICK_DECLARE_CYCLE_STAT(UDrEncounterSubsystem, STATGROUP_Tickables);
}

void UDrEncounterSubsystem::OnWorldBeginPlay(UWorld &InWorld) {
  Super::OnWorldBeginPlay(InWorld);
  ADrBossCharacter *FoundBoss = nullptr;
  AActor *FoundCheckpoint = nullptr;
  AActor *FoundDoor = nullptr;
  AActor *FoundStart = nullptr;
  for (TActorIterator<AActor> It(&InWorld); It; ++It) {
    if (It->ActorHasTag(TEXT("Dr.Boss"))) FoundBoss = Cast<ADrBossCharacter>(*It);
    if (It->ActorHasTag(TEXT("Dr.Checkpoint.Gate"))) FoundCheckpoint = *It;
    if (It->ActorHasTag(TEXT("Dr.Arena.Door"))) FoundDoor = *It;
    if (It->ActorHasTag(TEXT("Dr.Player.Start"))) FoundStart = *It;
  }
  if (!FoundCheckpoint || !FoundBoss)
    return;
  auto *PC = InWorld.GetFirstPlayerController();
  BindActors(PC ? Cast<ADrCharacter>(PC->GetPawn()) : nullptr, FoundBoss,
             FoundCheckpoint, FoundDoor, FoundStart);
}

void UDrEncounterSubsystem::BindPlayer(ADrCharacter *InPlayer) {
  if (Player.Get() == InPlayer)
    return;
  if (Player.IsValid()) {
    Player->GetAttributeSet()->OnOutOfHealth.RemoveAll(this);
    Player->OnEndPlay.RemoveDynamic(this, &UDrEncounterSubsystem::HandleActorEndPlay);
  }
  Player = InPlayer;
  if (InPlayer) {
    InPlayer->GetAttributeSet()->OnOutOfHealth.AddUObject(this, &UDrEncounterSubsystem::HandlePlayerDeath);
    InPlayer->OnEndPlay.AddUniqueDynamic(this, &UDrEncounterSubsystem::HandleActorEndPlay);
  }
}

void UDrEncounterSubsystem::UnbindActors() {
  BindPlayer(nullptr);
  if (Boss.IsValid()) {
    Boss->GetAttributes()->OnOutOfHealth.RemoveAll(this);
    Boss->OnEndPlay.RemoveDynamic(this, &UDrEncounterSubsystem::HandleActorEndPlay);
  }
  for (AActor *Actor : {Checkpoint.Get(), Door.Get(), Start.Get()})
    if (Actor) Actor->OnEndPlay.RemoveDynamic(this, &UDrEncounterSubsystem::HandleActorEndPlay);
  Boss.Reset();
  Checkpoint.Reset();
  Door.Reset();
  Start.Reset();
  OnActorsChanged.Broadcast();
  bRespawnPending = false;
  bDoorMoving = false;
}

void UDrEncounterSubsystem::BindActors(ADrCharacter *InPlayer, ADrBossCharacter *InBoss,
                                      AActor *InCheckpoint, AActor *InDoor, AActor *InStart) {
  UnbindActors();
  SetState(EDrEncounterState::Idle);
  bCheckpointActive = false;
  DeathElapsed = 0.f;
  if (!InBoss || !InCheckpoint)
    return;
  Boss = InBoss;
  Checkpoint = InCheckpoint;
  Door = InDoor;
  Start = InStart;
  BossHome = InBoss->GetActorTransform();
  InBoss->GetBrain()->SetEngageAllowed(false);
  BindPlayer(InPlayer);
  InBoss->GetAttributes()->OnOutOfHealth.AddUObject(this, &UDrEncounterSubsystem::HandleBossDeath);
  InBoss->OnEndPlay.AddUniqueDynamic(this, &UDrEncounterSubsystem::HandleActorEndPlay);
  OnActorsChanged.Broadcast();
  for (AActor *Actor : {InCheckpoint, InDoor, InStart})
    if (Actor) Actor->OnEndPlay.AddUniqueDynamic(this, &UDrEncounterSubsystem::HandleActorEndPlay);
  if (!InDoor)
    return;
  // 맵 원본은 정적 블록이다. 런타임 복제본만 이동 가능하게 바꾼다.
  if (InDoor->GetRootComponent()) InDoor->GetRootComponent()->SetMobility(EComponentMobility::Movable);
  DoorOpen = InDoor->GetActorLocation();
  DoorClosed = DoorOpen;
  ArenaDirection = (BossHome.GetLocation() - DoorOpen).GetSafeNormal2D();
  if (ArenaDirection.IsNearlyZero()) ArenaDirection = FVector::ForwardVector;
  FVector Origin, Extent;
  InDoor->GetActorBounds(true, Origin, Extent);
  FCollisionQueryParams Query(SCENE_QUERY_STAT(DrDoorFloor), false, InDoor);
  Query.AddIgnoredActor(InBoss);
  if (InPlayer) Query.AddIgnoredActor(InPlayer);
  FHitResult Hit;
  // 열린 문 아래에서 바닥을 찾고, 피벗과 실제 충돌 바운드 중심의 차이도 보정한다.
  const FVector TraceStart(Origin.X, Origin.Y, Origin.Z - Extent.Z - 1.f);
  if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceStart - FVector(0, 0, 10000), ECC_Visibility, Query))
    DoorClosed.Z = Hit.ImpactPoint.Z + Extent.Z + (DoorOpen.Z - Origin.Z);
}

void UDrEncounterSubsystem::SetDoorClosedZ(float InZ) {
  DoorClosed.Z = InZ;
}

FTransform UDrEncounterSubsystem::GetRespawnTransform() const {
  const AActor *Marker = bCheckpointActive ? Checkpoint.Get() : Start.Get();
  if (!Marker) Marker = Checkpoint.Get();
  if (!Marker) return FTransform::Identity;
  const FVector Location = Marker->GetActorLocation();
  const FRotator Rotation = Door.IsValid() ? (DoorOpen - Location).GetSafeNormal2D().Rotation() : Marker->GetActorRotation();
  return FTransform(Rotation, Location);
}

bool UDrEncounterSubsystem::IsBoundPlayer(const ADrCharacter *InPlayer) const {
  return Boss.IsValid() && Checkpoint.IsValid() && Player.Get() == InPlayer;
}

void UDrEncounterSubsystem::SetState(EDrEncounterState InState) {
  if (State == InState) return;
  const EDrEncounterState Old = State;
  State = InState;
  OnStateChanged.Broadcast(Old, State);
}

void UDrEncounterSubsystem::MoveDoor(bool bClose) {
  if (!Door.IsValid()) return;
  DoorFrom = Door->GetActorLocation();
  DoorTo = bClose ? DoorClosed : DoorOpen;
  DoorElapsed = 0.f;
  bDoorMoving = true;
  OnDoorMoving.Broadcast(Door.Get(), true);
  AdvanceDoor(0.f);
}

void UDrEncounterSubsystem::AdvanceDoor(float DeltaSeconds) {
  if (!bDoorMoving || !Door.IsValid()) return;
  DoorElapsed += DeltaSeconds;
  const float Alpha = DoorMoveDuration > 0.f ? FMath::Clamp(DoorElapsed / DoorMoveDuration, 0.f, 1.f) : 1.f;
  Door->SetActorLocation(FMath::Lerp(DoorFrom, DoorTo, Alpha), false, nullptr, ETeleportType::TeleportPhysics);
  bDoorMoving = Alpha < 1.f;
  if (!bDoorMoving) OnDoorMoving.Broadcast(Door.Get(), false);
}

void UDrEncounterSubsystem::RestartCheckpoint() {
  if (!Player.IsValid() || !Boss.IsValid() || !Checkpoint.IsValid()) return;
  bRespawnPending = true;
  DeathElapsed = RespawnDelay;
  Advance(KINDA_SMALL_NUMBER);
}

void UDrEncounterSubsystem::HandlePlayerDeath(AActor *, AActor *) {
  if (bRespawnPending || !Boss.IsValid() || !Checkpoint.IsValid()) return;
  bRespawnPending = true;
  DeathElapsed = 0.f;
  // 승리 후의 부활은 승리 상태와 보스 사망을 그대로 보존한다.
  if (State != EDrEncounterState::Victory) SetState(EDrEncounterState::PlayerDead);
}

void UDrEncounterSubsystem::HandleBossDeath(AActor *, AActor *) {
  if (State != EDrEncounterState::Fighting) return;
  SetState(EDrEncounterState::Victory);
  MoveDoor(false);
}

void UDrEncounterSubsystem::HandleActorEndPlay(AActor *Actor, EEndPlayReason::Type) {
  if (Actor == Player.Get()) BindPlayer(nullptr);
  else UnbindActors();
}

void UDrEncounterSubsystem::Deinitialize() {
  UnbindActors();
  OnStateChanged.Clear();
  OnActorsChanged.Clear();
  OnDoorMoving.Clear();
  Super::Deinitialize();
}

void UDrEncounterSubsystem::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);
  Advance(DeltaTime);
}

void UDrEncounterSubsystem::Advance(float DeltaSeconds) {
  if (DeltaSeconds <= 0.f || !Boss.IsValid() || !Checkpoint.IsValid()) return;
  if (!Player.IsValid()) {
    auto *PC = GetWorld()->GetFirstPlayerController();
    BindPlayer(PC ? Cast<ADrCharacter>(PC->GetPawn()) : nullptr);
  }
  AdvanceDoor(DeltaSeconds);
  if (!Player.IsValid()) return;
  if (bRespawnPending) {
    DeathElapsed += DeltaSeconds;
    if (DeathElapsed < FMath::Max(0.f, RespawnDelay)) return;
    if (State != EDrEncounterState::Victory) {
      Boss->ResetBoss(BossHome);
      Boss->GetBrain()->SetEngageAllowed(false);
    }
    Player->Revive(GetRespawnTransform());
    MoveDoor(false);
    bRespawnPending = false;
    if (State != EDrEncounterState::Victory) SetState(EDrEncounterState::Idle);
    return;
  }
  if (State != EDrEncounterState::Idle || Player->GetAttributeSet()->GetHealth() <= 0.f) return;
  if (!bCheckpointActive && FVector::Dist(Player->GetActorLocation(), Checkpoint->GetActorLocation()) <= CheckpointRadius)
    bCheckpointActive = true;
  if (!Door.IsValid()) return;
  FVector Origin, Extent;
  Door->GetActorBounds(true, Origin, Extent);
  const float DoorHalfDepth = FVector::DotProduct(Extent, ArenaDirection.GetAbs());
  const float SafeDepth = FMath::Max(ArenaEnterDepth, DoorHalfDepth + Player->GetCapsuleComponent()->GetScaledCapsuleRadius() + 1.f);
  if (FVector::DotProduct(Player->GetActorLocation() - DoorOpen, ArenaDirection) >= SafeDepth) {
    Boss->GetBrain()->SetEngageAllowed(true);
    SetState(EDrEncounterState::Fighting);
    MoveDoor(true);
  }
}
