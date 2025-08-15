#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/InventoryTypes.h"

#include "InventoryComponent.generated.h"

// ==========================================================================
//  Structures
// ==========================================================================

USTRUCT()
struct FPerishNode
{
	GENERATED_BODY()

	int64 Epoch = 0;
	FGuid InstanceGuid;
};

// ==========================================================================
//  UInventoryComponent
// ==========================================================================

UCLASS(ClassGroup = (Inventory), meta = (BlueprintSpawnableComponent))
class WARFALLCORE_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	// ========== FUNCTIONS ==========
	UInventoryComponent();

	/**
	 * Retrieves the total number of pockets in the inventory system.
	 *
	 * - This function provides the count of pocket containers currently available
	 *   within the inventory. Each pocket represents an organizational unit for items.
	 * - It is callable from Blueprints for runtime or editor usage.
	 *
	 * Attributes:
	 * - BlueprintCallable: Allows this function to be executed or called within Blueprint scripts.
	 * - Category = "Inventory|Containers": Groups the function under the specified category in the editor.
	 *
	 * @return The number of pockets in the inventory.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Containers")
	int32 GetPocketCount() const { return Pockets.Num(); }

	/**
	 * Retrieves all item entries currently available in the player's backpack.
	 *
	 * - This function provides access to the list of items stored in the backpack container.
	 * - It is designed for use within Blueprints to query the backpack's contents at runtime or during editing.
	 *
	 * Attributes:
	 * - BlueprintPure: Ensures the function does not modify the owning object or its assets.
	 * - Category = "Inventory|Containers": Groups the function under the specified category in the editor.
	 *
	 * @return An array of FContainerEntry objects representing the items in the backpack.
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory|Containers")
	TArray<FContainerEntry> GetBackpackItems() const { return Backpack.Entries; }

	/**
	 * Retrieves the list of items contained in a specific pocket of the inventory.
	 *
	 * - This function returns an array of container entries, representing the items stored
	 *   within the pocket specified by the given index. Each container entry contains item
	 *   data and associated properties.
	 * - It is callable from Blueprints for runtime or editor scenarios.
	 *
	 * Attributes:
	 * - BlueprintPure: Indicates that the function does not modify the state and can be called
	 *   in a Blueprint to obtain data without side-effects.
	 * - Category = "Inventory|Containers": Groups the function under the specified category in the editor.
	 *
	 * @param PocketIndex The index of the pocket whose items are to be retrieved.
	 * @return An array of FContainerEntry objects representing the items in the specified pocket.
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory|Containers")
	TArray<FContainerEntry> GetPocketItems(const int32 PocketIndex) const { return Pockets[PocketIndex].Entries; }

	/**
	 * Determines if the pocket filters for a specific pocket index can be edited.
	 *
	 * - Verifies if the provided pocket index is valid and checks whether the filters
	 *   associated with the specified pocket are editable.
	 * - This function is callable from Blueprints for runtime or editor use.
	 *
	 * Attributes:
	 * - BlueprintPure: Indicates the function does not modify the owning object and can be used for data retrieval.
	 * - Category = "Inventory|Containers": Groups the function under the specified category in the editor.
	 *
	 * @param PocketIndex The index of the pocket to check for filter editability.
	 * @return true if the pocket filters can be edited; otherwise, false.
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory|Containers")
	bool CanEditPocketFilters(const int32 PocketIndex) const { return PocketsMeta.IsValidIndex(PocketIndex) && PocketsMeta[PocketIndex].bFiltersEditable;}
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItemAuto(const FName RowID, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void MergeAll();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AutoArrangeAll();
	
	// ===== RPC SERVER ====
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

	// Build an ASCII snapshot of a container grid for on-screen debug (no UMG).
	void BuildAsciiGrid(const FContainerData& Data, const FContainerMeta& Meta, TArray<FString>& OutLines) const;
	
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
	bool FitsAt(const FContainerData& Data, const FContainerMeta& Meta, FGridCoord Pos, FIntPoint Size) const;

	void Perish_EnqueueEntry(const FItemInstance& Inst);
	void Perish_RemoveGuid(const FGuid& Guid);
	void Perish_RebuildFor(const FItemInstance& Inst);
	void Perish_ScheduleNext();
	void Perish_OnTimer();
	void Perish_RebuildIndex();

	bool FindAnyEntryByGuid(const FGuid& Guid, FContainerData*& OutData, FContainerMeta*& OutMeta, int32& OutIndex);
	
	static FContainerEntry* FindEntry(FContainerData& Data, const FGuid& InstanceGuid, int32* OutIndex=nullptr);
	static const FContainerEntry* FindEntry(const FContainerData& Data, const FGuid& InstanceGuid, int32* OutIndex=nullptr);
	
	static void MarkAdded(FContainerData& Data, FContainerEntry& Entry);
	static void MarkChanged(FContainerData& Data, FContainerEntry& Entry);
	void MarkRemoved(FContainerData& Data, int32 Index);
	

	// ========== VARIABLES ==========
public:	
	/**
	 * Represents the player's backpack container in the inventory system.
	 *
	 * - This property is synchronized between server and clients.
	 * - It is intended to be used for inventory-related operations, such as retrieving, adding,
	 *   or manipulating items within the backpack.
	 *
	 * Attributes:
	 * - Replicated: Ensures that the property is correctly synchronized in a networked environment.
	 * - VisibleAnywhere: The property is visible in the editor but cannot be edited directly.
	 * - BlueprintReadOnly: Allows read access to the property in Blueprints but disallows modification.
	 * - Category = "Inventory|Containers": Groups the property under the specified category in the editor.
	 */
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Containers")
	FContainerData Backpack;

	/**
	 * Metadata for the player's backpack container in the inventory system.
	 *
	 * - This property is synchronized between server and clients.
	 * - It is used to define static attributes or properties of the backpack container,
	 *   which influence its behavior or constraints in inventory-related operations.
	 *
	 * Attributes:
	 * - Replicated: Ensures the property is correctly synchronized in networked environments.
	 * - EditAnywhere: Allows the property to be edited in the editor, regardless of context.
	 * - BlueprintReadWrite: Enables both read and write access in Blueprints.
	 * - Category = "Inventory|Containers": Groups the property under the specified category in the editor.
	 */
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Inventory|Containers")
	FContainerMeta BackpackMeta;

	/**
	 * Represents the player's pocket containers in the inventory system.
	 *
	 * - This property is synchronized between server and clients.
	 * - It is used to store and manage individual pockets within the inventory,
	 *   typically for organizing or compartmentalizing items.
	 *
	 * Attributes:
	 * - Replicated: Ensures that the property is correctly synchronized in a networked environment.
	 * - VisibleAnywhere: The property is visible in the editor but cannot be edited directly.
	 * - BlueprintReadOnly: Allows read access to the property in Blueprints but disallows modification.
	 * - Category = "Inventory|Containers": Groups the property under the specified category in the editor.
	 */
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Containers")
	TArray<FContainerData> Pockets;

	/**
	 * Represents metadata for the player's pocket containers within the inventory system.
	 *
	 * - This property is synchronized between server and clients.
	 * - It is used to define and manage characteristics and static properties of the individual pockets,
	 *   which influence their behavior or constraints in inventory operations.
	 *
	 * Attributes:
	 * - Replicated: Ensures that the property is correctly synchronized in a networked environment.
	 * - EditAnywhere: Allows the property to be edited in the editor, regardless of context.
	 * - BlueprintReadWrite: Enables both read and write access in Blueprints.
	 * - Category = "Inventory|Containers": Groups the property under the specified category in the editor.
	 */
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Inventory|Containers")
	TArray<FContainerMeta> PocketsMeta;

private:
	FTimerHandle PerishTimer;
	TArray<FPerishNode> PerishHeap;
	TMap<FGuid, int32> PerishGuidIndex;
};