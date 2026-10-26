#include "UI/DrMenuWidget.h"
#include "TableData/DrGameAssets.h"

#include "UI/DrGlassButton.h"
#include "Game/DrPlayerController.h"
#include "Blueprint/WidgetTree.h"
#include "Components/BackgroundBlur.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Input/CommonUIInputTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Styling/CoreStyle.h"
#include "InputMappingContext.h"
#include "Internationalization/StringTable.h"

FText UDrMenuWidget::MenuText(const TCHAR* Key) {
  // 첫 타이틀 프레임에도 문구가 보이도록 비동기 요청 전에 표를 확실히 로드한다.
  auto* Table = LoadObject<UStringTable>(nullptr, *DrGameAssets::Path(TEXT("ST_UI")).ToString());
  return FText::FromStringTable(Table ? Table->GetStringTableId() : NAME_None, Key);
}
void UDrMenuWidget::ApplyItemSpacing(UVerticalBoxSlot* LayoutSlot) {
  LayoutSlot->SetPadding(FMargin(32, 10));
  LayoutSlot->SetHorizontalAlignment(HAlign_Center);
}
UDrMenuWidget::UDrMenuWidget() { bIsBackHandler = true; }
void UDrMenuWidget::NativeOnActivated() {
  InputMapping = LoadObject<UInputMappingContext>(nullptr, *DrGameAssets::Path(TEXT("IMC_UI")).ToString());
  InputMappingPriority = 200;
  Super::NativeOnActivated();
}
TOptional<FUIInputConfig> UDrMenuWidget::GetDesiredInputConfig() const {
  return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
}
TSharedRef<SWidget> UDrMenuWidget::RebuildWidget() {
  FirstFocus = nullptr;
  auto* Root = WidgetTree->ConstructWidget<UCanvasPanel>();
  WidgetTree->RootWidget = Root;
  auto* Dim = WidgetTree->ConstructWidget<UImage>();
  Dim->SetColorAndOpacity(FLinearColor(0, 0, 0, .25f));
  auto* DimSlot = Root->AddChildToCanvas(Dim);
  DimSlot->SetAnchors(FAnchors(0, 0, 1, 1)); DimSlot->SetOffsets(FMargin(0));
  auto* Layers = WidgetTree->ConstructWidget<UOverlay>();
  auto* PanelSlot = Root->AddChildToCanvas(Layers);
  PanelSlot->SetAnchors(FAnchors(.5f)); PanelSlot->SetAlignment(FVector2D(.5f));
  PanelSlot->SetAutoSize(true);
  auto Fill = [Layers](UWidget* Child) {
    auto* LayoutSlot = Layers->AddChildToOverlay(Child);
    LayoutSlot->SetHorizontalAlignment(HAlign_Fill); LayoutSlot->SetVerticalAlignment(VAlign_Fill);
  };
  auto* Blur = WidgetTree->ConstructWidget<UBackgroundBlur>();
  Blur->SetBlurStrength(28.f); Blur->SetCornerRadius(FVector4(36)); Fill(Blur);
  auto* Glass = WidgetTree->ConstructWidget<UImage>();
  if (auto* Parent = LoadObject<UMaterialInterface>(nullptr, *DrGameAssets::Path(TEXT("M_UI_Glass")).ToString())) {
    auto* MID = UMaterialInstanceDynamic::Create(Parent, this);
    MID->SetVectorParameterValue(TEXT("PanelSize"), FLinearColor(464, 500, 0));
    MID->SetScalarParameterValue(TEXT("CornerRadius"), 36.f);
    MID->SetScalarParameterValue(TEXT("FillOpacity"), .24f);
    Glass->SetBrushFromMaterial(MID);
  }
  Fill(Glass);
  auto* Contents = WidgetTree->ConstructWidget<UVerticalBox>();
  Fill(Contents);
  BuildMenu(Contents);
  return Super::RebuildWidget();
}
void UDrMenuWidget::AddHeading(UVerticalBox* Contents, const TCHAR* Key) {
  auto* Text = WidgetTree->ConstructWidget<UTextBlock>();
  Text->SetText(MenuText(Key)); Text->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Light"), 38));
  auto* LayoutSlot = Contents->AddChildToVerticalBox(Text);
  LayoutSlot->SetPadding(FMargin(32, 30, 32, 20)); LayoutSlot->SetHorizontalAlignment(HAlign_Center);
}
UDrGlassButton* UDrMenuWidget::AddButton(UVerticalBox* Contents, const TCHAR* Key) {
  // 위젯 트리로 만들어야 플레이어가 없는 썸네일 렌더링에서도 버튼이 생긴다.
  auto* Button = WidgetTree->ConstructWidget<UDrGlassButton>(UDrGlassButton::StaticClass());
  Button->SetLabel(MenuText(Key)); ApplyItemSpacing(Contents->AddChildToVerticalBox(Button));
  if (!FirstFocus) FirstFocus = Button;
  return Button;
}
UWidget* UDrMenuWidget::NativeGetDesiredFocusTarget() const { return FirstFocus; }
bool UDrMenuWidget::NativeOnHandleBackAction() {
  if (auto* PC = GetOwningPlayer<ADrPlayerController>()) PC->BackMenu();
  return true;
}
FReply UDrMenuWidget::NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) {
  if (Event.GetKey() == EKeys::Escape || Event.GetKey() == EKeys::Gamepad_Special_Right) {
    if (!Event.IsRepeat()) NativeOnHandleBackAction();
    return FReply::Handled();
  }
  return Super::NativeOnPreviewKeyDown(Geometry, Event);
}
