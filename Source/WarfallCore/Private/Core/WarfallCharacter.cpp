#include "Core/WarfallCharacter.h"

#include "Blueprint/UserWidget.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/InventoryTypes.h"
#include "Inventory/InventoryDebug.h"

using namespace WfInvDbg;

// Accepte directement un format TCHAR (utilise TEXT(...) à l'appel)
#define IVLOG(Format, ...) UE_LOG(LogTemp, Warning, Format, ##__VA_ARGS__)

static UInventoryComponent* GetInv(AWarfallCharacter* C)
{
	return C ? C->FindComponentByClass<UInventoryComponent>() : nullptr;
}

static const TCHAR* SideTag(UObject* Ctx)
{
    const ENetMode NM = Ctx && Ctx->GetWorld() ? Ctx->GetWorld()->GetNetMode() : NM_Standalone;
    return (NM == NM_Client) ? TEXT("[CL]") : ((NM == NM_ListenServer) ? TEXT("[SV]") : TEXT("[SG]")); // SG = standalone
}


static UDataTable* GetItemsDT()
{
	return UTables::GetTable(ETablePath::ItemsTable);
}

static FGameplayTag ResolveTagToken(const FString& NameToken)
{
    const FString N = NameToken.ToLower();

    // Alias courts
    if (N == TEXT("consumable") || N == TEXT("item.consumable"))
        return TAG_Consumable;
    if (N == TEXT("ammo") || N == TEXT("ammunition") || N == TEXT("item.ammunition"))
        return TAG_Ammunition;

    // Fallback: tenter la requête directe (sans crash si inconnu)
    return FGameplayTag::RequestGameplayTag(FName(*NameToken), /*ErrorIfNotFound*/ false);
}

static FString TagsToString(const FGameplayTagContainer& C)
{
    TArray<FString> Names; Names.Reserve(C.Num());
    for (const FGameplayTag& T : C) { Names.Add(T.ToString()); }
    return FString::Join(Names, TEXT(","));
}

// Parse "bp"/"backpack"/"b" or "p0"/"p1"/"pocket_2" into a FContainerId and return the associated data ptr
static bool ParseContainerId(UInventoryComponent* Inv, const FString& Which, FContainerId& OutId, FContainerData*& OutData)
{
    if (!Inv) return false;

    const FString L = Which.ToLower();

    if (L == TEXT("bp") || L == TEXT("b") || L == TEXT("backpack"))
    {
        OutId.Name = FName("Backpack");
        OutData = &Inv->Backpack;
        return true;
    }

    if (L.StartsWith(TEXT("pocket_")) || L.StartsWith(TEXT("p")))
    {
        int32 Index = -1;
        if (L.StartsWith(TEXT("pocket_"))) Index = FCString::Atoi(*L.Mid(7));
        else                               Index = FCString::Atoi(*L.Mid(1));

        if (Inv->Pockets.IsValidIndex(Index))
        {
            OutId.Name = FName(*FString::Printf(TEXT("Pocket_%d"), Index));
            OutData = &Inv->Pockets[Index];
            return true;
        }
    }

    return false;
}

// Returns the Instance GUID at (Which, Index)
static bool GetGuidAt(UInventoryComponent* Inv, const FString& Which, int32 Index, FGuid& OutGuid)
{
    FContainerId Id; FContainerData* Data=nullptr;
    if (!ParseContainerId(Inv, Which, Id, Data) || !Data) return false;
    if (!Data->Entries.IsValidIndex(Index)) return false;

    OutGuid = Data->Entries[Index].Instance.InstanceGuid;
    return OutGuid.IsValid();
}

// Sets default values
AWarfallCharacter::AWarfallCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Inventory = CreateDefaultSubobject<UInventoryComponent>("Inventory");
}

// Called when the game starts or when spawned
void AWarfallCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWarfallCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AWarfallCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AWarfallCharacter::iv_p2_setup(int32 PocketW, int32 PocketH, float PerishMult)
{
    if (auto* Inv = GetInv(this))
    {
        Inv->ServerDebug_Setup(PocketW, PocketH, PerishMult, /*bEditable=*/true);
        IVLOG(TEXT("%s [P2_SETUP] ask server W=%d H=%d mult=%.2f"),
              SideTag(this), PocketW, PocketH, PerishMult);
    }
}

void AWarfallCharacter::iv_p2_add(FName RowID, int32 Count)
{
    auto* DT = GetItemsDT();
    if (!DT) { IVLOG(TEXT("[ADD %s] Items DT not found"), SideTag(this)); return; }

    if (auto* Inv = GetInv(this))
    {
        Inv->ServerAddItemAuto(RowID, Count);
        IVLOG(TEXT("[ADD %s] %s x%d"), SideTag(this), *RowID.ToString(), Count);
    }
}

void AWarfallCharacter::iv_p2_mergeall()
{
    if (auto* Inv = GetInv(this))
    {
        Inv->ServerMergeAll();
        IVLOG(TEXT("[MERGEALL %s] Ask server"), SideTag(this));
    }
}

void AWarfallCharacter::iv_p2_arrange()
{
    if (auto* Inv = GetInv(this))
    {
        Inv->ServerAutoArrangeAll(); // <-- serveur (au lieu de AutoArrangeAll() local)
        IVLOG(TEXT("[ARRANGE %s] Ask server"), SideTag(this));
    }
}

void AWarfallCharacter::iv_p2_setfilters(int32 PocketIndex, const FString& OpsCSV)
{
    if (auto* Inv = GetInv(this))
    {
        if (!Inv->CanEditPocketFilters(PocketIndex))
        {
            IVLOG(TEXT("[SETFILTERS %s] Pocket %d not editable"), SideTag(this), PocketIndex);
            return;
        }

        FGameplayTagContainer White = Inv->PocketsMeta[PocketIndex].Whitelist;
        FGameplayTagContainer Black = Inv->PocketsMeta[PocketIndex].Blacklist;

        TArray<FString> Ops; OpsCSV.ParseIntoArray(Ops, TEXT(","), true);
        for (FString Op : Ops)
        {
            Op.TrimStartAndEndInline();
            if (Op.Len() <= 1) continue;

            const TCHAR Prefix = Op[0];
            const FString Token = Op.Mid(1);
            const FGameplayTag Tag = ResolveTagToken(Token);
            if (!Tag.IsValid()) { IVLOG(TEXT("[SETFILTERS %s] Unknown tag: %s"), SideTag(this), *Token); continue; }

            if (Prefix == TEXT('+')) { White.AddTag(Tag); Black.RemoveTag(Tag); }
            else if (Prefix == TEXT('-')) { Black.AddTag(Tag); White.RemoveTag(Tag); }
        }

        Inv->ServerSetPocketFilters(PocketIndex, White, Black);
        IVLOG(TEXT("[SETFILTERS %s] Pocket %d  White=[%s]  Black=[%s]"), SideTag(this),
              PocketIndex, *TagsToString(White), *TagsToString(Black));
    }
}

static FString HumanEpoch(int64 Epoch)
{
    if (Epoch <= 0) return TEXT("—");
    const FDateTime Dt = FDateTime::FromUnixTimestamp(Epoch);
    return Dt.ToString(TEXT("%Y.%m.%d-%H.%M.%S"));
}

void AWarfallCharacter::iv_p2_summary()
{
    if (auto* Inv = GetInv(this))
    {
        auto Dump = [&](const TCHAR* Name, const FContainerData& Data, const FContainerMeta& Meta)
        {
            IVLOG(TEXT("== %s (%dx%d) =="), Name, Meta.Grid.Width, Meta.Grid.Height);
            for (int32 i=0; i<Data.Entries.Num(); ++i)
            {
                const auto& E = Data.Entries[i];
                IVLOG(TEXT(" [%d] %s  Count=%d  Pos=(%d,%d)%s  FP=(%d,%d)  NextPerish=%s"),
                    i,
                    *E.Instance.RowID.ToString(),
                    E.Instance.Count,
                    E.TopLeft.X, E.TopLeft.Y,
                    E.bRotated90 ? TEXT(" Rot90") : TEXT(""),
                    E.FootprintPlaced.X, E.FootprintPlaced.Y,
                    *HumanEpoch(E.Instance.Perish.NextPerishEpoch));
            }
        };

        Dump(TEXT("Backpack"), Inv->Backpack, Inv->BackpackMeta);
        for (int32 p=0; p<Inv->Pockets.Num(); ++p)
        {
            Dump(*FString::Printf(TEXT("Pocket_%d"), p), Inv->Pockets[p], Inv->PocketsMeta[p]);
        }
    }
}

void AWarfallCharacter::iv_p2_overlay(const FString& Which)
{
    if (auto* Inv = GetInv(this))
    {
        TArray<FString> Lines;
        if (Which.Equals(TEXT("bp"), ESearchCase::IgnoreCase))
        {
            BuildAsciiGrid(Inv->Backpack, Inv->BackpackMeta, Lines);
            IVLOG(TEXT("== BACKPACK %s %dx%d =="), SideTag(this), Inv->BackpackMeta.Grid.Width, Inv->BackpackMeta.Grid.Height);
        }
        else if (Which.Len() >= 2 && (Which[0] == 'p' || Which[0] == 'P'))
        {
            const int32 Index = FCString::Atoi(*Which.Mid(1));
            if (Inv->Pockets.IsValidIndex(Index))
            {
                BuildAsciiGrid(Inv->Pockets[Index], Inv->PocketsMeta[Index], Lines);
                IVLOG(TEXT("== POCKET_%d %s %dx%d =="), Index, SideTag(this), Inv->PocketsMeta[Index].Grid.Width, Inv->PocketsMeta[Index].Grid.Height);
            }
            else
            {
                IVLOG(TEXT("[OVERLAY %s] Invalid pocket index: %s"), SideTag(this), *Which);
                return;
            }
        }
        else
        {
            IVLOG(TEXT("[OVERLAY %s] Usage: iv_p2_overlay bp | p0 | p1 | ..."), SideTag(this)); 
            return;
        }

        // Dump on screen (1s per line so it stays visible)
        int32 Key = 7000; // arbitrary key range for our overlay messages
        for (const FString& L : Lines)
        {
            if (GEngine) GEngine->AddOnScreenDebugMessage(Key++, 5.f, FColor::Cyan, L);
            IVLOG(TEXT("%s"), *L);
        }
    }
}

void AWarfallCharacter::iv_p3_guid(const FString& From, int32 Index)
{
    if (auto* Inv = GetInv(this))
    {
        FGuid G;
        if (GetGuidAt(Inv, From, Index, G))
        {
            IVLOG(TEXT("%s [GUID] %s[%d] = %s"),
                  SideTag(this), *From, Index, *G.ToString());
        }
        else
        {
            IVLOG(TEXT("%s [GUID] Invalid source %s[%d]"), SideTag(this), *From, Index);
        }
    }
}

void AWarfallCharacter::iv_p3_move(const FString& From, int32 Index, const FString& To, int32 X, int32 Y, int32 bRotate)
{
    if (auto* Inv = GetInv(this))
    {
        FGuid G;
        if (!GetGuidAt(Inv, From, Index, G))
        {
            IVLOG(TEXT("%s [MOVE] Invalid source %s[%d]"), SideTag(this), *From, Index);
            return;
        }

        // Build ContainerIds
        FContainerId FromId; FContainerData* Dummy=nullptr;
        FContainerId ToId;   FContainerData* Dummy2=nullptr;
        if (!ParseContainerId(Inv, From, FromId, Dummy) || !ParseContainerId(Inv, To, ToId, Dummy2))
        {
            IVLOG(TEXT("%s [MOVE] Bad container(s): from=%s to=%s"), SideTag(this), *From, *To);
            return;
        }

        const FGridCoord ToPos{ X, Y };
        const bool bRot = (bRotate != 0);

        Inv->ServerRequestMoveItem(G, FromId, ToId, ToPos, bRot);
        IVLOG(TEXT("%s [MOVE] %s[%d] -> %s Pos=(%d,%d) Rot=%s"),
              SideTag(this), *From, Index, *To, X, Y, bRot ? TEXT("90") : TEXT("0"));
    }
}

void AWarfallCharacter::iv_p3_move_auto(const FString& From, int32 Index, const FString& To)
{
    if (auto* Inv = GetInv(this))
    {
        FGuid G;
        if (!GetGuidAt(Inv, From, Index, G))
        {
            IVLOG(TEXT("%s [MOVE_AUTO] Invalid source %s[%d]"), SideTag(this), *From, Index);
            return;
        }

        FContainerId FromId; FContainerData* D1=nullptr;
        FContainerId ToId;   FContainerData* D2=nullptr;
        if (!ParseContainerId(Inv, From, FromId, D1) || !ParseContainerId(Inv, To, ToId, D2))
        {
            IVLOG(TEXT("%s [MOVE_AUTO] Bad container(s): from=%s to=%s"), SideTag(this), *From, *To);
            return;
        }

        // Auto-place: X=Y=-1, rotation ignorée
        Inv->ServerRequestMoveItem(G, FromId, ToId, FGridCoord{-1,-1}, false);
        IVLOG(TEXT("%s [MOVE_AUTO] %s[%d] -> %s (auto-place)"),
              SideTag(this), *From, Index, *To);
    }
}

void AWarfallCharacter::iv_p3_split(const FString& From, int32 Index, int32 Amount)
{
    if (auto* Inv = GetInv(this))
    {
        FGuid G;
        if (!GetGuidAt(Inv, From, Index, G))
        {
            IVLOG(TEXT("%s [SPLIT] Invalid source %s[%d]"), SideTag(this), *From, Index);
            return;
        }
        FContainerId FromId; FContainerData* D=nullptr;
        if (!ParseContainerId(Inv, From, FromId, D))
        {
            IVLOG(TEXT("%s [SPLIT] Bad container: %s"), SideTag(this), *From);
            return;
        }

        Inv->ServerRequestSplitItem(G, FromId, Amount);
        IVLOG(TEXT("%s [SPLIT] %s[%d] Amount=%d"), SideTag(this), *From, Index, Amount);
    }
}

void AWarfallCharacter::iv_p3_merge(const FString& From, int32 IndexFrom, const FString& To, int32 IndexTo)
{
    if (auto* Inv = GetInv(this))
    {
        FGuid GFrom, GTo;
        if (!GetGuidAt(Inv, From, IndexFrom, GFrom))
        {
            IVLOG(TEXT("%s [MERGE] Invalid source %s[%d]"), SideTag(this), *From, IndexFrom);
            return;
        }
        if (!GetGuidAt(Inv, To, IndexTo, GTo))
        {
            IVLOG(TEXT("%s [MERGE] Invalid target %s[%d]"), SideTag(this), *To, IndexTo);
            return;
        }

        Inv->ServerRequestMergeInstances(GFrom, GTo);
        IVLOG(TEXT("%s [MERGE] %s[%d] -> %s[%d]"),
              SideTag(this), *From, IndexFrom, *To, IndexTo);
    }
}

static TWeakObjectPtr<UUserWidget> GInvUI;

void AWarfallCharacter::iv_ui()
{
    if (GInvUI.IsValid())
    {
        GInvUI->RemoveFromParent(); GInvUI = nullptr;
        UE_LOG(LogTemp, Warning, TEXT("[UI] Closed"));
        return;
    }
    if (!InventoryDebugWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UI] InventoryDebugWidgetClass not set in Editor"));
        return;
    }

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) { UE_LOG(LogTemp, Warning, TEXT("[UI] No PlayerController")); return; }

    UUserWidget* W = CreateWidget<UUserWidget>(PC, InventoryDebugWidgetClass);
    if (!W) { UE_LOG(LogTemp, Warning, TEXT("[UI] CreateWidget failed")); return; }

    W->AddToViewport(500);
    GInvUI = W;
    UE_LOG(LogTemp, Warning, TEXT("[UI] Opened"));
}

void AWarfallCharacter::iv_p4_expire_in(const FString& From, int32 Index, int32 Seconds)
{
    if (auto* Inv = GetInv(this)) { Inv->ServerDebug_PerishIn(From, Index, Seconds); UE_LOG(LogTemp, Warning, TEXT("[P4] %s expire in %d s"), *From, Seconds); }
}
void AWarfallCharacter::iv_p4_expire_now(const FString& From, int32 Index)
{
    if (auto* Inv = GetInv(this)) { Inv->ServerDebug_PerishNow(From, Index); UE_LOG(LogTemp, Warning, TEXT("[P4] %s[%d] expire NOW"), *From, Index); }
}
void AWarfallCharacter::iv_p4_dump()
{
    if (auto* Inv = GetInv(this))
    {
        UE_LOG(LogTemp, Warning, TEXT("[P4] HeapSize=%d"), Inv->PerishHeap.Num());
        if (Inv->PerishHeap.Num() > 0)
        {
            const int64 Top = Inv->PerishHeap[0].Epoch;
            const FDateTime Dt = FDateTime::FromUnixTimestamp(Top);
            UE_LOG(LogTemp, Warning, TEXT("[P4] NextEpoch=%s"), *Dt.ToString(TEXT("%Y.%m.%d-%H.%M.%S")));
        }
    }
}
