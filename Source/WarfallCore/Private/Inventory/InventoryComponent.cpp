#include "Inventory/InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Utils/Tables.h"

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

bool UInventoryComponent::AddItemAuto(const FName RowID, int32 Amount)
{
	if (!UTables::GetTable(ETablePath::ItemsTable) || !RowID.IsValid() || Amount <= 0) return false;

	const FItemRowDetail* RowDetail = UTables::GetTable(ETablePath::ItemsTable)->FindRow<FItemRowDetail>(RowID, TEXT(""));
	if (!RowDetail) return false;
	const FItemRow& Row = RowDetail->Details;
	FItemInstance Inst;
	if (!WfInv::CreateInstance(Row, RowID, Amount, 1.f, Inst)) return false;
	return TryAutoPlaceInBest(Inst, Row);
}

bool UInventoryComponent::TryAutoPlaceInBest(FItemInstance& Instance, const FItemRow& Row)
{
	for (int32 i = 0; i < Pockets.Num(); ++i)
	{
		if (!WfInv::PassesFilters(PocketsMeta[i], Row)) continue;
		FItemInstance Tmp = Instance;
		if (TryAutoPlaceInContainer(Tmp, Row, Pockets[i], PocketsMeta[i]))
		{
			Instance = Tmp;
			if (Instance.Count <= 0) return true;
		}
	}

	if (TryAutoPlaceInContainer(Instance, Row, Backpack, BackpackMeta))
	{
		return true;
	}

	return false;
}

bool UInventoryComponent::TryAutoPlaceInContainer(FItemInstance& Instance, const FItemRow& Row, FContainerData& Data,
	const FContainerMeta& Meta)
{
	if (TryMergeIntoContainer(Instance, Row, Data, Meta))
	{
		if (Instance.Count <= 0) return true;
	}

	const int32 Placed = PlaceNewStacks(Instance, Row, Data, Meta);
	return Placed > 0;
}

bool UInventoryComponent::TryMergeIntoContainer(FItemInstance& Instance, const FItemRow& Row, FContainerData& Data,
	const FContainerMeta& Meta)
{
	bool bAny = false;
	const int32 MaxStack = FMath::Max(1, WfInv::RowMaxStackSize(Row));

	for (FContainerEntry& E : Data.Entries)
	{
		FItemInstance& Target = E.Instance;
		const bool AHasOwner = Instance.BoundOwnerId.IsValid();
		const bool BHasOwner = Target.BoundOwnerId.IsValid();
		if (Instance.RowID != Target.RowID) continue;
		if ((AHasOwner || BHasOwner) && !(AHasOwner && BHasOwner && Instance.BoundOwnerId == Target.BoundOwnerId))
			continue;

		const int32 Room = MaxStack - Target.Count;
		if (Room <= 0) continue;

		const int32 Move = FMath::Min(Room, Instance.Count);
		Target.Count += Move;
		Instance.Count -= Move;
		bAny = true;
		if (Instance.Count <= 0) break;
	}
	return bAny;
}

int32 UInventoryComponent::PlaceNewStacks(FItemInstance& Instance, const FItemRow& Row, FContainerData& Data,
	const FContainerMeta& Meta)
{
	int32 TotalPlaced = 0;
	const int32 MaxStack = FMath::Max(1, WfInv::RowMaxStackSize(Row));
	const FIntPoint FP = WfInv::RowFootprint(Row);

	while (Instance.Count > 0)
	{
		const int32 ThisStackCount = FMath::Min(MaxStack, Instance.Count);
		FGridCoord Pos; bool bRotate = false;

		const bool bFound = FindFreeSlot(FP, Data, Meta, Pos, bRotate);
		if (!bFound) break;

		FItemInstance NewStack = Instance;
		NewStack.InstanceGuid = FGuid::NewGuid();
		NewStack.Count = ThisStackCount;

		if (NewStack.Perish.NextPerishEpoch > 0 && Meta.PerishRateMultiplier != 1.f)
		{
			const int64 Now = FDateTime::Now().ToUnixTimestamp();
			const int64 Remain = FMath::Max<int64>(0, NewStack.Perish.NextPerishEpoch - Now);
			const int64 Adjust = (int64)(Remain * Meta.PerishRateMultiplier);
			NewStack.Perish.NextPerishEpoch = Now + Adjust;
		}

		FContainerEntry Entry;
		Entry.Instance = NewStack;
		Entry.TopLeft = Pos;
		Entry.bRotated90 = bRotate;
		Entry.FootprintPlaced = bRotate ? FIntPoint(FP.Y, FP.X) : FP;

		Data.Entries.Add(Entry);

		Instance.Count -= ThisStackCount;
		TotalPlaced += ThisStackCount;
	}
	return TotalPlaced;
}

void UInventoryComponent::BuildOccupancy(TArray<uint8>& Occ, const FContainerData& Data,
                                         const FContainerMeta& Meta) const
{
	const int32 W = Meta.Grid.Width;
	const int32 H = Meta.Grid.Height;
	Occ.SetNumZeroed(W * H);
	auto Mark = [&](int32 x, int32 y) { if (x >=0 && y >= 0 && x < W && y < H) Occ[y * W + x] = 1; };

	for (const FContainerEntry& E : Data.Entries)
	{
		for (int32 dx = 0; dx < E.FootprintPlaced.X; ++dx)
			for (int32 dy = 0; dy < E.FootprintPlaced.Y; ++dy)
				Mark(E.TopLeft.X + dx, E.TopLeft.Y + dy);
	}
}

bool UInventoryComponent::FindFreeSlot(const FIntPoint& Needs, const FContainerData& Data, const FContainerMeta& Meta,
	FGridCoord& OutPos, bool& bOutRotate)
{
	const int32 W = Meta.Grid.Width;
	const int32 H = Meta.Grid.Height;
	TArray<uint8> Occ; BuildOccupancy(Occ, Data, Meta);

	auto FitsAt = [&](int32 sx, int32 sy, const FIntPoint& Size)->bool
	{
		if (sx + Size.X > W || sy + Size.Y > H) return false;
		for (int32 x = 0; x < Size.X; ++x)
			for (int32 y = 0; y < Size.Y; ++y)
				if (Occ[(sy + y) * W + (sx + x)]) return false;
		return true;
	};

	const bool bCanRotate = (Needs.X != Needs.Y);
	const FIntPoint R0 = Needs;
	const FIntPoint R1 = FIntPoint(Needs.Y, Needs.X);

	for (int32 y = 0; y < H; ++y)
	{
		for (int32 x = 0; x < W; ++x)
		{
			if (FitsAt(x, y, R0)) { OutPos = {x, y}; bOutRotate = false; return true; }
			if (bCanRotate && FitsAt(x, y, R1)) { OutPos = {x, y}; bOutRotate = true; return true; }
		}
	}
	return false;
}

void UInventoryComponent::MergeAll()
{
	auto DoMerge = [&](FContainerData& Data, const FContainerMeta& Meta)
	{
		for (int32 i = 0; i < Data.Entries.Num(); ++i)
		{
			for (int32 j = i + 1; j < Data.Entries.Num();)
			{
				auto& A = Data.Entries[i].Instance;
				auto& B = Data.Entries[j].Instance;
				if (A.RowID == B.RowID)
				{
					const bool AHas = A.BoundOwnerId.IsValid();
					const bool BHas = B.BoundOwnerId.IsValid();
					if (!(AHas || BHas) || (AHas && BHas && A.BoundOwnerId == B.BoundOwnerId))
					{
						const int32 MaxStack = FMath::Max(1, WfInv::RowMaxStackSize(UTables::GetTable(ETablePath::ItemsTable)->FindRow<FItemRowDetail>(A.RowID, TEXT("MergeAll"))->Details));
						const int32 Room = MaxStack - A.Count;
						if (Room > 0)
						{
							const int32 Move = FMath::Min(Room, B.Count);
							A.Count += Move;
							B.Count -= Move;
							if (B.Count <= 0) { Data.Entries.RemoveAtSwap(j); continue; }
						}
					}
				}
				++j;
			}
		}
	};
	
	DoMerge(Backpack, BackpackMeta);
	for (int32 i = 0; i < Pockets.Num(); ++i) DoMerge(Pockets[i], PocketsMeta[i]);
}

void UInventoryComponent::AutoArrange(FContainerData& Data, const FContainerMeta& Meta)
{
	TArray<FContainerEntry> Entries = Data.Entries;
	Data.Entries.Reset();
	UDataTable* Table = UTables::GetTable(ETablePath::ItemsTable);

	Entries.Sort([&](const FContainerEntry& A, const FContainerEntry& B)
	{
		const FItemRowDetail* AR = Table ? Table->FindRow<FItemRowDetail>(A.Instance.RowID, TEXT("AutoArrange.A")) : nullptr;
		const FItemRowDetail* BR = Table ? Table->FindRow<FItemRowDetail>(B.Instance.RowID, TEXT("AutoArrange.B")) : nullptr;
		const int32 SA = AR ? WfInv::Surface(WfInv::RowFootprint(AR->Details)) : WfInv::Surface(A.FootprintPlaced);
		const int32 SB = BR ? WfInv::Surface(WfInv::RowFootprint(BR->Details)) : WfInv::Surface(B.FootprintPlaced);

		if (SA != SB) return SA > SB;
		return A.Instance.RowID.LexicalLess(B.Instance.RowID);
	});

	for (FContainerEntry& E : Entries)
	{
		const FItemRowDetail* Row = Table ? Table->FindRow<FItemRowDetail>(E.Instance.RowID, TEXT("AutoArrange.Place")) : nullptr;
		const FIntPoint FP = Row ? WfInv::RowFootprint(Row->Details) : E.FootprintPlaced;
		FGridCoord Pos; bool bRot = false;
		if (FindFreeSlot(FP, Data, Meta, Pos, bRot))
		{
			E.TopLeft = Pos;
			E.bRotated90 = bRot;
			E.FootprintPlaced = bRot ? FIntPoint(FP.Y, FP.X) : FP;
			Data.Entries.Add(E);
		}
		else
		{
			Data.Entries.Add(E);
		}
	}
}

void UInventoryComponent::AutoArrangeAll()
{
	AutoArrange(Backpack, BackpackMeta);
	for (int32 i = 0; i < Pockets.Num(); ++i) AutoArrange(Pockets[i], PocketsMeta[i]);
}


