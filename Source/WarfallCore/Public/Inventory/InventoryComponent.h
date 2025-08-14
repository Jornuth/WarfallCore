#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/InventoryContainers.h"

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

	// ===== RPC =====
	UFUNCTION(Server, Reliable) void ServerRequestMoveItem(const FGuid& InstanceGuid, const FContainerId& From, const FContainerId& To, const FGridCoord& ToPos, bool bRotate);
	UFUNCTION(Server, Reliable) void ServerRequestSplitItem(const FGuid& InstanceGuid, const FContainerId& From, int32 Amount);
	UFUNCTION(Server, Reliable) void ServerRequestMergeInstances(const FGuid& SourceGuid, const FGuid& TargetGuid);
	UFUNCTION(Server, Reliable) void ServerRequestEquip(const FGuid& InstanceGuid, FName Slot);
	UFUNCTION(Server, Reliable) void ServerRequestUnequip(FName Slot);
	UFUNCTION(Server, Reliable) void ServerRequestDrop(const FGuid& InstanceGuid, int32 Count);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};