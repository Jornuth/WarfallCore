#pragma once

#include "CoreMinimal.h"
#include "ColorPicker.generated.h"

class FDetailMessageRow;
/**
 * @brief A structure that handles color representation and management using stencil indices.
 *
 * This structure provides the functionality to manage and validate stencil-based color indices.
 * It also allows the retrieval of corresponding colors using a stencil index.
 */
USTRUCT(BlueprintType)
struct FColorPicker
{
	GENERATED_BODY()

	int32 FirstStencil = 1;
	int32 NumStencil = 52;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, DisplayName = "Color Index", meta = (EditCondition = false, EditConditionHides))
	int32 Stencil = FirstStencil;
		
	FColorPicker() = default;

	explicit FColorPicker(const int32 DefaultIndex) :
		Stencil(DefaultIndex)
	{}
	
	void UpdateIndex(const int32 Index) { Stencil = Index; }
	bool IsValidColor() const { return Stencil != INDEX_NONE && Stencil > FirstStencil && Stencil <= NumStencil; }
	FLinearColor GetColor(const int32 InStencil = INDEX_NONE) const;
	FColorPicker& operator = (const FColorPicker& Other)
	{
		if (this != &Other)
		{
			Stencil = Other.Stencil;
		}
		return *this;
	}
};

class WARFALLCORE_API FCustomColorPicker : public IPropertyTypeCustomization
{
	// ========== FUNCTIONS ==========
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShareable(new FCustomColorPicker); }
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle,IDetailChildrenBuilder& ChildBuilder,
								   IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	
private:
	TSharedRef<SWidget> CreatePickupColor();
	int32 GetStencilValue() const;
	FLinearColor GetColor(const int Stencil) const;
	void UpdateColor(const int Stencil) const;
	void RefreshColor();
	
	// ========== VARIABLES ==========
	
	TSharedPtr<IPropertyHandle> PropertyHandle;
	TSharedPtr<IPropertyUtilities> PropertyUtils;
	
	TSharedPtr<FDetailMessageRow> DetailMessage;
	mutable TSharedPtr<SVerticalBox> ColorPanel;
	
	TSharedPtr<SButton> RefreshButton;
	TSharedPtr<FSlateDynamicImageBrush> PaintBrush;
	
	
	int32 HoveredStencil = INDEX_NONE;
	
	FColorPicker* Handle = nullptr;
};