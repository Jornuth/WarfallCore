#include "Inventory/InventoryDebug.h"
using namespace WfInv;

void WfInvDbg::BuildAsciiGrid(const FContainerData& Data, const FContainerMeta& Meta, TArray<FString>& Out)
{
	const int32 W = Meta.Grid.Width, H = Meta.Grid.Height;
	Out.Reset();
	if (W <= 0 || H <= 0) return;

	TArray<TCHAR> Buf; Buf.SetNumUninitialized(W*H);
	for (int32 i=0;i<W*H;++i) Buf[i]=TEXT('.');

	auto Put=[&](int32 X,int32 Y,TCHAR C){ if (X>=0&&Y>=0&&X<W&&Y<H) Buf[Y*W+X]=C; };

	for (const FContainerEntry& E : Data.Entries)
	{
		const FString Name = E.Instance.RowID.ToString();
		const TCHAR Mark = Name.Len()>0 ? FChar::ToUpper(Name[0]) : TEXT('#');
		for (int32 dx=0;dx<E.FootprintPlaced.X;++dx)
			for (int32 dy=0;dy<E.FootprintPlaced.Y;++dy)
				Put(E.TopLeft.X+dx, E.TopLeft.Y+dy, Mark);
		Put(E.TopLeft.X, E.TopLeft.Y, TEXT('*'));
	}
	Out.Reserve(H);
	for (int32 y=0;y<H;++y){ FString L; L.Reserve(W); for (int32 x=0;x<W;++x) L.AppendChar(Buf[y*W+x]); Out.Add(L); }
}

bool WfInvDbg::UI_ParseContainerId(UInventoryComponent* Inv, const FString& Which,
								   FContainerId& OutId, FContainerData*& OutData)
{
	if (!Inv) return false;
	const FString L = Which.ToLower();
	if (L==TEXT("bp")||L==TEXT("b")||L==TEXT("backpack")) { OutId.Name=FName("Backpack"); OutData=&Inv->Backpack; return true; }
	if (L.StartsWith(TEXT("p")))
	{
		const int32 i = FCString::Atoi(*L.Mid(1));
		if (Inv->Pockets.IsValidIndex(i)) { OutId.Name=*FString::Printf(TEXT("Pocket_%d"), i); OutData=&Inv->Pockets[i]; return true; }
	}
	return false;
}