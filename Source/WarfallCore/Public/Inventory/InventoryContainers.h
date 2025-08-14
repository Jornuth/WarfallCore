#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "Inventory/InventoryType_Runtime.h"

#include "InventoryContainers.generated.h"

USTRUCT(BlueprintType)
struct FContainerId
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid Guid;

	FContainerId () :
	 Name(NAME_None),
	 Guid(FGuid::NewGuid())
	{}

	explicit FContainerId (const FName InName) :
	 Name(InName),
	 Guid(FGuid::NewGuid())
	{}
};

/**
 * @class FGridCoord
 * @brief Represents a coordinate within a grid structure.
 *
 * The FGridCoord class encapsulates the notion of an individual coordinate in
 * a grid layout, often used in grid-based systems. It defines the position
 * in terms of grid indices and provides utility for managing and manipulating
 * coordinates within this context.
 *
 * This class is commonly used to represent logical positions in grids, maps,
 * or matrices, where each coordinate corresponds to a specific grid cell.
 *
 * Features include:
 * - Defining grid-based spatial positions.
 * - Operations for coordinate manipulation.
 * - Compatibility with systems employing grid-based layouts.
 */
USTRUCT(BlueprintType)
struct FGridCoord
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 X = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Y = 0;
};

/**
 * @struct FGridSize
 * @brief Defines the dimensions of a grid in terms of width and height.
 *
 * The FGridSize structure provides a simple representation of grid dimensions,
 * typically used for managing layouts in grid-based systems. It specifies the
 * width and height of the grid, which can serve as input for determining grid
 * boundaries, cell arrangements, or for configuring grid-based components.
 *
 * Features include:
 * - Adjustable grid width and height.
 * - Blueprint compatibility for ease of use within editor-driven workflows.
 * - Support for configurations of grids in various systems.
 */
USTRUCT(BlueprintType)
struct FGridSize
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Width = 6;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Height = 6;
	
};

/**
 * @struct FContainerEntry
 * @brief Represents an individual entry within a container structure.
 *
 * The FContainerEntry structure defines a single item entry within a container
 * system, typically used in inventory management or spatial placement systems.
 * This structure not only encapsulates the item being stored but also defines
 * its spatial properties and orientation within the corresponding container.
 *
 * Properties of this structure include:
 * - The item instance being represented.
 * - The top-left grid coordinate of the item within the container.
 * - A flag indicating whether the item is rotated by 90 degrees.
 * - The footprint dimensions of the item as placed in the grid.
 */
USTRUCT(BlueprintType)
struct FContainerEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FItemInstance Instance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGridCoord TopLeft;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRotated90 = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint FootprintPlaced = FIntPoint(1, 1);
};

/**
 * @struct FContainerData
 * @brief Represents a container to store a collection of entries, supporting serialization.
 *
 * The FContainerData structure is designed to manage a collection of FContainerEntry objects
 * in a fast and efficient way using the FFastArraySerializer. It is used in systems where synchronization
 * and replication of container data are required.
 *
 * Features include:
 * - Efficient storage and serialization of container entries.
 * - Compatibility with network delta serialization for replication in multiplayer environments.
 * - Integration with Unreal's fast array serializer for optimized data exchange.
 *
 * The array 'Entries' contains individual FContainerEntry objects, each representing an item and its
 * associated metadata within the container.
 */
USTRUCT(BlueprintType)
struct FContainerData : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FContainerEntry> Entries;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FContainerEntry, FContainerData>(Entries, DeltaParms, *this);
	}	
};

template<> struct TStructOpsTypeTraits<FContainerData> : public TStructOpsTypeTraitsBase2<FContainerData>
{
	enum { WithNetDeltaSerializer = true, };
};

/**
 * @struct FContainerMeta
 * @brief Encapsulates metadata for an inventory container.
 *
 * The FContainerMeta structure defines the properties and characteristics
 * of a container in an inventory system. It provides configuration options
 * such as grid size, perish rate multiplier, and inclusion/exclusion filters
 * for items based on gameplay tags.
 *
 * Features include:
 * - A grid representation defining the container's storage dimensions.
 * - A perish rate multiplier for adjusting item decay rates.
 * - Whitelist and blacklist for controlling the types of items allowed in the container.
 *
 * This struct is commonly used in inventory systems for managing logical
 * storage capacities and applying specific rules or constraints on the
 * contents of the container.
 */
USTRUCT(BlueprintType)
struct FContainerMeta
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGridSize Grid = FGridSize();
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PerishRateMultiplier = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer Whitelist;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer Blacklist;
};