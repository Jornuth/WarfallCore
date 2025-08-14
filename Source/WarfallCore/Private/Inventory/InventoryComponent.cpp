#include "Inventory/InventoryComponent.h"
#include "Net/UnrealNetwork.h"

UInventoryComponent::UInventoryComponent()
{
	SetIsReplicatedByDefault(true);
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, Backpack);
	DOREPLIFETIME(UInventoryComponent, Pockets);
}

// RPC
void UInventoryComponent::ServerRequestMoveItem_Implementation(const FGuid&, const FContainerId&, const FContainerId&, const FGridCoord&, bool) {}
void UInventoryComponent::ServerRequestSplitItem_Implementation(const FGuid&, const FContainerId&, int32) {}
void UInventoryComponent::ServerRequestMergeInstances_Implementation(const FGuid&, const FGuid&) {}
void UInventoryComponent::ServerRequestEquip_Implementation(const FGuid&, FName) {}
void UInventoryComponent::ServerRequestUnequip_Implementation(FName) {}
void UInventoryComponent::ServerRequestDrop_Implementation(const FGuid&, int32) {}

