#include "Inventory/InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Utils/Tables.h"

using namespace WfInv;

static UDataTable* GetItemsDT()
{
	return UTables::GetTable(ETablePath::ItemsTable);
}

UInventoryComponent::UInventoryComponent()
{
	SetIsReplicatedByDefault(true);
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, Backpack);
	DOREPLIFETIME(UInventoryComponent, Pockets);
	DOREPLIFETIME(UInventoryComponent, BackpackMeta);
	DOREPLIFETIME(UInventoryComponent, PocketsMeta);
	DOREPLIFETIME(UInventoryComponent, ReplicationRevision);
}

// RPC

void UInventoryComponent::ServerAddItemAuto_Implementation(const FName RowID, const int32 Amount)
{
	const UDataTable* ItemsTable = GetItemsDT();
	if (!ItemsTable ||!RowID.IsValid() || Amount <= 0) return;

	const FItemRowDetail* Row = ItemsTable->FindRow<FItemRowDetail>(RowID, TEXT("ServerAddItemAuto"));
	if (!Row) return;
	
	FItemInstance Inst;
	if (!CreateInstance(Row->Details, RowID, Amount, 1.f, Inst)) return;
	
	TryAutoPlaceInBest(Inst, Row->Details);
	BumpRevision();
}

void UInventoryComponent::ServerMergeAll_Implementation()
{
	MergeAll();
	BumpRevision();
}

void UInventoryComponent::ServerAutoArrangeAll_Implementation()
{
	AutoArrangeAll();
	BumpRevision();
}

void UInventoryComponent::ServerRequestMergeAll_Implementation()
{
	if (GetOwnerRole() != ROLE_Authority) { MergeAll(); BumpRevision();}
	else { ServerMergeAll(); }
}

void UInventoryComponent::ServerRequestAutoArrangeAll_Implementation()
{
	if (GetOwnerRole() == ROLE_Authority) { AutoArrangeAll(); BumpRevision(); }
	else { ServerAutoArrangeAll(); }
}

void UInventoryComponent::ServerRequestMoveItem_Implementation(const FGuid& InstanceGuid,
    const FContainerId& From, const FContainerId& To, const FGridCoord& ToPos, bool bRotate)
{
    FContainerData *FromData=nullptr, *ToData=nullptr;
    FContainerMeta *FromMeta=nullptr, *ToMeta=nullptr;
    if (!ResolveContainer(From, FromData, FromMeta) || !ResolveContainer(To, ToData, ToMeta)) return;

    int32 FromIndex = INDEX_NONE;
    FContainerEntry* Entry = FindEntry(*FromData, InstanceGuid, &FromIndex);
    if (!Entry) return;

    const bool bAuto = (ToPos.X < 0 || ToPos.Y < 0);

    // Snapshot avant toute mutation
    const FGridCoord OldTopLeft = Entry->TopLeft;
    const bool       bOldRot    = Entry->bRotated90;
    const FIntPoint  FP         = Entry->FootprintPlaced;
    const FItemInstance Moving  = Entry->Instance;

    // Optionnel: si To != From, valider filtres
    const UDataTable* Items = GetItemsDT();
    const FItemRowDetail* Row = Items ? Items->FindRow<FItemRowDetail>(Moving.RowID, TEXT("Move")) : nullptr;
    if (!Row) return;
    if (ToData && ToMeta && !WfInv::PassesFilters(*ToMeta, Row->Details)) return; // respect des whitelists/blacklists

    // Retirer l'entrée source (libère la place dans le même container)
    MarkRemoved(*FromData, FromIndex);

    auto PushTo = [&](FContainerData& D, const FContainerMeta& M, const FGridCoord Pos, const bool bRot, const FIntPoint& Foot) -> bool
    {
        if (!FitsAt(D, M, Pos, Foot)) return false;
        FContainerEntry NewE;
        NewE.Instance = Moving;
        NewE.TopLeft = Pos;
        NewE.bRotated90 = bRot;
        NewE.FootprintPlaced = Foot;
        D.Entries.Add(NewE);
        MarkAdded(D, D.Entries.Last());
        Perish_RebuildFor(D.Entries.Last().Instance);
        return true;
    };

    bool bDone = false;

    if (!bAuto)
    {
        const FIntPoint TargetSize = bRotate ? FIntPoint(FP.Y, FP.X) : FP;
        bDone = PushTo(*ToData, *ToMeta, ToPos, bRotate, TargetSize);
        if (!bDone)
        {
            // Fallback: remettre EXACTEMENT où c’était
            bDone = PushTo(*FromData, *FromMeta, OldTopLeft, bOldRot, FP);
        }
    }
    else
    {
        // Auto-place dans le TO (ou, si échec, on remets dans FROM)
        const FIntPoint Canon = FP; // déjà corrigé pour la rotation existante
        if (!TryAutoPlaceInContainer(const_cast<FItemInstance&>(Moving), Row->Details, *ToData, *ToMeta))
        {
            // Fallback: back to FROM à la même position
            bDone = PushTo(*FromData, *FromMeta, OldTopLeft, bOldRot, Canon);
        }
        else
        {
            bDone = true;
        }
    }

    if (bDone)
    {
        Perish_ScheduleNext();
        BumpRevision();
    }
}

void UInventoryComponent::ServerRequestSplitItem_Implementation(const FGuid& InstanceGuid,
																const FContainerId& From,
																int32 Amount)
{
	FContainerData* FromData = nullptr; FContainerMeta* FromMeta = nullptr;
	if (!ResolveContainer(From, FromData, FromMeta)) return;

	int32 Index = INDEX_NONE;
	FContainerEntry* Entry = FindEntry(*FromData, InstanceGuid, &Index);
	if (!Entry || Amount <= 0 || Amount >= Entry->Instance.Count) return;

	// 1) Appliquer le split à la source
	FItemInstance NewInst;
	if (!Split(Entry->Instance, Amount, NewInst)) return;

	// La source a bien perdu 'Amount'
	MarkChanged(*FromData, *Entry);

	// 2) Créer un NOUVEAU stack dans le même container (PAS de merge dans ce chemin)
	UDataTable* ItemsTable = GetItemsDT();
	const FItemRowDetail* Row = ItemsTable ? ItemsTable->FindRow<FItemRowDetail>(NewInst.RowID, TEXT("Split")) : nullptr;
	if (!Row) return;

	// On tente de poser le nouveau stack tel quel (auto-place)
	int32 Placed = PlaceNewStacks(NewInst, Row->Details, *FromData, *FromMeta);

	if (Placed <= 0)
	{
		// Pas de place : on annule proprement le split (rollback source)
		Entry->Instance.Count += Amount;
		MarkChanged(*FromData, *Entry);
		BumpRevision();
		return;
	}

	// Si jamais il restait encore des unités non posées (rare, mais possible si MaxStack capte),
	// on peut essayer de les placer à nouveau tant qu'il en reste.
	while (NewInst.Count > 0)
	{
		const int32 Added = PlaceNewStacks(NewInst, Row->Details, *FromData, *FromMeta);
		if (Added <= 0) break; // plus de place
	}
	BumpRevision();
}


void UInventoryComponent::ServerRequestMergeInstances_Implementation(const FGuid& SourceGuid, const FGuid& TargetGuid)
{
	auto MergeIn = [&](FContainerData& Data, const FContainerMeta& Meta)->bool
	{
		int32 iS = -1, iT = -1;
		FContainerEntry* S = FindEntry(Data, SourceGuid, &iS);
		FContainerEntry* T = FindEntry(Data, TargetGuid, &iT);
		if (S && T)
		{
			// Même container
			const FItemRowDetail* Row = GetItemsDT()->FindRow<FItemRowDetail>(T->Instance.RowID, TEXT("MergeInstances"));
			if (!Row) return true;
			const int32 MaxStack = FMath::Max(1, RowMaxStackSize(Row->Details));
			const int32 Room = MaxStack - T->Instance.Count;
			if (Room <= 0) return true;

			const int32 Move = FMath::Min(Room, S->Instance.Count);
			T->Instance.Count += Move;
			S->Instance.Count -= Move;

			MarkChanged(Data, *T);
			Perish_RebuildFor(T->Instance);
			Perish_ScheduleNext();
			if (S->Instance.Count <= 0) { MarkRemoved(Data, iS); }
			else { MarkChanged(Data, *S); }
			BumpRevision();
			return true;
		}
		BumpRevision();
		return false;
	};

	if (MergeIn(Backpack, BackpackMeta)) return;
	for (int32 p = 0; p < Pockets.Num(); ++p) if (MergeIn(Pockets[p], PocketsMeta[p])) return;

	// Cross-container merge (Source & Target dans containers différents)
	FContainerData* SData = nullptr; FContainerMeta* SMeta = nullptr;
	FContainerData* TData = nullptr; FContainerMeta* TMeta = nullptr;
	int32 iS = -1, iT = -1;

	auto Locate = [&](const FGuid& Guid, FContainerData*& D, FContainerMeta*& M, int32& I)->bool
	{
		if (FindEntry(Backpack, Guid, &I)) { D = &Backpack; M = &BackpackMeta; return true; }
		for (int32 p = 0; p < Pockets.Num(); ++p)
		{
			if (FindEntry(Pockets[p], Guid, &I)) { D = &Pockets[p]; M = &PocketsMeta[p]; return true; }
		}
		BumpRevision();
		return false;
	};

	if (!Locate(SourceGuid, SData, SMeta, iS)) return;
	if (!Locate(TargetGuid, TData, TMeta, iT)) return;

	FContainerEntry* S = (iS >= 0) ? &SData->Entries[iS] : nullptr;
	FContainerEntry* T = (iT >= 0) ? &TData->Entries[iT] : nullptr;
	if (!S || !T) return;

	const FItemRowDetail* Row = GetItemsDT()->FindRow<FItemRowDetail>(T->Instance.RowID, TEXT("MergeInstances"));
	if (!Row) return;

	const bool AHas = S->Instance.BoundOwnerId.IsValid();
	const bool BHas = T->Instance.BoundOwnerId.IsValid();
	if ((AHas || BHas) && !(AHas && BHas && S->Instance.BoundOwnerId == T->Instance.BoundOwnerId)) return;

	const int32 MaxStack = FMath::Max(1, RowMaxStackSize(Row->Details));
	const int32 Room = MaxStack - T->Instance.Count;
	if (Room <= 0) return;

	const int32 Move = FMath::Min(Room, S->Instance.Count);
	T->Instance.Count += Move;
	S->Instance.Count -= Move;

	MarkChanged(*TData, *T);
	Perish_RebuildFor(T->Instance);
	Perish_ScheduleNext();
	if (S->Instance.Count <= 0) { MarkRemoved(*SData, iS); }
	else { MarkChanged(*SData, *S); }
	BumpRevision();
}

void UInventoryComponent::ServerRequestEquip_Implementation(const FGuid&, FName) {}
void UInventoryComponent::ServerRequestUnequip_Implementation(FName) {}
void UInventoryComponent::ServerRequestDrop_Implementation(const FGuid&, int32) {}

void UInventoryComponent::ServerSetPocketFilters_Implementation(int32 PocketIndex, const FGameplayTagContainer& NewWhitelist, const FGameplayTagContainer& NewBlacklist)
{
	if (!PocketsMeta.IsValidIndex(PocketIndex)) return;

	if (!PocketsMeta[PocketIndex].bFiltersEditable) return;

	PocketsMeta[PocketIndex].Whitelist = NewWhitelist;
	PocketsMeta[PocketIndex].Blacklist = NewBlacklist;
	
	if (AActor* Owner = GetOwner()) { Owner->ForceNetUpdate(); }
	BumpRevision();
}

bool UInventoryComponent::AddItemAuto(const FName RowID, int32 Amount)
{
	if (!GetItemsDT() || !RowID.IsValid() || Amount <= 0) return false;

	const FItemRowDetail* RowDetail = GetItemsDT()->FindRow<FItemRowDetail>(RowID, TEXT(""));
	if (!RowDetail) return false;
	const FItemRow& Row = RowDetail->Details;
	FItemInstance Inst;
	if (!CreateInstance(Row, RowID, Amount, 1.f, Inst)) return false;
	return TryAutoPlaceInBest(Inst, Row);
}

bool UInventoryComponent::TryAutoPlaceInBest(FItemInstance& Instance, const FItemRow& Row)
{
	for (int32 i = 0; i < Pockets.Num(); ++i)
	{
		if (!PassesFilters(PocketsMeta[i], Row)) continue;
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
	const int32 MaxStack = FMath::Max(1, RowMaxStackSize(Row));

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
		MarkChanged(Data, E);
		Perish_RebuildFor(Target);
		Instance.Count -= Move;
		bAny = true;
		if (Instance.Count <= 0) break;
	}
	if (bAny) { Perish_ScheduleNext(); }
	return bAny;
}

int32 UInventoryComponent::PlaceNewStacks(FItemInstance& Instance, const FItemRow& Row, FContainerData& Data,
	const FContainerMeta& Meta)
{
	int32 TotalPlaced = 0;
	const int32 MaxStack = FMath::Max(1, RowMaxStackSize(Row));
	const FIntPoint FP = RowFootprint(Row);

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
			const int64 Now = FDateTime::UtcNow().ToUnixTimestamp();
			const int64 Remain = FMath::Max<int64>(0, NewStack.Perish.NextPerishEpoch - Now);
			const float M = FMath::Max(0.01f, Meta.PerishRateMultiplier);
			// <1 = ralentit  => on DIVISE
			const int64 Adjust = (int64)FMath::RoundToInt(static_cast<double>(Remain) / static_cast<double>(M));
			NewStack.Perish.NextPerishEpoch = Now + Adjust; // (une seule fois)
		}

		FContainerEntry Entry;
		Entry.Instance = NewStack;
		Entry.TopLeft = Pos;
		Entry.bRotated90 = bRotate;
		Entry.FootprintPlaced = bRotate ? FIntPoint(FP.Y, FP.X) : FP;

		Data.Entries.Add(Entry);

		MarkAdded(Data, Data.Entries.Last());
		Perish_EnqueueEntry(Entry.Instance);
		Perish_ScheduleNext();

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
		for (int32 DX = 0; DX < E.FootprintPlaced.X; ++DX)
			for (int32 Dy = 0; Dy < E.FootprintPlaced.Y; ++Dy)
				Mark(E.TopLeft.X + DX, E.TopLeft.Y + Dy);
	}
}

bool UInventoryComponent::FindFreeSlot(const FIntPoint& Needs, const FContainerData& Data, const FContainerMeta& Meta,
	FGridCoord& OutPos, bool& bOutRotate)
{
	const int32 W = Meta.Grid.Width;
	const int32 H = Meta.Grid.Height;
	TArray<uint8> Occ; BuildOccupancy(Occ, Data, Meta);

	auto FitsAt = [&](const int32 Sx, const int32 Sy, const FIntPoint& Size)->bool
	{
		if (Sx + Size.X > W || Sy + Size.Y > H) return false;
		for (int32 x = 0; x < Size.X; ++x)
			for (int32 y = 0; y < Size.Y; ++y)
				if (Occ[(Sy + y) * W + (Sx + x)]) return false;
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
    bool bAny = false;

    auto DoMerge = [&](FContainerData& Data, const FContainerMeta& Meta)
    {
        for (int32 i = 0; i < Data.Entries.Num(); ++i)
        {
            for (int32 j = i + 1; j < Data.Entries.Num(); )
            {
                auto& A = Data.Entries[i].Instance;
                auto& B = Data.Entries[j].Instance;

                if (A.RowID == B.RowID)
                {
                    const bool AHas = A.BoundOwnerId.IsValid();
                    const bool BHas = B.BoundOwnerId.IsValid();
                    if (!(AHas || BHas) || (AHas && BHas && A.BoundOwnerId == B.BoundOwnerId))
                    {
                        const int32 MaxStack = FMath::Max(1, RowMaxStackSize(GetItemsDT()->FindRow<FItemRowDetail>(A.RowID, TEXT("MergeAll"))->Details));
                        const int32 Room = MaxStack - A.Count;

                        if (Room > 0)
                        {
                            const int32 Move = FMath::Min(Room, B.Count);

                            A.Count += Move;
                            MarkChanged(Data, Data.Entries[i]);
                            Perish_RebuildFor(Data.Entries[i].Instance); // <== garder le scheduler propre

                            B.Count -= Move;
                            if (B.Count <= 0)
                            {
                                MarkRemoved(Data, j);
                                bAny = true;
                                continue; // ne pas ++j : l'array a shrunk
                            }
                            else
                            {
                                MarkChanged(Data, Data.Entries[j]);
                            }
                            bAny = true;
                        }
                    }
                }
                ++j;
            }
        }
    };

    DoMerge(Backpack, BackpackMeta);
    for (int32 i = 0; i < Pockets.Num(); ++i) DoMerge(Pockets[i], PocketsMeta[i]);

    if (bAny) { Perish_ScheduleNext(); } // une seule replanification
}

void UInventoryComponent::AutoArrange(FContainerData& Data, const FContainerMeta& Meta)
{
    UDataTable* Table = GetItemsDT();

    // 1) Construire une vue triée (pointeurs + footprints théoriques)
    struct FView { FContainerEntry* E; FIntPoint Canon; };
    TArray<FView> Views;
    Views.Reserve(Data.Entries.Num());
    for (FContainerEntry& E : Data.Entries)
    {
        const FItemRowDetail* Row = Table ? Table->FindRow<FItemRowDetail>(E.Instance.RowID, TEXT("AutoArrange.View")) : nullptr;
        const FIntPoint FP = Row ? RowFootprint(Row->Details) : E.FootprintPlaced;
        Views.Add({ &E, FP });
    }
    Views.Sort([&](const FView& A, const FView& B)
    {
        const int32 SA = Surface(A.Canon);
        const int32 SB = Surface(B.Canon);
        if (SA != SB) return SA > SB;
        return A.E->Instance.RowID.LexicalLess(B.E->Instance.RowID);
    });

    // 2) Reconstruire une occupancy à vide et placer chaque entrée (sans toucher à l'array)
    const int32 W = Meta.Grid.Width, H = Meta.Grid.Height;
    TArray<uint8> Occ; Occ.Init(0, W*H);

    auto FitsAtOcc = [&](int32 sx, int32 sy, const FIntPoint& Size)->bool
    {
        if (sx + Size.X > W || sy + Size.Y > H) return false;
        for (int32 x=0; x<Size.X; ++x)
            for (int32 y=0; y<Size.Y; ++y)
                if (Occ[(sy+y)*W + (sx+x)]) return false;
        return true;
    };
    auto MarkOcc = [&](int32 sx, int32 sy, const FIntPoint& Size)
    {
        for (int32 x=0; x<Size.X; ++x)
            for (int32 y=0; y<Size.Y; ++y)
                Occ[(sy+y)*W + (sx+x)] = 1;
    };

    for (FView& V : Views)
    {
        const bool bCanRotate = (V.Canon.X != V.Canon.Y);
        const FIntPoint R0 = V.Canon;
        const FIntPoint R1 = FIntPoint(V.Canon.Y, V.Canon.X);

        FGridCoord Pos{ -1, -1 }; bool bRot = false;
        bool bPlaced = false;

        for (int32 y=0; y<H && !bPlaced; ++y)
        {
            for (int32 x=0; x<W && !bPlaced; ++x)
            {
                if (FitsAtOcc(x,y,R0)) { Pos={x,y}; bRot=false; bPlaced=true; }
                else if (bCanRotate && FitsAtOcc(x,y,R1)) { Pos={x,y}; bRot=true; bPlaced=true; }
            }
        }

        if (!bPlaced)
        {
            // Pas de place : on garde sa position actuelle (aucune modif)
            continue;
        }

        // Si la position/rotation change, on met à jour l'entrée et on MarkChanged
        const FIntPoint NewFP = bRot ? FIntPoint(V.Canon.Y, V.Canon.X) : V.Canon;
        if (V.E->TopLeft.X != Pos.X || V.E->TopLeft.Y != Pos.Y || V.E->bRotated90 != bRot || V.E->FootprintPlaced != NewFP)
        {
            V.E->TopLeft = Pos;
            V.E->bRotated90 = bRot;
            V.E->FootprintPlaced = NewFP;
            MarkChanged(Data, *V.E); // <<-- clé pour la réplication
        }

        MarkOcc(Pos.X, Pos.Y, NewFP);
    }
}

void UInventoryComponent::AutoArrangeAll()
{
	AutoArrange(Backpack, BackpackMeta);
	for (int32 i = 0; i < Pockets.Num(); ++i) AutoArrange(Pockets[i], PocketsMeta[i]);
	BumpRevision();
}

void UInventoryComponent::MarkAdded(FContainerData& Data, FContainerEntry& Entry)
{
	Data.MarkItemDirty(Entry);
	Data.MarkArrayDirty();
}

void UInventoryComponent::MarkChanged(FContainerData& Data, FContainerEntry& Entry)
{
	Data.MarkItemDirty(Entry);
}

void UInventoryComponent::MarkRemoved(FContainerData& Data, const int32 Index)
{
	// 1) Capturer le GUID AVANT la suppression
	FGuid Guid;
	if (Data.Entries.IsValidIndex(Index))
	{
		Guid = Data.Entries[Index].Instance.InstanceGuid;
	}

	// 2) Supprimer proprement dans la FastArray
	if (Data.Entries.IsValidIndex(Index))
	{
		Data.Entries.RemoveAtSwap(Index, EAllowShrinking::No);
		Data.MarkArrayDirty();
	}

	// 3) Débrancher du scheduler (si valide) + replanifier
	if (Guid.IsValid())
	{
		Perish_RemoveGuid(Guid);
		Perish_ScheduleNext();
	}
}

void UInventoryComponent::BumpRevision()
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		++ReplicationRevision;
		if (AActor* Owner = GetOwner()) { Owner->ForceNetUpdate(); }
		OnInventoryDirty.Broadcast();
	}
}

bool UInventoryComponent::ResolveContainer(const FContainerId& Id, FContainerData*& OutData, FContainerMeta*& OutMeta, int32* OutPocketIndex)
{
	OutData = nullptr; OutMeta = nullptr;
	if (Id.Name.IsNone() || Id.Name == FName("Backpack"))
	{
		OutData = &Backpack;
		OutMeta = &BackpackMeta;
		if (OutPocketIndex) *OutPocketIndex = INDEX_NONE;
		return true;
	}

	FString N = Id.Name.ToString();
	if (N.StartsWith(TEXT("Pocket_")))
	{
		int32 Index = FCString::Atoi(*N.Mid(7));
		if (Pockets.IsValidIndex(Index) && PocketsMeta.IsValidIndex(Index))
		{
			OutData = &Pockets[Index];
			OutMeta = &PocketsMeta[Index];
			if (OutPocketIndex) *OutPocketIndex = Index;
			return true;
		}
	}
	return false;
}

FContainerEntry* UInventoryComponent::FindEntry(FContainerData& Data, const FGuid& InstanceGuid, int32* OutIndex)
{
	for (int32 i = 0; i < Data.Entries.Num(); ++i)
	{
		if (Data.Entries[i].Instance.InstanceGuid == InstanceGuid)
		{
			if (OutIndex) * OutIndex = i;
			return &Data.Entries[i];
		}
	}
	return nullptr;
}

const FContainerEntry* UInventoryComponent::FindEntry(const FContainerData& Data, const FGuid& InstanceGuid,
	int32* OutIndex)
{
	for (int32 i = 0; i < Data.Entries.Num(); ++i)
	{
		if (Data.Entries[i].Instance.InstanceGuid == InstanceGuid)
		{
			if (OutIndex) * OutIndex = i;
			return &Data.Entries[i];
		}
	}
	return nullptr;
}

bool UInventoryComponent::FitsAt(const FContainerData& Data, const FContainerMeta& Meta, FGridCoord Pos, FIntPoint Size) const
{
	const int32 W = Meta.Grid.Width;
	const int32 H = Meta.Grid.Height;
	if (Pos.X < 0 || Pos.Y < 0 || Pos.X + Size.X > W || Pos.Y + Size.Y > H) return false;

	TArray<uint8> Occ; BuildOccupancy(Occ, Data, Meta);
	for (int32 dx = 0; dx < Size.X; ++dx)
		for (int32 dy = 0; dy < Size.Y; ++dy)
			if (Occ[(Pos.Y + dy) * W + (Pos.X + dx)]) return false;
	return true;
}

// Parse "bp"/"backpack"/"p0"/"p1"... -> FContainerId + pointer
static bool UI_ParseContainerId(UInventoryComponent* Inv, const FString& Which, FContainerId& OutId, FContainerData*& OutData)
{
    if (!Inv) return false;
    const FString L = Which.ToLower();
    if (L == TEXT("bp") || L == TEXT("b") || L == TEXT("backpack")) { OutId.Name=FName("Backpack"); OutData=&Inv->Backpack; return true; }
    if (L.StartsWith(TEXT("pocket_")) || L.StartsWith(TEXT("p")))
    {
        int32 Index = L.StartsWith(TEXT("pocket_")) ? FCString::Atoi(*L.Mid(7)) : FCString::Atoi(*L.Mid(1));
        if (Inv->Pockets.IsValidIndex(Index)) { OutId.Name=FName(*FString::Printf(TEXT("Pocket_%d"), Index)); OutData=&Inv->Pockets[Index]; return true; }
    }
    return false;
}

void UInventoryComponent::ServerMoveByIndex_Implementation(const FString& From, int32 Index, const FString& To, int32 X, int32 Y, bool bRotate)
{
    FContainerId FromId, ToId; FContainerData* FromData=nullptr; FContainerData* ToData=nullptr;
    if (!UI_ParseContainerId(this, From, FromId, FromData) || !UI_ParseContainerId(this, To, ToId, ToData)) return;
    if (!FromData || !FromData->Entries.IsValidIndex(Index)) return;
    const FGuid Guid = FromData->Entries[Index].Instance.InstanceGuid;
    ServerRequestMoveItem(Guid, FromId, ToId, FGridCoord{X,Y}, bRotate);
}

void UInventoryComponent::ServerMoveAutoByIndex_Implementation(const FString& From, int32 Index, const FString& To)
{
    FContainerId FromId, ToId; FContainerData* FromData=nullptr; FContainerData* ToData=nullptr;
    if (!UI_ParseContainerId(this, From, FromId, FromData) || !UI_ParseContainerId(this, To, ToId, ToData)) return;
    if (!FromData || !FromData->Entries.IsValidIndex(Index)) return;
    const FGuid Guid = FromData->Entries[Index].Instance.InstanceGuid;
    ServerRequestMoveItem(Guid, FromId, ToId, FGridCoord{-1,-1}, false);
}

void UInventoryComponent::ServerSplitByIndex_Implementation(const FString& From, int32 Index, int32 Amount)
{
    FContainerId FromId; FContainerData* FromData=nullptr;
    if (!UI_ParseContainerId(this, From, FromId, FromData)) return;
    if (!FromData || !FromData->Entries.IsValidIndex(Index)) return;
    const FGuid Guid = FromData->Entries[Index].Instance.InstanceGuid;
    ServerRequestSplitItem(Guid, FromId, Amount);
}

void UInventoryComponent::ServerMergeByIndex_Implementation(const FString& From, int32 IndexFrom, const FString& To, int32 IndexTo)
{
    FContainerId FromId; FContainerData* FromData=nullptr;
    FContainerId ToId;   FContainerData* ToData=nullptr;
    if (!UI_ParseContainerId(this, From, FromId, FromData) || !UI_ParseContainerId(this, To, ToId, ToData)) return;
    if (!FromData || !ToData || !FromData->Entries.IsValidIndex(IndexFrom) || !ToData->Entries.IsValidIndex(IndexTo)) return;

    const FGuid GFrom = FromData->Entries[IndexFrom].Instance.InstanceGuid;
    const FGuid GTo   = ToData  ->Entries[IndexTo]  .Instance.InstanceGuid;
    ServerRequestMergeInstances(GFrom, GTo);
}

bool UInventoryComponent::FindAnyEntryByGuid(const FGuid& Guid, FContainerData*& OutData, FContainerMeta*& OutMeta,
	int32& OutIndex)
{
	OutData = nullptr; OutMeta = nullptr; OutIndex = INDEX_NONE;
	for (int32 i = 0; i < Backpack.Entries.Num(); ++i)
		if (Backpack.Entries[i].Instance.InstanceGuid == Guid)
		{
			OutData = &Backpack;
			OutMeta = &BackpackMeta;
			OutIndex = i;
			return true;
		}
	for (int32 p = 0; p < Pockets.Num(); ++p)
		for (int32 i = 0; i < Pockets[p].Entries.Num(); ++i)
			if (Pockets[p].Entries[i].Instance.InstanceGuid == Guid) { OutData = &Pockets[p]; OutMeta = &PocketsMeta[p]; OutIndex = i; return true; }
	return false;
}

void UInventoryComponent::Perish_RebuildIndex()
{
	PerishGuidIndex.Reset();
	for (int32 i=0; i<PerishHeap.Num(); ++i)
	{
		PerishGuidIndex.Add(PerishHeap[i].InstanceGuid, i);
	}
}

void UInventoryComponent::Perish_EnqueueEntry(const FItemInstance& Inst)
{
	if (Inst.Perish.NextPerishEpoch <= 0 || Inst.Perish.NextPerishCount <= 0) return;
	if (PerishGuidIndex.Contains(Inst.InstanceGuid)) return;

	const int64 E = Inst.Perish.NextPerishEpoch;

	// insertion triée (Epoch croissant)
	int32 InsertAt = 0;
	while (InsertAt < PerishHeap.Num() && PerishHeap[InsertAt].Epoch <= E) { ++InsertAt; }

	PerishHeap.Insert({E, Inst.InstanceGuid}, InsertAt);
	Perish_RebuildIndex();
}

void UInventoryComponent::Perish_RemoveGuid(const FGuid& Guid)
{
	int32* Idx = PerishGuidIndex.Find(Guid);
	if (!Idx) return;

	PerishHeap.RemoveAt(*Idx, 1, EAllowShrinking::No);
	Perish_RebuildIndex();
}

void UInventoryComponent::Perish_RebuildFor(const FItemInstance& Inst)
{
	Perish_RemoveGuid(Inst.InstanceGuid);
	Perish_EnqueueEntry(Inst);
}

void UInventoryComponent::Perish_ScheduleNext()
{
	if (!GetWorld()) return;
	GetWorld()->GetTimerManager().ClearTimer(PerishTimer);

	if (PerishHeap.Num() == 0) return;

	const int64 Now = FDateTime::UtcNow().ToUnixTimestamp();
	const int64 TopEpoch = PerishHeap[0].Epoch;
	const double Delay = FMath::Max<double>(0.01, static_cast<double>(TopEpoch - Now));

	GetWorld()->GetTimerManager().SetTimer(PerishTimer, this, &UInventoryComponent::Perish_OnTimer, static_cast<float>(Delay), false);
}

void UInventoryComponent::Perish_OnTimer()
{
    if (PerishHeap.Num() == 0) { Perish_ScheduleNext(); return; }

    const int64 Now = FDateTime::UtcNow().ToUnixTimestamp();

    // Consomme toutes les échéances arrivées
    while (PerishHeap.Num() > 0 && PerishHeap[0].Epoch <= Now)
    {
        const FPerishNode Node = PerishHeap[0];
        // pop front
    	PerishHeap.RemoveAt(0, 1, EAllowShrinking::No);
        Perish_RebuildIndex();

        FContainerData* Data=nullptr; FContainerMeta* Meta=nullptr; int32 Index=-1;
        if (!FindAnyEntryByGuid(Node.InstanceGuid, Data, Meta, Index))
        {
            continue; // l’item a bougé/supprimé
        }

        FContainerEntry& E = Data->Entries[Index];
        FItemInstance&   I = E.Instance;

        const int32 ExpiringNow = FMath::Clamp(I.Perish.NextPerishCount, 0, I.Count);
        if (ExpiringNow > 0)
        {
            I.Count -= ExpiringNow;
            MarkChanged(*Data, E);

            // TODO (4.1) : spawn PerishTo si défini; sinon on ignore si plein

            if (I.Count <= 0)
            {
                // remove complet -> MarkRemoved s’occupera de Perish_RemoveGuid (ok)
                MarkRemoved(*Data, Index);
                continue;
            }
        }

        // Prépare la prochaine échéance
        if (I.Perish.LaterPerishEpoch > 0 && I.Perish.LaterPerishCount > 0)
        {
            I.Perish.PromoteLaterToNext();
        }
        else
        {
            UDataTable* DT = UTables::GetTable(ETablePath::ItemsTable);
            const FItemRowDetail* Row = DT ? DT->FindRow<FItemRowDetail>(I.RowID, TEXT("Perish.Next")) : nullptr;
            if (Row && Row->Details.GetPerishTime() > 0)
            {
                const float Mult = FMath::Max(0.01f, Meta ? Meta->PerishRateMultiplier : 1.f);
                const int64 Delay = (int64)FMath::RoundToInt(static_cast<double>(Row->Details.GetPerishTime()) / static_cast<double>(Mult));
                I.Perish.NextPerishEpoch = FDateTime::UtcNow().ToUnixTimestamp() + Delay;
                I.Perish.NextPerishCount = 1;
            }
            else
            {
                I.Perish.NextPerishEpoch = 0;
                I.Perish.NextPerishCount = 0;
            }
        }

        // Ré-enfiler si encore périssable
        if (I.Perish.NextPerishEpoch > 0 && I.Perish.NextPerishCount > 0)
        {
            Perish_EnqueueEntry(I);
        }
    }

    // Replanifie sur la nouvelle tête (s’il y en a une)
    Perish_ScheduleNext();
}

void UInventoryComponent::ServerDebug_PerishIn_Implementation(const FString& From, int32 Index, int32 Seconds)
{
	FContainerId Id; FContainerData* Data=nullptr;
	if (!UI_ParseContainerId(this, From, Id, Data) || !Data) return;
	if (!Data->Entries.IsValidIndex(Index)) return;

	FContainerEntry& E = Data->Entries[Index];
	FItemInstance& I   = E.Instance;

	const int64 Now = FDateTime::UtcNow().ToUnixTimestamp();
	const int32 CountToExpire = FMath::Clamp(I.Perish.NextPerishCount > 0 ? I.Perish.NextPerishCount : 1, 1, I.Count);
	I.Perish.NextPerishCount = CountToExpire;
	I.Perish.NextPerishEpoch = Now + FMath::Max(0, Seconds);

	MarkChanged(*Data, E);
	Perish_RebuildFor(I);
	Perish_ScheduleNext();
}

void UInventoryComponent::ServerDebug_PerishNow_Implementation(const FString& From, int32 Index)
{
	ServerDebug_PerishIn(From, Index, 0);
}

void UInventoryComponent::ServerDebug_Setup_Implementation(int32 PocketW, int32 PocketH, float PerishMult, bool bEditable)
{
	// reset propre + réplication
	Backpack.Entries.Reset(); Backpack.MarkArrayDirty();
	Pockets.Reset();
	PocketsMeta.Reset();

	// Backpack meta
	BackpackMeta.Grid = { 8, 6 };
	BackpackMeta.PerishRateMultiplier = 1.f;

	// 1 pocket de test
	FContainerData P;           Pockets.Add(P);
	FContainerMeta  M;
	M.Grid = { PocketW, PocketH };
	M.PerishRateMultiplier = PerishMult;
	M.bFiltersEditable = bEditable;
	M.Whitelist.AddTag(TAG_Consumable); // ta whitelist de test
	PocketsMeta.Add(M);

	if (AActor* Owner = GetOwner()) Owner->ForceNetUpdate();
}
