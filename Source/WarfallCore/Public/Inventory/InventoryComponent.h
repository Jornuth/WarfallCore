#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/InventoryTypes.h"

#include "InventoryComponent.generated.h"

UCLASS(ClassGroup = (Inventory), meta = (BlueprintSpawnableComponent))
class WARFALLCORE_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Containers")
	FContainerData Backpack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Containers")
	FContainerMeta BackpackMeta;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Containers")
	TArray<FContainerData> Pockets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Containers")
	TArray<FContainerMeta> PocketsMeta;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Containers")
	int32 GetPocketCount() const { return Pockets.Num(); }

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItemAuto(const FName RowID, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void MergeAll();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AutoArrangeAll();
	
	// ===== RPC =====
	UFUNCTION(Server, Reliable) void ServerRequestMoveItem(const FGuid& InstanceGuid, const FContainerId& From, const FContainerId& To, const FGridCoord& ToPos, bool bRotate);
	UFUNCTION(Server, Reliable) void ServerRequestSplitItem(const FGuid& InstanceGuid, const FContainerId& From, int32 Amount);
	UFUNCTION(Server, Reliable) void ServerRequestMergeInstances(const FGuid& SourceGuid, const FGuid& TargetGuid);
	UFUNCTION(Server, Reliable) void ServerRequestEquip(const FGuid& InstanceGuid, FName Slot);
	UFUNCTION(Server, Reliable) void ServerRequestUnequip(FName Slot);
	UFUNCTION(Server, Reliable) void ServerRequestDrop(const FGuid& InstanceGuid, int32 Count);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	bool TryAutoPlaceInBest(FItemInstance& Instance, const FItemRow& Row);
	
	bool TryAutoPlaceInContainer(FItemInstance& Instance, const FItemRow& Row, FContainerData& Data, const FContainerMeta& Meta);
	
	bool TryMergeIntoContainer(FItemInstance& Instance, const FItemRow& Row, FContainerData& Data, const FContainerMeta& Meta);
	
	int32 PlaceNewStacks(FItemInstance& Instance, const FItemRow& Row, FContainerData& Data, const FContainerMeta& Meta);
	
	bool FindFreeSlot(const FIntPoint& Needs, const FContainerData& Data, const FContainerMeta& Meta, FGridCoord& OutPos, bool& bOutRotate);

	void BuildOccupancy(TArray<uint8>& Occ, const FContainerData& Data, const FContainerMeta& Meta) const;

	void AutoArrange(FContainerData& Data, const FContainerMeta& Meta);
};