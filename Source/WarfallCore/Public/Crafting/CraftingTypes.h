#pragma once

#include "CoreMinimal.h"
#include "Custom/Variables/ChildsHandle.h"
#include "Inventory/ItemRowTypes.h"
#include "UObject/Object.h"
#include "CraftingTypes.generated.h"

/**
 * 
 */
UCLASS()
class WARFALLCORE_API UCraftingTypes : public UObject
{
	GENERATED_BODY()

	
};

USTRUCT(BlueprintType)
struct FItemIngredient
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FItemRowHandle Ingredient;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity;

	FItemIngredient() :
	 Ingredient(FItemRowHandle("Result")),
	Quantity(1)
	{}
};

USTRUCT(BlueprintType)
struct FItemResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FItemRowHandle Result;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity;

	FItemResult() :
	 Result(FItemRowHandle(NAME_None)),
	Quantity(1)
	{}
};

USTRUCT(BlueprintType)
struct FRecipeRow
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FItemIngredient> Ingredients;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FItemResult> Results;

	FRecipeRow() :
	 Name(FText::GetEmpty())
	,Description(FText::GetEmpty())	
	{}

	TArray<FItemMetaEntry> OutMeta(const int32 Index) const
	{
		const UDataTable* Table = UTables::GetTable(ETablePath::ItemsTable);
		if (!Table) { return TArray<FItemMetaEntry>(); }
		const FName ID = Results[Index].Result.ID;
		if (!ID.IsValid()) { return TArray<FItemMetaEntry>(); }

		FItemRowDetail* RowDetail = Table->FindRow<FItemRowDetail>(ID, TEXT("Finding"));
		if (!RowDetail) { return TArray<FItemMetaEntry>(); }

		return RowDetail->Details.MetaModifiers;
	};

	TArray<
};