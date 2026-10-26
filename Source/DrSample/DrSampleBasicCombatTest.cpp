// 실제 샘플 맵과 입력·GAS·보스 판단을 함께 검증한다.
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "DrSampleContent.h"
#include "Abilities/DrAbilitySystemComponent.h"
#include "Abilities/DrAttributeSet.h"
#include "Boss/DrBossBrainComponent.h"
#include "Characters/DrCharacter.h"
#include "Core/DrGameplayTags.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "HAL/FileManager.h"
#include "ImageUtils.h"
#include "InputAction.h"
#include "Misc/App.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "UnrealClient.h"

namespace DrSampleTests {
class FBasicCombat : public IAutomationLatentCommand {
public:
 explicit FBasicCombat(FAutomationTestBase* InTest) : Test(InTest) {}
 bool Update() override;
private:
 bool Finish(const FString& Error = FString()) {
  if (!Error.IsEmpty()) Test->AddError(Error);
  FApp::SetUseFixedTimeStep(bPreviousFixed);
  FApp::SetFixedDeltaTime(PreviousDelta);
  return true;
 }
 void Next(const TCHAR* Name) {
  Test->AddInfo(FString::Printf(TEXT("샘플 통과: %s"), Name));
  ++Stage; Frames = 0;
 }
 bool Shot(const TCHAR* Name) {
  if (!FApp::CanEverRender()) return true;
  const FString Path = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("Screenshots/DrSample") / Name);
  IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path), true);
  FScreenshotRequest::RequestScreenshot(Path, false, false);
  return true;
 }
 FAutomationTestBase* Test;
 int32 Stage = 0, Frames = 0, Total = 0;
 FVector Start;
 float EnemyHealth = 0.f, PlayerHealth = 0.f;
 bool bPreviousFixed = false;
 double PreviousDelta = 0;
 bool bStarted = false;
 bool bEnemyReaction = false, bPlayerReaction = false;
};
bool FBasicCombat::Update() {
 if (!bStarted) {
  bStarted = true;
  bPreviousFixed = FApp::UseFixedTimeStep(); PreviousDelta = FApp::GetFixedDeltaTime();
  FApp::SetFixedDeltaTime(1.0 / 64.0); FApp::SetUseFixedTimeStep(true);
 }
 if (++Total > 6400) return Finish(TEXT("샘플 PIE 준비/전투 시간 초과"));
 UWorld* World = GEditor ? GEditor->PlayWorld.Get() : nullptr;
 APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
 ADrCharacter* Player = PC ? Cast<ADrCharacter>(PC->GetPawn()) : nullptr;
 ADrSampleEnemy* Enemy = nullptr;
 if (World) for (TActorIterator<ADrSampleEnemy> It(World); It; ++It) { Enemy = *It; break; }
 if (!Player || !Enemy) return false;
 ++Frames;
 auto* Brain = Enemy->GetBrain();
 auto* Input = PC->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
 auto* Move = LoadObject<UInputAction>(nullptr, TEXT("/Game/DrSample/Input/IA_DrSample_Move.IA_DrSample_Move"));
 USkeletalMeshComponent* Visible = nullptr;
 for (USkeletalMeshComponent* Mesh : TInlineComponentArray<USkeletalMeshComponent*>(Player))
  if (Mesh->GetFName() == TEXT("MannyMesh")) Visible = Mesh;
 if (!Visible || !Visible->GetAnimInstance() || !Input || !Move)
  return Finish(TEXT("샘플 메시·애님 BP·입력 연결 누락"));
 switch (Stage) {
 case 0:
  Brain->SetEngageAllowed(false); Brain->SetRandomSeed(64);
  if (Frames < 64) return false;
  Start = Player->GetActorLocation();
  Shot(TEXT("01_idle.png")); Next(TEXT("대기와 애님 BP")); break;
 case 1:
  Input->InjectInputForAction(Move, FInputActionValue(FVector2D(0,1)), {}, {});
  if (Frames < 32) return false;
  Test->TestTrue(TEXT("a: 이동 입력으로 위치 변화"), FVector::Dist2D(Start, Player->GetActorLocation()) > 20.f);
  EnemyHealth = Enemy->GetAttributes()->GetHealth();
  Player->SetActorLocation(Enemy->GetActorLocation() + FVector(-180,0,0));
  Player->SetActorRotation(FRotator::ZeroRotator);
  PC->SetControlRotation(FRotator::ZeroRotator);
  Test->TestTrue(TEXT("기본 공격 입력 수락"), Player->InputAttackLight());
  Next(TEXT("a 이동")); break;
 case 2:
  if (Enemy->GetMesh()->GetAnimInstance() && Enemy->GetMesh()->GetAnimInstance()->IsAnyMontagePlaying()) bEnemyReaction = true;
  if (Enemy->GetAttributes()->GetHealth() < EnemyHealth) {
   Test->TestTrue(TEXT("b: 적 체력 감소"), true);
   Test->TestTrue(TEXT("b: 적 피격 몽타주"), bEnemyReaction);
   Shot(TEXT("02_attack_hit.png")); Next(TEXT("b 공격 명중·피격"));
  } else if (Frames > 192) return Finish(TEXT("b: 기본 공격이 적에게 명중하지 않음"));
  break;
 case 3:
  if (Frames < 128) return false;
  PlayerHealth = Player->GetAttributeSet()->GetHealth();
  Enemy->SetActorLocation(Player->GetActorLocation() + FVector(650,0,0));
  Enemy->SetActorRotation(FRotator(0,180,0));
  Start = Enemy->GetActorLocation();
  Brain->SetTarget(Player); Brain->SetEngageAllowed(true);
  Next(TEXT("적 추적 시작")); break;
 case 4:
  if (Visible->GetAnimInstance()->IsAnyMontagePlaying()) bPlayerReaction = true;
  if (Player->GetAttributeSet()->GetHealth() < PlayerHealth) {
   Test->TestTrue(TEXT("c: 적 추적 위치 변화"), FVector::Dist2D(Start, Enemy->GetActorLocation()) > 30.f);
   Test->TestTrue(TEXT("c: 플레이어 피격 몽타주"), bPlayerReaction);
   Shot(TEXT("03_player_hit.png")); Brain->SetEngageAllowed(false);
   Next(TEXT("c 적 추적·공격·플레이어 피격"));
  } else if (Frames > 1024) return Finish(TEXT("c: 적 추적/공격으로 플레이어 체력이 감소하지 않음"));
  break;
 case 5:
  if (Frames < 128) return false;
  Enemy->GetDrASC()->SetNumericAttributeBase(UDrAttributeSet::GetHealthAttribute(), 1.f);
  Player->SetActorLocation(Enemy->GetActorLocation() + FVector(-180,0,0));
  Player->SetActorRotation(FRotator::ZeroRotator);
  Test->TestTrue(TEXT("마지막 기본 공격 입력"), Player->InputAttackLight());
  Next(TEXT("사망 검증 공격")); break;
 case 6:
  if (Enemy->GetAttributes()->GetHealth() <= 0.f) {
   Test->TestTrue(TEXT("d: 적 사망 상태"), Brain->GetState() == EDrBossState::Dead);
   auto* Single = Enemy->GetMesh()->GetSingleNodeInstance();
   Test->TestTrue(TEXT("d: 적 사망 애니메이션"), Single && Single->GetCurrentAsset() && Single->GetCurrentAsset()->GetName().Contains(TEXT("Death")));
   Shot(TEXT("04_death.png")); Next(TEXT("d 사망"));
  } else if (Frames > 192) return Finish(TEXT("d: 적이 사망하지 않음"));
  break;
 case 7:
  if (Frames < 32) return false;
  return Finish();
 }
 return false;
}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrSampleBasicCombat, "DrSample.PIE.BasicCombat",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FDrSampleBasicCombat::RunTest(const FString&) {
 AutomationOpenMap(TEXT("/Game/DrSample/Maps/L_DrSample"));
 ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
 ADD_LATENT_AUTOMATION_COMMAND(DrSampleTests::FBasicCombat(this));
 ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
 return true;
}
#endif
