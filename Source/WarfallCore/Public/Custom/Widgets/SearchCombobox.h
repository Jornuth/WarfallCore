#pragma once

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"

/**
 * @class SSearchCombobox
 * @brief Represents a customizable search-enabled combo box.
 *
 * The SSearchCombobox class provides a user interface component that offers
 * a combination of a drop-down list and a search functionality. It enables
 * users to filter or search within the list as they type, improving usability
 * for handling a large number of selectable items.
 *
 * This class is typically used in applications requiring a dynamic and efficient
 * method to search and select an item from a list.
 *
 * Features:
 * - Dropdown menu for displaying selectable items.
 * - Inline search functionality to filter the items in real-time.
 * - Customizable behavior for search queries and result display.
 *
 * Thread Safety:
 * - This class is not thread-safe. It should be accessed from a single UI thread.
 *
 * Note:
 * - Ensure that the data source attached to the component is updated properly if
 *   dynamic changes to the item list are expected.
 *
 * Common Use Cases:
 * - Autocomplete fields in forms.
 * - Searching and selecting items from a large dataset.
 *
 */
class SSearchCombobox : public SCompoundWidget
{
private:
	DECLARE_DELEGATE_RetVal_TwoParams(TSharedRef<ITableRow>, FOnGenerateRowForCombo, TSharedPtr<FString>, const TSharedRef<STableViewBase>&);
	DECLARE_DELEGATE_TwoParams(FOnComboSelectionChanged, TSharedPtr<FString>, ESelectInfo::Type);

public:
	SLATE_BEGIN_ARGS(SSearchCombobox) {}
	SLATE_ARGUMENT(TArray<TSharedPtr<FString>>*, Options)
	SLATE_ARGUMENT(TSharedPtr<FString>, CurrentItem)
	SLATE_ARGUMENT(bool, KeepFormat)
	SLATE_ARGUMENT(FString, DefaultOption)
	SLATE_ARGUMENT(FString, SourceName)
	SLATE_ARGUMENT(FString, Filters)
	SLATE_EVENT(FOnGenerateRowForCombo, OnGenerateWidget)
	SLATE_EVENT(FOnComboSelectionChanged, OnSelectionChanged)
	SLATE_EVENT(FSimpleDelegate, OnMenuOpenChanged)	
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TArray<TSharedPtr<FString>>*					OptionsSource = nullptr;
	TArray<TSharedPtr<FString>>						FilteredOptions;
	TArray<TSharedPtr<FString>>						DisplayOptions;

	TSharedPtr<SMenuAnchor>							MenuAnchor;
	TSharedPtr<SEditableTextBox>					SearchBox;
	TSharedPtr<SListView<TSharedPtr<FString>>>		ListView;
	TSharedPtr<SButton>								ComboButton;

	TSharedPtr<FString>								SelectedItem;

	bool bMenuAnchorIsOpen = false;
	bool bKeepFormat = false;
	bool bSortAscending = true;

	FString DefaultOption = "Select an option";
	FString SourceName = "";
	FString Filters = "";

	FOnGenerateRowForCombo OnGenerateWidget;
	FOnComboSelectionChanged OnSelectionChanged;
	FSimpleDelegate OnMenuOpenChanged;

	TSharedRef<SWidget> GenerateMenuContent();
	void SetOptions(const TArray<TSharedPtr<FString>>& InOptions);
	void OnSearchTextChanged(const FText& InSearchText);
	FReply OnComboClicked();
	void OnSelectionChanged_Internal(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);
	void OnMenuOpenChanged_Internal(bool bIsOpen);
	FText GetSelectedText() const;
	static FText FormatOptionText(const FString& InString, bool bKeepFormat);
};
