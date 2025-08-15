#include "Core/WarfallCharacter.h"
#include "Inventory/InventoryTypes.h"


// Sets default values
AWarfallCharacter::AWarfallCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
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

bool AWarfallCharacter::AddItem(const FName RowID, const int32 Count, FItemRow& OutRow, FItemInstance& Out)
{
	const FItemRowDetail* RowDetail = UTables::GetTable(ETablePath::ItemsTable)->FindRow<FItemRowDetail>(RowID, TEXT(""));
	OutRow = RowDetail->Details;
	
	if (!RowDetail) return false;

	return WfInv::CreateInstance(OutRow, RowID, Count, 1, Out);
}

void AWarfallCharacter::Test1()
{
	FItemRow Row;
	FItemInstance Instance;

	if (AddItem("Apple", 3, Row, Instance))
	{
		UE_LOG(LogTemp, Log, TEXT("Item %s is create successfully!"), *Row.Name.ToString());
		UE_LOG(LogTemp, Warning, TEXT("-------Instance-------"))
		UE_LOG(LogTemp, Warning, TEXT("Count = %d"), Instance.Count)
		UE_LOG(LogTemp, Warning, TEXT("NextPerishEpoch = %s"), *FDateTime::FromUnixTimestamp(Instance.Perish.NextPerishEpoch).ToString())
		UE_LOG(LogTemp, Warning, TEXT("NextPerishCount = %d"), Instance.Perish.NextPerishCount)
		UE_LOG(LogTemp, Warning, TEXT("UsesDurability  = %s"), WfInv::UsesDurability(Row, Instance) ? TEXT("True") : TEXT("False"))
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Item %s is not create successfully!"), *Row.Name.ToString());
	}
}

void AWarfallCharacter::Test2()
{
	FItemRow Row;
	FItemInstance Instance;

	if (AddItem("Iron_Ore", 5, Row, Instance))
	{
		UE_LOG(LogTemp, Log, TEXT("Item %s is create successfully!"), *Row.Name.ToString());
		UE_LOG(LogTemp, Warning, TEXT("-------Instance-------"))
		UE_LOG(LogTemp, Warning, TEXT("Count = %d"), Instance.Count)
		UE_LOG(LogTemp, Warning, TEXT("NextPerishEpoch = %s"), *FDateTime::FromUnixTimestamp(Instance.Perish.NextPerishEpoch).ToString())
		UE_LOG(LogTemp, Warning, TEXT("UsesDurability  = %s"), WfInv::UsesDurability(Row, Instance) ? TEXT("True") : TEXT("False"))
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Item %s is not create successfully!"), *Row.Name.ToString());
	}
}

void AWarfallCharacter::Test3()
{
	FItemRow Row;
	FItemInstance Instance;

	if (AddItem("Axe", 1, Row, Instance))
	{
		UE_LOG(LogTemp, Log, TEXT("Item %s is create successfully!"), *Row.Name.ToString());
		UE_LOG(LogTemp, Warning, TEXT("-------Instance-------"))
		UE_LOG(LogTemp, Warning, TEXT("Count = %d"), Instance.Count)
		UE_LOG(LogTemp, Warning, TEXT("CurrentDurability = %f"), Instance.CurrentDurability)
		UE_LOG(LogTemp, Warning, TEXT("MaxWear = %f"), Row.MaxWear)
		UE_LOG(LogTemp, Warning, TEXT("CurrentWear = %f"), Instance.CurrentWear)
		UE_LOG(LogTemp, Warning, TEXT("UsesDurability  = %s"), WfInv::UsesDurability(Row, Instance) ? TEXT("True") : TEXT("False"))
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Item %s is not create successfully!"), *Row.Name.ToString());
	}
}

void AWarfallCharacter::Test4()
{
	FItemRow Row;
	FItemInstance Instance1;
	FItemInstance Instance2;

	if (AddItem("Apple", 2, Row, Instance1))
	{
		UE_LOG(LogTemp, Log, TEXT("Item %s is create successfully!"), *Row.Name.ToString());
		UE_LOG(LogTemp, Warning, TEXT("-------Instance1-------"))
		UE_LOG(LogTemp, Warning, TEXT("Count = %d"), Instance1.Count)
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create first instance!"));
	}
	
	if (AddItem("Apple", 3, Row, Instance2))
	{
		UE_LOG(LogTemp, Log, TEXT("Item %s is create successfully!"), *Row.Name.ToString());
		UE_LOG(LogTemp, Warning, TEXT("-------Instance2-------"))
		UE_LOG(LogTemp, Warning, TEXT("Count = %d"), Instance2.Count)
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create second instance!"));
	}

	if (WfInv::Merge(Instance1, Instance2))
	{
		UE_LOG(LogTemp, Log, TEXT("Item %s is create successfully!"), *Row.Name.ToString());
		UE_LOG(LogTemp, Warning, TEXT("-------Merged-------"))
		UE_LOG(LogTemp, Warning, TEXT("Count = %d"), Instance1.Count)
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to merge instances!"));
	}
}

void AWarfallCharacter::Test5()
{
	FItemRow Row;
	FItemInstance Instance;
	FItemInstance SplitInstance;

	if (AddItem("Iron_Ore", 10, Row, Instance))
	{
		UE_LOG(LogTemp, Log, TEXT("Item %s is create successfully!"), *Row.Name.ToString());
		UE_LOG(LogTemp, Warning, TEXT("-------Instance-------"))
		UE_LOG(LogTemp, Warning, TEXT("Count = %d"), Instance.Count)
		UE_LOG(LogTemp, Warning, TEXT("NextPerishEpoch = %lld"), Instance.Perish.NextPerishEpoch)
		UE_LOG(LogTemp, Warning, TEXT("NextPerishCount = %d"), Instance.Perish.NextPerishCount)
		UE_LOG(LogTemp, Warning, TEXT("UsesDurability  = %s"), WfInv::UsesDurability(Row, Instance) ? TEXT("True") : TEXT("False"))
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Item %s is not create successfully!"), *Row.Name.ToString());
	}
	if (WfInv::Split(Instance, 5, SplitInstance))
	{
		UE_LOG(LogTemp, Log, TEXT("Item %s is create successfully!"), *Row.Name.ToString());
		UE_LOG(LogTemp, Warning, TEXT("-------SplitInstance-------"))
		UE_LOG(LogTemp, Warning, TEXT("Count = %d"), SplitInstance.Count)
		UE_LOG(LogTemp, Warning, TEXT("NextPerishEpoch = %lld"), SplitInstance.Perish.NextPerishEpoch)
		UE_LOG(LogTemp, Warning, TEXT("NextPerishCount = %d"), SplitInstance.Perish.NextPerishCount)
		UE_LOG(LogTemp, Warning, TEXT("UsesDurability  = %s"), WfInv::UsesDurability(Row, SplitInstance) ? TEXT("True") : TEXT("False"))
		UE_LOG(LogTemp, Warning, TEXT("-------Instance-------"))
		UE_LOG(LogTemp, Warning, TEXT("Count = %d"), Instance.Count)
		UE_LOG(LogTemp, Warning, TEXT("NextPerishEpoch = %lld"), Instance.Perish.NextPerishEpoch)
		UE_LOG(LogTemp, Warning, TEXT("NextPerishCount = %d"), Instance.Perish.NextPerishCount)
		UE_LOG(LogTemp, Warning, TEXT("UsesDurability  = %s"), WfInv::UsesDurability(Row, Instance) ? TEXT("True") : TEXT("False"))
	}
}

