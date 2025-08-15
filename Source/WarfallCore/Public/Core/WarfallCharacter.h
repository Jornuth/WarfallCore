#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Utils/Tables.h"
#include "WarfallCharacter.generated.h"

class UInventoryComponent;

UCLASS()
class WARFALLCORE_API AWarfallCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AWarfallCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UInventoryComponent> Inventory;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Warfall|UI")
	TSubclassOf<class UUserWidget> InventoryDebugWidgetClass;

	UFUNCTION(Exec) void iv_ui();
	
	UFUNCTION(Exec) void iv_p2_setup(int32 PocketW = 3, int32 PocketH = 3, float PerishMult = 0.5f);
	UFUNCTION(Exec) void iv_p2_add(FName RowID, int32 Count = 1);
	UFUNCTION(Exec) void iv_p2_arrange();
	UFUNCTION(Exec) void iv_p2_setfilters(int32 PocketIndex, const FString& Ops); // ex: "+Food,+Herb,-Ammo"
	UFUNCTION(Exec) void iv_p2_summary();
	UFUNCTION(Exec) void iv_p2_overlay(const FString& Which /* "bp" = backpack, "p0","p1",... */);
	
	// ReSharper disable once CppUE4CodingStandardNamingViolationWarning
	UFUNCTION(Exec) void iv_p3_move(const FString& From, int32 Index, const FString& To, int32 X, int32 Y, int32 bRotate);
    UFUNCTION(Exec) void iv_p3_move_auto(const FString& From, int32 Index, const FString& To);
    UFUNCTION(Exec) void iv_p3_split(const FString& From, int32 Index, int32 Amount);
    UFUNCTION(Exec) void iv_p3_merge(const FString& From, int32 IndexFrom, const FString& To, int32 IndexTo);
    UFUNCTION(Exec) void iv_p3_guid(const FString& From, int32 Index);   // utilitaire: afficher le GUID d’une entrée
	
};
