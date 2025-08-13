#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Utils/GlobalTools.h"
#include "Thumbnail.generated.h"

class FDetailMessageRow;
class AThumbnailMaker;
class SThumbnailPilote;
/**
 * Struct holding all necessary data for building a thumbnail preview.
 * Manages textures, dimensions, mesh references, location, rotation, and scaling.
 */
USTRUCT(BlueprintType)
struct FThumbnail
{
	GENERATED_BODY()

	/** Final texture used as a thumbnail */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Thumbnail;
	
	/** Optional layer texture overlay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Layer;
	
	/** Material instance dynamically created at runtime */
	UPROPERTY()
	UMaterialInstanceDynamic* Material;
	
	UPROPERTY()
	AThumbnailMaker* ThumbnailMaker = nullptr;
	FThumbnail* Temp = nullptr;
	TSharedPtr<SThumbnailPilote> ThumbnailPilote;

private:
	/** Internal thumbnail asset name */
	UPROPERTY(VisibleAnywhere, meta=(AllowPrivateAccess="true"))
	FString Name;
	
	/** Desired dimensions for the thumbnail */
	UPROPERTY(VisibleAnywhere, meta=(AllowPrivateAccess="true"))
	FIntPoint Dimensions;
	
	/** Static mesh reference for the capture */
	UPROPERTY(VisibleAnywhere, meta=(AllowPrivateAccess="true"))
	UStaticMesh* Mesh;

	/** Skeletal mesh reference for the capture */
	UPROPERTY(VisibleAnywhere, meta=(AllowPrivateAccess="true"))
	USkeletalMesh* SkeletalMesh;

	/** Local rotation (Euler) of the thumbnail object */
	UPROPERTY(VisibleAnywhere, meta=(AllowPrivateAccess="true"))
	FRotator RotationEuler;

	/** Local location (translation) of the thumbnail object */
	UPROPERTY(VisibleAnywhere, meta=(AllowPrivateAccess="true"))
	FVector Location;

	/** Local scale applied to the thumbnail object */
	UPROPERTY(VisibleAnywhere, meta=(AllowPrivateAccess="true"))
	FVector Scale;

	UPROPERTY(VisibleAnywhere, meta=(AllowPrivateAccess="true"))
	bool bWasRefresh = false;

	/** Lock uniform scaling between X/Y/Z axes */
	UPROPERTY(VisibleAnywhere, meta=(AllowPrivateAccess="true"))
	bool bScaleLocked = false;

	/** Is using reverse rotation. */
	UPROPERTY(VisibleAnywhere, meta=(AllowPrivateAccess="true"))
	bool bReverseRotation = false;

	/** Is rotating */
	UPROPERTY(VisibleAnywhere, meta=(AllowPrivateAccess="true"))
	bool bRotate = false;
	
	DECLARE_DELEGATE(FOpen);
	DECLARE_DELEGATE_OneParam(FChanged, FName);
	
public:

	FOpen OnOpen;
	FChanged OnChanged;
	
	FThumbnail() :
	 Thumbnail(nullptr)
	,Layer(nullptr)
	,Material(nullptr)
	,Name("Default")
	,Dimensions(1,1)
	,Mesh(nullptr)
	,SkeletalMesh(nullptr)
	,RotationEuler(FRotator::ZeroRotator)
	,Location(FVector::ZeroVector)
	,Scale(FVector::OneVector)
	{}

	UMaterialInstanceDynamic* CreateMaterial()
	{
		if (!UGlobalTools::GetMaterial(EMaterialPath::Thumbnail) || !Thumbnail) return nullptr;
		
		UMaterialInstanceDynamic* NewMaterial = UMaterialInstanceDynamic::Create(UGlobalTools::GetMaterial(EMaterialPath::Thumbnail), nullptr);
		NewMaterial->SetTextureParameterValue("Icon", Thumbnail);
		NewMaterial->SetScalarParameterValue("HasLayer", 0.f);
		NewMaterial->SetScalarParameterValue("Rotation", bRotate ? bReverseRotation ? -0.25f : 0.25f : 0.f);
		if (!Layer) return Material = NewMaterial;
		NewMaterial->SetScalarParameterValue("HasLayer", 1.f);
		NewMaterial->SetTextureParameterValue("Layer", Layer);
		return Material = NewMaterial;;
	}

	void RegisterThumbnail(FThumbnail* InThumbnail)
	{
		Thumbnail = InThumbnail->Thumbnail;
		Name = InThumbnail->Name;
		Dimensions = InThumbnail->Dimensions;
		Mesh = InThumbnail->Mesh;
		SkeletalMesh = InThumbnail->SkeletalMesh;
		RotationEuler = InThumbnail->RotationEuler;
		Location = InThumbnail->Location;
		Scale = InThumbnail->Scale;
		bScaleLocked = InThumbnail->bScaleLocked;
		bReverseRotation = InThumbnail->bReverseRotation;
	}

	void UpdateCustomizer();

	void NotifyOpen() const
	{
		if (OnOpen.IsBound())
		{
			OnOpen.Execute();
		}
	}
	
	void NotifyChanged(const FName Tag) const
	{
		if (OnChanged.IsBound())
		{
			OnChanged.Execute(Tag);
		}
	}
		
	void OverrideRefresh(const bool bRefresh)
	{
		bWasRefresh = bRefresh;
	}
		
	FString& GetName()
	{
		return Name;
	}
	
	FIntPoint& GetDimensions()
	{
		return Dimensions;
	}
	
	UStaticMesh*& GetMesh()
	{
		return Mesh;
	}
	
	USkeletalMesh*& GetSkeletalMesh()
	{
		return SkeletalMesh;
	}
	
	FRotator& GetRotationEuler()
	{
		return RotationEuler;
	}
	
	FVector& GetLocation()
	{
		return Location;
	}
	
	FVector& GetScale()
	{
		return Scale;
	}
	
	bool& GetWasRefresh()
	{
		return bWasRefresh;
	}

	bool& GetScaleLocked()
	{
		return bScaleLocked;
	}

	bool& GetReverseRotation()
	{
		return bReverseRotation;
	}

	bool& GetRotate()
	{
		return bRotate;
	}

	UMaterialInstanceDynamic*& GetMaterial()
	{
		if (Material)
		{
			Material->SetScalarParameterValue("Rotation", bRotate ? bReverseRotation ? -0.25f : 0.25f : 0.f);
		}
		return Material;
	}

	void ToggleRotation()
	{
		bRotate = !bRotate;
		if (Material)
		{
			Material->SetScalarParameterValue("Rotation", bRotate ? bReverseRotation ? -0.25f : 0.25f : 0.f);
		}
	}
	void OverrideRotation(const bool bRotation)
	{
		bRotate = bRotation;
		if (Material)
		{
			Material->SetScalarParameterValue("Rotation", bRotate ? bReverseRotation ? -0.25f : 0.25f : 0.f);
		}
	}

	bool HasChangedComparedTo(const FThumbnail& Other) const
	{
		return
			Name != Other.Name ||
			Dimensions != Other.Dimensions ||
			Mesh != Other.Mesh ||
			SkeletalMesh != Other.SkeletalMesh ||
			RotationEuler != Other.RotationEuler ||
			Location != Other.Location ||
			Scale != Other.Scale;
	}

	FThumbnail& operator = (const FThumbnail& Other)
	{
		if (this != &Other)
		{
			Thumbnail = Other.Thumbnail;
			Layer = Other.Layer;
			Name = Other.Name;
			Dimensions = Other.Dimensions;
			Mesh = Other.Mesh;
			SkeletalMesh = Other.SkeletalMesh;
			RotationEuler = Other.RotationEuler;
			Location = Other.Location;
			Scale = Other.Scale;
			bScaleLocked = Other.bScaleLocked;
			bReverseRotation = Other.bReverseRotation;
			bRotate = Other.bRotate;
			Temp = Other.Temp;
		}
		return *this;
	}
};

class FCustomThumbnail : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle,
								 FDetailWidgetRow& HeaderRow,
								 IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle,
								   IDetailChildrenBuilder& ChildBuilder,
								   IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:
	TSharedRef<SWidget> NameContentWidget();
	TSharedRef<SWidget> ValueContentWidget();
	TSharedRef<SWidget> InputsControlsWidget();
	
	FReply OnFinalizeClicked();
	void ClearAndInvalidateThumbnail();
	void Refresh();
	
	void TogglePreviewVisibility();
	void AssignProperties();
	bool IsValidEntry();

	template<typename StructType>
	static StructType* GetStructPropertyPtr(const TSharedRef<IPropertyHandle> Handle)
	{
		const FStructProperty* StructProp = CastField<FStructProperty>(Handle->GetProperty());

		if (void* RawData = nullptr; StructProp && Handle->GetValueData(RawData) == FPropertyAccess::Success)
		{
			return static_cast<StructType*>(RawData);
		}

		return nullptr;
	}
		
	AThumbnailMaker* ThumbnailMaker = nullptr;
	FThumbnail* ThumbnailHandle = nullptr;
	FThumbnail ThumbnailTemp;

	TSharedPtr<FSlateDynamicImageBrush> LeftMouseBrush;
	TSharedPtr<FSlateDynamicImageBrush> RightMouseBrush;
	TSharedPtr<FSlateDynamicImageBrush> WheelMouseBrush;
	TSharedPtr<FSlateDynamicImageBrush> MoveAzertyBrush;
	TSharedPtr<FSlateDynamicImageBrush> MoveQwertyBrush;
	
	bool bIsCreatorVisible = false;
	bool bIsControlsVisible = false;
	TSharedPtr<SThumbnailPilote> ThumbnailPilote;
	TSharedPtr<SButton> RefreshButton;
	TSharedPtr<SOverlay> MessageBanner;
	
	TSharedPtr<IPropertyHandle> StructHandle;
	TSharedPtr<IPropertyUtilities> PropertyUtils;
	TSharedPtr<FDetailMessageRow> DetailMessage;
		
};
