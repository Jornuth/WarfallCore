#include "Custom/Widgets/SearchCombobox.h"

#include "DetailLayoutBuilder.h"
#include "Widgets/Layout/SBox.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateIconFinder.h"
#include "Styling/SlateStyleRegistry.h"
#include "Widgets/Input/SSearchBox.h"

void SSearchCombobox::Construct(const FArguments& InArgs)
{
	OptionsSource = InArgs._Options;
	OnGenerateWidget = InArgs._OnGenerateWidget;
	OnSelectionChanged = InArgs._OnSelectionChanged;
	OnMenuOpenChanged = InArgs._OnMenuOpenChanged;
	SelectedItem = InArgs._CurrentItem;

	bKeepFormat = InArgs._KeepFormat;
	if (!InArgs._DefaultOption.IsEmpty()) DefaultOption = InArgs._DefaultOption;
	SourceName = InArgs._SourceName;
	Filters = InArgs._Filters;

	FilteredOptions = *OptionsSource;

	FilteredOptions.Sort([this] (const TSharedPtr<FString>& A, const TSharedPtr<FString>& B)
	{
		if (!A.IsValid() || !B.IsValid()) return false;
		return bSortAscending ? (*A < *B) : (*A > *B);
	});

	SetOptions(FilteredOptions);

	const FComboButtonStyle& ComboButtonStyle = FCoreStyle::Get().GetWidgetStyle<FComboButtonStyle>("ComboButton");
	const FButtonStyle& ButtonStyle = ComboButtonStyle.ButtonStyle;

	ChildSlot
	[
		SAssignNew(MenuAnchor, SMenuAnchor)
		.Placement(MenuPlacement_ComboBox)
		.OnGetMenuContent(this, &SSearchCombobox::GenerateMenuContent)
		.OnMenuOpenChanged(this, &SSearchCombobox::OnMenuOpenChanged_Internal)
		.IsCollapsedByParent(true)
		[
			SAssignNew(ComboButton, SButton)
			.ButtonStyle(&ButtonStyle)
			.ContentPadding(0)
			.OnClicked(this, &SSearchCombobox::OnComboClicked)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillContentWidth(1)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Left)
				[
					SNew(STextBlock)
					.Text_Lambda([this]() -> FText
					{
						return GetSelectedText();
					})
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Right)
				[
					SNew(SImage)
					.Image_Lambda([this]() -> const FSlateBrush*
					{
						const FSlateIcon& Icon = ComboButton->IsHovered()
							? FSlateIconFinder::FindIcon("DetailsView.PulldownArrow.Down.Hovered")
							: FSlateIconFinder::FindIcon("DetailsView.PulldownArrow.Down");

						return Icon.GetIcon();
					})
				]
			]
		]
	];
}

TSharedRef<SWidget> SSearchCombobox::GenerateMenuContent()
{
		const ISlateStyle* Style = FSlateStyleRegistry::FindSlateStyle("DatasmithContentEditorStyle");
	const FButtonStyle& FilterButtonStyle = Style->GetWidgetStyle<FButtonStyle>("DatasmithDataprepEditor.ButtonLeft");
	const FButtonStyle& NoneButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("Window.CloseButtonHover");
	
	return SNew(SBorder)
	.BorderImage(FSlateIconFinder::FindIcon("ToolTip.Background").GetIcon())
	[
		SNew(SVerticalBox)
		// Current Asset Slot
		+ SVerticalBox::Slot().AutoHeight()
		.Padding(10, 10, 10, 0)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(STextBlock)
					.Text(FText::FromString("SOURCE INFOS"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.ColorAndOpacity(FLinearColor(0.3529f, 0.3529f, 0.3529f, 1.0f))
				]
				+ SHorizontalBox::Slot().AutoWidth()
				.FillWidth(1.f)
				.VAlign(VAlign_Center)
				.Padding(8.f, 0.f, 0.f, 0.f)
				[
					SNew(SBox)
					.HeightOverride(1.f)
					[
						SNew(SImage)
						.ColorAndOpacity(FLinearColor(0.3529f, 0.3529f, 0.3529f, 1.0f)) // Couleur de la ligne
						.Image(FAppStyle::Get().GetBrush("WhiteBrush")) // Pinceau de base unicolore
					]
				]
			]
			+ SVerticalBox::Slot().AutoHeight()
			.Padding(5, 10, 0, 0)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(16)
					.HeightOverride(16)
					[
						SNew(SImage)
						.Image(FSlateIconFinder::FindIcon("DerivedData.RemoteCache.UnavailableBG").GetIcon())
					]
				]
				+ SHorizontalBox::Slot().AutoWidth()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				.Padding(5,0,0,0)
				[
					SNew(STextBlock)
					.Text(FText::FromString("Source : "))
				]
				+ SHorizontalBox::Slot().AutoWidth()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				.Padding(5,0,0,0)
				[
					SNew(STextBlock)
					.Text(FText::FromString(SourceName))
				]
			]
			+ SVerticalBox::Slot().AutoHeight()
			.Padding(5, 5, 0, 5)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(16)
					.HeightOverride(16)
					[
						SNew(SImage)
						.Image(FSlateIconFinder::FindIcon("Icons.ClassicFilterConfig").GetIcon())
					]
				]
				+ SHorizontalBox::Slot().AutoWidth()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				.Padding(5,0,0,0)
				[
					SNew(STextBlock)
					.Text(FText::FromString("Filters   : "))
				]
				+ SHorizontalBox::Slot().AutoWidth()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				.Padding(5,0,0,0)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Filters))
				]
			]
		]
		// Search Box Slot
		+ SVerticalBox::Slot().AutoHeight()
		.Padding(10, 10, 10, 0)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(STextBlock)
					.Text(FText::FromString("BROWSE"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.ColorAndOpacity(FLinearColor(0.3529f, 0.3529f, 0.3529f, 1.0f))
				]
				+ SHorizontalBox::Slot().AutoWidth()
				.FillWidth(1.f)
				.VAlign(VAlign_Center)
				.Padding(8.f, 0.f, 0.f, 0.f)
				[
					SNew(SBox)
					.HeightOverride(1.f)
					[
						SNew(SImage)
						.ColorAndOpacity(FLinearColor(0.3529f, 0.3529f, 0.3529f, 1.0f)) // Couleur de la ligne
						.Image(FAppStyle::Get().GetBrush("WhiteBrush")) // Pinceau de base unicolore
					]
				]
			]
			+ SVerticalBox::Slot().AutoHeight()
			.Padding(0, 10, 0, 0)
			[
				 SAssignNew(SearchBox, SSearchBox)
				.Style(&FAppStyle::Get().GetWidgetStyle<FSearchBoxStyle>("SearchBox"))
				.OnTextChanged(this, &SSearchCombobox::OnSearchTextChanged)
			]
		]
		// Content Box Slot
		+ SVerticalBox::Slot().AutoHeight()
		.Padding(0, 10, 0, 10)
		[
			SNew(SBorder)
			.BorderImage(FSlateIconFinder::FindIcon("AssetEditorToolbar.Background").GetIcon())
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				.Padding(5, 5)
				[
					SNew(SBorder)
					.BorderImage(FSlateIconFinder::FindIcon("Brushes.Background").GetIcon())
					[
						SNew(SBox)
						.MinDesiredHeight(200)
						.MaxDesiredHeight(200)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()
							.VAlign(VAlign_Center)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().AutoWidth()
								.Padding(0, 0, 0, 1)
								.FillContentWidth(1)
								[
									SNew(SButton)
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Center)
									.ButtonStyle(&NoneButtonStyle)
									.ContentPadding(5)
									.IsEnabled_Lambda([this] () -> bool
									{
										return SelectedItem.IsValid() && !SelectedItem.Get()->IsEmpty();
									})
									.OnClicked_Lambda([this]() -> FReply
									{
										OnSelectionChanged_Internal(MakeShared<FString>(""), ESelectInfo::Type::OnMouseClick);
										return FReply::Handled();
									})
									[
										SNew(STextBlock)
										.Text(FText::FromString("|   None   |"))
										.Font(IDetailLayoutBuilder::GetDetailFont())
									]
								]
								+ SHorizontalBox::Slot().AutoWidth()
								[
									SNew(SSpacer)
									.Size(FVector2D(10, 0))
								]
								+ SHorizontalBox::Slot().AutoWidth()
								.Padding(0, 0, 0, 1)
								[
									SNew(SButton)
									.ContentPadding(0)
									.ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Center)
									.OnClicked_Lambda([this]() -> FReply
									{
										bSortAscending = !bSortAscending;

										FilteredOptions.Sort([this](const TSharedPtr<FString>& A, const TSharedPtr<FString>& B)
										{
											if (!A.IsValid() || !B.IsValid()) return false;
											return bSortAscending ? (*A < *B) : (*A > *B);
										});

										SetOptions(FilteredOptions);
										if (ListView.IsValid())
										{
											ListView->RequestListRefresh();
										}
										return FReply::Handled();
									})
									.ButtonStyle(&FilterButtonStyle)
									[
										SNew(SBox)
										.WidthOverride(8)
										.HeightOverride(8)
										[
											SNew(SImage)
											.Image_Lambda([this] () -> const FSlateBrush*
											{
												return FSlateIconFinder::FindIcon(bSortAscending ? "Icons.ChevronDown" : "Icons.ChevronUp").GetIcon();
											})
										]
									]
								]
							]
							+ SVerticalBox::Slot().AutoHeight()
							.FillHeight(1)
							[
								SAssignNew(ListView, SListView<TSharedPtr<FString>>)
								.ListItemsSource(&DisplayOptions)
								.OnGenerateRow(OnGenerateWidget)
								.OnSelectionChanged(this, &SSearchCombobox::OnSelectionChanged_Internal)
							]
						]
					]
				]
				+ SVerticalBox::Slot().AutoHeight()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Bottom)
				.Padding(FMargin(5, 10, 5, 5))
				[
					SNew(STextBlock)
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.Text_Lambda([this] () -> FText
					{
						const FString Result = FString::Printf(TEXT("%d items %s"), FilteredOptions.Num(), SelectedItem.IsValid() && !SelectedItem.Get()->IsEmpty() ? TEXT("(1 selected)") : TEXT(""));
						return FText::FromString(Result);
					})
				]
			]
		]
	];
}

void SSearchCombobox::OnSearchTextChanged(const FText& InSearchText)
{
	FilteredOptions.Empty();
	const FString Filter = InSearchText.ToString();
	for (const TSharedPtr<FString>& Option : *OptionsSource)
	{
		if (Filter.IsEmpty() || (Option.IsValid() && Option->Contains(Filter)))
		{
			FilteredOptions.Add(Option);
		}
	}
	SetOptions(FilteredOptions);
	if (ListView.IsValid())
	{
		ListView->RequestListRefresh();
	}
}

FReply SSearchCombobox::OnComboClicked()
{
	if (!bMenuAnchorIsOpen)
	{
		MenuAnchor->SetIsOpen(true, false);
		bMenuAnchorIsOpen = true;
		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([SearchBoxRef = SearchBox](float)
		{
			if (SearchBoxRef.IsValid())
			{
				SearchBoxRef->SetText(FText::GetEmpty());
				FSlateApplication::Get().SetKeyboardFocus(SearchBoxRef);
			}
			return false;
		}), 0.01f);
		return FReply::Handled();
	}
	MenuAnchor->SetIsOpen(false, false);
	bMenuAnchorIsOpen = false;
	return FReply::Handled();
}

void SSearchCombobox::OnSelectionChanged_Internal(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	if (NewSelection.IsValid() && !NewSelection->IsEmpty())
	{
		if (bKeepFormat)
		{
			SelectedItem = NewSelection;
		}
		else
		{
			// Format inverse : retrouver l'option brute depuis le texte affiché
			const FString& Formatted = *NewSelection;
			for (const TSharedPtr<FString>& Option : FilteredOptions)
			{
				if (FormatOptionText(*Option, false).ToString() == Formatted)
				{
					SelectedItem = Option;
					break;
				}
			}
		}
	}
	else
	{
		SelectedItem = MakeShared<FString>(""); // Vide
	}

	if (OnSelectionChanged.IsBound())
	{
		OnSelectionChanged.Execute(SelectedItem, SelectInfo); // Passer la vraie option brute
	}

	MenuAnchor->SetIsOpen(false);
	bMenuAnchorIsOpen = false;
}

void SSearchCombobox::OnMenuOpenChanged_Internal(const bool bIsOpen)
{
	if (OnMenuOpenChanged.IsBound())
	{
		OnMenuOpenChanged.Execute();
	}
	if (bIsOpen)
	{
		return;
	}
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this](float)
	{
		bMenuAnchorIsOpen = false;
		return false;
	}), 0.2f);
}

FText SSearchCombobox::GetSelectedText() const
{
	if (SelectedItem.IsValid() && !SelectedItem.Get()->IsEmpty())
	{
		return FormatOptionText(*SelectedItem, bKeepFormat);
	}
	return FText::FromString(DefaultOption);
}

FText SSearchCombobox::FormatOptionText(const FString& InString, bool bKeepFormat)
{
	if (bKeepFormat)
	{
		return FText::FromString(InString);
	}

	FString Source = InString.ToLower();
	Source.ReplaceInline(TEXT("_"), TEXT(" "));

	TArray<FString> Words;
	Source.ParseIntoArray(Words, TEXT(" "), true);

	for (FString& Word : Words)
	{
		if (!Word.IsEmpty())
		{
			Word[0] = FChar::ToUpper(Word[0]);
		}
	}

	const FString FinalName = FString::Join(Words, TEXT(" "));
	return FText::FromString(FinalName);
}

void SSearchCombobox::SetOptions(const TArray<TSharedPtr<FString>>& InOptions)
{
	if (bKeepFormat) { DisplayOptions = InOptions; return; }
	
	TArray<TSharedPtr<FString>> FormatOptions;
	for (const auto& Option : InOptions)
	{
		if (Option.IsValid())
		{
			FormatOptions.Add(MakeShared<FString>(FormatOptionText(*Option, false).ToString()));
		}
	}
	DisplayOptions = FormatOptions;
}