#include "Audio/DrAudioSubsystem.h"
#include "TableData/DrGameAssets.h"

#include "AudioMixerBlueprintLibrary.h"
#include "AudioModulationStatics.h"
#include "Boss/DrBossCharacter.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "HAL/FileManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"
#include "SoundControlBus.h"
#include "Game/DrGameUserSettings.h"
#include "Abilities/DrAttributeSet.h"
#include "AbilitySystemInterface.h"
#include "Characters/DrCharacter.h"

bool UDrAudioSubsystem::DoesSupportWorldType(EWorldType::Type Type) const {
  return Type == EWorldType::Game || Type == EWorldType::PIE;
}
const UDrAudioData* UDrAudioSubsystem::GetData() {
  if (!Data) Data = LoadObject<UDrAudioData>(nullptr, *DrGameAssets::Path(TEXT("DA_Audio")).ToString());
  return Data;
}
TStatId UDrAudioSubsystem::GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(UDrAudioSubsystem, STATGROUP_Tickables); }
void UDrAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection) {
  Super::Initialize(Collection);
  if (auto* InEncounter = Collection.InitializeDependency<UDrEncounterSubsystem>()) {
    Encounter = InEncounter;
    InEncounter->OnStateChanged.AddUObject(this, &ThisClass::HandleEncounterStateChanged);
    InEncounter->OnActorsChanged.AddUObject(this, &ThisClass::HandleEncounterActorsChanged);
    InEncounter->OnDoorMoving.AddUObject(this, &ThisClass::MoveDoor);
  }
}
void UDrAudioSubsystem::HandleEncounterStateChanged(EDrEncounterState OldState, EDrEncounterState NewState) {
  EncounterChanged(NewState, Encounter.IsValid() ? Encounter->GetPlayer() : nullptr,
                   Encounter.IsValid() ? Encounter->GetBoss() : nullptr);
}
void UDrAudioSubsystem::UnbindDamageEvents() {
  for (const TWeakObjectPtr<AActor>& Source : DamageSources)
    if (const auto* Owner = Cast<IAbilitySystemInterface>(Source.Get()))
      if (const auto* ASC = Owner->GetAbilitySystemComponent())
        if (const auto* Attributes = ASC->GetSet<UDrAttributeSet>()) Attributes->OnDamaged.RemoveAll(this);
  DamageSources.Reset();
}
void UDrAudioSubsystem::HandleEncounterActorsChanged() {
  UnbindDamageEvents();
  if (!Encounter.IsValid()) return;
  for (AActor* Actor : {static_cast<AActor*>(Encounter->GetPlayer()), static_cast<AActor*>(Encounter->GetBoss())}) {
    const auto* Owner = Cast<IAbilitySystemInterface>(Actor);
    const auto* ASC = Owner ? Owner->GetAbilitySystemComponent() : nullptr;
    const auto* Attributes = ASC ? ASC->GetSet<UDrAttributeSet>() : nullptr;
    if (!Attributes) continue;
    TWeakObjectPtr<AActor> Victim = Actor;
    Attributes->OnDamaged.AddWeakLambda(this, [this, Victim, Attributes](float Damage, AActor*, AActor*) {
      if (Victim.IsValid()) PlayDamage(Victim.Get(), Damage, Attributes->GetHealth(), Attributes->GetMaxHealth());
    });
    DamageSources.Add(Victim);
  }
}
void UDrAudioSubsystem::OnWorldBeginPlay(UWorld& InWorld) {
  Super::OnWorldBeginPlay(InWorld);
  const auto* Config = GetData();
  if (!Config || InWorld.GetNetMode() == NM_DedicatedServer) return;
  for (int32 I = 0; I < 3; ++I) {
    const auto Channel = static_cast<EDrAudioChannel>(I);
    const auto* Settings = UDrGameUserSettings::Get();
    SetChannelVolume(Channel, Settings ? Settings->GetVolume(Channel) : Volumes[I]);
  }
  if (FParse::Param(FCommandLine::Get(), TEXT("DrCaptureAudio")) && InWorld.GetAudioDevice().IsValid()) {
    UAudioMixerBlueprintLibrary::StartRecordingOutput(this, 180.f, nullptr);
    bRecording = true;
  }
  if (Config->Wind) EnvironmentComponents.Add(UGameplayStatics::SpawnSound2D(this, Config->Wind, .35f, 1.f, 0.f, nullptr, false, false));
  for (TActorIterator<AActor> It(&InWorld); It; ++It) {
    if (It->ActorHasTag(TEXT("Dr.Checkpoint.Gate")) && Config->Fire)
      EnvironmentComponents.Add(UGameplayStatics::SpawnSoundAtLocation(this, Config->Fire, It->GetActorLocation(), FRotator::ZeroRotator, .3f, 1.f, 0.f, Config->EnvironmentAttenuation, nullptr, false));
  }
}
USoundBase* UDrAudioSubsystem::ResolveBossAttack(EDrBossAttackKind Kind) {
  const auto* Config = GetData();
  const int32 Index = static_cast<int32>(Kind);
  return Config && Config->BossAttacks.IsValidIndex(Index) ? Config->BossAttacks[Index].Get() : nullptr;
}
void UDrAudioSubsystem::PlayAt(USoundBase* Sound, AActor* Actor, bool bBoss) {
  const auto* Config = GetData();
  if (!Sound || !Actor || !Config || GetWorld()->GetNetMode() == NM_DedicatedServer) return;
  UGameplayStatics::PlaySoundAtLocation(this, Sound, Actor->GetActorLocation(), 1.f, 1.f, 0.f,
      bBoss ? Config->BossAttenuation.Get() : nullptr);
}
void UDrAudioSubsystem::PlayBossAttack(AActor* Boss, EDrBossAttackKind Kind) {
  PlayAt(ResolveBossAttack(Kind), Boss, true);
  const auto* Config = GetData();
  if (!Config) return;
  const int32 Voice = Kind == EDrBossAttackKind::Siphon ? 7 : Kind == EDrBossAttackKind::Area ? 6 : 0;
  if (Config->BossVoices.IsValidIndex(Voice)) PlayAt(Config->BossVoices[Voice], Boss, true);
}
void UDrAudioSubsystem::PlayDamage(AActor* Victim, float Damage, float Health, float MaxHealth) {
  const auto* Config = GetData();
  if (!Config || !Victim || Damage <= 0.f) return;
  const bool bBoss = Victim->IsA<ADrBossCharacter>();
  if (Config->Impacts.Num()) PlayAt(Config->Impacts[FMath::RandHelper(Config->Impacts.Num())], Victim, bBoss);
  // 사망 음성은 엔카운터 상태 전이에서 한 번만 재생한다.
  const int32 Index = Health <= MaxHealth * .3f && Damage >= MaxHealth * .1f ? 2 : 1;
  const auto& Voices = bBoss ? Config->BossVoices : Config->PlayerVoices;
  if (Health > 0.f && Voices.IsValidIndex(Index)) PlayAt(Voices[Index], Victim, bBoss);
}
void UDrAudioSubsystem::EncounterChanged(EDrEncounterState State, AActor* Player, AActor* Boss) {
  const auto* Config = GetData();
  MusicState = State;
  BossActor = Boss;
  if (!Config) return;
  if (State == EDrEncounterState::Fighting) {
    SetPhase(1);
    if (!MusicComponent) {
      MusicComponent = NewObject<UAudioComponent>(GetWorld());
      MusicComponent->bAutoActivate = false;
      MusicComponent->bAutoDestroy = false;
      MusicComponent->bIsUISound = true;
      MusicComponent->SetSound(Config->Music);
      MusicComponent->RegisterComponentWithWorld(GetWorld());
    }
    MusicComponent->SetFloatParameter(TEXT("Phase"), static_cast<float>(Phase));
    if (Config->Music) MusicComponent->FadeIn(2.f, .65f);
    if (Config->BossVoices.IsValidIndex(4)) PlayAt(Config->BossVoices[4], Boss, true);
  } else if (State == EDrEncounterState::PlayerDead) {
    if (MusicComponent) MusicComponent->AdjustVolume(.3f, .12f);
    if (Config->PlayerVoices.IsValidIndex(3)) PlayAt(Config->PlayerVoices[3], Player, false);
    if (Config->BossVoices.IsValidIndex(5)) PlayAt(Config->BossVoices[5], Boss, true);
  } else {
    if (MusicComponent) MusicComponent->FadeOut(State == EDrEncounterState::Victory ? 3.f : .5f, 0.f);
    if (State == EDrEncounterState::Victory && Config->BossVoices.IsValidIndex(3)) PlayAt(Config->BossVoices[3], Boss, true);
    if (State == EDrEncounterState::Idle) SetPhase(1);
  }
}
void UDrAudioSubsystem::SetPhase(int32 InPhase) {
  Phase = FMath::Clamp(InPhase, 1, 2);
  if (MusicComponent) MusicComponent->SetFloatParameter(TEXT("Phase"), static_cast<float>(Phase));
}
void UDrAudioSubsystem::MoveDoor(AActor* Door, bool bMoving) {
  if (!bMoving) { if (DoorComponent) DoorComponent->Stop(); return; }
  const auto* Config = GetData();
  if (!Config || !Door || !Door->GetRootComponent()) return;
  if (DoorComponent) DoorComponent->Stop();
  DoorComponent = UGameplayStatics::SpawnSoundAttached(Config->Slide, Door->GetRootComponent(), NAME_None,
      FVector::ZeroVector, EAttachLocation::KeepRelativeOffset, true, .7f, 1.f, 0.f, Config->BossAttenuation, nullptr, false);
}
void UDrAudioSubsystem::SetChannelVolume(EDrAudioChannel Channel, float Value) {
  const int32 Index = static_cast<int32>(Channel);
  if (Index < 0 || Index >= 3) return;
  Volumes[Index] = FMath::Clamp(Value, 0.f, 1.f);
  const auto* Config = GetData();
  if (Config && Config->Buses.IsValidIndex(Index) && Config->Buses[Index])
    UAudioModulationStatics::SetGlobalBusMixValue(this, Config->Buses[Index], Volumes[Index], .1f);
}
float UDrAudioSubsystem::GetChannelVolume(EDrAudioChannel Channel) const {
  const int32 Index = static_cast<int32>(Channel);
  return Index >= 0 && Index < 3 ? Volumes[Index] : 0.f;
}
void UDrAudioSubsystem::Tick(float DeltaTime) {
  const auto* Config = GetData();
  if (!Config || DeltaTime <= 0.f || GetWorld()->GetNetMode() == NM_DedicatedServer) return;
  // 간격·샘플 선택은 MetaSound가 담당하고, 월드는 원거리 위치만 갱신한다.
  DistantRemaining -= DeltaTime;
  if (DistantRemaining <= 0.f) {
    DistantRemaining = FMath::FRandRange(16.f, 24.f);
    const auto* PC = GetWorld()->GetFirstPlayerController();
    const FVector Center = PC && PC->GetPawn() ? PC->GetPawn()->GetActorLocation() : FVector::ZeroVector;
    const float Angle = FMath::FRandRange(0.f, 2.f * PI);
    const FVector Location = Center + FVector(FMath::Cos(Angle), FMath::Sin(Angle), .15f) * FMath::FRandRange(1800.f, 3200.f);
    UGameplayStatics::PlaySoundAtLocation(this, Config->Distant, Location, 1.f, 1.f, 0.f, Config->EnvironmentAttenuation);
  }
  FootstepRemaining -= DeltaTime;
  if (BossActor.IsValid() && BossActor->GetVelocity().Size2D() > 80.f && FootstepRemaining <= 0.f && Config->Footsteps.Num()) {
    FootstepRemaining = .65f;
    PlayAt(Config->Footsteps[FMath::RandHelper(Config->Footsteps.Num())], BossActor.Get(), true);
  }
}
void UDrAudioSubsystem::StopOwnedAudio() {
  if (MusicComponent) { MusicComponent->Stop(); MusicComponent->DestroyComponent(); MusicComponent = nullptr; }
  if (DoorComponent) { DoorComponent->Stop(); DoorComponent->DestroyComponent(); DoorComponent = nullptr; }
  for (auto Component : EnvironmentComponents) if (Component) { Component->Stop(); Component->DestroyComponent(); }
  EnvironmentComponents.Reset();
}
void UDrAudioSubsystem::Deinitialize() {
  if (bRecording) {
    const FString Directory = FPaths::ProjectSavedDir() / TEXT("CombatVisual");
    IFileManager::Get().MakeDirectory(*Directory, true);
    UAudioMixerBlueprintLibrary::StopRecordingOutput(this, EAudioRecordingExportType::WavFile, TEXT("audio"), Directory, nullptr, nullptr);
    bRecording = false;
  }
  StopOwnedAudio();
  UnbindDamageEvents();
  if (Encounter.IsValid()) {
    Encounter->OnStateChanged.RemoveAll(this);
    Encounter->OnActorsChanged.RemoveAll(this);
    Encounter->OnDoorMoving.RemoveAll(this);
  }
  Super::Deinitialize();
}
