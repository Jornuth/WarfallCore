#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Utils/GlobalTools.h"
#include "MassCalculator.generated.h"

class SSlider;
class FDetailMessageRow;

static const TMap<FName, float> SolidDensityTable =
{
	{ "Steel",    7850.f },
	{ "Bronze",   8900.f },
	{ "Silver",  10500.f },
	{ "Gold",    19300.f },
	{ "Wood",      600.f },
	{ "Clothes",   150.f },
	{ "Paper",    1200.f },
	{ "Leather",   600.f },
	{ "Skin",     1010.f },
};

static const TMap<FName, float> HollowDensityTable =
{
	{ "Steel",    6000.f },
	{ "Bronze",   6500.f },
	{ "Silver",   7500.f },
	{ "Gold",    13000.f },
	{ "Wood",      300.f },
	{ "Clothes",    80.f },
	{ "Paper",     600.f },
	{ "Leather",   300.f },
	{ "Skin",      600.f },
};

USTRUCT(BlueprintType)
struct FMassRatio
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = false, EditConditionHides))
	FName Material;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0", EditCondition = false, EditConditionHides))
	float Percentage;

private:
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	bool bIsSolid;

public:
	FMassRatio() :
	 Material(NAME_None)
	,Percentage(0.f)
	,bIsSolid(true)
	{}

	float& GetPercentage() { return Percentage; }
	FName& GetMaterial() { return Material; }
	bool& GetIsSolid() { return bIsSolid; }
	
	float GetDensity() const
	{
		if (Percentage == 0.0f) return 0.0f;
		const float Ratio = Percentage / 100.0f;
		if (bIsSolid)
		{
			if (SolidDensityTable.Contains(Material))
			{
				return SolidDensityTable[Material] * Ratio;
			}
		}
		else
		{
			if (HollowDensityTable.Contains(Material))
			{
				return HollowDensityTable[Material] * Ratio;
			}
		}
		return 0.0f;
	}
	
	FMassRatio& operator = (const FMassRatio& Other)
	{
		if (this != &Other)
		{
			Material = Other.Material;
			Percentage = Other.Percentage;
			bIsSolid = Other.bIsSolid;
		}
		return *this;
	}
};

USTRUCT(BlueprintType)
struct FMassObject
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FixedWeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AirFriction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Friction;
		
	UPROPERTY(EditAnywhere)
	UStaticMesh* StaticMesh;
	
	UPROPERTY(EditAnywhere)
	USkeletalMesh* SkeletalMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMassRatio> Materials;
	
private:
	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	int32 Type;

public:
	FMassObject() :
	 FixedWeight(0.0f)
	,AirFriction(0.01f)
	,Friction(0.f)
	,StaticMesh(nullptr)
	,SkeletalMesh(nullptr)
	,Materials(TArray<FMassRatio>())
	,Type(0)
	{}
	
	int32& GetType() { return Type; }
	FName GetTypeOption() const { return Type == 0 ? "Fixed" : "Dynamic"; }
	UStaticMesh*& GetStaticMesh() { return StaticMesh; }
	USkeletalMesh*& GetSkeletalMesh() { return SkeletalMesh; }
	
	float GetDensity()
	{
		if (Materials.IsEmpty()) return 0.0f;

		float Total = 0.0f;
		for (const FMassRatio& Material : Materials)
		{
			if (Material.Material.IsNone()) continue;
			Total += Material.GetDensity();
		}
		return Total;
	}
	
	float GetMass() 
	{
		if (Type == 0) return FixedWeight;
		if (!GetStaticMesh() && !GetSkeletalMesh()) return 0.0f;
		return UGlobalTools::MassObjectInKg(GetStaticMesh(), GetSkeletalMesh(), GetDensity());
	}
	
	FMassObject& operator = (const FMassObject& Other)
	{
		if (this != &Other)
		{
			FixedWeight = Other.FixedWeight;
			AirFriction = Other.AirFriction;
			Friction = Other.Friction;
			StaticMesh = Other.StaticMesh;
            SkeletalMesh = Other.SkeletalMesh;
			Materials = Other.Materials;
			Type = Other.Type;
		}
		return *this;
	}
};

// ===============================[ Property Customization Factory ]============================
// ======================================== FMASSRATIO =========================================
// =============================================================================================

class WARFALLCORE_API FCustomMassRatio : public IPropertyTypeCustomization
{
	// ========== FUNCTIONS ==========
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShareable(new FCustomMassRatio); }
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle,IDetailChildrenBuilder& ChildBuilder,
								   IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

protected:
	void RefreshOptions();
	float GetPercentage() const;
	void SetPercentage(float NewValue) const;
	
	void BalancedAfterSelection() const;
	static TSharedPtr<FString> GetDefaultOption() { return MakeShared<FString>(TEXT("- Select a material -")); }
	
	// ========== VARIABLES ==========
	TSharedPtr<IPropertyHandle> PropertyHandle;
	TSharedPtr<IPropertyUtilities> PropertyUtils;
	TSharedPtr<TArray<TSharedPtr<FString>>> MaterialsHandles;
		
	TSharedPtr<SVerticalBox> BoxContent;
	TSharedPtr<SSlider> PercentageSlider;

protected:
	FMassRatio* Handle = nullptr;
};

// ===============================[ Property Customization Factory ]============================
// ======================================== FMASSOBJECT ========================================
// =============================================================================================

class WARFALLCORE_API FCustomMassObject : public IPropertyTypeCustomization
{
	// ========== FUNCTIONS ==========
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShareable(new FCustomMassObject); }
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle,IDetailChildrenBuilder& ChildBuilder,
								   IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

protected:
	void RefreshOptions();

	// ========== VARIABLES ==========
	TSharedPtr<IPropertyHandle> PropertyHandle;
	TSharedPtr<IPropertyUtilities> PropertyUtils;
	TSharedPtr<TArray<TSharedPtr<FString>>> TypesHandles;
	
	TSharedPtr<FDetailMessageRow> DetailMessage;
	
protected:
	FMassObject* Handle = nullptr;
};