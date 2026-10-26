#include "UI/DrHUDWidget.h"
TOptional<FUIInputConfig> UDrHUDWidget::GetDesiredInputConfig() const { return FUIInputConfig(ECommonInputMode::Game,EMouseCaptureMode::CapturePermanently); }
