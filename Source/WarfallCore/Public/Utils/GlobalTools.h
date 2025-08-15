#pragma once

#include "CoreMinimal.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include <windows.h>
#include "Windows/HideWindowsPlatformTypes.h"

#include "Utils/Paths.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "GlobalTools.generated.h"

enum class EMaterialPath
{
	None,
	Shape,
	Thumbnail,
	Outline,
	RenderTarget,
};

enum class EMaterialInstPath
{
	None,
	Outline,
};

UCLASS(Abstract)
class WARFALLCORE_API UGlobalTools : public UObject
{
	GENERATED_BODY()

public:
    /**
     * Returns a loaded material interface by its logical name.
     *
     * @param MaterialName One of: "Shape", "Thumbnail", "Outline".
     * @return A pointer to the material if found, otherwise nullptr.
     */
    static UMaterialInterface* GetMaterial(const EMaterialPath& MaterialName)
    {
    	static const TMap<EMaterialPath, FString> TableMap =
    		{
    		{EMaterialPath::Shape, SHAPE_MATERIAL_PATH},
    		{EMaterialPath::Thumbnail, THUMBNAIL_MATERIAL_PATH},
    		{EMaterialPath::Outline, OUTLINES_MATERIAL_PATH},
    		{EMaterialPath::RenderTarget, RENDER_TARGET_MATERIAL_PATH}
    		};
    	
    	const FString* PathPtr = TableMap.Find(MaterialName);
    	if (!PathPtr)
    	{
    		return nullptr;
    	}
    	return Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, **PathPtr));
    }

	/**
	 * Returns a loaded material instance by its logical name.
	 *
	 * @param MaterialName The material instance path name, such as "Outline".
	 * @return A pointer to the material instance if found, otherwise nullptr.
	 */
	static UMaterialInstance* GetMaterialInstance(const EMaterialInstPath& MaterialName)
    {
    	static const TMap<EMaterialInstPath, FString> TableMap =
			{
    		{EMaterialInstPath::Outline, OUTLINES_MATERIAL_INST_PATH},
			};
    	const FString* PathPtr = TableMap.Find(MaterialName);
    	if (!PathPtr)
    	{
    		return nullptr;
    	}
    	return Cast<UMaterialInstance>(StaticLoadObject(UMaterialInstance::StaticClass(), nullptr, **PathPtr));
    }
	
    /**
     * Approximates the physical mass of a mesh (static or skeletal) based on its volume and a provided density.
     *
     * @param StaticMesh Optional static mesh.
     * @param SkeletalMesh Optional skeletal mesh.
     * @param Density Mass density in kg/m³.
     * @return The estimated mass in kilograms.
     */	
    static float MassObjectInKg(const UStaticMesh* StaticMesh = nullptr, const USkeletalMesh* SkeletalMesh = nullptr, const float Density = 0.0f)
    {
    	if (!StaticMesh && !SkeletalMesh) return 0.0f;

    	FVector Bounds = FVector::ZeroVector;

    	if (StaticMesh)
    	{
    		Bounds = StaticMesh->GetBounds().BoxExtent * 2.0f;
    	}
    	else if (SkeletalMesh)
    	{
    		Bounds = SkeletalMesh->GetBounds().BoxExtent * 2.0f;
    	}
    	const FVector BoundsMeters = Bounds/100.f;
    	const float Volume = BoundsMeters.X * BoundsMeters.Y * BoundsMeters.Z;
    	return Volume * Density;
    }
    /**
     * Performs a line trace from the player's camera forward by a specified distance, using the given collision channel.
     * Optionally draws debug visuals depending on the DrawDebugType.
     *
     * @param WorldContextObject World context for spawning debug visuals.
     * @param OutHit The hit result returned by the trace.
     * @param TraceChannel Collision channel to use.
     * @param Distance Maximum trace distance.
     * @param DrawDebugType Debug visualization mode (None, OneFrame, Duration, Persistent).
     * @param DrawDuration Duration to keep debug visuals if applicable.
     * @return True if something was hit; false otherwise.
     */	
    static bool LineTraceByChannel(const UObject* WorldContextObject, struct FHitResult& OutHit, const ECollisionChannel TraceChannel,
    	const float Distance, const TEnumAsByte<EDrawDebugTrace::Type> DrawDebugType = EDrawDebugTrace::None, const float DrawDuration = 5.f)
    {
    	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

    	if (!World) return false;
    	if (const APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(World, 0))
    	{
    		const FVector Start = CameraManager->GetCameraLocation();
    		const FVector End = Start + CameraManager->GetCameraRotation().Vector() * Distance;
    		const bool bHit = World->LineTraceSingleByChannel(OutHit, Start, End, TraceChannel, FCollisionQueryParams(), FCollisionResponseParams());

    		const FColor LineColor = bHit ? FColor::Green : FColor::Red;
    		const FColor ImpactColor = bHit ? FColor::Green : FColor::Red;
    		
    		switch (DrawDebugType)
    		{
    		case EDrawDebugTrace::ForOneFrame:
    			DrawDebugLine(World, Start, End, LineColor, false, -1.f, 0, 1.f);
    			if (bHit) DrawDebugPoint(World, OutHit.ImpactPoint, 10.f, ImpactColor, false, -1.f, 0);
    			break;
    		case EDrawDebugTrace::ForDuration:
    			DrawDebugLine(World, Start, End, LineColor, false, DrawDuration, 0, 1.f);
    			if (bHit) DrawDebugPoint(World, OutHit.ImpactPoint, 10.f, ImpactColor, false, DrawDuration, 0);
    			break;
    		case EDrawDebugTrace::Persistent:
    			DrawDebugLine(World, Start, End, LineColor, true, -1.f, 0, 1.f);
    			if (bHit) DrawDebugPoint(World, OutHit.ImpactPoint, 10.f, ImpactColor, true, -1.f, 0);
    			break;
    		default:
    			break;
    		}
    		return bHit;
    	}
    	return false;
    }

	/**
	 * Determines the type of keyboard layout currently in use on the system.
	 *
	 * @return A string representing the keyboard type ("Azerty", "Qwerty"), or "UNKNOWN_TYPE" if the layout is unrecognized.
	 */
	static FString GetKeyboardType()
    {
    	const UINT Layout = LOWORD(reinterpret_cast<uintptr_t>(GetKeyboardLayout(0)));

    	static const TMap<UINT, FString> TableMap =
    	{
    		{0x040C, "Azerty"},
    		{0x0409, "Qwerty"},
    	};

    	if (!TableMap.Contains(Layout)) return "UNKNOWN_TYPE";
    	
    	const FString& Type = TableMap.FindRef(Layout);
    	return Type;
    }

	/**
	 * Compiles a Blueprint specified by its file path and provides detailed results.
	 *
	 * @param BlueprintPath The file path to the Blueprint that needs to be compiled.
	 * @param bOutSuccess Output parameter indicating whether the compilation was successful.
	 * @param OutInfoMessage Output parameter containing detailed log messages, warnings, and errors from the compilation process.
	 */
	static void CompileBlueprint(const FString& BlueprintPath, bool& bOutSuccess, FString& OutInfoMessage)
    {
	    UBlueprint* Blueprint = Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(), nullptr, *BlueprintPath));

    	if (!Blueprint)
    	{
    		bOutSuccess = false;
    		OutInfoMessage = FString::Printf(TEXT("Compile Blueprint Failed - Path doesn't lead to a valid Blueprint. %s"), *BlueprintPath);
    		return;
    	}
    	FCompilerResultsLog Result;
    	FKismetEditorUtilities::CompileBlueprint(Blueprint, EBlueprintCompileOptions::SkipSave, &Result);

    	FString Logs = Result.Messages.Num() > Result.NumWarnings + Result.NumErrors ? "\n--- Logs ---" : "";
    	FString Warnings = Result.NumWarnings > 0 ? "\n--- Warnings ---" : "";
    	FString Errors = Result.NumErrors > 0 ? "\n--- Errors ---" : "";

    	for (TSharedRef<FTokenizedMessage> Message : Result.Messages)
    	{
    		switch (Message.Get().GetSeverity())
    		{
    		default:
    		case EMessageSeverity::Type::Info:
    			Logs += "\n" + Message.Get().ToText().ToString();
    			break;
    			case EMessageSeverity::Type::Warning:
    			case EMessageSeverity::Type::PerformanceWarning:
    			Warnings += "\n" + Message.Get().ToText().ToString();
    			break;
    			case EMessageSeverity::Type::Error:
    			Errors += "\n" + Message.Get().ToText().ToString();
    			break;
    		}
    	}
    	bOutSuccess = Result.NumErrors == 0;
    	const FString SuccessOfFailed = bOutSuccess ? "Succeeded" : "Failed";
    	const FString Messages = Logs + Warnings + Errors;
    	OutInfoMessage = FString::Printf(TEXT("Compile Blueprint %s - %s %s"), *SuccessOfFailed, *BlueprintPath, *Messages);
    }
};

/**
 * Represents a 2D line segment using a start and end point.
 * Useful for basic geometry drawing, UI lines, or interaction debug visuals.
 */
struct FLines
{
	/** Starting point of the line segment (in 2D space). */
	FVector2D Start;
	/** Ending point of the line segment (in 2D space). */
	FVector2D End;
	
	/** Default constructor initializing both points to zero vector. */	
	FLines() :
	 Start(FVector2D::ZeroVector)
	,End(FVector2D::ZeroVector)
	{}
	/**
	 * Constructs a 2D line from two explicit points.
	 *
	 * @param InStart Starting point of the line.
	 * @param InEnd Ending point of the line.
	 */	
	FLines(const FVector2D& InStart, const FVector2D& InEnd) :
	 Start(InStart)
	,End(InEnd)
	{}
};

/**
 * Enum representing simple mathematical operations.
 * Can be used to drive value modification logic dynamically in code or Blueprint.
 */
UENUM(BlueprintType)
enum class EMathOperation : uint8
{
	/** Adds the value to the current one (final = current + value) */
	E_Add UMETA(DisplayName = "Add"),
	/** Multiplies the current value by the given value (final = current * value) */
	E_Multiply UMETA(DisplayName = "Multiply"),
	/** Divides the current value by the given value (final = current / value) */
	E_Divide UMETA(DisplayName = "Divide"),
	/** Overrides the current value with the given one (final = value) */
	E_Override UMETA(DisplayName = "Override"),
	/** Invalid operation, used for error handling or default states */
	E_Invalide UMETA(DisplayName = "Invalide"),
};