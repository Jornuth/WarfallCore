#include "Custom/Variables/ColorPicker.h"

#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "Custom/Widgets/DetailMessageRow.h"
#include "Utils/GlobalTools.h"
#include "Widgets/Input/SNumericEntryBox.h"

#define LOCTEXT_NAMESPACE "CustomColorPicker"

FLinearColor FColorPicker::GetColor(const int32 InStencil) const
{
	int32 NewStencil = InStencil;
	if (NewStencil == INDEX_NONE) NewStencil = Stencil;
	if (UMaterialInstance* MaterialInstance = UGlobalTools::GetMaterialInstance(EMaterialInstPath::Outline); MaterialInstance)
	{
		const UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(MaterialInstance, nullptr);
		if (!DynamicMaterial) return FLinearColor::Transparent;

		const FString ParameterName = FString::Printf(TEXT("Color_%d"), NewStencil);
		FLinearColor Color = FLinearColor::Transparent;
		DynamicMaterial->GetVectorParameterValue(FName(*ParameterName), Color);
		return Color;
	}
	return FLinearColor::Transparent;
}

void FCustomColorPicker::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow,
                                         IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	PropertyHandle = StructPropertyHandle;
	PropertyUtils = CustomizationUtils.GetPropertyUtilities();

	if (void* Dummy = nullptr; StructPropertyHandle->GetValueData(Dummy) == FPropertyAccess::Success)
	{
		TArray<void*> RawData;
		StructPropertyHandle->AccessRawData(RawData);
		if (RawData.IsEmpty()) return;
				
		Handle = static_cast<FColorPicker*>(RawData[0]);
	}

	HeaderRow
	.NameContent()
	[
		StructPropertyHandle->CreatePropertyNameWidget()
	];
}

void FCustomColorPicker::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle,IDetailChildrenBuilder& ChildBuilder,
	IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	const FString PaintDir = FPaths::ProjectPluginsDir() / TEXT("GameCore/Resources/");
	PaintBrush = MakeShareable(new FSlateDynamicImageBrush(
		FName(*(PaintDir / FString(TEXT("T_Ellipse_Base_256x256.PNG")))),
		FVector2D(256, 256) 
	));
	PaintBrush.Reset(); // Don't want to use it!

	DetailMessage = FDetailMessageRow::Create(ChildBuilder);
	
	if (!Handle) { DetailMessage->NewMessage("Error: Struct is corrupted!", EMessageType::Error, true, 0, 10); return; }
	ChildBuilder.AddCustomRow(FText::FromString("ColorPickerCustomization"))
	.NameContent()
	[
		SNew(STextBlock)
		.Text(FText::FromString(StructPropertyHandle->GetPropertyDisplayName().ToString()))
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.Margin(FMargin(0.f, 0.f, 6.f, 0.f))
	]
	.ValueContent()
	.MinDesiredWidth(200.f)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.Padding(0.f, 10.f, 0.f, 0.f)
		.AutoHeight()
		.HAlign(HAlign_Left)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(FText::FromString("Color Index"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(FMargin(5.f, 0.f, 0.f, 0.f))
			[
				SNew(SNumericEntryBox<int32>)
				.MinValue(1)
				.MaxValue(52)
				.Value_Lambda([this]() -> TOptional<int32>
				{
					return GetStencilValue();
				})
				.OnValueCommitted_Lambda([this](const int32 NewValue, ETextCommit::Type Commit)
				{
					const int32 Value = FMath::Clamp(NewValue, Handle->FirstStencil, Handle->NumStencil);
					UpdateColor(Value);
				})
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 10.f, 0.f, 10.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			.HAlign(HAlign_Left)
			[
				SAssignNew(ColorPanel, SVerticalBox)
				.Cursor(EMouseCursor::EyeDropper)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					.VAlign(VAlign_Fill)
					.HAlign(HAlign_Fill)
					[
						SNew(SButton)
						.ButtonColorAndOpacity(FLinearColor::Transparent)
						.ButtonStyle(FCoreStyle::Get(), "NoBorder")
						.Cursor(EMouseCursor::EyeDropper)
					]
					+ SOverlay::Slot()
					.VAlign(VAlign_Fill)
					.HAlign(HAlign_Fill)
					[
						CreatePickupColor()
					]
				]
			]
			+ SHorizontalBox::Slot().AutoWidth()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Fill)
			.Padding(5.f, 0.f, 0.f, 0.f)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.VAlign(VAlign_Top)
				.HAlign(HAlign_Left)
				[
					SAssignNew(RefreshButton, SButton)
					.ButtonStyle(FAppStyle::Get(), "SimpleButton")
					.ContentPadding(FMargin(2.f))
					.ButtonColorAndOpacity_Lambda([this]() -> FLinearColor
					{
						return RefreshButton.IsValid() && RefreshButton->IsHovered()
							? FLinearColor(0.05f, 0.05f, 0.05f, 1.0f)
							: FLinearColor::Transparent;
					})
					.ToolTipText(FText::FromString("Refresh all outlines colors."))
					.OnClicked_Lambda([this]() -> FReply
					{
						RefreshColor();
						return FReply::Handled();
					})
					[
						SNew(SImage)
						.Image(FAppStyle::GetBrush("SourceControl.Actions.Refresh"))
					]
				]
				+ SOverlay::Slot()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Left)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.HAlign(HAlign_Center)
						[
							SNew(STextBlock)
							.Text(FText::FromString("Preview"))
							.Font(IDetailLayoutBuilder::GetDetailFont())
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 4.f, 0.f, 0.f)
						.HAlign(HAlign_Center)
						[
							SNew(SBox)
							.WidthOverride(50.f)
							.HeightOverride(50.f)
							[
								SNew(SImage)
								.Image(PaintBrush.IsValid() ? PaintBrush.Get() : FCoreStyle::Get().GetBrush("WhiteBrush"))
								.ColorAndOpacity_Lambda([this]() -> FLinearColor
								{
									if (HoveredStencil != INDEX_NONE)
									{
										return GetColor(HoveredStencil);
									}
									return GetColor(GetStencilValue());
								})
							]
						]
					]
					+ SHorizontalBox::Slot()
					.Padding(5.f, 10.f, 0.f, 0.f)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Left)
					[
						SNew(SEditableText)
						.IsReadOnly(true)
						.MinDesiredWidth(100.f)
						.Text_Lambda([this]() -> FText
						{
							FLinearColor Color = GetColor(GetStencilValue());
							
							if (HoveredStencil != INDEX_NONE)
							{
								Color = GetColor(HoveredStencil);
							}
							const FColor SRGBColor = Color.ToFColorSRGB();
							const FString HexCode = FString::Printf(TEXT("#%02X%02X%02X"), SRGBColor.R, SRGBColor.G, SRGBColor.B);
							return FText::FromString(HexCode);
						})
					]
				]
			]
		]
	];
}

TSharedRef<SWidget> FCustomColorPicker::CreatePickupColor()
{
	constexpr int32 NumColumns = 8;
	const int32 NumStencils = Handle->NumStencil;
	constexpr float SlotSize = 20.f;
	constexpr float Padding = 2.f;

	const TSharedRef<SGridPanel> Grid = SNew(SGridPanel);
	
	for (int32 i = 0; i < NumStencils; ++i)
	{
		const int32 FirstStencil = Handle->FirstStencil;
		const int32 Stencil = FirstStencil + i;
		const int32 Row = i / NumColumns;
		const int32 Col = i % NumColumns;

		const FLinearColor Color = GetColor(Stencil);
		
		Grid->AddSlot(Col, Row)
		.Padding(Padding)
		[
			SNew(SBox)
			.WidthOverride(SlotSize)
			.HeightOverride(SlotSize)
			[
				SNew(SBorder)
				.BorderImage(PaintBrush.IsValid() ? PaintBrush.Get() : FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor_Lambda([this, Stencil] () -> FLinearColor
				{
					return GetStencilValue() == Stencil ? FLinearColor::White : FLinearColor::Transparent;
				})
				.Padding(1.f)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SImage)
						.Image(PaintBrush.IsValid() ? PaintBrush.Get() : FCoreStyle::Get().GetBrush("WhiteBrush"))
						.ColorAndOpacity(Color)
					]
					+ SOverlay::Slot()
					[
						SNew(SButton)
						.ButtonColorAndOpacity(FLinearColor::Transparent)
						.ButtonStyle(FCoreStyle::Get(), "NoBorder")
						.Cursor(EMouseCursor::EyeDropper)
						.OnClicked_Lambda([Stencil, this]() -> FReply
						{
							UpdateColor(Stencil);
							return FReply::Handled();
						})
						.OnHovered_Lambda([Stencil, this]()
						{
							HoveredStencil = Stencil;
						})
						.OnUnhovered_Lambda([this]()
						{
							HoveredStencil = INDEX_NONE;
						})
						.ContentPadding(0)
						.ToolTipText(FText::FromString(FString::Printf(TEXT("Color %d"), Stencil)))
					]
				]
			]
		];
	}

	constexpr float TotalWidth = NumColumns * (SlotSize + 2 * Padding);
	const int32 NumRows = FMath::CeilToInt(static_cast<float>(NumStencils) / NumColumns);
	const float TotalHeight = NumRows * (SlotSize + 2 * Padding);

	return SNew(SBox)
		.WidthOverride(TotalWidth)
		.HeightOverride(TotalHeight)
		[
			Grid
		];
}

int32 FCustomColorPicker::GetStencilValue() const
{
	if (!Handle) return INDEX_NONE;
	return Handle->Stencil;
}

FLinearColor FCustomColorPicker::GetColor(const int Stencil) const
{
	return Handle ? Handle->GetColor(Stencil) : FLinearColor::Transparent;
}

void FCustomColorPicker::UpdateColor(const int Stencil) const
{
	if (!Handle) return;
	if (Handle->Stencil == Stencil) return;
	Handle->UpdateIndex(Stencil);
}

void FCustomColorPicker::RefreshColor()
{
	ColorPanel->ClearChildren();
	ColorPanel->AddSlot().AutoHeight()
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		[
			SNew(SButton)
			.ButtonColorAndOpacity(FLinearColor::Transparent)
			.ButtonStyle(FCoreStyle::Get(), "NoBorder")
			.Cursor(EMouseCursor::EyeDropper)
		]
		+ SOverlay::Slot()
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		[
			CreatePickupColor()
		]
	];
	DetailMessage->NewMessage("Colors refreshed!", EMessageType::Success, false, 1, 10);
}

#undef LOCTEXT_NAMESPACE