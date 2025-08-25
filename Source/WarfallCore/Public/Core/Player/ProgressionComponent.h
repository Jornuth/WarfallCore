#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProgressionComponent.generated.h"




UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WARFALLCORE_API UProgressionComponent : public UActorComponent
{
	GENERATED_BODY()

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNewDiscovery, FDataTableRowHandle, Discovery);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FForgetDiscovery, FDataTableRowHandle, Discovery);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLearnRecipe, FDataTableRowHandle, Recipe);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FForgetRecipe, FDataTableRowHandle, Recipe);
	// ========== FUNCTIONS ==========
public:
	void AddNewDiscovery(const FDataTableRowHandle& NewDiscovery);
	void RemoveDiscovery(const FDataTableRowHandle& DiscoveryToRemove);
	void ClearDiscoveries();
	bool HasDiscovery(const FDataTableRowHandle& DiscoveryToCheck) const;
	TArray<FDataTableRowHandle> GetDiscoveries() { return Discovery;}

	UFUNCTION(BlueprintCallable)
	void LearnRecipe(const FDataTableRowHandle& Recipe);
	UFUNCTION(BlueprintCallable)
	void ForgetRecipe(const FDataTableRowHandle& Recipe);
	UFUNCTION(BlueprintCallable)
	bool HasRecipe(const FDataTableRowHandle& Recipe) const;
	
	void ClearRecipes();
	
	/**
	 * Delegate that is triggered when a new discovery is added.
	 *
	 * This dynamic multicast delegate is intended for use within the UProgressionComponent class
	 * to notify when a new discovery has been successfully added using the associated data
	 * from a DataTableRowHandle.
	 *
	 * The delegate is marked as BlueprintAssignable, enabling event handling directly within Blueprints,
	 * offering designers and developers an intuitive way to respond to the addition of new discoveries.
	 *
	 * @param Discovery The data table row handle associated with the discovery that is added.
	 */
	UPROPERTY(BlueprintAssignable)
	FNewDiscovery OnNewDiscovery;
	
	/**
	 * Delegate that is triggered when a discovery is Forgeted or removed.
	 *
	 * This dynamic multicast delegate is bound to the blueprint system and can be used
	 * to notify when a specific discovery has been Forgeted using the associated data
	 * from a DataTableRowHandle.
	 *
	 * It operates within the UProgressionComponent class and serves as a point of
	 * interaction or response for managing Forgeted discovery events during gameplay.
	 *
	 * BlueprintAssignable property allows this to be assigned and used directly in
	 * Blueprints, providing intuitive and accessible event handling for designers.
	 *
	 * @param Discovery The data table row handle associated with the discovery that is Forgeted.
	 */
	UPROPERTY(BlueprintAssignable)
	FForgetDiscovery OnForgetDiscovery;

	UPROPERTY(BlueprintAssignable)
	FLearnRecipe OnLearnNewRecipe;

	UPROPERTY(BlueprintAssignable)
	FForgetRecipe OnForgetRecipe;
	
	// ========== VARIABLES ==========
	
private:
	TArray<FDataTableRowHandle> Discovery;
	TArray<FDataTableRowHandle> KnownRecipes;
};
