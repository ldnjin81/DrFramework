#include "Game/DrPlayerController.h"
#include "UI/DrUISettings.h"
#include "UI/DrUIRootLayout.h"
#include "UI/DrHUDWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/App.h"
void ADrPlayerController::SetupInputComponent() {
 Super::SetupInputComponent(); const auto* Settings=GetDefault<UDrUISettings>();
 PauseAction=Settings->PauseAction.LoadSynchronous(); PauseContext=Settings->MenuInputContext.LoadSynchronous();
 if(auto* Enhanced=Cast<UEnhancedInputComponent>(InputComponent)) if(PauseAction) Enhanced->BindAction(PauseAction,ETriggerEvent::Started,this,&ThisClass::TogglePauseMenu);
 if(GetLocalPlayer() && PauseContext) if(auto* Input=GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()) Input->AddMappingContext(PauseContext,100);
}
bool ADrPlayerController::IsTitleScreen() const { const auto& Map=GetDefault<UDrUISettings>()->TitleMap; return GetWorld() && !Map.IsNull() && UGameplayStatics::GetCurrentLevelName(this,true)==FPackageName::GetShortName(Map.GetLongPackageName()); }
void ADrPlayerController::ApplyInputMode(bool bMenu) { bMenuInput=bMenu; bShowMouseCursor=bMenu; if(!GetLocalPlayer()) return; if(bMenu) { FInputModeUIOnly Mode; Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); SetInputMode(Mode); } else SetInputMode(FInputModeGameOnly()); }
bool ADrPlayerController::OpenPauseMenu() {
 if(IsTitleScreen() || bPauseOpen || !CanOpenPauseMenu()) return false;
 auto* MenuClass=GetDefault<UDrUISettings>()->PauseMenuClass.LoadSynchronous();
 if(!MenuClass || !SetPause(true)) return false;
 bPauseOpen=true; if(RootLayout) RootLayout->PushMenu(MenuClass); ApplyInputMode(true); OnMenuChanged(true); return true;
}
void ADrPlayerController::ClosePauseMenu() { if(!bPauseOpen) return; SettingsMenu=nullptr; if(RootLayout) RootLayout->ClearMenus(); bPauseOpen=false; SetPause(false); ApplyInputMode(false); OnMenuChanged(false); }
void ADrPlayerController::TogglePauseMenu() { if(bPauseOpen) BackMenu(); else OpenPauseMenu(); }
void ADrPlayerController::OpenSettings() { if(SettingsMenu || (!bPauseOpen && !IsTitleScreen()) || !RootLayout) return; if(auto* Type=GetDefault<UDrUISettings>()->SettingsMenuClass.LoadSynchronous()) { SettingsMenu=RootLayout->PushMenu(Type); ApplyInputMode(true); } }
void ADrPlayerController::BackMenu() { if(SettingsMenu) { if(RootLayout) RootLayout->PopMenu(); SettingsMenu=nullptr; } else if(!IsTitleScreen()) ClosePauseMenu(); }
void ADrPlayerController::StartGame() { if(!IsTitleScreen()) return; const auto& Map=GetDefault<UDrUISettings>()->GameMap; if(!Map.IsNull()) { SetPause(false); UGameplayStatics::OpenLevel(this,*Map.GetLongPackageName()); } }
void ADrPlayerController::ReturnToTitle() { ClosePauseMenu(); const auto& Map=GetDefault<UDrUISettings>()->TitleMap; if(!Map.IsNull()) UGameplayStatics::OpenLevel(this,*Map.GetLongPackageName()); }
void ADrPlayerController::RequestRestart_Implementation() { ClosePauseMenu(); }
void ADrPlayerController::BeginPlay() {
 Super::BeginPlay(); if(!IsLocalController()) return;
 const auto* Settings=GetDefault<UDrUISettings>();
 if(FApp::CanEverRender() && GetLocalPlayer()) { RootLayout=CreateWidget<UDrUIRootLayout>(this); RootLayout->AddToPlayerScreen(); }
 OnPossessedPawnChanged.AddUniqueDynamic(this,&ThisClass::HandlePawnChanged);
 if(IsTitleScreen()) { if(RootLayout) if(auto* Type=Settings->TitleMenuClass.LoadSynchronous()) RootLayout->PushMenu(Type); ApplyInputMode(true); }
 else { ApplyInputMode(false); if(RootLayout) if(auto* Type=Settings->HUDClass.LoadSynchronous()) HUD=Cast<UDrHUDWidget>(RootLayout->PushGame(Type)); OnHUDCreated(HUD); }
 HandlePawnChanged(nullptr,GetPawn());
}
void ADrPlayerController::HandlePawnChanged(APawn* PreviousPawn,APawn* NewPawn) { if(HUD) HUD->OnOwnerPawnChanged(PreviousPawn,NewPawn); OnPawnChanged(PreviousPawn,NewPawn); }
void ADrPlayerController::EndPlay(const EEndPlayReason::Type Reason) { OnPossessedPawnChanged.RemoveDynamic(this,&ThisClass::HandlePawnChanged); if(RootLayout) RootLayout->RemoveFromParent(); if(GetLocalPlayer() && PauseContext) if(auto* Input=GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()) Input->RemoveMappingContext(PauseContext); Super::EndPlay(Reason); }
