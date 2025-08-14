#include "Custom/Variables/BaseHandle.h"

#include "Utils/Tables.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "Custom/Widgets/SearchCombobox.h"
#include "Styling/SlateIconFinder.h"

#define LOCTEXT_NAMESPACE "CustomBaseHandle"

ETablePath FCustomBaseHandle::GetDataTable()
{
	return ETablePath::None;
}

void FCustomBaseHandle::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	PropertyHandle = StructPropertyHandle;
	PropertyUtils = CustomizationUtils.GetPropertyUtilities();

	if (void* Dummy = nullptr; StructPropertyHandle->GetValueData(Dummy) == FPropertyAccess::Success)
	{
		TArray<void*> RawData;
		StructPropertyHandle->AccessRawData(RawData);
		if (RawData.IsEmpty()) return;

		if (const FStructProperty* StructProperty = CastField<FStructProperty>(StructPropertyHandle->GetProperty()))
		{
			if (const UScriptStruct* StructType = StructProperty->Struct; !StructType || !StructType->IsChildOf(FBaseHandle::StaticStruct()))
			{
				UE_LOG(LogTemp, Error, TEXT("The structure '%s' does not inherit from FBaseHandle."), *StructType->GetName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Property is not recognized as an FStructProperty."));
			return;
		}
		
		Handle = static_cast<FBaseHandle*>(RawData[0]);
		DataTable = UTables::GetTable(GetDataTable());
		RefreshHandle();
	}
	HeaderRow
	.NameContent()
	[
		StructPropertyHandle->CreatePropertyNameWidget()
	];
}

void FCustomBaseHandle::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle,IDetailChildrenBuilder& ChildBuilder,
	IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	if (!Handle) return;
	TSharedPtr<FString> CurrentOption;
		
	if (const FName CurrentID = Handle->GetID(); !CurrentID.IsNone())
	{
		const FString CurrentStr = CurrentID.ToString();
		for (const TSharedPtr<FString>& Row : *RowHandles)
		{
			if (Row.IsValid() && *Row == CurrentStr)
			{
				CurrentOption = Row;
				break;
			}
		}
	}

	Handle->OnMarkToRefresh.BindLambda([this]()
	{
		RefreshHandle();
		CreateSearchCombobox();
	});
		
	ChildBuilder.AddCustomRow(FText::FromString("HandleCustomization"))
	.NameContent()
	[
		SNew(STextBlock)
			.Text(FText::FromString(StructPropertyHandle->GetPropertyDisplayName().ToString()))
			.Font(IDetailLayoutBuilder::GetDetailFont()) // Style cohérent avec le reste
			.Margin(FMargin(0.f, 0.f, 6.f, 0.f))
	]
	.ValueContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		[
			SAssignNew(BoxContent, SVerticalBox)
			+ SVerticalBox::Slot()
			[
				SNew(SBox)
				.MinDesiredWidth(250.f)
				[
					SNew(SSearchCombobox)
					.Options(RowHandles.Get())
					.DefaultOption(*DefaultOption())
					.SourceName(DataTable ? DataTable->GetName() : "Not Existing")
					.Filters(DefaultFilter())
					.KeepFormat(false)
					.CurrentItem(CurrentOption)
					.OnGenerateWidget_Lambda([](const TSharedPtr<FString>& InItem, const TSharedRef<STableViewBase>& Owner) -> TSharedRef<ITableRow>
					{
						return SNew(STableRow<TSharedPtr<FString>>, Owner)
						[
							SNew(SBox)
							.HeightOverride(25.f)
							.Padding(5, 0, 0, 0)
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								.VAlign(VAlign_Center)
								[
									 SNew(STextBlock)
									.Text(FText::FromString(*InItem))
									.Font(IDetailLayoutBuilder::GetDetailFont())
								]
							]
						];
					})
					.OnSelectionChanged_Lambda([this](const TSharedPtr<FString>& NewSelection, ESelectInfo::Type)
					{
						if (!NewSelection.IsValid()) return;
						const FName SelectedName = NewSelection.Get()->IsEmpty() ? NAME_None : FName(*NewSelection);
						if (Handle)
						{
							Handle->GetID() = SelectedName;
						}
					})
				]
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(4.f, 0.f)
		[
			SAssignNew(RefreshButton, SButton)
			.ButtonStyle(FAppStyle::Get(), "SimpleButton")
			.ContentPadding(FMargin(2.f))
			.ButtonColorAndOpacity_Lambda([this]() -> FLinearColor
			{
				return (RefreshButton.IsValid() && RefreshButton->IsHovered())
					? FLinearColor(0.05f, 0.05f, 0.05f, 1.0f)
					: FLinearColor::Transparent;
			})
			.ToolTipText(NSLOCTEXT("HandleCustomization", "TooltipReload", "Reload available rows."))
			.OnClicked_Lambda([this]()
			{
				RefreshHandle();
				CreateSearchCombobox();
				return FReply::Handled();
			})
			[
				SNew(SImage)
				.Image(FSlateIconFinder::FindIcon("SourceControl.Actions.Refresh").GetIcon())
			]
		]
	];
}

FString FCustomBaseHandle::DefaultFilter() const
{
	FString FilterString = FString();
	const bool bHasTag = !Handle->GetTag().IsNone();
	FilterString += bHasTag ? FString::Printf(TEXT("Tag: '%s'"), *Handle->GetTag().ToString()) : TEXT("");
	return FilterString;
}

void FCustomBaseHandle::RefreshHandle()
{
	if (!Handle) return;
	
	if (!RowHandles.IsValid())
	{
		RowHandles = MakeShared<TArray<TSharedPtr<FString>>>();
	}
	RowHandles->Empty();
	const FName Tag = Handle->GetTag();

	if (DataTable)
	{
		for (const FName& RowName : DataTable->GetRowNames())
		{
			if (GetFilterConditions(RowName, Tag))
			{
				RowHandles->Add(MakeShared<FString>(RowName.ToString()));
			}
		}	
	}
	FilteredRowHandles = *RowHandles;
}

void FCustomBaseHandle::CreateSearchCombobox() const
{
	if (!BoxContent.IsValid()) return;

	TSharedPtr<FString> CurrentOption;
		
	if (const FName CurrentID = Handle->GetID(); !CurrentID.IsNone())
	{
		const FString CurrentStr = CurrentID.ToString();
		for (const TSharedPtr<FString>& Row : *RowHandles)
		{
			if (Row.IsValid() && *Row == CurrentStr)
			{
				CurrentOption = Row;
				break;
			}
		}
	}
	BoxContent->ClearChildren();
	BoxContent->AddSlot()
	[
		SNew(SBox)
		.MinDesiredWidth(250.f)
		[
			SNew(SSearchCombobox)
			.Options(RowHandles.Get())
			.DefaultOption(*DefaultOption())
			.SourceName(DataTable ? DataTable->GetName() : "Not Existing")
			.Filters(DefaultFilter())
			.KeepFormat(false)
			.CurrentItem(CurrentOption)
			.OnGenerateWidget_Lambda([](const TSharedPtr<FString>& InItem, const TSharedRef<STableViewBase>& Owner) -> TSharedRef<ITableRow>
			{
				return SNew(STableRow<TSharedPtr<FString>>, Owner)
				[
					SNew(SBox)
					.HeightOverride(25.f)
					.Padding(5, 0, 0, 0)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.VAlign(VAlign_Center)
						[
							 SNew(STextBlock)
							.Text(FText::FromString(*InItem))
							.Font(IDetailLayoutBuilder::GetDetailFont())
						]
					]
				];
			})
			.OnSelectionChanged_Lambda([this](const TSharedPtr<FString>& NewSelection, ESelectInfo::Type)
			{
				if (!NewSelection.IsValid()) return;
				const FName SelectedName = NewSelection.Get()->IsEmpty() ? NAME_None : FName(*NewSelection);
				if (Handle)
				{
					Handle->GetID() = SelectedName;
				}
			})
		]
	];
}

#undef LOCTEXT_NAMESPACE
