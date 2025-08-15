#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityActor.h"
#include "Utils/GlobalTools.h"

#include "ThumbnailMaker.generated.h"

struct FThumbnail;
class SThumbnailPilote;
class URectLightComponent;

// ==========================================================================
//  ATHUMBNAILMAKER
// ==========================================================================

UCLASS()
class WARFALLCORE_API AThumbnailMaker : public AEditorUtilityActor
{
	GENERATED_BODY()

	// ========== FUNCTIONS ==========
public:
	AThumbnailMaker();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Destroyed() override;

	UTextureRenderTarget2D* CreateRenderTarget();
	UTextureRenderTarget2D* GetRenderTarget() const { return RenderTarget; };
	FVector GetRenderScale() const;

	UFUNCTION(CallInEditor, Category = "Default")
	void SaveRenderTargetToDisk() const;
	UFUNCTION(CallInEditor, Category = "Thumbnail")
	void ImportAndAssignTextureToRow(const FString& FileName) const;
	
	void UpdateThumbnailMaker(FThumbnail* InThumbnailRessource, SThumbnailPilote* InPilote);
	void ClearThumbnailMaker();
	void AfterInit();
	void UpdateMesh() const;
	void UpdateTransform() const;

	void Refresh() const;
	
	// ========== VARIABLES ==========
	FThumbnail* ThumbnailRessource = nullptr;
	SThumbnailPilote* PilotePointer = nullptr;
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial = nullptr;
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* DefaultSceneRoot;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* MeshObject;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,Category = "Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* SkeletalObject;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Background;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Render;
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneCaptureComponent2D* Camera;
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,Category = "Components", meta = (AllowPrivateAccess = "true"))
	URectLightComponent* Light;
	
	UPROPERTY()
	UTextureRenderTarget2D* RenderTarget;
	UPROPERTY()
	UMaterialInterface* RenderMaterial;
	UPROPERTY()
	UMaterialInterface* PlaneMaterial = nullptr;
	FTransform CameraDefaultTransform;
	FTransform DefaultMeshTransform = FTransform::Identity;
	
};

// ==========================================================================
//  STHUMBNAILPILOTE
// ==========================================================================

class WARFALLCORE_API SThumbnailPilote : public SBorder
{
public:
	SLATE_BEGIN_ARGS(SThumbnailPilote)
		: _Content()
		, _HAlign(HAlign_Fill)
		, _VAlign(VAlign_Fill)
		, _ContentPadding(FMargin(4.0, 2.0))
		, _ClickMethod(EButtonClickMethod::DownAndUp)
		, _TouchMethod(EButtonTouchMethod::DownAndUp)
		, _PressMethod(EButtonPressMethod::DownAndUp)
		, _DesiredSizeScale(FVector2D(1, 1))
		, _ContentScale(FVector2D(1, 1))
		, _IsFocusable(true)
		{
		}

	SLATE_DEFAULT_SLOT(FArguments, Content)
	SLATE_ARGUMENT(EHorizontalAlignment, HAlign)
	SLATE_ARGUMENT(EVerticalAlignment, VAlign)
	SLATE_ATTRIBUTE(FMargin, ContentPadding)
	SLATE_EVENT(FOnClicked, OnClicked)
	SLATE_EVENT(FSimpleDelegate, OnPressed)
	SLATE_EVENT(FSimpleDelegate, OnReleased)
	SLATE_EVENT(FSimpleDelegate, OnHovered)
	SLATE_EVENT(FSimpleDelegate, OnUnhovered)
	SLATE_ARGUMENT(EButtonClickMethod::Type, ClickMethod)
	SLATE_ARGUMENT(EButtonTouchMethod::Type, TouchMethod)
	SLATE_ARGUMENT(EButtonPressMethod::Type, PressMethod)
	SLATE_ATTRIBUTE(FVector2D, DesiredSizeScale)
	SLATE_ATTRIBUTE(FVector2D, ContentScale)
	SLATE_ARGUMENT(bool, IsFocusable)
	SLATE_END_ARGS()

	bool IsPressed() const
	{
		return bIsPressed;
	}
	
	void Construct(const FArguments& InArgs);
	
	void ClearAndInvalidate();
	void UpdateThumbnailMaker(AThumbnailMaker* InThumbnailMaker, FThumbnail* InThumbnailRessource);

	void SetOnClicked(const FOnClicked& InOnClicked);
	void SetOnHovered(const FSimpleDelegate& InOnHovered);
	void SetOnUnhovered(const FSimpleDelegate& InOnUnhovered);
	
	void SetClickMethod(EButtonClickMethod::Type InClickMethod);
	void SetTouchMethod(EButtonTouchMethod::Type InTouchMethod);
	void SetPressMethod(EButtonPressMethod::Type InPressMethod);
	
	//~ SWidget overrides
	virtual bool SupportsKeyboardFocus() const override;
	virtual void OnFocusLost( const FFocusEvent& InFocusEvent ) override;
	virtual FReply OnKeyDown( const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent ) override;
	virtual FReply OnKeyUp( const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent ) override;
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent ) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDown( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent ) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply OnMouseButtonUp( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent ) override;
	virtual void OnMouseCaptureLost(const FCaptureLostEvent& CaptureLostEvent) override;
	virtual bool IsInteractable() const override;
	virtual TOptional<EMouseCursor::Type> GetCursor() const override;

protected:
	virtual void Press();
	virtual void Release();
	FReply ExecuteOnClick();
	TEnumAsByte<EButtonClickMethod::Type> GetClickMethodFromInputType(const FPointerEvent& MouseEvent) const;

	void SetIsFocusable(bool bInIsFocusable)
	{
		bIsFocusable = bInIsFocusable;
	}
	void SetupGizmoTextures();
	
private:
	FVector2D PressedScreenSpacePosition = FVector2D(0, 0);
	float DesiredPreviewScale = 324.f;
	
	FOnClicked OnClicked;
	FSimpleDelegate OnPressed;
	FSimpleDelegate OnReleased;
	FSimpleDelegate OnHovered;
	FSimpleDelegate OnUnhovered;
	
	TEnumAsByte<EButtonClickMethod::Type> ClickMethod = TEnumAsByte<EButtonClickMethod::Type>(0);
	TEnumAsByte<EButtonTouchMethod::Type> TouchMethod = TEnumAsByte<EButtonTouchMethod::Type>(0);
	TEnumAsByte<EButtonPressMethod::Type> PressMethod = TEnumAsByte<EButtonPressMethod::Type>(0);

	bool bIsFocusable = true;
	bool bIsPressed = false;

	EHorizontalAlignment HAlign = HAlign_Fill;
	EVerticalAlignment VAlign = VAlign_Fill;
	TAttribute<FVector2D> DesiredSizeScale;
	TAttribute<FVector2D> ContentScale;

	TSharedPtr<SBorder> Render;
	
	TSharedPtr<FSlateDynamicImageBrush> GizmoPitchBrush;
	TSharedPtr<FSlateDynamicImageBrush> GizmoRollBrush;
	TSharedPtr<FSlateDynamicImageBrush> GizmoYawBrush;
	TSharedPtr<FSlateDynamicImageBrush> GizmoDummyBrush;
	
	AThumbnailMaker* ThumbnailMaker = nullptr;
	FThumbnail* ThumbnailRessource = nullptr;
	UMaterialInstanceDynamic* DynamicMaterial = nullptr;
	FSlateBrush* RenderBrush = nullptr;

	bool bPitch = false;
	bool bRoll = false;
	bool bYaw = false;

	enum class ETransformMode
	{
		None,
		Rotation,
		Translation,
		Zoom,
		Scale
	};

	ETransformMode TransformMode = ETransformMode::None;
	void ChangeTransformMode(const ETransformMode Mode)
	{
		if (TransformMode != Mode)
		{
			TransformMode = Mode;
			FSlateApplication::Get().InvalidateAllWidgets(true);
		}
	}
	
	FVector2D LastMousePosition = FVector2D::ZeroVector;

};