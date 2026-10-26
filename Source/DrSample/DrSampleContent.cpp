#include "DrSampleContent.h"

#include "Abilities/DrAttributeSet.h"
#include "Boss/DrBossBrainComponent.h"
#include "Boss/DrBossData.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "Engine/AssetUserData.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameFramework/Pawn.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/SkeletalBodySetup.h"
#include "Components/SkeletalMeshComponent.h"
#if WITH_EDITOR
#include "AnimGraphNode_BlendSpacePlayer.h"
#include "AnimGraphNode_Root.h"
#include "AnimGraphNode_Slot.h"
#include "K2Node_VariableGet.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#endif

void UDrSampleAnimInstance::NativeUpdateAnimation(float DeltaSeconds) {
 Super::NativeUpdateAnimation(DeltaSeconds);
 const APawn* Pawn = TryGetPawnOwner();
 Speed = Pawn ? Pawn->GetVelocity().Size2D() : 0.f;
}
ADrSampleEnemy::ADrSampleEnemy() { PrimaryActorTick.bCanEverTick = true; }
void ADrSampleEnemy::BeginPlay() {
 Super::BeginPlay();
 GetAttributes()->OnDamaged.AddUObject(this, &ADrSampleEnemy::OnSampleDamaged);
}
void ADrSampleEnemy::OnSampleDamaged(float Damage, AActor*, AActor*) {
 if (Damage <= 0.f || GetAttributes()->GetHealth() <= 0.f || !HitMontage) return;
 GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
 if (UAnimInstance* Anim = GetMesh()->GetAnimInstance()) {
  Anim->Montage_Play(HitMontage);
  ReactionRemaining = HitMontage->GetPlayLength();
 }
}
void ADrSampleEnemy::Tick(float DeltaSeconds) {
 Super::Tick(DeltaSeconds);
 if (ReactionRemaining <= 0.f) return;
 ReactionRemaining -= DeltaSeconds;
 if (ReactionRemaining > 0.f || GetBrain()->GetState() == EDrBossState::Dead) return;
 // 다음 판단 상태가 전환되기 전까지 기본 대기를 복구한다.
 PlaySequence(GetData()->IdleAnim, true);
}
void UDrSampleContent::CleanPhysics(UPhysicsAsset* Physics) {
#if WITH_EDITOR
 if (!Physics) return;
 for (USkeletalBodySetup* Body : Physics->SkeletalBodySetups)
  if (Body) Body->PhysMaterial = nullptr;
 Physics->MarkPackageDirty();
#endif
}
int32 UDrSampleContent::CleanAnimation(UAnimSequence* Sequence) {
 int32 Removed = 0;
#if WITH_EDITOR
 if (!Sequence) return 0;
 if (const auto* UserData = Sequence->GetAssetUserDataArray()) {
  const auto Copy = *UserData;
  for (UAssetUserData* Data : Copy)
   if (Data && Data->GetClass()->GetName().Contains(TEXT("AnimationModifiers"))) {
    Sequence->RemoveUserDataOfClass(Data->GetClass()); ++Removed;
   }
 }
 Removed += Sequence->Notifies.RemoveAll([](const FAnimNotifyEvent& Event) {
  // Lyra 전용 BP 노티파이는 샘플의 판정·소리 경로가 아니므로 가져오지 않는다.
  const UObject* Notify = Event.Notify ? static_cast<UObject*>(Event.Notify) : static_cast<UObject*>(Event.NotifyStateClass);
  return !Notify || Notify->GetClass()->ClassGeneratedBy || Notify->GetClass()->GetPathName().Contains(TEXT("/Script/LyraGame"));
 });
 if (Removed) { Sequence->RefreshCacheData(); Sequence->MarkPackageDirty(); }
#endif
 return Removed;
}
bool UDrSampleContent::BuildAnimationBlueprint(UAnimBlueprint* Blueprint, UBlendSpace* Movement) {
#if WITH_EDITOR
 if (!Blueprint || !Movement) return false;
 UEdGraph* Graph = FindObject<UEdGraph>(Blueprint, TEXT("AnimGraph"));
 if (!Graph) {
  for (UEdGraph* Candidate : Blueprint->FunctionGraphs)
   if (Candidate->GetFName() == TEXT("AnimGraph")) Graph = Candidate;
 }
 if (!Graph) return false;
 UAnimGraphNode_Root* Root = nullptr;
 const auto Nodes = Graph->Nodes;
 for (UEdGraphNode* Node : Nodes) {
  if (auto* Result = Cast<UAnimGraphNode_Root>(Node)) Root = Result;
  else Graph->RemoveNode(Node);
 }
 if (!Root) return false;
 auto* Blend = NewObject<UAnimGraphNode_BlendSpacePlayer>(Graph);
 Graph->AddNode(Blend); Blend->CreateNewGuid();
 Blend->Node.SetBlendSpace(Movement); Blend->AllocateDefaultPins();
 auto* Slot = NewObject<UAnimGraphNode_Slot>(Graph);
 Graph->AddNode(Slot); Slot->CreateNewGuid(); Slot->Node.SlotName = TEXT("DefaultSlot"); Slot->AllocateDefaultPins();
 auto* Speed = NewObject<UK2Node_VariableGet>(Graph);
 Speed->VariableReference.SetExternalMember(GET_MEMBER_NAME_CHECKED(UDrSampleAnimInstance, Speed), UDrSampleAnimInstance::StaticClass());
 Graph->AddNode(Speed); Speed->CreateNewGuid(); Speed->AllocateDefaultPins();
 const UEdGraphSchema* Schema = Graph->GetSchema();
 const bool bConnected = Schema->TryCreateConnection(Blend->FindPinChecked(TEXT("Pose")), Slot->FindPinChecked(TEXT("Source"))) &&
  Schema->TryCreateConnection(Slot->FindPinChecked(TEXT("Pose")), Root->FindPinChecked(TEXT("Result"))) &&
  Schema->TryCreateConnection(Speed->FindPinChecked(TEXT("Speed")), Blend->FindPinChecked(TEXT("X")));
 FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
 FKismetEditorUtilities::CompileBlueprint(Blueprint);
 return bConnected && Blueprint->Status != BS_Error;
#else
 return false;
#endif
}
