#include "Inventory/InventoryTypes.h"

#include "Inventory/VisualItem.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailGroup.h"
#include "IPropertyUtilities.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Input/SSpinBox.h"

FItemRow::FItemRow()
{
	Name = FText::FromString("");
	ShortDesc = FText::FromString("");
	LongDesc = FText::FromString("");
	Type = EItemType::E_None;
	VisualItemClass = AVisualItem::StaticClass();
	DropMesh = nullptr;
	Flags = DefaultItemFlags();
	Thumbnail = FThumbnail();
	BindRule = EBindRule::None;
	Quality = 0;
	MaxStackSize = 1;

	bIsPocket = false;
	Bag = FBagSpec();
	Pocket = FPocketSpec();

	bIsUsingOutlines = true;
	InteractionDuration = 0.f;
	MinInteractionDistance = 200.f;
	OutlinesColor = FColorPicker(18);

	WeightConfig = FMassObject();

	Mesh = nullptr;
	SkeletalMesh = nullptr;
	
	WeaponType = EWeaponType::E_None;
	Handling = 0;
	CompatibleAmmunition = TArray<FItemRowHandle>();

	WeaponDamage = 1.f;
	ArmorPenetration = 30.f;
	FleshDamage = 70.f;
	RessourceCost = 1.f;
	MinConeAngle = 90.f;
	MaxConeAngle = 180.f;
	SpeedMultiplier = 1.f;
	MaxRange = 1000.f;
	AimingSpeedMultiplier = 1.f;
	ReloadTimeMultiplier = 1.f;
	
	ArmorType = 0;
	ArmorSlot = 0;
	ArmorValue = 1;
	Male = nullptr;
	Female = nullptr;

	ToolType = EToolType::E_None;
	HarvestMultiplier = 1.f;

	AmmunitionType = EAmmunitionType::E_None;
	ProjectileDamage = 1.f;
	ProjectileSpeedMultiplier = 1.f;
	
	FoodValue = 0.f;
	WaterValue = 0.f;
	
	PerishRate = 1.f;
	PerishTo = FItemRowHandle("Consumable");
	MaxDurability = 1.f;
	MaxWear = 1.f;
	RepairTimeMultiplier = 1.f;
	RepairData = TArray<FItemRepairData>();

	Modifiers = TMap<FName, float>({
		{"FirstModifier", 0.f},
		{"SecondModifier", 0.f},
		{"ThirdModifier", 0.f},
		{"FourthModifier", 0.f}});
	
	Tags = TArray<FName>({""});
}

bool FItemRow::HasFlag(const EItemFlags Flag) const
{
	return (Flags & static_cast<uint8>(Flag)) != 0;
}

#define LOCTEXT_NAMESPACE "CustomItemRow"

// ===============================[ Property Customization Factory ]============================
// ========================================== FCUSTOMITEMROW =========================================
// =============================================================================================

void FCustomItemRow::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow,
                                           IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	PropertyHandle = StructPropertyHandle;
	PropertyUtils = CustomizationUtils.GetPropertyUtilities();

	if (void* Dummy = nullptr; StructPropertyHandle->GetValueData(Dummy) == FPropertyAccess::Success)
	{
		TArray<void*> RawData;
		StructPropertyHandle->AccessRawData(RawData);
		if (RawData.IsEmpty()) return;
				
		Handle = static_cast<FItemRow*>(RawData[0]);
	}
	HeaderRow
	.NameContent()
	[
		StructPropertyHandle->CreatePropertyNameWidget()
	];
}

void FCustomItemRow::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle,
                                       IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	if (!PropertyHandle || !Handle) return;

	DeclareProperties();
	BindDelegatesProperties();

	CurrentFlags = Handle->Flags;
	CurrentType = Handle->Type;

	SetupTag();
	
	ChildBuilder.AddProperty(NameHandle.ToSharedRef());
	ChildBuilder.AddProperty(ShortDescHandle.ToSharedRef());
	ChildBuilder.AddProperty(LongDescHandle.ToSharedRef());
	ChildBuilder.AddProperty(TypeHandle.ToSharedRef());
	ChildBuilder.AddProperty(VisualItemClassHandle.ToSharedRef());
	ChildBuilder.AddProperty(DropMeshHandle.ToSharedRef());
	ChildBuilder.AddProperty(FlagsHandle.ToSharedRef());
	ChildBuilder.AddProperty(ThumbnailHandle.ToSharedRef());
	ChildBuilder.AddProperty(BindRuleHandle.ToSharedRef());
	ChildBuilder.AddProperty(QualityHandle.ToSharedRef());
	ChildBuilder.AddProperty(MaxStackSizeHandle.ToSharedRef());
	
	IDetailGroup& ExtrasGroup = ChildBuilder.AddGroup("Extras", FText::FromString("Extras Settings"));
	if (Handle->HasContainer())
	{
		IDetailGroup& ContainerGroup = ExtrasGroup.AddGroup("Container", FText::FromString("Container Settings"));
		ContainerGroup.AddPropertyRow(IsPocketHandle.ToSharedRef());
		ContainerGroup.AddPropertyRow(BagHandle.ToSharedRef());
		ContainerGroup.AddPropertyRow(PocketHandle.ToSharedRef());
	}
	
	if (Handle->GetInteractionVisible())
	{
		IDetailGroup& InteractionGroup = ExtrasGroup.AddGroup("Interaction", FText::FromString("Interaction Settings"));
		InteractionGroup.AddPropertyRow(InteractionDurationHandle.ToSharedRef());
		InteractionGroup.AddPropertyRow(MinInteractionDistanceHandle.ToSharedRef());
		IDetailGroup& OutlinesGroup = InteractionGroup.AddGroup("Outlines", FText::FromString("Outlines Settings"));
			OutlinesGroup.AddPropertyRow(IsUsingOutlinesHandle.ToSharedRef());
			OutlinesGroup.AddPropertyRow(OutlinesColorHandle.ToSharedRef());
	}
	
	IDetailGroup& WeightGroup = ExtrasGroup.AddGroup("Weight", FText::FromString("Weight Settings"));
	WeightGroup.AddPropertyRow(WeightConfigHandle.ToSharedRef());
	
	if (Handle->GetEquipeableVisible())
	{
		IDetailGroup& EquipeableGroup = ChildBuilder.AddGroup("Equipable", FText::FromString("Equipable"));
		if (Handle->GetWeaponVisible())
		{
			IDetailGroup& WeaponGroup = EquipeableGroup.AddGroup("Weapon", FText::FromString("Weapon"));
			WeaponGroup.AddPropertyRow(WeaponTypeHandle.ToSharedRef());
			WeaponGroup.AddPropertyRow(HandlingHandle.ToSharedRef());
			WeaponGroup.AddPropertyRow(MeshHandle.ToSharedRef());
			WeaponGroup.AddPropertyRow(SkeletalMeshHandle.ToSharedRef());
			if (Handle->IsUsingAmmunition())
			{
				WeaponGroup.AddPropertyRow(CompatibleAmmunitionHandle.ToSharedRef());
			}
			IDetailGroup& DataGroup = WeaponGroup.AddGroup("Data", FText::FromString("Data"));
			DataGroup.AddPropertyRow(WeaponDamageHandle.ToSharedRef());
			CreateDamageSliders(DataGroup);
			DataGroup.AddPropertyRow(RessourceCostHandle.ToSharedRef());
			if (Handle->WeaponType != EWeaponType::E_Crossbow && Handle->WeaponType != EWeaponType::E_Bow)
			{
				DataGroup.AddPropertyRow(MinConeAngleHandle.ToSharedRef());
				DataGroup.AddPropertyRow(MaxConeAngleHandle.ToSharedRef());
				DataGroup.AddPropertyRow(SpeedMultiplierHandle.ToSharedRef());
			}
			else
			{
				DataGroup.AddPropertyRow(MaxRangeHandle.ToSharedRef());
				DataGroup.AddPropertyRow(AimingSpeedHandle.ToSharedRef());
				DataGroup.AddPropertyRow(ReloadTimeMultiplierHandle.ToSharedRef());
			}
		}
		
		if (Handle->GetArmorVisible())
		{
			IDetailGroup& ArmorGroup = EquipeableGroup.AddGroup("Armor", FText::FromString("Armor"));
			ArmorGroup.AddPropertyRow(ArmorTypeHandle.ToSharedRef());
			ArmorGroup.AddPropertyRow(ArmorSlotHandle.ToSharedRef());
			ArmorGroup.AddPropertyRow(ArmorValueHandle.ToSharedRef());
			ArmorGroup.AddPropertyRow(MaleHandle.ToSharedRef());
			ArmorGroup.AddPropertyRow(FemaleHandle.ToSharedRef());
		}
		
		if (Handle->GetToolVisible())
		{
			IDetailGroup& ToolGroup = EquipeableGroup.AddGroup("Tool", FText::FromString("Tool"));
			ToolGroup.AddPropertyRow(ToolTypeHandle.ToSharedRef());
			ToolGroup.AddPropertyRow(HarvestMultiplierHandle.ToSharedRef());
		}
		
		if (Handle->GetAmmunitionVisible())
		{
			IDetailGroup& AmmunitionGroup = EquipeableGroup.AddGroup("Ammunition", FText::FromString("Ammunition"));
			AmmunitionGroup.AddPropertyRow(AmmunitionTypeHandle.ToSharedRef());
			AmmunitionGroup.AddPropertyRow(DamageHandle.ToSharedRef());
			CreateDamageSliders(AmmunitionGroup);
			AmmunitionGroup.AddPropertyRow(ProjectileSpeedMultiplierHandle.ToSharedRef());
		}
	}
	
	if (Handle->GetConsumableVisible())
	{
		IDetailGroup& ConsumableGroup = ChildBuilder.AddGroup("Consumable", FText::FromString("Consumable"));
		ConsumableGroup.AddPropertyRow(FoodValueHandle.ToSharedRef());
		ConsumableGroup.AddPropertyRow(WaterValueHandle.ToSharedRef());
		if (Handle->CanPerish())
		{
			ConsumableGroup.AddWidgetRow()
			.NameContent()
			[
				PerishRateHandle->CreatePropertyNameWidget()
			]
			.ValueContent()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SBox)
					.WidthOverride(125.0f)
					[
						PerishRateHandle->CreatePropertyValueWidget()
					]
				]
				+ SHorizontalBox::Slot().AutoWidth()
				.Padding(5.0f, 0.0f)
				[
					SNew(SSpinBox<float>)
					.MinValue(0.f)
					.MinSliderValue(0.f)
					.MaxFractionalDigits(2)
					.MinDesiredWidth(52.f)
					.Justification(ETextJustify::Center)
					.IsEnabled(false)
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.Value_Lambda([this] () -> float
					{
						return GetDisplayDuration(PerishRateHandle, DefaultPerishTime());
					})					
				]
				+ SHorizontalBox::Slot().AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(3.0f, 0.0f)
				[
					SNew(STextBlock)
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.Text_Lambda([this] () -> FText
					{
						return GetDurationUnitText(PerishRateHandle, DefaultPerishTime());
					})
				]
			];
			ConsumableGroup.AddPropertyRow(PerishToHandle.ToSharedRef());
		}
	}
	
	if (Handle->HasDurability())
	{
		IDetailGroup& DurabilityGroup = ChildBuilder.AddGroup("Durability", FText::FromString("Durability"));
		DurabilityGroup.AddPropertyRow(MaxDurabilityHandle.ToSharedRef());
		DurabilityGroup.AddPropertyRow(MaxWearHandle.ToSharedRef());
		if (Handle->CanBeRepair())
		{
			DurabilityGroup.AddWidgetRow()
			.NameContent()
			[
				RepairTimeMultiplierHandle->CreatePropertyNameWidget()
			]
			.ValueContent()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SBox)
					.WidthOverride(125.0f)
					[
						RepairTimeMultiplierHandle->CreatePropertyValueWidget()
					]
				]
				+ SHorizontalBox::Slot().AutoWidth()
				.Padding(5.0f, 0.0f)
				[
					SNew(SSpinBox<float>)
					.MinValue(0.f)
					.MinSliderValue(0.f)
					.MaxFractionalDigits(2)
					.MinDesiredWidth(52.f)
					.Justification(ETextJustify::Center)
					.IsEnabled(false)
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.Value_Lambda([this] () -> float
					{
						return GetDisplayDuration(RepairTimeMultiplierHandle, DefaultRepairTime());
					})					
				]
				+ SHorizontalBox::Slot().AutoWidth()
				.Padding(3.0f, 0.0f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.Text_Lambda([this] () -> FText
					{
						return GetDurationUnitText(RepairTimeMultiplierHandle, DefaultRepairTime());
					})
				]
			];
			DurabilityGroup.AddPropertyRow(RepairDataHandle.ToSharedRef());
		}
	}

	ChildBuilder.AddProperty(ModifiersHandle.ToSharedRef());
}

void FCustomItemRow::DeclareProperties()
{
	//Default category
	NameHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, Name));
	ShortDescHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, ShortDesc));
	LongDescHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, LongDesc));
	TypeHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, Type));
	VisualItemClassHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, VisualItemClass));
	DropMeshHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, DropMesh));
	FlagsHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, Flags));
	ThumbnailHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, Thumbnail));
	BindRuleHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, BindRule));
	QualityHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, Quality));
	MaxStackSizeHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, MaxStackSize));

	IsPocketHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, bIsPocket));
	BagHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, Bag));
	PocketHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, Pocket));
	
	InteractionDurationHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, InteractionDuration));
	MinInteractionDistanceHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, MinInteractionDistance));
	IsUsingOutlinesHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, bIsUsingOutlines));
	OutlinesColorHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, OutlinesColor));

	WeightConfigHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, WeightConfig));

	MeshHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, Mesh));
	SkeletalMeshHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, SkeletalMesh));
	
	WeaponTypeHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, WeaponType));
	HandlingHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, Handling));
	CompatibleAmmunitionHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, CompatibleAmmunition));

	WeaponDamageHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, WeaponDamage));
	ArmorPenetrationHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, ArmorPenetration));
	FleshDamageHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, FleshDamage));
	RessourceCostHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, RessourceCost));
	MinConeAngleHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, MinConeAngle));
	MaxConeAngleHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, MaxConeAngle));
	SpeedMultiplierHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, SpeedMultiplier));
	MaxRangeHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, MaxRange));
	AimingSpeedHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, AimingSpeedMultiplier));
	ReloadTimeMultiplierHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, ReloadTimeMultiplier));

	ArmorTypeHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, ArmorType));
	ArmorSlotHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, ArmorSlot));
	ArmorValueHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, ArmorValue));
	MaleHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, Male));
	FemaleHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, Female));

	ToolTypeHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, ToolType));
	HarvestMultiplierHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, HarvestMultiplier));

	AmmunitionTypeHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, AmmunitionType));
	DamageHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, ProjectileDamage));
	ProjectileSpeedMultiplierHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, ProjectileSpeedMultiplier));
	
	FoodValueHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, FoodValue));
	WaterValueHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, WaterValue));

	PerishRateHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, PerishRate));
	PerishToHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, PerishTo));
	MaxDurabilityHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, MaxDurability));
	MaxWearHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, MaxWear));
	RepairTimeMultiplierHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, RepairTimeMultiplier));
	RepairDataHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, RepairData));

	ModifiersHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemRow, Modifiers));
	
}

void FCustomItemRow::BindDelegatesProperties()
{
	TypeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([this]()
	{
		OnTypeChanged();
	}));
	FlagsHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([this]()
	{
		OnFlagsChanged();
	}));
	WeaponTypeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([this]()
	{
		if (CurrentType != EItemType::E_Weapon) return;
		PropertyUtils->ForceRefresh();
	}));
}

void FCustomItemRow::SetupTag() const
{
	if (Handle->GetConsumableVisible())
	{
		Handle->AddTag("Consumable", 0);
	}
	else if (Handle->Type == EItemType::E_Ammunition)
	{
		Handle->AddTag("Ammunition", 0);
	}
	else
	{
		Handle->RemoveTag(0);
	}
}

void FCustomItemRow::CreateDamageSliders(IDetailGroup& Group) const
{
	auto MakeDamageSlider = [this, &Group](const FString& Label, TSharedPtr<IPropertyHandle> Main, TSharedPtr<IPropertyHandle> Opposite)
	{
		Group.AddWidgetRow()
		.NameContent()
		[
			SNew(STextBlock)
			.Text(FText::FromString(Label))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0, 10)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				.Padding(0.0f, 0, 0.0f, 5.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(SBox)
						.WidthOverride(125.0f)
						[
							Main->CreatePropertyValueWidget()
						]
					]
					+ SHorizontalBox::Slot().AutoWidth()
					.Padding(5.0f, 0.0f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromString("%"))
						.Font(IDetailLayoutBuilder::GetDetailFont())
					]
				]
				+ SVerticalBox::Slot().AutoHeight()
				.Padding(0.0f, 5.f, 0.0f, 0.0f)
				[
					SNew(SSlider)
					.MinValue(0.f)
					.MaxValue(100.f)
					.Value_Lambda([Main]()
					{
						float Value = 0.f;
						Main->GetValue(Value);
						return Value;
					})
					.OnValueChanged_Lambda([Main, Opposite](float NewValue)
					{
						const float Clamped = FMath::Clamp(NewValue, 0.f, 100.f);
						Main->SetValue(Clamped);
						Opposite->SetValue(100.f - Clamped);
					})
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.MinDesiredWidth(50)
				.Text_Lambda([this, Main]()
				{
					float Percent = 0.f;
					Main->GetValue(Percent);

					float TotalDamage;
					if (Handle->Type == EItemType::E_Ammunition)
						TotalDamage = Handle->ProjectileDamage;
					else
						TotalDamage = Handle->WeaponDamage;

					const float RealDamage = TotalDamage * (Percent / 100.f);
					return FText::FromString(FString::Printf(TEXT("%.2f"), RealDamage));
				})
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
			+ SHorizontalBox::Slot().AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(" Damages"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
		];
		Main->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([Main, Opposite]()
		{
			if (float NewValue; Main->GetValue(NewValue) == FPropertyAccess::Success)
			{
				const float Clamped = FMath::Clamp(NewValue, 0.f, 100.f);
						Main->SetValue(Clamped);
						Opposite->SetValue(100.f - Clamped);
			}
		}));
	};

	MakeDamageSlider("Armor Penetration", ArmorPenetrationHandle, FleshDamageHandle);
	MakeDamageSlider("Flesh Damage", FleshDamageHandle, ArmorPenetrationHandle);
}

void FCustomItemRow::OnFlagsChanged()
{
	constexpr uint8 Mask =
		static_cast<uint8>(EItemFlags::E_CanPerished) |
		static_cast<uint8>(EItemFlags::E_CanInteract) |
		static_cast<uint8>(EItemFlags::E_CanRepair)	  |
		static_cast<uint8>(EItemFlags::E_HasContainer);

	const uint8 OldRelevant = CurrentFlags & Mask;
	const uint8 NewRelevant = Handle->Flags & Mask;
	
	if (OldRelevant != NewRelevant)
	{
		PropertyUtils->ForceRefresh();
	}

	CurrentFlags = Handle->Flags;
}

void FCustomItemRow::OnTypeChanged()
{
	if (CurrentType == Handle->Type)
		return;
	Handle->RemoveTag(0);
	if (Handle->GetConsumableVisible()) Handle->AddTag("Consumable", 0);
	if (Handle->Type == EItemType::E_Ammunition) Handle->AddTag("Ammunition", 0);
	PropertyUtils->ForceRefresh();
	CurrentType = Handle->Type;
}

float FCustomItemRow::GetDurationInSeconds(const TSharedPtr<IPropertyHandle>& PropertyHandle, const float BaseDuration)
{
	if (float Value = 0.f; PropertyHandle.Get()->GetValue(Value) == FPropertyAccess::Success)
	{
		return Value * BaseDuration;
	}
	return BaseDuration;
}

float FCustomItemRow::GetDisplayDuration(const TSharedPtr<IPropertyHandle>& PropertyHandle, const float BaseDuration)
{
	const float Seconds = GetDurationInSeconds(PropertyHandle, BaseDuration);

	if (Seconds >= 3600.f)
	{
		// Format: H.MM where MM is minutes
		const int32 Hours = static_cast<int32>(Seconds / 3600.f);
		const int32 Minutes = static_cast<int32>(FMath::Fmod(Seconds, 3600.f) / 60.f);
		return static_cast<float>(Hours) + Minutes / 100.f;
	}
	if (Seconds >= 60.f)
	{
		// Format: M.SS where SS is seconds
		const int32 Minutes = static_cast<int32>(Seconds / 60.f);
		const int32 RemainingSec = static_cast<int32>(FMath::Fmod(Seconds, 60.f));
		return static_cast<float>(Minutes) + RemainingSec / 100.f;
	}
	return Seconds;
}

FText FCustomItemRow::GetDurationUnitText(const TSharedPtr<IPropertyHandle>& PropertyHandle, const float BaseDuration)
{
	if (const float Duration = GetDurationInSeconds(PropertyHandle, BaseDuration); Duration >= 3600.f)
	{
		return FText::FromString("Hours");
	}
	else if (Duration >= 60.f)
	{
		return FText::FromString("Minutes");
	}
	return FText::FromString("Seconds");
}

#undef LOCTEXT_NAMESPACE
