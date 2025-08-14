// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VisualItem.generated.h"

UCLASS()
class WARFALLCORE_API AVisualItem : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVisualItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
