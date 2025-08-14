#pragma once

#include "CoreMinimal.h"
#include "BaseHandle.generated.h"

class FDetailMessageRow;
enum class ETablePath;

USTRUCT(NotBlueprintType)
struct FBaseHandle
{
	GENERATED_BODY()

private:
	FName Base_ID = NAME_None;
	FName Base_Tag = NAME_None;

public:
	FBaseHandle() = default;

private:
	DECLARE_DELEGATE(FMarkToRefresh);

public:
	FMarkToRefresh OnMarkToRefresh;

	static FText GetName(const FName RowName)
	{
		if (RowName.IsNone()) return FText::FromString("None");

		FString Source = RowName.ToString().ToLower();
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

	void MarkToRefresh() const
	{
		if (OnMarkToRefresh.IsBound())
		{
			OnMarkToRefresh.Execute();
		}
	}
	
	virtual FName& GetID() { return Base_ID; }
	virtual FName& GetTag() { return Base_Tag; }
	
	FBaseHandle& operator = (const FBaseHandle& Other)
	{
		if (this != &Other)
		{
			Base_ID = Other.Base_ID;
			Base_ID = Other.Base_ID;
		}
		return *this;
	}
	virtual ~FBaseHandle() {}
};

class WARFALLCORE_API FCustomBaseHandle : public IPropertyTypeCustomization
{
	// ========== FUNCTIONS ==========
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShareable(new FCustomBaseHandle); }
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle,IDetailChildrenBuilder& ChildBuilder,
								   IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

protected:
	virtual FString GetDefaultOption() const { return "row"; }
	virtual FString DefaultFilter() const;
	virtual bool GetFilterConditions(const FName RowName, const FName HandleTag) const { return true; }
	virtual ETablePath GetDataTable();
	
private:
	TSharedPtr<FString> DefaultOption() const { return MakeShared<FString>(FString::Printf(TEXT("- Select %s -"), *GetDefaultOption())); }
	
	void RefreshHandle();
	void CreateSearchCombobox() const;

	// ========== VARIABLES ==========
	TSharedPtr<IPropertyHandle> PropertyHandle;
	TSharedPtr<IPropertyUtilities> PropertyUtils;
	TSharedPtr<TArray<TSharedPtr<FString>>> RowHandles;
	TArray<TSharedPtr<FString>> FilteredRowHandles;
		
	TSharedPtr<SVerticalBox> BoxContent;
	TSharedPtr<SListView<TSharedPtr<FString>>> ListView;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> ComboBox;
	TSharedPtr<SButton> RefreshButton;

protected:
	FBaseHandle* Handle = nullptr;
	UDataTable* DataTable = nullptr;
};