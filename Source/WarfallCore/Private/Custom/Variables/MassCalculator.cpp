#include "Custom/Variables/MassCalculator.h"

#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyUtilities.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Input/SSpinBox.h"


// ===============================[ Property Customization Factory ]============================
// ======================================== FMASSRATIO =========================================
// =============================================================================================

#define LOCTEXT_NAMESPACE "CustomMassRatio"

void FCustomMassRatio::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	PropertyHandle = StructPropertyHandle;
	PropertyUtils = CustomizationUtils.GetPropertyUtilities();

	if (void* Dummy = nullptr; StructPropertyHandle->GetValueData(Dummy) == FPropertyAccess::Success)
	{
		TArray<void*> RawData;
		StructPropertyHandle->AccessRawData(RawData);
		if (RawData.IsEmpty()) return;
		
		Handle = static_cast<FMassRatio*>(RawData[0]);
		RefreshOptions();
	}
	HeaderRow
	.NameContent()
	[
		StructPropertyHandle->CreatePropertyNameWidget()
	];
}

void FCustomMassRatio::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle,
	IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	if (!Handle) return;
	TSharedPtr<FString> CurrentOption;

	if (const FName CurrentMaterial = Handle->GetMaterial(); !CurrentMaterial.IsNone())
	{
		const FString CurrentStr = CurrentMaterial.ToString();
		
		for (const TSharedPtr<FString>& Row : *MaterialsHandles)
		{
			if (Row.IsValid() && *Row == CurrentStr)
			{
				CurrentOption = Row;
				break;
			}
		}
	}

	ChildBuilder.AddCustomRow(FText::FromString("MassRatioCustomization"))
	.NameContent()
	[
		SNew(STextBlock)
		.Text(FText::FromString(StructPropertyHandle->GetPropertyDisplayName().ToString()))
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.Margin(FMargin(0.f, 0.f, 6.f, 0.f))
	]
	.ValueContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth()
		.Padding(0.f, 10.f, 0.f, 10.f)
		[
			SAssignNew(BoxContent, SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SBox)
				.MinDesiredWidth(150.f)
				.MinDesiredHeight(25.f)
				[
					SNew(SComboBox<TSharedPtr<FString>>)
					.OptionsSource(MaterialsHandles.Get())
					.InitiallySelectedItem(CurrentOption)
					.OnGenerateWidget_Lambda([this](const TSharedPtr<FString>& InItem) -> TSharedRef<SWidget>
					{
						if (!InItem.IsValid()) return SNew(STextBlock).Text(FText::FromString("Invalide"));
						if (const FName Material(**InItem); SolidDensityTable.Contains(Material))
						{
							return SNew(STextBlock).Text(FText::FromString(Material.ToString()));
						}
						return SNew(STextBlock).Text(FText::FromString(*InItem));
					})
					.OnSelectionChanged_Lambda([this](const TSharedPtr<FString>& NewSelection, ESelectInfo::Type)
					{
						if (!NewSelection.IsValid()) return;
						const FName SelectedName = (*NewSelection == *GetDefaultOption()) ? NAME_None : FName(*NewSelection);
						if (Handle && PropertyHandle.IsValid())
						{
							// Récupère le FMassObject parent
							TArray<void*> RawData;
							PropertyHandle->GetParentHandle()->GetParentHandle()->AccessRawData(RawData);
							if (RawData.IsEmpty()) return;

							FMassObject* MassObject = static_cast<FMassObject*>(RawData[0]);
							if (!MassObject) return;

							// Vérifie les doublons
							for (const FMassRatio& Ratio : MassObject->Materials)
							{
								if (Ratio.Material == SelectedName)
								{
									// Do nothing or optionally log / display
									return;
								}
							}
							Handle->GetMaterial() = SelectedName;
							BalancedAfterSelection();
						}
					})
					.Content()
					[
						SNew(STextBlock)
						.Font(IDetailLayoutBuilder::GetDetailFont())
						.Text_Lambda([this]()
						{
							if (Handle)
							{
								if (const FName Material = Handle->GetMaterial(); !Material.IsNone())
								{
									if (SolidDensityTable.Contains(Material)) return FText::FromString(Material.ToString());
								}
							}
							return FText::FromString(*GetDefaultOption());
						})
					]
				]
			]
			+ SVerticalBox::Slot().AutoHeight()
			.Padding(FMargin(0.f, 5.f, 0.f, 0.f))
			[
				SAssignNew(PercentageSlider, SSlider)
				.MinValue (0.f)
				.MaxValue (100.f)
				.MouseUsesStep(true)
				.Visibility_Lambda([this]() -> EVisibility
				{
					if (Handle)
					{
						if (const FName Material = Handle->GetMaterial(); !Material.IsNone())
						{
							if (SolidDensityTable.Contains(Material)) return EVisibility::Visible;
						}
					}
					return EVisibility::Collapsed;
				})
				.Value_Lambda([this]() -> float
				{
					return GetPercentage();
				})
				.OnValueChanged_Lambda([this](const float NewValue)
				{
					SetPercentage(NewValue);
				})
			]
			+ SVerticalBox::Slot().AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(FMargin(0.f, 5.f, 0.f, 0.f))
			[
				SNew(SSpinBox<float>)
				.MinDesiredWidth(52.f)
				.MaxFractionalDigits(2)
				.Justification(ETextJustify::Center)
				.Delta(1)
				.MinValue (0.f)
				.MaxValue (100.f)
				.Visibility_Lambda([this]() -> EVisibility
				{
					if (Handle)
					{
						if (const FName Material = Handle->GetMaterial(); !Material.IsNone())
						{
							if (SolidDensityTable.Contains(Material)) return EVisibility::Visible;
						}
					}
					return EVisibility::Collapsed;
				})
				.Value_Lambda([this]() -> float
				{
					return GetPercentage();
				})
				.OnValueChanged_Lambda([this](float NewValue)
				{
					SetPercentage(NewValue);
				})
			]
		]
		+ SHorizontalBox::Slot().AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(10.f, 0.f, 0.f, 0.f)
		[
			SNew(STextBlock)
			.Text_Lambda([this]
			{
				if (!Handle) return FText::FromString("g/cm³");
				const FString String = FString::Printf(TEXT("%.2f g/cm³"), Handle->GetDensity() / 1000.f);
				return FText::FromString(String);
			})
		]
		+ SHorizontalBox::Slot().AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(10.f, 0.f, 0.f, 0.f)
		[
			SNew(SCheckBox)
			.IsChecked_Lambda([this]() -> ECheckBoxState
			{
				if (!Handle) return ECheckBoxState::Unchecked;
				return Handle->GetIsSolid() ? ECheckBoxState::Unchecked : ECheckBoxState::Checked;
			})
			.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
			{
				if (!Handle) return;
				Handle->GetIsSolid() = NewState == ECheckBoxState::Unchecked;
			})
			.Style(FAppStyle::Get(), "ToggleButtonCheckbox")
			.Content()
			[
				SNew(STextBlock)
				.MinDesiredWidth(50.f)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Justification(ETextJustify::Center)
				.Text_Lambda([this]() -> FText
				{
					if (!Handle) return FText::FromString("Solid");
					return Handle->GetIsSolid() ? FText::FromString("Solid") : FText::FromString("Hollow");
				})
			]
		]
	];
}

void FCustomMassRatio::RefreshOptions()
{
	if (!Handle) return;

	if (!MaterialsHandles.IsValid())
	{
		MaterialsHandles = MakeShared<TArray<TSharedPtr<FString>>>();
	}
	MaterialsHandles->Empty();
	MaterialsHandles->Add(GetDefaultOption());

	for (const auto& Material : SolidDensityTable)
	{
		MaterialsHandles->Add(MakeShared<FString>(Material.Key.ToString()));
	}
}

float FCustomMassRatio::GetPercentage() const
{
	return Handle ? Handle->GetPercentage() : 0.f;
}

void FCustomMassRatio::SetPercentage(const float NewValue) const
{
	if (!Handle || !PropertyHandle.IsValid()) return;
	
	const float Clamped = FMath::Clamp(NewValue, 0.f, 100.f);
	Handle->GetPercentage() = Clamped;
	
	TArray<void*> RawData;
	PropertyHandle->GetParentHandle()->GetParentHandle()->AccessRawData(RawData);
	if (RawData.IsEmpty()) return;
	
	FMassObject* MassObject = static_cast<FMassObject*>(RawData[0]);
	if (!MassObject) return;
	
	TOptional<uint32> IndexOpt = PropertyHandle->GetIndexInArray();
	if (!IndexOpt.IsSet()) return;
	
	const int32 SelfIndex = IndexOpt.GetValue();

	// Calcul du total des autres éléments valides
	const float Remaining = 100.f - Clamped;
	float TotalOther = 0.f;

	TArray<int32> ValidIndices;
	for (int32 i = 0; i < MassObject->Materials.Num(); ++i)
	{
		if (i == SelfIndex) continue;
		if (!MassObject->Materials[i].Material.IsNone())
		{
			ValidIndices.Add(i);
			TotalOther += MassObject->Materials[i].Percentage;
		}
	}

	if (ValidIndices.IsEmpty()) return;
	
	for (const int32 i : ValidIndices)
	{
		const float Old = MassObject->Materials[i].Percentage;
		const float New = TotalOther > 0.f ? (Old / TotalOther) * Remaining : (Remaining / ValidIndices.Num());
		MassObject->Materials[i].Percentage = New;
	}
	
}

void FCustomMassRatio::BalancedAfterSelection() const
{
	TArray<void*> RawData;
	PropertyHandle->GetParentHandle()->GetParentHandle()->AccessRawData(RawData);
	if (RawData.IsEmpty()) return;
	

	FMassObject* MassObject = static_cast<FMassObject*>(RawData[0]);
	if (!MassObject) return;
	

	const int32 Count = MassObject->Materials.Num();
	if (Count > 1)
	{
		const float Equal = 100.f / static_cast<float>(Count);
		for (FMassRatio& Ratio : MassObject->Materials)
		{
			Ratio.Percentage = Equal;
		}
		if (PropertyUtils.IsValid()) PropertyUtils->ForceRefresh();
		UE_LOG(LogTemp, Log, TEXT("[MassRatio] Auto-balanced all to %.2f%%"), Equal);
	}
}

#undef LOCTEXT_NAMESPACE


// ===============================[ Property Customization Factory ]============================
// ======================================== FMASSOBJECT ========================================
// =============================================================================================

#define LOCTEXT_NAMESPACE "CustomMassObject"

void FCustomMassObject::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	PropertyHandle = StructPropertyHandle;
	PropertyUtils = CustomizationUtils.GetPropertyUtilities();

	if (void* Dummy = nullptr; StructPropertyHandle->GetValueData(Dummy) == FPropertyAccess::Success)
	{
		TArray<void*> RawData;
		StructPropertyHandle->AccessRawData(RawData);
		if (RawData.IsEmpty()) return;
		
		Handle = static_cast<FMassObject*>(RawData[0]);
		RefreshOptions();
	}
	HeaderRow
	.NameContent()
	[
		StructPropertyHandle->CreatePropertyNameWidget()
	];
}

void FCustomMassObject::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle,
	IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	if (!Handle) return;

	const TSharedPtr<IPropertyHandle> FixedWeightHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FMassObject, FixedWeight));
	const TSharedPtr<IPropertyHandle> AirFrictionHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FMassObject, AirFriction));
	const TSharedPtr<IPropertyHandle> FrictionHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FMassObject, Friction));
	const TSharedPtr<IPropertyHandle> MaterialsHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FMassObject, Materials));
	const TSharedPtr<IPropertyHandle> StaticMeshHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FMassObject, StaticMesh));
	const TSharedPtr<IPropertyHandle> SkeletalMeshHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FMassObject, SkeletalMesh));
	
	TSharedPtr<FString> CurrentOption;

	if (const FName CurrentType = Handle->GetTypeOption() ; !CurrentType.IsNone())
	{
		const FString CurrentStr = CurrentType.ToString();
		
		for (const TSharedPtr<FString>& Row : *TypesHandles)
		{
			if (Row.IsValid() && *Row == CurrentStr)
			{
				CurrentOption = Row;
				break;
			}
		}
	}
	
	ChildBuilder.AddCustomRow(FText::FromString("TyperCustomization"))
	.NameContent()
	[
		SNew(STextBlock)
		.Text(FText::FromString("Type"))
		.Font(IDetailLayoutBuilder::GetDetailFont())
	]
	.ValueContent()
	[
		SNew(SBox)
		.MinDesiredWidth(150.f)
		[
			SNew(SComboBox<TSharedPtr<FString>>)
			.OptionsSource(TypesHandles.Get())
			.InitiallySelectedItem(CurrentOption)
			.OnGenerateWidget_Lambda([this](const TSharedPtr<FString>& InItem) -> TSharedRef<SWidget>
			{
				if (!InItem.IsValid()) return SNew(STextBlock).Text(FText::FromString("Invalide"));
				if (const FName Type(**InItem); !Type.IsNone())
				{
					return SNew(STextBlock).Text(FText::FromString(Type.ToString()));
				}
				return SNew(STextBlock).Text(FText::FromString(*InItem));
			})
			.OnSelectionChanged_Lambda([this](const TSharedPtr<FString>& NewSelection, ESelectInfo::Type)
			{
				if (!NewSelection.IsValid()) return;
				const int32 SelectedType = *NewSelection == "Fixed" ? 0 : 1;
				if (Handle)
				{
					Handle->GetType() = SelectedType;
					PropertyUtils->ForceRefresh();
				}
			})
			.Content()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text_Lambda([this]()
				{
					if (Handle)
					{
						if (const FName Type = Handle->GetTypeOption(); !Type.IsNone())
						{
							return FText::FromString(Type.ToString());
						}
					}
					return FText::FromString("");
				})
			]
		]
	];
	
	if(Handle->GetType() == 0)
	{
		ChildBuilder.AddProperty(FixedWeightHandle.ToSharedRef());
		ChildBuilder.AddProperty(AirFrictionHandle.ToSharedRef());
		ChildBuilder.AddProperty(FrictionHandle.ToSharedRef());
	}
	else
	{
		ChildBuilder.AddProperty(StaticMeshHandle.ToSharedRef());
		ChildBuilder.AddProperty(SkeletalMeshHandle.ToSharedRef());
		ChildBuilder.AddProperty(AirFrictionHandle.ToSharedRef());
		ChildBuilder.AddProperty(FrictionHandle.ToSharedRef());
		ChildBuilder.AddProperty(MaterialsHandle.ToSharedRef()).ShouldAutoExpand(true);
		ChildBuilder.AddCustomRow(FText::FromString("FinalWeightCustomization"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(FText::FromString("Weight"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			.Padding(0.f, 10.f, 0.f, 10.f)
			[
				SNew(SSpinBox<float>)
				.MinDesiredWidth(52.f)
				.MaxFractionalDigits(2)
				.IsEnabled(false)
				.Justification(ETextJustify::Center)
				.Delta(1)
				.MinValue (0.f)
				.MaxValue (1000.f)
				.Value_Lambda([this] ()
				{
					if (!Handle) return 0.f;
					return Handle->GetMass();
				})
			]
			+ SHorizontalBox::Slot().AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString("kg"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
		];
	}
	
	StaticMeshHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([this, StaticMeshHandle, SkeletalMeshHandle]()
	{
		if (!StaticMeshHandle.IsValid() || !SkeletalMeshHandle.IsValid()) return;

		UObject* StaticMesh = nullptr;
		UObject* SkeletalMesh = nullptr;

		const bool bHasStatic = StaticMeshHandle->GetValue(StaticMesh) == FPropertyAccess::Success && StaticMesh;
		const bool bHasSkeletal = SkeletalMeshHandle->GetValue(SkeletalMesh) == FPropertyAccess::Success && SkeletalMesh;
				
		if (bHasStatic && bHasSkeletal)
		{
			SkeletalMeshHandle->SetValue(static_cast<USkeletalMesh*>(nullptr));
		}
	}));
	SkeletalMeshHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([this, StaticMeshHandle, SkeletalMeshHandle]()
	{
		if (!StaticMeshHandle.IsValid() || !SkeletalMeshHandle.IsValid()) return;

		UObject* StaticMesh = nullptr;
		UObject* SkeletalMesh = nullptr;

		const bool bHasStatic = StaticMeshHandle->GetValue(StaticMesh) == FPropertyAccess::Success && StaticMesh;
		const bool bHasSkeletal = SkeletalMeshHandle->GetValue(SkeletalMesh) == FPropertyAccess::Success && SkeletalMesh;
		
		if (bHasSkeletal && bHasStatic)
		{
			StaticMeshHandle->SetValue(static_cast<UStaticMesh*>(nullptr));
		}
	}));
}

void FCustomMassObject::RefreshOptions()
{
	if (!Handle) return;

	if (!TypesHandles.IsValid())
	{
		TypesHandles = MakeShared<TArray<TSharedPtr<FString>>>();
	}
	TypesHandles->Empty();
	TypesHandles->Add(MakeShared<FString>(TEXT("Fixed")));
	TypesHandles->Add(MakeShared<FString>("Dynamic"));
	
}

#undef LOCTEXT_NAMESPACE
