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

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Inventory|Containers")
	FContainerMeta BackpackMeta;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Containers")
	TArray<FContainerData> Pockets;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Inventory|Containers")
	TArray<FContainerMeta> PocketsMeta;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Containers")
	int32 GetPocketCount() const { return Pockets.Num(); }

	UFUNCTION(BlueprintPure, Category = "Inventory|Containers")
	TArray<FContainerEntry> GetBackpackItems() const { return Backpack.Entries; }

	UFUNCTION(BlueprintPure, Category = "Inventory|Containers")
	TArray<FContainerEntry> GetPocketItems(const int32 PocketIndex) const { return Pockets[PocketIndex].Entries; }
	
	UFUNCTION(BlueprintPure, Category = "Inventory|Containers")
	bool CanEditPocketFilters(const int32 PocketIndex) const { return PocketsMeta.IsValidIndex(PocketIndex) && PocketsMeta[PocketIndex].bFiltersEditable;}
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItemAuto(const FName RowID, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void MergeAll();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AutoArrangeAll();
	
	// ===== RPC ====
	UFUNCTION(Server,Reliable) void ServerAddItemAuto(FName RowID, int32 Amount);	
	UFUNCTION(Server, Reliable) void ServerRequestMoveItem(const FGuid& InstanceGuid, const FContainerId& From, const FContainerId& To, const FGridCoord& ToPos, bool bRotate);
	UFUNCTION(Server, Reliable) void ServerRequestSplitItem(const FGuid& InstanceGuid, const FContainerId& From, int32 Amount);
	UFUNCTION(Server, Reliable) void ServerRequestMergeInstances(const FGuid& SourceGuid, const FGuid& TargetGuid);
	UFUNCTION(Server, Reliable) void ServerRequestEquip(const FGuid& InstanceGuid, FName Slot);
	UFUNCTION(Server, Reliable) void ServerRequestUnequip(FName Slot);
	UFUNCTION(Server, Reliable) void ServerRequestDrop(const FGuid& InstanceGuid, int32 Count);
	UFUNCTION(Server, Reliable) void ServerSetPocketFilters(int32 PocketIndex, const FGameplayTagContainer& NewWhitelist, const FGameplayTagContainer& NewBlacklist);

	// === UI debug wrappers (server-authoritative) ===
	UFUNCTION(BlueprintCallable, Category="Warfall|Inventory|UI")
	void ServerMoveByIndex(const FString& From, int32 Index, const FString& To, int32 X, int32 Y, bool bRotate);
	UFUNCTION(BlueprintCallable, Category="Warfall|Inventory|UI")
	void ServerMoveAutoByIndex(const FString& From, int32 Index, const FString& To);
	UFUNCTION(BlueprintCallable, Category="Warfall|Inventory|UI")
	void ServerSplitByIndex(const FString& From, int32 Index, int32 Amount);
	UFUNCTION(BlueprintCallable, Category="Warfall|Inventory|UI")
	void ServerMergeByIndex(const FString& From, int32 IndexFrom, const FString& To, int32 IndexTo);

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

	bool ResolveContainer(const FContainerId& Id, FContainerData*& OutData, FContainerMeta*& OutMeta, int32* OutPocketIndex=nullptr);

	static FContainerEntry* FindEntry(FContainerData& Data, const FGuid& InstanceGuid, int32* OutIndex=nullptr);
	static const FContainerEntry* FindEntry(const FContainerData& Data, const FGuid& InstanceGuid, int32* OutIndex=nullptr);

	// Validation de pose exacte (sans auto-rotation) à une position donnée
	bool FitsAt(const FContainerData& Data, const FContainerMeta& Meta, FGridCoord Pos, FIntPoint Size) const;

	// FastArray: marquages pour réplication
	static void MarkAdded(FContainerData& Data, FContainerEntry& Entry);
	static void MarkChanged(FContainerData& Data, FContainerEntry& Entry);
	static void MarkRemoved(FContainerData& Data, int32 Index);
	
public:
	// Build an ASCII snapshot of a container grid for on-screen debug (no UMG).
	void BuildAsciiGrid(const FContainerData& Data, const FContainerMeta& Meta, TArray<FString>& OutLines) const;

};