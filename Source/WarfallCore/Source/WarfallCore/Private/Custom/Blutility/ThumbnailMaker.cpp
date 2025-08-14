#include "Custom/Blutility/ThumbnailMaker.h"

#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/RectLightComponent.h"
#include "Components/SceneCaptureComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Factories/TextureFactory.h"
#include "UObject/SavePackage.h"
#include "Custom/Variables/Thumbnail.h"

#if WITH_EDITOR
#include "TextureCompiler.h"                  // FTextureCompilingManager
#include "AssetCompilingManager.h"
#endif

// ==========================================================================
//  STHUMBNAILPILOTE
// ==========================================================================

void SThumbnailPilote::Construct(const FArguments& InArgs)
{
	bIsPressed = false;
	bIsFocusable = InArgs._IsFocusable;

	OnClicked = InArgs._OnClicked;
	OnPressed = InArgs._OnPressed;
	OnReleased = InArgs._OnReleased;
	OnHovered = InArgs._OnHovered;
	OnUnhovered = InArgs._OnUnhovered;

	ClickMethod = InArgs._ClickMethod;
	TouchMethod = InArgs._TouchMethod;
	PressMethod = InArgs._PressMethod;

	HAlign = InArgs._HAlign;
	VAlign = InArgs._VAlign;
	ContentScale = InArgs._ContentScale;
	DesiredSizeScale = InArgs._DesiredSizeScale;
	
	SetupGizmoTextures();
	ClearAndInvalidate();
}
void SThumbnailPilote::SetupGizmoTextures()
{
	const FString GizmoDir = FPaths::ProjectPluginsDir() / TEXT("WarfallCore/Resources/Gizmo/");

	GizmoPitchBrush = MakeShareable(new FSlateDynamicImageBrush(
		FName(*(GizmoDir / FString(TEXT("GizmoPitch.png")))),
		FVector2D(64.f, 64.f)
	));

	GizmoRollBrush = MakeShareable(new FSlateDynamicImageBrush(
		FName(*(GizmoDir / FString(TEXT("GizmoRoll.png")))),
		FVector2D(64.f, 64.f)
	));

	GizmoYawBrush = MakeShareable(new FSlateDynamicImageBrush(
		FName(*(GizmoDir / FString(TEXT("GizmoYaw.png")))),
		FVector2D(64.f, 64.f)
	));

	GizmoDummyBrush = MakeShareable(new FSlateDynamicImageBrush(
		FName(*(GizmoDir / FString(TEXT("GizmoPitchRoll.png")))),
		FVector2D(64.f, 64.f)
	));
}

void SThumbnailPilote::ClearAndInvalidate()
{
	if (ThumbnailMaker)
	{
		ThumbnailMaker = nullptr;
	}

	SAssignNew(Render, SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.01f, 0.01f, 0.01f, 1.0f))
		.Padding(0.f)
		[
			SNew(SBox)
			.WidthOverride(DesiredPreviewScale)
			.HeightOverride(DesiredPreviewScale)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SImage)
					.Image(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.ColorAndOpacity(FLinearColor(0.01f, 0.01f, 0.01f, 1.0f))
					.DesiredSizeOverride(FVector2D(DesiredPreviewScale, DesiredPreviewScale))
				]
				+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString("Invalid Preview"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 20))
					.ColorAndOpacity(FLinearColor::White)
					.ShadowOffset(FVector2D(2, 2))
					.ShadowColorAndOpacity(FLinearColor::Black)
				]
			]
		];
	
	SBorder::Construct(SBorder::FArguments()
		.ContentScale(ContentScale)
		.DesiredSizeScale(DesiredSizeScale)
		.HAlign(HAlign)
		.VAlign(VAlign)
		[
			Render.ToSharedRef()
		]
	);
}

void SThumbnailPilote::UpdateThumbnailMaker(AThumbnailMaker* InThumbnailMaker, FThumbnail* InThumbnailRessource)
{
	if (InThumbnailMaker && InThumbnailRessource)
	{
		ThumbnailMaker = InThumbnailMaker;
		ThumbnailRessource = InThumbnailRessource;
		DynamicMaterial = ThumbnailMaker->DynamicMaterial;
		RenderBrush = new FSlateBrush();
		RenderBrush->SetResourceObject(DynamicMaterial); // Associe ton material dynamique
		
		SAssignNew(Render, SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.01f, 0.01f, 0.01f, 1.0f))
		.Cursor(this, &SThumbnailPilote::GetCursor)
		.Padding(0.f)
		[
			SNew(SBox)
			.WidthOverride(DesiredPreviewScale)
			.HeightOverride(DesiredPreviewScale)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					// 🧱 Bloc contenant l'image ET les bordures, redimensionné selon le ratio
					SNew(SBox)
					.WidthOverride_Lambda([this]() {
						const FVector2D& Dim = ThumbnailMaker->ThumbnailRessource->GetDimensions();
						const float Ratio = Dim.X / Dim.Y;
						return (Ratio >= 1.f) ? DesiredPreviewScale : DesiredPreviewScale * Ratio;
					})
					.HeightOverride_Lambda([this]() {
						const FVector2D& Dim = ThumbnailMaker->ThumbnailRessource->GetDimensions();
						const float Ratio = Dim.X / Dim.Y;
						return (Ratio >= 1.f) ? DesiredPreviewScale / Ratio : DesiredPreviewScale;
					})
					[
						SNew(SOverlay)

						// IMAGE
						+ SOverlay::Slot()
						[
							SNew(SImage)
							.Image(RenderBrush)
						]

						// BORDURE GAUCHE
						+ SOverlay::Slot()
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Fill)
						[
							SNew(SBox)
							.WidthOverride_Lambda([this]() {
								const FVector2D& Dim = ThumbnailMaker->ThumbnailRessource->GetDimensions();
								return (Dim.X < Dim.Y) ? 1.f : 0.f;
							})
							[
								SNew(SImage).ColorAndOpacity(FLinearColor::Yellow)
							]
						]

						// BORDURE DROITE
						+ SOverlay::Slot()
						.HAlign(HAlign_Right)
						.VAlign(VAlign_Fill)
						[
							SNew(SBox)
							.WidthOverride_Lambda([this]() {
								const FVector2D& Dim = ThumbnailMaker->ThumbnailRessource->GetDimensions();
								return (Dim.X < Dim.Y) ? 1.f : 0.f;
							})
							[
								SNew(SImage).ColorAndOpacity(FLinearColor::Yellow)
							]
						]

						// BORDURE HAUT
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Top)
						[
							SNew(SBox)
							.HeightOverride_Lambda([this]() {
								const FVector2D& Dim = ThumbnailMaker->ThumbnailRessource->GetDimensions();
								return (Dim.Y < Dim.X) ? 1.f : 0.f;
							})
							[
								SNew(SImage).ColorAndOpacity(FLinearColor::Yellow)
							]
						]

						// BORDURE BAS
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Bottom)
						[
							SNew(SBox)
							.HeightOverride_Lambda([this]() {
								const FVector2D& Dim = ThumbnailMaker->ThumbnailRessource->GetDimensions();
								return (Dim.Y < Dim.X) ? 1.f : 0.f;
							})
							[
								SNew(SImage).ColorAndOpacity(FLinearColor::Yellow)
							]
						]
					]
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				[
					SNew(SBox)
					.WidthOverride(64.f)
					.HeightOverride(64.f)
					.Visibility_Lambda([this]() { return bPitch || bRoll || bYaw ? EVisibility::Visible : EVisibility::Collapsed; })
					[
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							SNew(SImage)
							.Image(GizmoPitchBrush.IsValid() ? GizmoPitchBrush.Get() : nullptr)
							.Visibility_Lambda([this] { return !bPitch && bRoll ? EVisibility::Visible : EVisibility::Collapsed; })
						]
						+ SOverlay::Slot()
						[
							SNew(SImage)
							.Image(GizmoRollBrush.IsValid() ? GizmoRollBrush.Get() : nullptr)
							.Visibility_Lambda([this] { return !bRoll && bPitch ? EVisibility::Visible : EVisibility::Collapsed; })
						]
						+ SOverlay::Slot()
						[
							SNew(SImage)
							.Image(GizmoDummyBrush.IsValid() ? GizmoDummyBrush.Get() : nullptr)
							.Visibility_Lambda([this] { return bRoll && bPitch ? EVisibility::Visible : EVisibility::Collapsed; })
						]
						+ SOverlay::Slot()
						[
							SNew(SImage)
							.Image(GizmoYawBrush.IsValid() ? GizmoYawBrush.Get() : nullptr)
							.Visibility_Lambda([this] { return bYaw ? EVisibility::Visible : EVisibility::Collapsed; })
						]
					]
				]
			]
		];
		
		SBorder::Construct(SBorder::FArguments()
		.ContentScale(ContentScale)
		.DesiredSizeScale(DesiredSizeScale)
		.HAlign(HAlign)
		.VAlign(VAlign)
		[
			Render.ToSharedRef()
		]
		);
		return;
	}
	ClearAndInvalidate();
}

TOptional<EMouseCursor::Type> SThumbnailPilote::GetCursor() const
{
	switch (TransformMode)
	{
	case ETransformMode::None:
		return EMouseCursor::Default;
	case ETransformMode::Translation:
		return EMouseCursor::CardinalCross;
	case ETransformMode::Rotation:
		return EMouseCursor::CardinalCross;
	case ETransformMode::Scale:
		return EMouseCursor::CardinalCross;
	case ETransformMode::Zoom:
		return EMouseCursor::CardinalCross;
	};
	return EMouseCursor::Default;
}

bool SThumbnailPilote::SupportsKeyboardFocus() const
{
	return bIsFocusable;
}

void SThumbnailPilote::OnFocusLost(const FFocusEvent& InFocusEvent)
{
	SBorder::OnFocusLost(InFocusEvent);

	Release();
}

FReply SThumbnailPilote::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey InKey = InKeyEvent.GetKey();

	if (ThumbnailMaker && ThumbnailRessource)
	{
		if (UGlobalTools::GetKeyboardType() == "Azerty")
		{
			if (InKey == EKeys::Z)
			{
				FVector Loc = ThumbnailRessource->GetLocation();
				Loc.Z += 5.f;
				ThumbnailRessource->GetLocation() = Loc;
				ThumbnailRessource->GetLocation().Z = FMath::Clamp(ThumbnailRessource->GetLocation().Z, -9999, 9999);
				bPitch = false;
				bRoll = false;
				bYaw = false;
				ThumbnailMaker->UpdateTransform();
			}
			else if (InKey == EKeys::Q)
			{
				FVector Loc = ThumbnailRessource->GetLocation();
				Loc.X += -5.f;
				ThumbnailRessource->GetLocation() = Loc;
				ThumbnailRessource->GetLocation().X = FMath::Clamp(ThumbnailRessource->GetLocation().X, -9999, 9999);
				bPitch = false;
				bRoll = false;
				bYaw = false;
				ThumbnailMaker->UpdateTransform();
			}
		}
		else if (UGlobalTools::GetKeyboardType() == "Qwerty")
		{
			if (InKey == EKeys::W)
			{
				FVector Loc = ThumbnailRessource->GetLocation();
				Loc.Z += 5.f;
				ThumbnailRessource->GetLocation() = Loc;
				ThumbnailRessource->GetLocation().Z = FMath::Clamp(ThumbnailRessource->GetLocation().Z, -9999, 9999);
				bPitch = false;
				bRoll = false;
				bYaw = false;
				ThumbnailMaker->UpdateTransform();
			}
			else if (InKey == EKeys::A)
			{
				FVector Loc = ThumbnailRessource->GetLocation();
				Loc.X += -5.f;
				ThumbnailRessource->GetLocation() = Loc;
				ThumbnailRessource->GetLocation().X = FMath::Clamp(ThumbnailRessource->GetLocation().X, -9999, 9999);
				bPitch = false;
				bRoll = false;
				bYaw = false;
				ThumbnailMaker->UpdateTransform();
			}
		}
		if (InKey == EKeys::S)
		{
			FVector Loc = ThumbnailRessource->GetLocation();
			Loc.Z += -5.f;
			ThumbnailRessource->GetLocation() = Loc;
			ThumbnailRessource->GetLocation().Z = FMath::Clamp(ThumbnailRessource->GetLocation().Z, -9999, 9999);
			bPitch = false;
			bRoll = false;
			bYaw = false;
			ThumbnailMaker->UpdateTransform();
		}
		else if (InKey == EKeys::D)
		{
			FVector Loc = ThumbnailRessource->GetLocation();
			Loc.X += 5.f;
			ThumbnailRessource->GetLocation() = Loc;
			ThumbnailRessource->GetLocation().X = FMath::Clamp(ThumbnailRessource->GetLocation().X, -9999, 9999);
			bPitch = false;
			bRoll = false;
			bYaw = false;
			ThumbnailMaker->UpdateTransform();
		}	
	}
	return FReply::Handled();
}

FReply SThumbnailPilote::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	bPitch = false;
	bRoll = false;
	bYaw = false;
	return FReply::Unhandled();
}

void SThumbnailPilote::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FSlateApplication::Get().SetKeyboardFocus(SharedThis(this), EFocusCause::SetDirectly);
}

void SThumbnailPilote::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	if (bIsPressed) return;
	FSlateApplication::Get().ClearKeyboardFocus(EFocusCause::Cleared);
}

FReply SThumbnailPilote::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (!bIsPressed || !ThumbnailMaker || !ThumbnailRessource) return FReply::Unhandled();

	const FVector2D CurrentPos = MouseEvent.GetScreenSpacePosition();
	const FVector2D Delta = CurrentPos - LastMousePosition;
	constexpr float RotationSpeed = 0.5f;
	LastMousePosition = CurrentPos;
	
	// État des touches
	
	if (MouseEvent.IsShiftDown() && MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
	{
		const float RollDelta = -Delta.Y * RotationSpeed;
		ThumbnailRessource->GetRotationEuler().Roll += RollDelta;
		ThumbnailRessource->GetRotationEuler().Roll = FMath::Clamp(ThumbnailRessource->GetRotationEuler().Roll, -9999, 9999);
		bRoll = true;
		bPitch = false;
		bYaw = false;
		ChangeTransformMode(ETransformMode::Rotation);
	}
	else if (MouseEvent.IsControlDown() && MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
	{
		const float YawDelta = -Delta.X * RotationSpeed;
		ThumbnailRessource->GetRotationEuler().Yaw += YawDelta;
		ThumbnailRessource->GetRotationEuler().Yaw = FMath::Clamp(ThumbnailRessource->GetRotationEuler().Yaw, -9999, 9999);
		bYaw = true;
		bPitch = false;
		bRoll = false;
		ChangeTransformMode(ETransformMode::Rotation);
	}
	else if (MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
	{
		const float PitchDelta = -Delta.X * RotationSpeed;
		ThumbnailRessource->GetRotationEuler().Pitch += PitchDelta;
		ThumbnailRessource->GetRotationEuler().Pitch = FMath::Clamp(ThumbnailRessource->GetRotationEuler().Pitch, -9999, 9999);
		bPitch = true;
		bYaw = false;
		bRoll = false;
		ChangeTransformMode(ETransformMode::Rotation);
	}
	else if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		FVector Loc = ThumbnailRessource->GetLocation();
		Loc.X += Delta.X * 0.5f;
		Loc.Z += -Delta.Y * 0.5f;
		ThumbnailRessource->GetLocation() = Loc;
		ThumbnailRessource->GetLocation().X = FMath::Clamp(ThumbnailRessource->GetLocation().X, -9999, 9999);
		ThumbnailRessource->GetLocation().Z = FMath::Clamp(ThumbnailRessource->GetLocation().Z, -9999, 9999);
		bPitch = false;
		bRoll = false;
		bYaw = false;
		ChangeTransformMode(ETransformMode::Translation);
	}
	ThumbnailMaker->UpdateTransform();
	return FReply::Handled();
}

FReply SThumbnailPilote::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FVector Loc = ThumbnailRessource->GetLocation();
	FVector Scale = ThumbnailRessource->GetScale();
	const float WheelDelta = MouseEvent.GetWheelDelta();
		
	if (MouseEvent.IsShiftDown())
	{
		const float ScaleDelta = WheelDelta * 0.1f;
		Scale.X += ScaleDelta;
		Scale.Y += ScaleDelta;
		Scale.Z += ScaleDelta;
		ThumbnailRessource->GetScale() = Scale;
		ThumbnailRessource->GetScale().X = FMath::Clamp(ThumbnailRessource->GetScale().X, 0.01f, 9999);
		ThumbnailRessource->GetScale().Y = FMath::Clamp(ThumbnailRessource->GetScale().Y, 0.01f, 9999);
		ThumbnailRessource->GetScale().Z = FMath::Clamp(ThumbnailRessource->GetScale().Z, 0.01f, 9999);
	}
	else
	{
		Loc.Y += WheelDelta * 5.0f;
		ThumbnailRessource->GetLocation() = Loc;
		ThumbnailRessource->GetLocation().Y = FMath::Clamp(ThumbnailRessource->GetLocation().Y, -9999, 0);
	}
	
	ThumbnailMaker->UpdateTransform();
	return FReply::Handled();
}

FReply SThumbnailPilote::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	Press();
	LastMousePosition = MouseEvent.GetScreenSpacePosition();	
	return FReply::Handled().CaptureMouse(AsShared());
}

FReply SThumbnailPilote::OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FReply Reply = SBorder::OnMouseButtonDoubleClick(MyGeometry, MouseEvent);
	if (Reply.IsEventHandled())
	{
		return Reply;
	}

	// We didn't handle the double click, treat it as a single click
	return OnMouseButtonDown(MyGeometry, MouseEvent);
}

FReply SThumbnailPilote::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	Release();
	bPitch = false;
	bRoll = false;
	bYaw = false;
	ChangeTransformMode(ETransformMode::None);
	return FReply::Handled().ReleaseMouseCapture();

}

void SThumbnailPilote::OnMouseCaptureLost(const FCaptureLostEvent& CaptureLostEvent)
{
	Release();
}

FReply SThumbnailPilote::ExecuteOnClick()
{
	if (OnClicked.IsBound())
	{
		FReply Reply = OnClicked.Execute();
#if WITH_ACCESSIBILITY
		FSlateApplicationBase::Get().GetAccessibleMessageHandler()->OnWidgetEventRaised(FSlateAccessibleMessageHandler::FSlateWidgetAccessibleEventArgs(AsShared(), EAccessibleEvent::Activate));
#endif
		return Reply;
	}
	else
	{
		return FReply::Handled();
	}
}

void SThumbnailPilote::Press()
{
	if ( !bIsPressed )
	{
		bIsPressed = true;
		if (OnPressed.IsBound())
		{
			OnPressed.Execute();
		}
	}
}

void SThumbnailPilote::Release()
{
	if ( bIsPressed )
	{
		bIsPressed = false;
		if (OnReleased.IsBound())
		{
			OnReleased.Execute();
		}
	}
}

bool SThumbnailPilote::IsInteractable() const
{
	return IsEnabled();
}

TEnumAsByte<EButtonClickMethod::Type> SThumbnailPilote::GetClickMethodFromInputType(const FPointerEvent& MouseEvent) const
{
	if (MouseEvent.IsTouchEvent())
	{
		switch (TouchMethod)
		{
		case EButtonTouchMethod::Down:
			return EButtonClickMethod::MouseDown;
		case EButtonTouchMethod::DownAndUp:
			return EButtonClickMethod::DownAndUp;
		case EButtonTouchMethod::PreciseTap:
			return EButtonClickMethod::PreciseClick;
		default: ;
		}
	}

	return ClickMethod;
}

void SThumbnailPilote::SetOnClicked(const FOnClicked& InOnClicked)
{
	OnClicked = InOnClicked;
}

void SThumbnailPilote::SetOnHovered(const FSimpleDelegate& InOnHovered)
{
	OnHovered = InOnHovered;
}

void SThumbnailPilote::SetOnUnhovered(const FSimpleDelegate& InOnUnhovered)
{
	OnUnhovered = InOnUnhovered;
}

void SThumbnailPilote::SetClickMethod(const EButtonClickMethod::Type InClickMethod)
{
	ClickMethod = InClickMethod;
}

void SThumbnailPilote::SetTouchMethod(const EButtonTouchMethod::Type InTouchMethod)
{
	TouchMethod = InTouchMethod;
}

void SThumbnailPilote::SetPressMethod(const EButtonPressMethod::Type InPressMethod)
{
	PressMethod = InPressMethod;
}

// ==========================================================================
//  ATHUMBNAILMAKER
// ==========================================================================

AThumbnailMaker::AThumbnailMaker()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMeshAsset(TEXT("/Engine/BasicShapes/Plane.Plane"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> FoundMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (FoundMaterial.Succeeded())
	{
		PlaneMaterial = FoundMaterial.Object;
	}
	
	PrimaryActorTick.bCanEverTick = true;
	
	if (UGlobalTools::GetMaterial(EMaterialPath::RenderTarget))
	{
		RenderMaterial = UGlobalTools::GetMaterial(EMaterialPath::RenderTarget);
	}
		
	CameraDefaultTransform.SetLocation(FVector(0.f, 45.f, 0.f));
	CameraDefaultTransform.SetRotation(FRotator(0.f, -90.f, 0.f).Quaternion());
	CameraDefaultTransform.SetScale3D(FVector(1.f, 1.f, 1.f));
	
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>("DefaultSceneRoot");
	RootComponent = DefaultSceneRoot;

	MeshObject = CreateDefaultSubobject<UStaticMeshComponent>("MeshObject");
	MeshObject->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshObject->SetCastShadow(false);
	MeshObject->bCastDynamicShadow = false;
	MeshObject->bCastStaticShadow = false;
	MeshObject->bAffectDistanceFieldLighting = false;
	MeshObject->bAffectDynamicIndirectLighting = false;
	MeshObject->SetupAttachment(DefaultSceneRoot);

	SkeletalObject = CreateDefaultSubobject<USkeletalMeshComponent>("SkeletalObject");
	MeshObject->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SkeletalObject->SetCastShadow(false);
	SkeletalObject->bCastDynamicShadow = false;
	SkeletalObject->bCastStaticShadow = false;
	SkeletalObject->bAffectDistanceFieldLighting = false;
	SkeletalObject->bAffectDynamicIndirectLighting = false;
	SkeletalObject->SetupAttachment(MeshObject);

	Render = CreateDefaultSubobject<UStaticMeshComponent>("Render");
	Render->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Render->SetRelativeLocation(FVector(154.0f, -211.0f, 0.0f));
	Render->SetRelativeRotation(FRotator(0.0f, 0.0f, 90.0f));
	Render->SetupAttachment(DefaultSceneRoot);
	
	Background = CreateDefaultSubobject<UStaticMeshComponent>("Background");
	Background->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Background->SetRelativeLocation(FVector(0.0f, 0.0f, -1.0f));
	Background->SetupAttachment(Render);

	Camera = CreateDefaultSubobject<USceneCaptureComponent2D>("Camera");
	Camera->SetRelativeTransform(CameraDefaultTransform);
	Camera->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	Camera->ShowFlags.SetAtmosphere(false); // désactive effets globaux si besoin
	Camera->ShowOnlyComponents.Empty(); // au cas où
	Camera->ShowOnlyComponents.Add(MeshObject);
	Camera->ShowOnlyComponents.Add(SkeletalObject);
	Camera->CaptureSource = SCS_SceneColorHDR;
	Camera->bCaptureEveryFrame = false;
	Camera->bCaptureOnMovement = false;
	Camera->SetupAttachment(DefaultSceneRoot);

	Light = CreateDefaultSubobject<URectLightComponent>("Light");
	Light->Intensity = 15000.0f;
	Light->AttenuationRadius = 1500.0f;
	Light->SourceWidth = 600.0f;
	Light->SourceHeight = 1000.0f;
	Light->SetupAttachment(Camera);

	if (PlaneMeshAsset.Succeeded())
	{
		if (Background)
		{
			Background->SetStaticMesh(PlaneMeshAsset.Object);
		}
		if (Render)
		{
			Render->SetStaticMesh(PlaneMeshAsset.Object);
			Render->SetMaterial(0, PlaneMaterial);
		}
	}
	const FVector DefaultLocation = FVector(0.f, -60.f, -50.f);
	DefaultMeshTransform.SetLocation(DefaultLocation);
}

void AThumbnailMaker::UpdateThumbnailMaker(FThumbnail* InThumbnailRessource, SThumbnailPilote* InPilote)
{
	
	ThumbnailRessource = InThumbnailRessource;
	PilotePointer = InPilote;
	if (!InThumbnailRessource)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid thumbnail ressource provided to ThumbnailMaker."));
		return;
	}
	AfterInit();
}

void AThumbnailMaker::ClearThumbnailMaker()
{
	ThumbnailRessource = nullptr;
	PilotePointer = nullptr;
}

void AThumbnailMaker::AfterInit()
{
	if (MeshObject && SkeletalObject)
	{
		if (ThumbnailRessource)
		{
			UpdateTransform();
			// Set new assets
			UpdateMesh();
			if (Camera)
			{
				CreateRenderTarget();
			}
		}
	}	
}

void AThumbnailMaker::UpdateMesh() const
{
	if (ThumbnailRessource)
	{
		SkeletalObject->SetSkeletalMeshAsset(nullptr);
		MeshObject->SetStaticMesh(nullptr);
		
		if (ThumbnailRessource->GetSkeletalMesh())
		{
			SkeletalObject->SetSkeletalMeshAsset(ThumbnailRessource->GetSkeletalMesh().LoadSynchronous());
			return;
		}
		if (ThumbnailRessource->GetMesh())
		{
			MeshObject->SetStaticMesh(ThumbnailRessource->GetMesh().LoadSynchronous());
			SkeletalObject->SetSkeletalMeshAsset(nullptr);
			Refresh();
		}
	}
}

void AThumbnailMaker::UpdateTransform() const
{
	if (ThumbnailRessource)
	{
		const FTransform Computed = FTransform(
			FQuat(ThumbnailRessource->GetRotationEuler()),
			ThumbnailRessource->GetLocation(),
			ThumbnailRessource->GetScale());
		MeshObject->SetRelativeRotation(Computed.GetRotation());
		MeshObject->SetRelativeLocation(DefaultMeshTransform.GetLocation() + Computed.GetLocation());
		MeshObject->SetRelativeScale3D(Computed.GetScale3D());
		Refresh();
	}
}

void AThumbnailMaker::Refresh() const
{
	if (MeshObject && MeshObject->GetStaticMesh())
	{
		MeshObject->MarkRenderStateDirty();
		MeshObject->RecreateRenderState_Concurrent();
		MeshObject->UpdateComponentToWorld();
	}
	if (SkeletalObject && SkeletalObject->GetSkeletalMeshAsset())
	{
		SkeletalObject->MarkRenderStateDirty();
		SkeletalObject->RecreateRenderState_Concurrent();
		SkeletalObject->UpdateComponentToWorld();
	}
	Camera->CaptureScene();
}

void AThumbnailMaker::OnConstruction(const FTransform& Transform)
{
	if (!RenderMaterial && UGlobalTools::GetMaterial(EMaterialPath::RenderTarget))
	{
		RenderMaterial = UGlobalTools::GetMaterial(EMaterialPath::RenderTarget);
	}

	if (!PlaneMaterial)
	{
		PlaneMaterial = Cast<UMaterialInterface>(
			StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"))
		);
	}

	if (Background && PlaneMaterial)
	{
		UMaterialInstanceDynamic* Dynamic = UMaterialInstanceDynamic::Create(PlaneMaterial, this);
		Dynamic->SetVectorParameterValue("Color", FLinearColor(0.01f, 0.01f, 0.01f, 1.0f));
		Background->SetMaterial(0, Dynamic);
	}
		
	if (Camera)
	{
		Camera->SetRelativeTransform(CameraDefaultTransform);
		Camera->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
		Camera->ShowFlags.SetAtmosphere(false); // désactive effets globaux si besoin
		Camera->ShowOnlyComponents.Empty(); // au cas où
		Camera->ShowOnlyComponents.Add(MeshObject);
		Camera->ShowOnlyComponents.Add(SkeletalObject);
	}
}

void AThumbnailMaker::Destroyed()
{
	if (PilotePointer)
	{
		PilotePointer->ClearAndInvalidate();
		ClearThumbnailMaker();
	}
	Super::Destroyed();
}

UTextureRenderTarget2D* AThumbnailMaker::CreateRenderTarget()
{
	if (RenderTarget)
	{
		RenderTarget->ConditionalBeginDestroy();
		RenderTarget = nullptr;
	}

	UTextureRenderTarget2D* NewRenderTarget = NewObject<UTextureRenderTarget2D>(this, UTextureRenderTarget2D::StaticClass(), NAME_None, RF_Transient);
	if (!NewRenderTarget)
	{
		UE_LOG(LogTemp, Error, TEXT("ThumbnailMaker: Failed to create RenderTarget!"));
		return nullptr;
	}

	NewRenderTarget->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
	NewRenderTarget->ClearColor = FLinearColor::Transparent;
	NewRenderTarget->TargetGamma = 2.2f;

	NewRenderTarget->InitAutoFormat(512, 512);

	if (ThumbnailRessource)
	{
		const int32 SizeX = FMath::Max(1, ThumbnailRessource->GetDimensions().X) * 512;
		const int32 SizeY = FMath::Max(1, ThumbnailRessource->GetDimensions().Y) * 512;
		NewRenderTarget->ResizeTarget(SizeX, SizeY);
	}
	RenderTarget = NewRenderTarget;

	if (RenderMaterial && Render && RenderTarget)
	{
		DynamicMaterial = UMaterialInstanceDynamic::Create(RenderMaterial, this);
		if (DynamicMaterial)
		{
			DynamicMaterial->SetTextureParameterValue("Icon", RenderTarget);
			Render->SetMaterial(0, DynamicMaterial);
			Render->SetRelativeScale3D(GetRenderScale());
		}
	}
	Camera->TextureTarget = RenderTarget;
	Refresh();
	return RenderTarget;
}


FVector AThumbnailMaker::GetRenderScale() const
{
	FVector Scale = FVector(1.0f, 1.0f, 1.0f);

	if (ThumbnailRessource)
	{
		Scale = FVector(ThumbnailRessource->GetDimensions().X, ThumbnailRessource->GetDimensions().Y,1.0f);
	}
	return Scale;
}

void AThumbnailMaker::SaveRenderTargetToDisk() const
{
	if (!RenderTarget || !ThumbnailRessource)
	{
		UE_LOG(LogTemp, Error, TEXT("RenderTarget is null."));
		return;
	}

	FRenderTarget* RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();
	if (!RenderTargetResource)
	{
		UE_LOG(LogTemp, Error, TEXT("RenderTarget resource is invalid."));
		return;
	}

	TArray<FColor> Bitmap;
	if (!RenderTargetResource->ReadPixels(Bitmap))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to read pixels from RenderTarget."));
		return;
	}

	for (FColor& Pixel : Bitmap)
	{
		Pixel.A = 255 - Pixel.A;
	}

	const int32 Width = RenderTarget->SizeX;
	const int32 Height = RenderTarget->SizeY;

	FString RawName = ThumbnailRessource->GetName();
	RawName = RawName.Replace(TEXT(" "), TEXT("_"));
	const FString FileName = FString::Printf(TEXT("T_%s_Icon"), *RawName);
	
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	const TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	if (!ImageWrapper->SetRaw(Bitmap.GetData(), Bitmap.GetAllocatedSize(), Width, Height, ERGBFormat::BGRA, 8))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to set raw data for PNG encoding."));
		return;
	}

	TArray64<uint8> Compressed64 = ImageWrapper->GetCompressed(100);

	TArray<uint8> PNGData;
	PNGData.Append(Compressed64.GetData(), Compressed64.Num());
	
	const FString FullPath = FPaths::ProjectSavedDir() + TEXT("Thumbnails/") + FileName + TEXT(".png");

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.CreateDirectoryTree(*FPaths::GetPath(FullPath));

	if (!FFileHelper::SaveArrayToFile(PNGData, *FullPath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to write PNG to disk: %s"), *FullPath);
		return;
	}
	
	ImportAndAssignTextureToRow(FileName);
	UE_LOG(LogTemp, Log, TEXT("Successfully saved thumbnail to: %s"), *FullPath);
}

void AThumbnailMaker::ImportAndAssignTextureToRow(const FString& FileName) const
{
	if (!ThumbnailRessource)
	{
		UE_LOG(LogTemp, Error, TEXT("No Data to assign texture to."));
		return;
	}

	// Fichier PNG que tu viens d’écrire dans Saved/Thumbnails
	const FString SourceImagePath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Thumbnails/"), FileName + TEXT(".png"));
	if (!FPaths::FileExists(SourceImagePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Image file not found: %s"), *SourceImagePath);
		return;
	}

	const FString PackagePath = THUMBNAIL_FOLDER_PATH;                    // e.g. "/Game/UI/Thumbnails"
	const FString PackageName = PackagePath + TEXT("/") + FileName;       // e.g. "/Game/UI/Thumbnails/T_MonIcon_01"
	const FString ObjectPath  = PackageName + TEXT(".") + FileName;       // e.g. "/Game/UI/Thumbnails/T_MonIcon_01.T_MonIcon_01"

	// 1) Tenter de charger une texture existante

	// Charger les octets du PNG et les décoder en BGRA8
	TArray<uint8> PngBytes;
	if (!FFileHelper::LoadFileToArray(PngBytes, *SourceImagePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to read PNG from disk: %s"), *SourceImagePath);
		return;
	}

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	if (!ImageWrapper->SetCompressed(PngBytes.GetData(), PngBytes.Num()))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to decode PNG: %s"), *SourceImagePath);
		return;
	}

	const int32 Width  = ImageWrapper->GetWidth();
	const int32 Height = ImageWrapper->GetHeight();

	TArray64<uint8> RawBGRA;
	if (!ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, RawBGRA))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to extract BGRA8 from PNG: %s"), *SourceImagePath);
		return;
	}

	// Validation de la taille (BGRA8 => 4 octets par pixel)
	const int64 ExpectedSize = static_cast<int64>(Width) * static_cast<int64>(Height) * 4;
	if (RawBGRA.Num() != ExpectedSize)
	{
		UE_LOG(LogTemp, Error, TEXT("Unexpected BGRA buffer size: got %lld, expected %lld"), RawBGRA.Num(), ExpectedSize);
		return;
	}

	#if WITH_EDITOR
    // 0) On s'assure qu'aucune compile en cours ne traîne
    FAssetCompilingManager::Get().FinishAllCompilation();
#endif

	TSoftObjectPtr<UTexture2D> SoftTex(ObjectPath);

	if (UTexture2D* Existing = SoftTex.LoadSynchronous())
    {
#if WITH_EDITOR
        // 1) UPDATE IN PLACE — encadrer les modifs et attendre les compiles
        FTextureCompilingManager::Get().FinishCompilation({ Existing });
        Existing->PreEditChange(nullptr);

        Existing->Source.Init(Width, Height, 1, 1, TSF_BGRA8, RawBGRA.GetData());
        Existing->MipGenSettings      = TMGS_NoMipmaps;   // si tu veux des icônes nettes
        Existing->CompressionSettings = TC_EditorIcon;
        Existing->SRGB                = true;

        Existing->PostEditChange();                       // déclenche la recompile/maj ressource
        Existing->MarkPackageDirty();
        FTextureCompilingManager::Get().FinishCompilation({ Existing }); // pas de save pendant une compile
#endif

        // Structure côté UI
        ThumbnailRessource->Thumbnail = Existing;

        // Save
        FSavePackageArgs SaveArgs; SaveArgs.TopLevelFlags = RF_Public | RF_Standalone; SaveArgs.Error = GError;
        UPackage* Pkg = Existing->GetOutermost();
        const FString PkgFilename = FPackageName::LongPackageNameToFilename(Pkg->GetName(), FPackageName::GetAssetPackageExtension());
        UPackage::SavePackage(Pkg, Existing, *PkgFilename, SaveArgs);
        return;
    }

    // 2) CRÉATION SI INEXISTANT — pas de delete !
    UPackage* Package = CreatePackage(*PackageName);
    UTextureFactory* TextureFactory = NewObject<UTextureFactory>();
    TextureFactory->AddToRoot();
    TextureFactory->SuppressImportOverwriteDialog();
    TextureFactory->bEditorImport = true;

    bool bOutCanceled = false;
    UObject* Imported = TextureFactory->FactoryCreateFile(
        UTexture2D::StaticClass(), Package, *FileName,
        RF_Public | RF_Standalone, *SourceImagePath, nullptr, GWarn, bOutCanceled);
    TextureFactory->RemoveFromRoot();

    UTexture2D* NewTex = Cast<UTexture2D>(Imported);
    if (!NewTex) { UE_LOG(LogTemp, Error, TEXT("Import failed: %s"), *SourceImagePath); return; }

#if WITH_EDITOR
    // La factory peut avoir lancé une compile ; on attend, puis on encadre nos tweaks
    FTextureCompilingManager::Get().FinishCompilation({ NewTex });
    NewTex->PreEditChange(nullptr);

    NewTex->MipGenSettings      = TMGS_NoMipmaps;
    NewTex->CompressionSettings = TC_EditorIcon;
    NewTex->SRGB                = true;

    NewTex->PostEditChange();
    NewTex->MarkPackageDirty();
    FAssetRegistryModule::AssetCreated(NewTex);
    FTextureCompilingManager::Get().FinishCompilation({ NewTex });
#endif

    const FString FinalPkgFilename = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
    FSavePackageArgs SaveArgs; SaveArgs.TopLevelFlags = RF_Public | RF_Standalone; SaveArgs.Error = GError;
    UPackage::SavePackage(Package, NewTex, *FinalPkgFilename, SaveArgs);

    ThumbnailRessource->Thumbnail = NewTex;
}
