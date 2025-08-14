#pragma once
#include "CoreMinimal.h"

#include "InventoryType_Runtime.generated.h"

/**
 * @struct FPerishBuckets
 *
 * Represents a structure to manage perish data with two distinct perish buckets.
 * Each bucket contains an epoch timestamp and a count of items associated with that timestamp.
 * The structure provides functionality to check the validity of the buckets and to promote
 * the "Later" perish bucket to the "Next" perish bucket for sequential processing.
 */
USTRUCT(NotBlueprintType)
struct FPerishBuckets
{
	GENERATED_BODY()

	UPROPERTY()
	int64 NextPerishEpoch = 0;
	UPROPERTY()
	int32 NextPerishCount = 0;
	UPROPERTY()
	int64 LaterPerishEpoch = 0;
	UPROPERTY()
	int32 LaterPerishCount = 0;

	bool HasNext() const { return NextPerishEpoch > 0 && NextPerishCount > 0; }
	bool HasLater() const { return LaterPerishEpoch > 0 && LaterPerishCount > 0; }

	void PromoteLaterToNext()
	{
		NextPerishEpoch = LaterPerishEpoch;
		NextPerishCount = LaterPerishCount;
		LaterPerishEpoch = 0;
		LaterPerishCount = 0;
	}
};

/**
 * @struct FItemInstance
 *
 * Represents an instance of an item with associated properties and state information.
 * This structure includes unique identification, count, and state-related data such as
 * durability, wear, and owner binding. It also incorporates perishability information
 * managed through a `FPerishBuckets` structure for tracking the perish status of the item.
 *
 * Key Properties:
 * - `InstanceGuid`: A unique identifier for the item instance.
 * - `RowID`: A reference to the item's data row, identifying the type or variant of the item.
 * - `Count`: The quantity of items represented by this instance.
 * - `BoundOwnerId`: A unique network identifier of the owner to which this instance is bound.
 * - `CurrentDurability`: The current durability level of the item.
 * - `CurrentWear`: The current wear level of the item.
 * - `Perish`: Manages perishability using two distinct buckets for tracking perish data over time.
 *
 * This structure provides constructors for initialization and copying of instances.
 */
USTRUCT(BlueprintType)
struct FItemInstance
{
	GENERATED_BODY()

	/**
	 * @property FGuid InstanceGuid
	 *
	 * Represents the unique identifier for an individual item instance.
	 * This GUID allows tracking and differentiation of specific item instances,
	 * ensuring that each instance can be uniquely referenced within the system.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid InstanceGuid;
	/**
	 * @property FName RowID
	 *
	 * Serves as a unique identifier for referencing rows within a data table or similar structure.
	 * This property allows systems to associate the item instance with specific pre-defined data,
	 * ensuring consistent linkage between runtime instances and static data.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RowID;
	/**
	 * @property int32 Count
	 *
	 * Represents the quantity or count of items within an item instance.
	 * This property is used to manage the number of units of a particular item.
	 * It can be directly edited and read in the editor or through Blueprints for
	 * dynamic gameplay adjustments.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Count;
	/**
	 * @property FUniqueNetIdRepl BoundOwnerId
	 *
	 * Represents the unique network identifier of the owner bound to the item instance.
	 * This property is used to associate the item with a specific owner, enabling
	 * ownership tracking and ensuring that gameplay systems can reference the correct player
	 * or entity that possesses the item. It can be edited or accessed in the editor and
	 * through Blueprints for dynamic ownership management.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bind")
	FUniqueNetIdRepl BoundOwnerId;
	/**
	 * @property float CurrentDurability
	 *
	 * Represents the current durability of an item instance in the system.
	 * This value determines the item's remaining usability or functionality, providing
	 * a mechanism to track and manage wear. It can be updated or read dynamically through
	 * gameplay systems or Blueprints to reflect changes over time or as a result of specific actions.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float CurrentDurability;
	/**
	 * @property float CurrentWear
	 *
	 * Represents the current level of wear of an item instance within the system.
	 * This property is used to track the deterioration or usage of an item over time.
	 * It is editable and readable in the editor or through Blueprints, offering flexibility
	 * for dynamic gameplay mechanics to reflect changes to the item's condition.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float CurrentWear;
	
	UPROPERTY()
	FPerishBuckets Perish;

	FItemInstance() :
	 RowID(NAME_None),
	InstanceGuid(FGuid::NewGuid()),
	Count(0),
	CurrentDurability(0.f),
	CurrentWear(0.f)
	{}
};