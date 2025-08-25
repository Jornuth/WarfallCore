#pragma once

#include "CoreMinimal.h"
#include "BaseHandle.h"
#include "ChildsHandle.generated.h"

// ===============================[ Property Customization Factory ]============================
// ====================================== FITEMROWHANDLE =======================================
// =============================================================================================

USTRUCT(BlueprintType)
struct FItemRowHandle : public FBaseHandle
{
	GENERATED_BODY()
	

	/**
	 * Unique identifier for a specific row in a data table.
	 * 
	 * Read-only property visible in editor and blueprints but not manually editable.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (EditCondition = false, EditConditionHides))
	FName ID = NAME_None;

	/**
	 * Tag associated with the current handle instance, serving as a unique identifier
	 * for grouping or categorizing data rows within a data table.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (EditCondition = false, EditConditionHides))
	FName Tag = "Ammunition";

	FItemRowHandle() = default;
	explicit FItemRowHandle(const FName InTag) :
	 Tag(InTag)
	{}

	virtual FName& GetID() override { return ID; }
	virtual FName& GetTag() override { return Tag; }
	
	FItemRowHandle& operator = (const FItemRowHandle& Other)
	{
		if (this != &Other)
		{
			ID = Other.ID;
			Tag = Other.Tag;
		}
		return *this;
	}
};

class WARFALLCORE_API FCustomItemRowHandle : public FCustomBaseHandle
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShareable(new FCustomItemRowHandle); }
	virtual FString GetDefaultOption() const override { return "an item"; };
	virtual bool GetFilterConditions(const FName RowName, const FName HandleTag) const override;
	virtual ETablePath GetDataTable() override;
	virtual FString DefaultFilter() const override;
};