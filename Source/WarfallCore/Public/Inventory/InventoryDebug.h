#pragma once
#include "Inventory/InventoryTypes.h"
#include "Inventory/InventoryComponent.h"

namespace WfInvDbg
{
	void BuildAsciiGrid(const FContainerData& Data, const FContainerMeta& Meta, TArray<FString>& OutLines);
	bool UI_ParseContainerId(UInventoryComponent* Inv, const FString& Which, FContainerId& OutId, FContainerData*& OutData);
}