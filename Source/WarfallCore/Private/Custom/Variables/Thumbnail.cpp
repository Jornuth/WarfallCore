#include "Custom/Variables/Thumbnail.h"



#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "EngineUtils.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyUtilities.h"
#include "PropertyCustomizationHelpers.h"
#include "Custom/Widgets/DetailMessageRow.h"
#include "Custom/Blutility/ThumbnailMaker.h"
#include "Styling/SlateIconFinder.h"
#include "Widgets/Input/SSpinBox.h"

#define LOCTEXT_NAMESPACE "ThumbnailHandleCustomization"

void FThumbnail::UpdateCustomizer()
{
	if (!ThumbnailMaker || !ThumbnailPilote.IsValid()) return;
	ThumbnailMaker->UpdateThumbnailMaker(this, ThumbnailPilote.Get());
	ThumbnailPilote->UpdateThumbnailMaker(ThumbnailMaker, this);
}

TSharedRef<IPropertyTypeCustomization> FCustomThumbnail::MakeInstance()
{
	return MakeShareable(new FCustomThumbnail);
}

void FCustomThumbnail::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle,
	FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	StructHandle = StructPropertyHandle;
	PropertyUtils = StructCustomizationUtils.GetPropertyUtilities();
	ThumbnailTemp = FThumbnail();
	
	ThumbnailHandle = GetStructPropertyPtr<FThumbnail>(StructPropertyHandle);
	ThumbnailTemp = *ThumbnailHandle;
	ThumbnailTemp.RegisterThumbnail(ThumbnailHandle);
	ThumbnailHandle->Temp = &ThumbnailTemp;
	
	bIsCreatorVisible = ThumbnailHandle->GetWasRefresh();
	if (bIsCreatorVisible)
	{
		ThumbnailPilote = SNew(SThumbnailPilote).IsFocusable(true);
		ThumbnailTemp.ThumbnailPilote = ThumbnailPilote.ToSharedRef();
		AssignProperties();
		ThumbnailHandle->NotifyOpen();
	}
	ThumbnailHandle->OverrideRefresh(false);
	
	HeaderRow
	.NameContent()
	[
		StructPropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(250.f)
	.MaxDesiredWidth(600.f)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text_Lambda([this]() { return bIsCreatorVisible ? LOCTEXT("ReduceThumbnail", "Reduce") : LOCTEXT("CreateThumbnail", "Create"); })
			.OnClicked_Lambda([this]() {
				TogglePreviewVisibility();
				if (!bIsCreatorVisible)
				{
					ClearAndInvalidateThumbnail();
				}
				ThumbnailHandle->OverrideRefresh(bIsCreatorVisible);
				PropertyUtils->ForceRefresh();
				return FReply::Handled();
			})
			.ToolTipText_Lambda([this]() -> FText
			{
				return bIsCreatorVisible
					? LOCTEXT("Tooltip_CollapseThumbnail", "Collapse thumbnail properties panel.")
					: LOCTEXT("Tooltip_CreateThumbnail", "Create a new thumbnail by configuring properties manually.");
			})
		]
	];
}

void FCustomThumbnail::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle,
	IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	// Show the Texture2D field Thumbnail first
	uint32 NumChildren;
	StructPropertyHandle->GetNumChildren(NumChildren);
	TSharedPtr<IPropertyHandle> MeshHandle;
	TSharedPtr<IPropertyHandle> SkeletalMeshHandle;
		
	for (uint32 i = 0; i < NumChildren; ++i)
	{
		TSharedPtr<IPropertyHandle> ChildHandle = StructPropertyHandle->GetChildHandle(i);
		if (!ChildHandle.IsValid()) continue;

		if (const FName PropertyName = ChildHandle->GetProperty()->GetFName(); PropertyName == GET_MEMBER_NAME_CHECKED(FThumbnail, Thumbnail))
		{
			ChildBuilder.AddProperty(ChildHandle.ToSharedRef());
		}
		else if (PropertyName == GET_MEMBER_NAME_CHECKED(FThumbnail, Layer))
		{
			ChildBuilder.AddProperty(ChildHandle.ToSharedRef());
		}
	}
	
	ChildBuilder.AddCustomRow(FText::FromString("ReverseRotationCustomization"))
	.NameContent()
	[
		SNew(STextBlock)
		.Text(FText::FromString("Reverse rotation"))
		.Font(IDetailLayoutBuilder::GetDetailFont())
	]
	.ValueContent()
	[
		SNew(SCheckBox)
		.IsChecked_Lambda([this]() -> ECheckBoxState
		{
			return ThumbnailHandle->GetReverseRotation() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
		})
		.OnCheckStateChanged_Lambda([this](const ECheckBoxState NewState)
		{
			ThumbnailHandle->GetReverseRotation() = (NewState == ECheckBoxState::Checked);
		})
	];
		
	DetailMessage = FDetailMessageRow::Create(ChildBuilder, bIsCreatorVisible ? EVisibility::Visible : EVisibility::Collapsed);
	
	ChildBuilder.AddCustomRow(FText::FromString("ThumbnailCustomization"))
	.Visibility(bIsCreatorVisible ? EVisibility::Visible : EVisibility::Collapsed)
	.NameContent()
	[
		SNew(SBox)
		.MaxDesiredWidth(270.f)
		.Padding(20.f, 20.f, 0.f, 20.f)
		[
			NameContentWidget()
		]
	]
	.ValueContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.AutoWidth()
		.Padding(20.f, 0.f, 20.f, 20.f)
		[
			ValueContentWidget()
		]
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			SNew(SBox)
			.MaxDesiredWidth(220.f)
			.Visibility_Lambda([this]() { return bIsControlsVisible ? EVisibility::Visible : EVisibility::Hidden; })
			[
				InputsControlsWidget()
			]
		]
	];
}

TSharedRef<SWidget> FCustomThumbnail::NameContentWidget()
{
    auto UpdateProperties = [this]() -> void
    {
        if (ThumbnailMaker && ThumbnailHandle)
        {
            AssignProperties();
        }
    };

    auto UpdateTransform = [this]() -> void
    {
        if (ThumbnailMaker)
        {
            ThumbnailMaker->UpdateTransform();
        }
    };

    return SNew(SVerticalBox)

    // Thumbnail Properties Section
    + SVerticalBox::Slot().HAlign(HAlign_Left)
    .AutoHeight()
    .Padding(0.f, 0.f, 0.f, 10.f)
    [
        SNew(STextBlock)
        .Text(FText::FromString("Thumbnail Properties"))
        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
    ]

    + SVerticalBox::Slot().HAlign(HAlign_Left)
    .AutoHeight()
    [
        SNew(STextBlock)
        .Text(FText::FromString("Name"))
    	.Font(IDetailLayoutBuilder::GetDetailFont())
    	.ToolTipText(FText::FromString("The name assigned to the generated thumbnail asset."))
    ]

    + SVerticalBox::Slot().HAlign(HAlign_Left)
    .AutoHeight()
    [
        SNew(SEditableTextBox)
        .MinDesiredWidth(100.f)
        .Text_Lambda([this]() { return FText::FromString(ThumbnailTemp.GetName()) ; })
	    .OnTextCommitted_Lambda([this] (const FText& NewName, ETextCommit::Type CommitType) mutable { ThumbnailTemp.GetName() = NewName.ToString(); ThumbnailHandle->NotifyChanged("Name");  })
    	.ToolTipText(FText::FromString("The name assigned to the generated thumbnail asset."))
    ]
    	
    // Padding après le Widget
	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0, 0, 0, 3) // PADDING DE 3 ICI
	[
		SNullWidget::NullWidget
	]

    + SVerticalBox::Slot().HAlign(HAlign_Left)
    .AutoHeight()
    [
        SNew(STextBlock)
        .Text(FText::FromString("Dimensions"))
    	.Font(IDetailLayoutBuilder::GetDetailFont())
    	.ToolTipText(FText::FromString("The output dimensions (width and height) of the thumbnail."))
    ]

    + SVerticalBox::Slot().HAlign(HAlign_Left)
    .AutoHeight()
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .AutoWidth()
        [
            SNew(SSpinBox<int32>)
            .MinDesiredWidth(20.f)
        	.MinValue(1)
			.MaxValue(4)
			.Delta(1)
			.SupportDynamicSliderMaxValue(false)
			.SupportDynamicSliderMinValue(false)
			.Justification(ETextJustify::Center)
            .Value_Lambda([this]() { return ThumbnailTemp.GetDimensions().X; })
            .OnValueChanged_Lambda([this, UpdateProperties](const int32 NewValue) { ThumbnailTemp.GetDimensions().X = NewValue; UpdateProperties(); })
        	.ToolTipText(FText::FromString("Width of the thumbnail in pixels."))
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
        [
            SNew(SSpinBox<int32>)
            .MinDesiredWidth(20.f)
        	.MinValue(1)
			.MaxValue(4)
			.Delta(1)
			.SupportDynamicSliderMaxValue(false)
	        .SupportDynamicSliderMinValue(false)
	        .Justification(ETextJustify::Center)
            .Value_Lambda([this]() { return ThumbnailTemp.GetDimensions().Y; })
            .OnValueChanged_Lambda([this, UpdateProperties](const int32 NewValue) { ThumbnailTemp.GetDimensions().Y = NewValue; UpdateProperties(); })
        	.ToolTipText(FText::FromString("Height of the thumbnail in pixels."))
        ]
    ]

    // Padding après le Widget
	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0, 0, 0, 3) // PADDING DE 3 ICI
	[
		SNullWidget::NullWidget
	]	

    + SVerticalBox::Slot().HAlign(HAlign_Left)
    .AutoHeight()
    [
        SNew(STextBlock)
        .Text(FText::FromString("Mesh"))
    	.Font(IDetailLayoutBuilder::GetDetailFont())
    	.ToolTipText(FText::FromString("Static mesh to render for thumbnail generation."))
    ]

    + SVerticalBox::Slot().HAlign(HAlign_Left)
    .AutoHeight()
    [
        SNew(SObjectPropertyEntryBox)
        .AllowedClass(UStaticMesh::StaticClass())
        .ObjectPath_Lambda([this]() { return ThumbnailTemp.GetMesh()->GetPathName(); })
        .OnObjectChanged_Lambda([this, UpdateProperties](const FAssetData& AssetData)
        {
        	if (ThumbnailTemp.GetSkeletalMesh()) ThumbnailTemp.GetSkeletalMesh() = nullptr;
	        ThumbnailTemp.GetMesh() = Cast<UStaticMesh>(AssetData.GetAsset());
        	UpdateProperties();
        	ThumbnailHandle->NotifyChanged("Mesh");
        })
    	.ToolTipText(FText::FromString("Static mesh to render for thumbnail generation."))
    ]
    	
    // Padding après le Widget
	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0, 0, 0, 3) // PADDING DE 3 ICI
	[
		SNullWidget::NullWidget
	]	
    	
    + SVerticalBox::Slot().HAlign(HAlign_Left)
    .AutoHeight()
    [
        SNew(STextBlock)
        .Text(FText::FromString("Skeletal Mesh"))
    	.Font(IDetailLayoutBuilder::GetDetailFont())
    	.ToolTipText(FText::FromString("Skeletal mesh to render for thumbnail generation."))
    ]

    + SVerticalBox::Slot().HAlign(HAlign_Left)
    .AutoHeight()
    [
        SNew(SObjectPropertyEntryBox)
        .AllowedClass(USkeletalMesh::StaticClass())
        .ObjectPath_Lambda([this]() { return ThumbnailTemp.GetSkeletalMesh()->GetPathName(); })
        .OnObjectChanged_Lambda([this, UpdateProperties](const FAssetData& AssetData)
        {
        	if (ThumbnailTemp.GetMesh()) ThumbnailTemp.GetMesh() = nullptr;
	        ThumbnailTemp.GetSkeletalMesh() = Cast<USkeletalMesh>(AssetData.GetAsset());
        	UpdateProperties();
        	ThumbnailHandle->NotifyChanged("SkeletalMesh");
        })
    	.ToolTipText(FText::FromString("Skeletal mesh to render for thumbnail generation."))
    ]
    	
    // Transform Settings Section
    + SVerticalBox::Slot().HAlign(HAlign_Left)
    .AutoHeight()
    .Padding(0.f, 20.f, 0.f,  10.f)
    [
        SNew(STextBlock)
        .Text(FText::FromString("Transform Settings"))
        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
    ]

    // Location
    + SVerticalBox::Slot().HAlign(HAlign_Left)
    .AutoHeight()
    [
        SNew(STextBlock)
        .Text(FText::FromString("Location"))
    	.Font(IDetailLayoutBuilder::GetDetailFont())
    	.ToolTipText(FText::FromString("Relative location offset applied to the rendered mesh."))
    ]

    + SVerticalBox::Slot().HAlign(HAlign_Left)
    .AutoHeight()
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .AutoWidth()
        [
            SNew(SSpinBox<float>)
            .MinDesiredWidth(52.f)
        	.MaxFractionalDigits(2)
            .Delta(1)
	        .MinValue(-9999.9)
	        .MaxValue(9999.9)
	        .Justification(ETextJustify::Center)
            .Value_Lambda([this]() { return ThumbnailTemp.GetLocation().Z; })
            .OnValueChanged_Lambda([this, UpdateTransform](const float NewValue) { ThumbnailTemp.GetLocation().Z = NewValue; UpdateTransform(); })
        	.ToolTipText(FText::FromString("Relative location X offset applied to the rendered mesh."))
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
        [
            SNew(SSpinBox<float>)
            .MinDesiredWidth(52.f)
        	.MaxFractionalDigits(2)
            .Delta(1)
        	.MinValue(-9999.9)
			.MaxValue(9999.9)
			.Justification(ETextJustify::Center)
            .Value_Lambda([this]() { return ThumbnailTemp.GetLocation().X; })
            .OnValueChanged_Lambda([this, UpdateTransform](const float NewValue) { ThumbnailTemp.GetLocation().X = NewValue; UpdateTransform(); })
        	.ToolTipText(FText::FromString("Relative location Y offset applied to the rendered mesh."))
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
        [
            SNew(SSpinBox<float>)
            .MinDesiredWidth(52.f)
        	.MaxFractionalDigits(2)
            .Delta(1)
        	.MinValue(0)
			.MaxValue(9999.9)
			.Justification(ETextJustify::Center)
            .Value_Lambda([this]() { return -ThumbnailTemp.GetLocation().Y; })
            .OnValueChanged_Lambda([this, UpdateTransform](const float NewValue) { ThumbnailTemp.GetLocation().Y = -NewValue; UpdateTransform(); })
        	.ToolTipText(FText::FromString("Relative zoom offset applied to the rendered mesh."))
        ]
    ]

    // Padding après le Widget
	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0, 0, 0, 3) // PADDING DE 3 ICI
	[
		SNullWidget::NullWidget
	]		

    // Rotation
    + SVerticalBox::Slot().HAlign(HAlign_Left)
    .AutoHeight()
    [
        SNew(STextBlock)
        .Text(FText::FromString("Rotation"))
    	.Font(IDetailLayoutBuilder::GetDetailFont())
    	.ToolTipText(FText::FromString("Relative rotation applied to the rendered mesh."))
    ]

    + SVerticalBox::Slot().HAlign(HAlign_Left)
    .AutoHeight()
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .AutoWidth()
        [
            SNew(SSpinBox<float>)
            .MinDesiredWidth(52.f)
        	.MaxFractionalDigits(2)
            .Delta(1)
        	.MinValue(-9999.9)
			.MaxValue(9999.9)
			.Justification(ETextJustify::Center)
            .Value_Lambda([this]() { return ThumbnailTemp.GetRotationEuler().Pitch; })
            .OnValueChanged_Lambda([this, UpdateTransform](const float NewValue) { ThumbnailTemp.GetRotationEuler().Pitch = NewValue; UpdateTransform(); })
        	.ToolTipText(FText::FromString("Relative rotation pitch applied to the rendered mesh."))
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
        [
            SNew(SSpinBox<float>)
            .MinDesiredWidth(52.f)
        	.MaxFractionalDigits(2)
            .Delta(1)
        	.MinValue(-9999.9)
			.MaxValue(9999.9)
			.Justification(ETextJustify::Center)
            .Value_Lambda([this]() { return ThumbnailTemp.GetRotationEuler().Roll; })
            .OnValueChanged_Lambda([this, UpdateTransform](const float NewValue) { ThumbnailTemp.GetRotationEuler().Roll = NewValue; UpdateTransform(); })
        	.ToolTipText(FText::FromString("Relative rotation roll applied to the rendered mesh."))
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
        [
            SNew(SSpinBox<float>)
            .MinDesiredWidth(52.f)
        	.MaxFractionalDigits(2)
            .Delta(1)
        	.MinValue(-9999.9)
			.MaxValue(9999.9)
			.Justification(ETextJustify::Center)
            .Value_Lambda([this]() { return ThumbnailTemp.GetRotationEuler().Yaw; })
            .OnValueChanged_Lambda([this, UpdateTransform](const float NewValue) { ThumbnailTemp.GetRotationEuler().Yaw = NewValue; UpdateTransform(); })
        	.ToolTipText(FText::FromString("Relative rotation yaw applied to the rendered mesh."))
        ]
    ]

    // Padding après le Widget
	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0, 0, 0, 3) // PADDING DE 3 ICI
	[
		SNullWidget::NullWidget
	]		

    // Scale + Lock
    + SVerticalBox::Slot().HAlign(HAlign_Left)
    .AutoHeight()
    [
        SNew(STextBlock)
        .Text(FText::FromString("Scale"))
    	.Font(IDetailLayoutBuilder::GetDetailFont())
    	.ToolTipText(FText::FromString("Relative scale applied to the rendered mesh."))
    ]

    + SVerticalBox::Slot().HAlign(HAlign_Left)
    .AutoHeight()
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .AutoWidth()
        [
            SNew(SSpinBox<float>)
            .MinDesiredWidth(52.f)
        	.MaxFractionalDigits(2)
            .Delta(0.01)
        	.MinValue(0.01)
			.MaxValue(9999.9)
			.Justification(ETextJustify::Center)
            .Value_Lambda([this]() { return ThumbnailTemp.GetScale().X; })
	        .OnValueChanged_Lambda([this] (const float NewValue)
	        {
	        	if (ThumbnailHandle->GetScaleLocked())
	        	{
					const float SafeOldValue = FMath::Max(ThumbnailTemp.GetScale().X, 0.0001f);
					const float Ratio = (NewValue != 0.f) ? (NewValue / SafeOldValue) : 1.f;
					ThumbnailTemp.GetScale().X = FMath::Clamp(NewValue, 0.01f, 9999.9f);
					ThumbnailTemp.GetScale().Y = FMath::Clamp(ThumbnailTemp.GetScale().Y * Ratio, 0.01f, 9999.9f);
					ThumbnailTemp.GetScale().Z = FMath::Clamp(ThumbnailTemp.GetScale().Z * Ratio, 0.01f, 9999.9f);

				}
				else
				{
					ThumbnailTemp.GetScale().X = NewValue;
				}
	        })
        	.OnValueCommitted_Lambda([this, UpdateTransform](const float NewValue, ETextCommit::Type CommitType ) mutable
            {
            	UpdateTransform();
            })
        	.ToolTipText(FText::FromString("Relative scale X applied to the rendered mesh."))
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
        [
            SNew(SSpinBox<float>)
            .MinDesiredWidth(52.f)
        	.MaxFractionalDigits(2)
        	.Delta(0.01)
        	.MinValue(0.01)
			.MaxValue(9999.9)
			.Justification(ETextJustify::Center)
            .Value_Lambda([this]() { return ThumbnailTemp.GetScale().Y; })
	        .OnValueChanged_Lambda([this] (const float NewValue)
	        {
	        	if (ThumbnailHandle->GetScaleLocked())
	        	{
					const float SafeOldValue = FMath::Max(ThumbnailTemp.GetScale().Y, 0.0001f);
					const float Ratio = (NewValue != 0.f) ? (NewValue / SafeOldValue) : 1.f;
					ThumbnailTemp.GetScale().X = FMath::Clamp(ThumbnailTemp.GetScale().X * Ratio, 0.01f, 9999.9f);
					ThumbnailTemp.GetScale().Y = FMath::Clamp(NewValue, 0.01f, 9999.9f);
					ThumbnailTemp.GetScale().Z = FMath::Clamp(ThumbnailTemp.GetScale().Z * Ratio, 0.01f, 9999.9f);
				}
				else
				{
					ThumbnailTemp.GetScale().Y = NewValue;
				}
	        })
        	.OnValueCommitted_Lambda([this, UpdateTransform](const float NewValue, ETextCommit::Type CommitType ) mutable
            {
				UpdateTransform();
            })
        	.ToolTipText(FText::FromString("Relative scale Y applied to the rendered mesh."))
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
        [
            SNew(SSpinBox<float>)
            .MinDesiredWidth(52.f)
        	.MaxFractionalDigits(2)
        	.Delta(0.01)
        	.MinValue(0.01)
			.MaxValue(9999.9)
			.Justification(ETextJustify::Center)
            .Value_Lambda([this]() { return ThumbnailTemp.GetScale().Z; })
	        .OnValueChanged_Lambda([this] (const float NewValue)
	        {
	        	if (ThumbnailHandle->GetScaleLocked())
	        	{
					const float SafeOldValue = FMath::Max(ThumbnailTemp.GetScale().Z, 0.0001f);
					const float Ratio = (NewValue != 0.f) ? (NewValue / SafeOldValue) : 1.f;
					ThumbnailTemp.GetScale().X = FMath::Clamp(ThumbnailTemp.GetScale().X * Ratio, 0.01f, 9999.9f);
					ThumbnailTemp.GetScale().Y = FMath::Clamp(ThumbnailTemp.GetScale().Y * Ratio, 0.01f, 9999.9f);
					ThumbnailTemp.GetScale().Z = FMath::Clamp(NewValue, 0.01f, 9999.9f);
				}
				else
				{
					ThumbnailTemp.GetScale().Z = NewValue;
				}
	        })
            .OnValueCommitted_Lambda([this, UpdateTransform](const float NewValue, ETextCommit::Type CommitType ) mutable
            {
				UpdateTransform();
            })
        	.ToolTipText(FText::FromString("Relative scale Z applied to the rendered mesh."))
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
        [
            SNew(SCheckBox)
        	.IsChecked_Lambda([this]() -> ECheckBoxState
			{
				return ThumbnailHandle->GetScaleLocked() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			})
            .OnCheckStateChanged_Lambda([this](const ECheckBoxState NewState)
            {
                ThumbnailHandle->GetScaleLocked() = (NewState == ECheckBoxState::Checked);
            	DetailMessage->NewMessage(NewState == ECheckBoxState::Checked ? "Scale locked" : "Scale unlocked", EMessageType::Warning, false, 1, 0);
            })
            .Style(FAppStyle::Get(), "ToggleButtonCheckbox")
            .Content()
            [
            	SNew(SImage)
                .Image_Lambda([this]() -> const FSlateBrush*
                {
                    return ThumbnailHandle->GetScaleLocked() ? FSlateIconFinder::FindIcon("RevisionControl.Locked").GetIcon() : FSlateIconFinder::FindIcon("RevisionControl.Unlocked").GetIcon();
                })
            ]
        	.ToolTipText(FText::FromString("Lock or unlock scaling to preserve aspect ratio."))
        ]
    ]

    // Finalize Button
    + SVerticalBox::Slot()
    .AutoHeight()
    .Padding(0.f, 20.f)
    .HAlign(HAlign_Left)
    [
        SNew(SButton)
        .Text(FText::FromString("Finalize & Save Thumbnail"))
        .OnClicked(this, &FCustomThumbnail::OnFinalizeClicked)
    	.ToolTipText(FText::FromString("Finalize the setup and save the generated thumbnail to disk."))
	    .IsEnabled_Lambda([this]
	    {
		    return IsValidEntry();
	    })
    ];
}

TSharedRef<SWidget> FCustomThumbnail::ValueContentWidget()
{
    return SNew(SVerticalBox)

    + SVerticalBox::Slot()
    .AutoHeight()
    [
        SNew(SOverlay)
        + SOverlay::Slot()
        [
            SNew(SBox)
            .WidthOverride(384.f)
            .HeightOverride(384.f)
            [
                ThumbnailPilote.IsValid() ? ThumbnailPilote.ToSharedRef() : SNullWidget::NullWidget
            ]
        ]
        + SOverlay::Slot()
        .HAlign(HAlign_Right)
        .VAlign(VAlign_Top)
        .Padding(10.f,10.f)
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
            .ToolTipText(FText::FromString("Refresh the current thumbnail preview."))
            .OnClicked_Lambda([this]() -> FReply
            {
                if (ThumbnailMaker)
                {
                    Refresh();
                }
                return FReply::Handled();
            })
            [
                SNew(SImage)
                .Image(FAppStyle::GetBrush("SourceControl.Actions.Refresh"))
            ]
        ]
	    + SOverlay::Slot()
	    .HAlign(HAlign_Center)
	    .VAlign(VAlign_Bottom)
	    .Padding(0.f, 10)
	    [
		    SNew(STextBlock)
		    .Text_Lambda([this]
		    {
		    	const float X = ThumbnailTemp.GetDimensions().X * 256;
		    	const float Y = ThumbnailTemp.GetDimensions().Y * 256;

		    	const FString Value = FString::Printf(TEXT("%dx%d"), FMath::RoundToInt(X), FMath::RoundToInt(Y));
			    return FText::FromString(Value);
		    })
	    ]
	    + SOverlay::Slot()
	    .HAlign(HAlign_Right)
	    .VAlign(VAlign_Bottom)
	    .Padding(10)
	    [
		    SNew(SHorizontalBox)
		    + SHorizontalBox::Slot().AutoWidth()
		    .VAlign(VAlign_Center)
		    [
			    SNew(STextBlock)
			    .Text(FText::FromString("Controls: "))
		    ]
		    + SHorizontalBox::Slot().AutoWidth()
	    	.VAlign(VAlign_Center)
		    .Padding(4.f, 0.f)
		    [
			    SNew(SCheckBox)
			    .IsChecked(ECheckBoxState::Unchecked)
		    	.OnCheckStateChanged_Lambda([this](const ECheckBoxState NewState)
				{
					bIsControlsVisible = NewState == ECheckBoxState::Checked;
				})
		    	.Type(ESlateCheckBoxType::Type::ToggleButton)
			    .Cursor(EMouseCursor::Hand)
		    	.IsFocusable(false)
		    	.ToolTipText(FText::FromString("Show or hide the controls panel."))
				.Content()
				[
					SNew(SBox)
					.WidthOverride(16)
					.HeightOverride(16)
					[
						SNew(SImage)
						.Image_Lambda([this]() -> const FSlateBrush*
						{
							return bIsControlsVisible ? FSlateIconFinder::FindIcon("Level.VisibleIcon16x").GetIcon() : FSlateIconFinder::FindIcon("Level.NotVisibleIcon16x").GetIcon();
						})
					]
				]
		    ]
	    ]
    ];
}

TSharedRef<SWidget> FCustomThumbnail::InputsControlsWidget()
{
	const FString MouseDir = FPaths::ProjectPluginsDir() / TEXT("GameCore/Resources/Inputs/Mouse/");
	const FString InputsDir = FPaths::ProjectPluginsDir() / TEXT("GameCore/Resources/Inputs/");
	constexpr float TargetHeight = 24.f;

	LeftMouseBrush = MakeShareable(new FSlateDynamicImageBrush(
		FName(*(MouseDir / FString(TEXT("T_leftclick_128px_gray.PNG")))),
		FVector2D(TargetHeight, TargetHeight) 
	));

	RightMouseBrush = MakeShareable(new FSlateDynamicImageBrush(
		FName(*(MouseDir / FString(TEXT("T_rightclick_128px_gray.PNG")))),
		FVector2D(TargetHeight, TargetHeight) 
	));

	WheelMouseBrush = MakeShareable(new FSlateDynamicImageBrush(
		FName(*(MouseDir / FString(TEXT("T_middle_128px_gray.PNG")))),
		FVector2D(TargetHeight, TargetHeight) 
	));

	MoveAzertyBrush = MakeShareable(new FSlateDynamicImageBrush(
		FName(*(InputsDir / FString(TEXT("T_Move_Azerty.png")))),
		FVector2D((TargetHeight * 188/130) * 2, TargetHeight * 2) 
	));

	MoveQwertyBrush = MakeShareable(new FSlateDynamicImageBrush(
		FName(*(InputsDir / FString(TEXT("T_Move_Qwerty.png")))),
		FVector2D((TargetHeight * 188/130) * 2, TargetHeight * 2) 
	));
	
	if (!LeftMouseBrush.IsValid() || !RightMouseBrush.IsValid()
		|| !WheelMouseBrush.IsValid() || !MoveAzertyBrush || !MoveQwertyBrush) return SNullWidget::NullWidget;
	
    return SNew(SVerticalBox)

    // Title "Control Inputs"
    + SVerticalBox::Slot()
    .AutoHeight()
    .Padding(0, 0, 0, 10)
    [
        SNew(STextBlock)
        .Text(NSLOCTEXT("CoreX", "ControlsTitle", "Control Inputs"))
        .Font(FCoreStyle::GetDefaultFontStyle("BoldItalic", 16))
        .ColorAndOpacity(FLinearColor::White)
    ]

    // Movement
    + SVerticalBox::Slot()
    .AutoHeight()
    .Padding(0, 2)
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .AutoWidth()
    	.VAlign(VAlign_Center)
	    .Padding(0, 0, 4, 0)
        [
            SNew(STextBlock)
            .Text(NSLOCTEXT("CoreX", "MoveInput", "Moves "))
        	.Font(FCoreStyle::GetDefaultFontStyle("Italic", 10))
            .ColorAndOpacity(FLinearColor::White)
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
    	.VAlign(VAlign_Center)
        [
            SNew(SImage)
            .Image(LeftMouseBrush.Get())
        ]
    	+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(5, 0)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("CoreX", "OrSign", " || "))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
			.ColorAndOpacity(FLinearColor::White)
		]
    	+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SImage)
			.Image_Lambda([this]() -> const FSlateBrush*
			{
				return UGlobalTools::GetKeyboardType() == "Azerty" ? MoveAzertyBrush.Get() : MoveQwertyBrush.Get();
			})
		]
    ]

    // Pitch
    + SVerticalBox::Slot()
    .AutoHeight()
    .Padding(0, 2)
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .AutoWidth()
	    .VAlign(VAlign_Center)
    	.Padding(0, 0, 4, 0)
        [
            SNew(STextBlock)
            .Text(NSLOCTEXT("CoreX", "PitchInput", "Pitch "))
        	.Font(FCoreStyle::GetDefaultFontStyle("Italic", 10))
            .ColorAndOpacity(FLinearColor::White)
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
    	.VAlign(VAlign_Center)
        [
            SNew(SImage)
            .Image(RightMouseBrush.Get())
        ]
    ]

    // Roll (Shift + RightMouse)
    + SVerticalBox::Slot()
    .AutoHeight()
    .Padding(0, 2)
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .AutoWidth()
    	.VAlign(VAlign_Center)
    	.Padding(0, 0, 4, 0)
        [
            SNew(STextBlock)
            .Text(NSLOCTEXT("CoreX", "RollInput", "Roll "))
        	.Font(FCoreStyle::GetDefaultFontStyle("Italic", 10))
            .ColorAndOpacity(FLinearColor::White)
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
	    .Padding(2, 0)
        [
            SNew(STextBlock)
            .Text(NSLOCTEXT("CoreX", "PlusSign", " Shift  + "))
        	.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
            .ColorAndOpacity(FLinearColor::White)
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
    	.VAlign(VAlign_Center)
        [
            SNew(SImage)
            .Image(RightMouseBrush.Get())
        ]
    ]

    // Yaw (Ctrl + RightMouse)
    + SVerticalBox::Slot()
    .AutoHeight()
    .Padding(0, 2)
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .AutoWidth()
    	.VAlign(VAlign_Center)
    	.Padding(0, 0, 4, 0)
        [
            SNew(STextBlock)
            .Text(NSLOCTEXT("CoreX", "YawInput", "Yaw "))
        	.Font(FCoreStyle::GetDefaultFontStyle("Italic", 10))
            .ColorAndOpacity(FLinearColor::White)
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
	    .Padding(2, 0)
        [
            SNew(STextBlock)
            .Text(NSLOCTEXT("CoreX", "PlusSign2", " Control  + "))
        	.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
            .ColorAndOpacity(FLinearColor::White)
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
    	.VAlign(VAlign_Center)
        [
            SNew(SImage)
            .Image(RightMouseBrush.Get())
        ]
    ]

    // Zoom
    + SVerticalBox::Slot()
    .AutoHeight()
    .Padding(0, 2)
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .AutoWidth()
    	.VAlign(VAlign_Center)
    	.Padding(0, 0, 4, 0)
        [
            SNew(STextBlock)
            .Text(NSLOCTEXT("CoreX", "ZoomInput", "Zoom "))
        	.Font(FCoreStyle::GetDefaultFontStyle("Italic", 10))
            .ColorAndOpacity(FLinearColor::White)
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
    	.VAlign(VAlign_Center)
        [
            SNew(SImage)
            .Image(WheelMouseBrush.Get())
        ]
    ]

    // Scale (Shift + Wheel)
    + SVerticalBox::Slot()
    .AutoHeight()
    .Padding(0, 2)
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .AutoWidth()
    	.VAlign(VAlign_Center)
    	.Padding(0, 0, 4, 0)
        [
            SNew(STextBlock)
            .Text(NSLOCTEXT("CoreX", "ScaleInput", "Scale "))
        	.Font(FCoreStyle::GetDefaultFontStyle("Italic", 10))
            .ColorAndOpacity(FLinearColor::White)
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
	    .Padding(2, 0)
        [
            SNew(STextBlock)
            .Text(NSLOCTEXT("CoreX", "PlusSign3", " Shift  + "))
        	.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
            .ColorAndOpacity(FLinearColor::White)
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
    	.VAlign(VAlign_Center)
        [
            SNew(SImage)
            .Image(WheelMouseBrush.Get())
        ]
    ];
}

FReply FCustomThumbnail::OnFinalizeClicked()
{
	if (ThumbnailMaker)
	{
		const FString AssetName = ThumbnailTemp.Thumbnail ? ThumbnailTemp.Thumbnail->GetName() : TEXT("Unknown");
		const FString Message = FString::Printf(TEXT("Thumbnail for '%s' has been successfully created and saved."), *AssetName);
		DetailMessage->NewMessage(Message, EMessageType::Success, false, 1.5, 9);
		ThumbnailMaker->SaveRenderTargetToDisk();
		ThumbnailHandle->RegisterThumbnail(&ThumbnailTemp);
		
	}
	return FReply::Handled();
}

void FCustomThumbnail::ClearAndInvalidateThumbnail()
{
	if (ThumbnailMaker && ThumbnailMaker->ThumbnailRessource == &ThumbnailTemp)
	{
		ThumbnailPilote->ClearAndInvalidate();
		ThumbnailMaker->ClearThumbnailMaker();
		ThumbnailMaker = nullptr;
	}
	ThumbnailPilote.Reset();
}

void FCustomThumbnail::Refresh()
{
	ClearAndInvalidateThumbnail();
	if (ThumbnailHandle) ThumbnailHandle->OverrideRefresh(bIsCreatorVisible);
	PropertyUtils->ForceRefresh();
}

void FCustomThumbnail::TogglePreviewVisibility()
{
	bIsCreatorVisible = !bIsCreatorVisible;
}

void FCustomThumbnail::AssignProperties()
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	for (const TActorIterator<AThumbnailMaker> It(World); It;)
	{
		ThumbnailMaker = *It;
		break;
	}
	if (!ThumbnailMaker && World)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		const FVector SpawnLocation(100000.f, 1000000.f, 100000.f);
		const FRotator SpawnRotation = FRotator::ZeroRotator;

		ThumbnailMaker = World->SpawnActor<AThumbnailMaker>(
			AThumbnailMaker::StaticClass(),
			SpawnLocation,
			SpawnRotation,
			SpawnParams
		);
	}
	if (ThumbnailMaker && ThumbnailHandle)
	{
		ThumbnailMaker->UpdateThumbnailMaker(&ThumbnailTemp, ThumbnailPilote.Get());
		ThumbnailPilote->UpdateThumbnailMaker(ThumbnailMaker, &ThumbnailTemp);
		ThumbnailTemp.ThumbnailMaker = ThumbnailMaker;
	}
}

bool FCustomThumbnail::IsValidEntry()
{
	if (!ThumbnailMaker || !ThumbnailHandle || !ThumbnailPilote)
	{
		return false;
	}
	
	FString Name = ThumbnailTemp.GetName().TrimStartAndEnd();
	FString ErrorMessage;
	if (Name.IsEmpty())
	{
		ErrorMessage = "The thumbnail name cannot be empty.";
		DetailMessage->NewMessage(ErrorMessage, EMessageType::Error, true, 0, 10);
		return false;
	}
	
	for (const TCHAR Char : Name)
	{
		if (!FChar::IsAlpha(Char))
		{
			ErrorMessage = "The thumbnail name must contain only letters (A–Z) with no spaces or special characters.";
			DetailMessage->NewMessage(ErrorMessage, EMessageType::Error, true, 0, 10);
			return false;
		}
	}
	
	if (!ThumbnailTemp.GetMesh() && !ThumbnailTemp.GetSkeletalMesh())
	{
		ErrorMessage = "A Static Mesh or Skeletal Mesh must be assigned.";
		DetailMessage->NewMessage(ErrorMessage, EMessageType::Error, true, 0, 10);
		return false;
	}

	if (DetailMessage->IsWaitingForChangement(10))
	{
		DetailMessage->ClearMessage();
	}
	
	return ThumbnailTemp.HasChangedComparedTo(*ThumbnailHandle);
}

#undef LOCTEXT_NAMESPACE
