#include "Game/DrGameMode.h"
#include "TableData/DrGameAssets.h"

#include "Characters/DrCharacter.h"
#include "Game/DrPlayerController.h"
#include "UObject/SoftObjectPath.h"

ADrGameMode::ADrGameMode() {
  DefaultPawnClass = ADrCharacter::StaticClass();
  PlayerControllerClass = ADrPlayerController::StaticClass();
}

void ADrGameMode::InitGame(const FString &MapName, const FString &Options,
                           FString &ErrorMessage) {
  Super::InitGame(MapName, Options, ErrorMessage);
  const FSoftObjectPath Path = DrGameAssets::Path(TEXT("Player"));
  if (UClass* CharacterClass = FSoftClassPath(Path.ToString()).TryLoadClass<ADrCharacter>()) {
    DefaultPawnClass = CharacterClass;
  } else {
    UE_LOG(LogTemp, Error, TEXT("GameAssets.Player 플레이어 클래스를 불러올 수 없습니다: %s"), *Path.ToString());
  }
}
