// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Utils/Tables.h"
#include "WarfallCharacter.generated.h"

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

	UFUNCTION()
	static bool AddItem(FName RowID, int32 Count, FItemRow& OutRow, FItemInstance& Out);

	UFUNCTION(Exec)
	static void Test1();
	UFUNCTION(Exec)
	static void Test2();
	UFUNCTION(Exec)
	static void Test3();
	UFUNCTION(Exec)
	static void Test4();
	UFUNCTION(Exec)
	static void Test5();
};
