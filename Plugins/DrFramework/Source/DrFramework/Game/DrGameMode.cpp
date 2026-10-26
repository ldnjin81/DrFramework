#include "Game/DrGameMode.h"
#include "Game/DrPlayerController.h"
ADrGameMode::ADrGameMode() { DefaultPawnClass=nullptr; PlayerControllerClass=ADrPlayerController::StaticClass(); }
