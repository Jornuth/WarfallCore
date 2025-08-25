#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Custom/Variables/ChildsHandle.h"
#include "Custom/Variables/Thumbnail.h"
#include "Custom/Variables/ColorPicker.h"
#include "Custom/Variables/MassCalculator.h"
#include "NativeGameplayTags.h"
#include "Utils/Tables.h"
#include "ItemRowTypes.generated.h"

class IDetailGroup;

// =========================[ Item GameplayTags ]============================
UE_DECLARE_GAMEPLAY_TAG_EXTERN(ITEM_Consumable);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(ITEM_Ammunition);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(ITEM_Ingredient);

// ========================[ Item.State GameplayTags ]=======================
UE_DECLARE_GAMEPLAY_TAG_EXTERN(ITEM_STATE_Lootable);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(ITEM_STATE_Perishable);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(ITEM_STATE_Repairable);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(ITEM_STATE_Dismantleable);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(ITEM_STATE_Container);

// ========================[ Item.Meta.Weapon GameplayTags ]========================
UE_DECLARE_GAMEPLAY_TAG_EXTERN(ITEM_META_WEAPON_Damage);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(ITEM_META_WEAPON_AttackSpeed);

// ========================[ Item.Meta.Armor GameplayTags ]========================
UE_DECLARE_GAMEPLAY_TAG_EXTERN(ITEM_META_ARMOR_ArmorValue);

// ========================[ Item.Meta.Harvesting GameplayTags ]========================

// ========================[ Item.Meta.Movements GameplayTags ]========================

// ========================[ Item.Meta.Utils GameplayTags ]========================
UE_DECLARE_GAMEPLAY_TAG_EXTERN(ITEM_META_UTILS_MaxDurability);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(ITEM_META_UTILS_MaxWear);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(ITEM_META_UTILS_Weight);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(ITEM_META_UTILS_Capacity);

// ====================[ Ingredients.Ressource GameplayTags ]================
UE_DECLARE_GAMEPLAY_TAG_EXTERN(INGREDIENTS_RESSOURCES_WoodLog);

// ====================[ Ingredients.Component GameplayTags ]================
UE_DECLARE_GAMEPLAY_TAG_EXTERN(INGREDIENTS_COMPONENTS_MetalBar);

// ===============================[ Enumerations ]===========================

/**
 * Enumeration representing different types of items within the game.
 * Designed to classify items into distinct categories for handling purposes.
 */
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EItemType : uint8
{
	E_None			= 0		 UMETA(DisplayName = "None", Hidden),
	E_Weapon		= 1 << 0 UMETA(DisplayName = "Weapon"),
	E_Armor			= 1 << 1 UMETA(DisplayName = "Armor"),
	E_Consumable	= 1 << 2 UMETA(DisplayName = "Consumable"),
	E_Tool			= 1 << 3 UMETA(DisplayName = "Tool"),
	E_Ammunition	= 1 << 4 UMETA(DisplayName = "Ammunition"),
	E_Ingredient	= 1 << 5 UMETA(DisplayName = "Ingredient"),
	E_Recipe		= 1 << 6 UMETA(DisplayName = "Recipe"),
};
ENUM_CLASS_FLAGS(EItemType)

/**
 * Enumeration representing a collection of flags that define specific behaviors or characteristics
 * of an item. Each flag can be combined using bitwise operations for more complex configurations.
 */
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EItemFlags : uint8
{
	/** No flags applied */
	E_None				= 0	  UMETA(DisplayName = "[No Flags]", Hidden),
	/** Item can be picked up and added to inventory */
	E_CanLooted			= 1 << 0 UMETA(DisplayName = "Can be Looted"),
	/** Item deteriorates over time and can spoil/decay */
	E_CanPerished		= 1 << 1 UMETA(DisplayName = "Can be Perished"), 
	/** Item can be broken down into component parts */
	E_CanDismantled		= 1 << 2 UMETA(DisplayName = "Can be Dismantled"),
	/** Item can be exchanged with NPCs/other players */
	E_CanTraded			= 1 << 3 UMETA(DisplayName = "Can be Traded"),
	/** Item can be sold to vendors for currency */
	E_CanSell			= 1 << 4 UMETA(DisplayName = "Can be Selled"),
	/** Item supports interaction mechanics */
	E_CanInteract		= 1 << 5 UMETA(DisplayName = "Can be Interacted"),
	/** Item's durability can be restored through repairs */
	E_CanRepair			= 1 << 6 UMETA(DisplayName = "Can be Repaired"),
	/** Item has an integrated container for storing other items */
	E_HasContainer		= 1 << 7 UMETA(DisplayName = "Has Container"),
};
ENUM_CLASS_FLAGS(EItemFlags)

/**
 * Enumeration defining the various types of weapons available.
 * Used to categorize weapon items for gameplay and inventory management.
 */
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	E_None UMETA(DisplayName = "None"),
	E_Sword UMETA(DisplayName = "Sword"),
	E_Axe UMETA(DisplayName = "Axe"),
	E_Hammer UMETA(DisplayName = "Hammer"),
	E_Dagger UMETA(DisplayName = "Dagger"),
	E_Shield UMETA(DisplayName = "Shield"),
	E_Bow UMETA(DisplayName = "Bow"),
	E_Crossbow UMETA(DisplayName = "Crossbow"),
};

/**
 * Enumeration defining various types of tools available within the game.
 * Used to categorize items that serve specific purposes based on their tool type.
 */
UENUM(BlueprintType)
enum class EToolType : uint8
{
	E_None UMETA(DisplayName = "None"),
	E_Pickaxe UMETA(DisplayName = "Pickaxe"),
	E_Shovel UMETA(DisplayName = "Shovel"),
	E_Axe UMETA(DisplayName = "Axe"),
	E_Knife UMETA(DisplayName = "Knife"),
	E_Slicer UMETA(DisplayName = "Slicer")
};

/**
 * Enumeration specifying various types of ammunition available in the game.
 * This is used to categorize and handle different ammunition types effectively.
 */
UENUM(BlueprintType)
enum class EAmmunitionType : uint8
{
	E_None UMETA(DisplayName = "None"),
	E_Arrow UMETA(DisplayName = "Arrow"),
	E_Bolt UMETA(DisplayName = "Bolt"),
	E_Stone UMETA(DisplayName = "Stone"),
	E_Bullet UMETA(DisplayName = "Bullet"),
	E_Shell UMETA(DisplayName = "Shell"),
	E_Explosive UMETA(DisplayName = "Explosive"),
	E_Energy UMETA(DisplayName = "Energy")
};

/**
 * Enumeration defining the rules for binding items to a character or player.
 * Specifies the conditions under which binding actions are applied to the item.
 */
UENUM(BlueprintType)
enum class EBindRule : uint8
{
	None UMETA(DisplayName = "Unbound"),
	OnPickup UMETA(DisplayName = "On Pickup"),
	OnEquip UMETA(DisplayName = "On Equip"),
};

// ===============================[ Defaults values ]============================

constexpr float DefaultPerishTime()
{
	return 60.0f;
}
constexpr float DefaultRepairTime()
{
	return 20.0f;
}

// ===============================[ Structures ]============================

/**
 * Structure representing the data required for repairing items in the game.
 * This includes the material needed for the repair and the quantity required.
 * It is used to define item repair mechanics and dependencies.
 */
USTRUCT(BlueprintType)
struct FItemRepairData
{
	GENERATED_BODY()

	/** Item or material required to perform repairs */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FItemRowHandle MaterialToRepair;

	/** Amount of repair material needed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity;

	FItemRepairData() :
	 MaterialToRepair(FItemRowHandle(NAME_None)),
	Quantity(1)
	{}
};

/**
 * Structure representing the specification of a bag in a grid-based inventory system.
 * Defines the size and capacity of the bag through its dimensions.
 */
USTRUCT(BlueprintType)
struct FBagSpec
{
	GENERATED_BODY()

	/**
	 * Represents the dimensions of the bag in terms of width and height.
	 * Utilized to define the size and capacity of inventory bags in a grid-based system.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint Dimensions;

	FBagSpec() :
	 Dimensions(FIntPoint(1, 1))
	{}
};

/**
 * Structure representing the specification of a pocket or container within a system.
 * It defines the dimensions, allowed and restricted classifications,
 * as well as the perish rate adjustment for items stored within.
 */
USTRUCT(BlueprintType)
struct FPocketSpec
{
	GENERATED_BODY()

	/**
	 * Represents the dimensions of a pocket or container in terms of grid size.
	 * Used to define the width (X) and height (Y) of the storage space.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint Dimensions;
	/**
 * Indicates whether the pocket or container can be managed by its owner.
 * Determines whether ownership-specific actions or permissions are allowed for this container.
 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPlayerEditableFilters;
	/**
	 * A container of gameplay tags representing allowed classifications or categories.
	 * Utilized to restrict items or entities to only those that match specified tags.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer Whitelist;
	/**
	 * A container of gameplay tags used to define restricted classifications or categories.
	 * Functions as a mechanism to block items or entities that match the specified tags from being used or stored.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer Blacklist;
	/**
	 * Multiplier applied to the base perish rate of items within a container or pocket.
	 * Used to adjust how quickly stored items degrade or expire over time.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PerishRateMultiplier;

	FPocketSpec() :
	 Dimensions(FIntPoint(4, 3)),
	bPlayerEditableFilters(true),
	PerishRateMultiplier(1.0f)
	{}
};

/**
 * Struct representing an entry of item metadata used to define modifications to game statistics.
 * Each entry consists of a key identifying the statistic and a value determining the modification amount.
 */
USTRUCT(BlueprintType)
struct FItemMetaEntry
{
	GENERATED_BODY()

	/**
	 * Represents a gameplay tag used as a key within item metadata.
	 * Identifies which statistic will be modified by the item when used.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "Item.Meta"))
	FGameplayTag Key;
	/**
	 * Represents the magnitude or numerical amount associated with the corresponding key in item metadata.
	 * Used to define the value that modifies a specific statistic when the item is utilized.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value = 0;
};

// ===============================[ Property Customization Factory ]============================
// ========================================== FITEMROW =========================================
// =============================================================================================

/**
 * Represents a complete definition of an item in the game.
 * This structure encapsulates all data related to item type, visuals, interaction, behavior, 
 * gameplay logic (combat, armor, tools, consumables), durability, and customization.
 */
USTRUCT(BlueprintType)
struct FItemRow
{
	GENERATED_BODY()
	
	/** Display the name of the item. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Name;
	/** Short description displayed in tooltips or UI previews. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ShortDesc;
	/** Long-form description of the item, used in detailed views or logs. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText LongDesc;
	/** The primary type that defines the core behavior and usage of the item. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemType Type;
	/** World-side class used to spawn the object visually into the world. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AVisualItem> VisualItemClass;
	/** Static mesh used to represent the object physically in the world. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UStaticMesh> DropMesh;
	/** Flags defining item capabilities like loot ability, repairability, tradability, etc. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "Item.State"))
	FGameplayTagContainer Flags;
	/** Thumbnail data, including icon texture, dimensions, and visual setup for UI inventory. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FThumbnail Thumbnail;
	/**
	 * Specifies rules for binding, controlling how bindings are applied or enforced.
	 * Utilized to manage the interaction and constraints associated with bindings.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBindRule BindRule;
	/** Quality level of the item. Can affect bonuses, rarity, or effects. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quality;
	/** Maximum number of items that can be stacked in a single inventory slot. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 1))
	int32 MaxStackSize;

	/**
	 * Represents a gameplay tag used to define and categorize different types of ingredients.
	 * Intended for use in gameplay logic and data organization.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient", meta = (Categories = "Ingredients"))
	FGameplayTag IngredientsType;
	/**
	 * Array of item meta entries used to define additional modifiers or attributes.
	 * Configurable through the editor and accessible in blueprints for flexibility.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient")
	TArray<FItemMetaEntry> MetaModifiers;
	
	// ========================== Extras Settings: Container Settings ==========================
	
	/**
	 * Boolean flag that determines if the container is considered a pocket.
	 * Used to specify whether the container behaves as a pocket within the system.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ExtrasSettings|ContainerSettings")
	bool bIsPocket;
	/**
	 * Struct property representing the specifications of a bag within the container system.
	 * Allows customization and configuration through the editor with specific conditions.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Container", meta=(EditCondition="!bIsPocket", EditConditionHides))
	FBagSpec Bag;
	/**
	 * Represents the specifications of a pocket within a container.
	 * Used to define and manage the characteristics of a pocket in inventory systems.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Container", meta=(EditCondition="bIsPocket", EditConditionHides))
	FPocketSpec Pocket;

	// ========================== Extras Settings: Interaction Settings ==========================
    
	/** Duration in seconds required to interact with the item (e.g., pick up, use). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ExtrasSettings|InteractionSettings")
	float InteractionDuration;
	/** Minimum distance required to allow interaction with the item. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ExtrasSettings|InteractionSettings")
	float MinInteractionDistance;
	/** Whether this item uses outlines during interactions. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ExtrasSettings|InteractionSettings|Outlines")
	bool bIsUsingOutlines;
	/** Outline color used when the item is interactable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ExtrasSettings|InteractionSettings|Outlines")
	FColorPicker OutlinesColor;

	// ========================== Extras Settings: Physical Properties ==========================

	/** Configuration of the item's weight and material composition. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ExtrasSettings|WeightSettings")
	FMassObject WeightConfig;

	// ========================== Equipable: Weapon ==========================

	/** Specific weapon type (e.g., Sword, Bow, Hammer). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipeable|Weapon", DisplayName = "Type")
	EWeaponType WeaponType;
	/** Number of hands required to use the weapon.
	 *
	 * Index values:
	 *   0 = One-handed
	 *   1 = Two-Handed
	 *   2 = Ambidextrous
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipeable|Weapon", meta = (ClampMin = 0, ClampMax = 2))
	int32 Handling;
	/** Compatible ammunition for ranged weapons. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipeable|Weapon")
	TArray<FItemRowHandle> CompatibleAmmunition;

	/**
	 * Soft object pointer referencing a static mesh asset.
	 * Can be edited in the editor and accessed from Blueprints, categorized under "Weapon".
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSoftObjectPtr<UStaticMesh> Mesh;
	/**
	 * A soft object pointer to a skeletal mesh used for representing weapon visuals.
	 * Allows assignment and editing within the editor and serves as a blueprint-accessible property.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;
	/**
	 * Represents the amount of damage dealt by a weapon in the game.
	 * Used to determine the impact power when the weapon is utilized.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Data", DisplayName = "Damage")
	float WeaponDamage;
	/**
	 * A configurable property representing the amount of armor penetration applied by a weapon.
	 * Determines the degree to which the weapon bypasses an enemy's armor, with a range between 0.0 and 100.0.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Data", meta = (ClampMin = 0.0f, ClampMax = 100.0f))
	float ArmorPenetration;
	/**
	 * Represents the amount of damage this weapon inflicts on flesh targets.
	 * Configurable within the specified range to balance gameplay mechanics.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Data", meta = (ClampMin = 0.0f, ClampMax = 100.0f))
	float FleshDamage;
	/** Resource cost: for melee, per attack; for ranged, per 0.5s aiming. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Data")
	float RessourceCost;
	/** Minimum cone angle of attack for melee weapons. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Data")
	float MinConeAngle;
	/** Maximum cone angle of attack for melee weapons. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Data")
	float MaxConeAngle;
	/** Speed multiplier affecting weapon swing or attack frequency. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Data")
	float SpeedMultiplier;
	/** Maximum effective range for ranged weapons. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Data")
	float MaxRange;
	/** Speed at which the character transitions into aiming mode. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Data")
	float AimingSpeedMultiplier;
	/** Multiplier affecting how fast the weapon reloads. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Data")
	float ReloadTimeMultiplier;

	// ========================== Equipable: Armor ==========================

	/** Type of armor (can affect stat modifiers or set bonuses).
	 *
	 * Index values:
	 *   0 = Light
	 *   1 = Medium
	 *   2 = Heavy
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipeable|Armor", DisplayName = "Type", meta = (ClampMin = 0, ClampMax = 2))
	int32 ArmorType;
	/**
	 * Equipment slot the armor occupies. Determines where the armor is equipped on the character.
	 *
	 * Index values:
	 *   0 = Head
	 *   1 = Shoulders
	 *   2 = Chest
	 *   3 = Hands
	 *   4 = Legs
	 *   5 = Feets
	 *   6 = Back
	 *   7 = Neck
	 *   8 = Wrists
	 *   9 = Fingers
	 *  10 = Waist
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipeable|Armor", DisplayName = "Slot", meta = (ClampMin = 0, ClampMax = 10))
	int32 ArmorSlot;
	/**
	 * Represents the defensive value of an armor item within the game.
	 * Determines how much damage reduction the armor provides to the wearer.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipeable|Armor", DisplayName = "Value")
	float ArmorValue;
	/** Male version of the equipped mesh. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipeable|Armor")
	USkeletalMesh* Male;
	/** Female version of the equipped mesh. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipeable|Armor")
	USkeletalMesh* Female;

	// ========================== Equipable: Tool ==========================
	/** Specific tool type (e.g., Pickaxe, Knife). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipeable|Tool", DisplayName = "Type")
	EToolType ToolType;
	/** Harvesting multiplier applied to resource gathering. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipeable|Tool")
	float HarvestMultiplier;

	// ========================== Equipable: Ammunition ==========================

	/** Type of ammunition this item represents. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipeable|Ammunition", DisplayName = "Type")
	EAmmunitionType AmmunitionType;
	/** Damage applied when used as a projectile. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipeable|Ammunition", DisplayName = "Damage")
	float ProjectileDamage;
	/** Speed multiplier for projectile flight. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipeable|Ammunition", DisplayName = "SpeedMultiplier")
	float ProjectileSpeedMultiplier;

	// ========================== Consumable ==========================

	/** Food value restored when consumed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consumable")
	float FoodValue;
	/** Water value restored when consumed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consumable")
	float WaterValue;
	
	// ========================== Durability ==========================

	/** Rate at which the item perishes over time.
	 *
	 * Based on 60 sec. (60x1) = 1 min.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consumable", meta = (ClampMin = 0.0f))
	float PerishRate;
	/** Item that this turns into when it perishes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consumable")
	FItemRowHandle PerishTo;
	/** Maximum durability of the item before breaking. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Durability")
	float MaxDurability;
	/**
	 * Maximum wear value determining the durability limit of an item.
	 * Represents the upper threshold for wear and tear an item can withstand.
	 * 0 = Unbreakable
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Durability")
	float MaxWear;
	/** Multiplier affecting how fast repairs occur.
	 *
	 * Based on 20 sec. (20x1) = 20 sec.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Durability", DisplayName = "Repair Time", meta = (ClampMin = 0.0f))
	float RepairTimeMultiplier;
	/** Materials required to repair the item. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Durability")
	TArray<FItemRepairData> RepairData;
	/** Modifiers applied to stats when equipped or consumed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, float> Modifiers;
	/** Gameplay tags used for filtering, effects, or UI. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer Tags;
	
	FItemRow();
	
	
	bool GetEquipeableVisible() const { return GetWeaponVisible() || GetArmorVisible() || GetToolVisible() || GetAmmunitionVisible(); }
	bool GetWeaponVisible() const { return Type == EItemType::E_Weapon; }
	bool GetArmorVisible() const { return Type == EItemType::E_Armor; }
	bool GetToolVisible() const { return Type == EItemType::E_Tool; }
	bool GetAmmunitionVisible() const { return Type == EItemType::E_Ammunition; }
	
	bool GetConsumableVisible() const { return Type == EItemType::E_Consumable; }

	bool HasContainer() const { return Flags.HasTag(ITEM_STATE_Container) ; }
		
	bool CanPerish() const { return GetConsumableVisible() && Flags.HasTag(ITEM_STATE_Perishable) && PerishRate > 0.f; }
	bool HasDurability() const { return GetWeaponVisible() || GetArmorVisible() || GetToolVisible(); }
	float GetMaxWear () const { return MaxDurability * MaxWear; }
	
	bool IsUsingAmmunition() const
	{
		const bool IsValidType = WeaponType == EWeaponType::E_Bow || WeaponType == EWeaponType::E_Crossbow;
		return GetWeaponVisible() && IsValidType;
	}
	bool CanBeRepair() const
	{
		return HasDurability() && Flags.HasTag(ITEM_STATE_Repairable) && RepairTimeMultiplier > 0.f;
	}

	int32 GetPerishTime() const { return PerishRate * DefaultPerishTime(); }
	int32 GetRepairTime() const { return RepairTimeMultiplier * DefaultRepairTime(); }

	FItemRow& operator = (const FItemRow& Other)
	{
		if (this != &Other)
		{
			Name = Other.Name;
			ShortDesc = Other.ShortDesc;
			LongDesc = Other.LongDesc;
			Type = Other.Type;
			VisualItemClass = Other.VisualItemClass;
			DropMesh = Other.DropMesh;
			Flags = Other.Flags;
			Thumbnail = Other.Thumbnail;
			BindRule = Other.BindRule;
			Quality = Other.Quality;
			MaxStackSize = Other.MaxStackSize;

			IngredientsType = Other.IngredientsType;
			MetaModifiers = Other.MetaModifiers;
			
			bIsPocket = Other.bIsPocket;
			Bag = Other.Bag;
			Pocket = Other.Pocket;
						
			InteractionDuration = Other.InteractionDuration;
			MinInteractionDistance = Other.MinInteractionDistance;
			bIsUsingOutlines = Other.bIsUsingOutlines;
			OutlinesColor = Other.OutlinesColor;

			WeightConfig = Other.WeightConfig;
						
			WeaponType = Other.WeaponType;
			Handling = Other.Handling;
			CompatibleAmmunition = Other.CompatibleAmmunition;

			Mesh = Other.Mesh;
			SkeletalMesh = Other.SkeletalMesh;
			
			WeaponDamage = Other.WeaponDamage;
			ArmorPenetration = Other.ArmorPenetration;
			FleshDamage = Other.FleshDamage;
			RessourceCost = Other.RessourceCost;
			MinConeAngle = Other.MinConeAngle;
			MaxConeAngle = Other.MaxConeAngle;
			SpeedMultiplier = Other.SpeedMultiplier;
			MaxRange = Other.MaxRange;
			AimingSpeedMultiplier = Other.AimingSpeedMultiplier;
			ReloadTimeMultiplier = Other.ReloadTimeMultiplier;
			
			ArmorType = Other.ArmorType;
			ArmorSlot = Other.ArmorSlot;
			ArmorValue = Other.ArmorValue;
			Male = Other.Male;
			Female = Other.Female;
	
			ToolType = Other.ToolType;
			HarvestMultiplier = Other.HarvestMultiplier;

			AmmunitionType = Other.AmmunitionType;
			ProjectileDamage = Other.ProjectileDamage;
			ProjectileSpeedMultiplier = Other.ProjectileSpeedMultiplier;
			
			FoodValue = Other.FoodValue;
			WaterValue = Other.WaterValue;
			
			PerishRate = Other.PerishRate;
			PerishTo = Other.PerishTo;
			MaxDurability = Other.MaxDurability;
			MaxWear = Other.MaxWear;
			RepairTimeMultiplier = Other.RepairTimeMultiplier;
			RepairData = Other.RepairData;
			
			Modifiers = Other.Modifiers;
			Tags = Other.Tags;
		}
		return *this;
	}
};

USTRUCT(BlueprintType)
struct FItemInstance
{
	GENERATED_BODY()

	FGuid GuidID;
	
};

// ===============================[ Property Customization Factory ]============================
// ======================================== FCUSTOMITEMROW =====================================
// =============================================================================================

class WARFALLCORE_API FCustomItemRow : public IPropertyTypeCustomization
{
	// ========== FUNCTIONS ==========
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShareable(new FCustomItemRow); }
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle,IDetailChildrenBuilder& ChildBuilder,
								   IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

protected:
	void DeclareProperties();
	void BindDelegatesProperties();

private:
	void CreateDamageSliders(IDetailGroup& Group) const;
	
	void OnFlagsChanged();
	void OnTypeChanged();

	static float GetDurationInSeconds(const TSharedPtr<IPropertyHandle>& PropertyHandle, float BaseDuration);
	static float GetDisplayDuration(const TSharedPtr<IPropertyHandle>& PropertyHandle, float BaseDuration);
	static FText GetDurationUnitText(const TSharedPtr<IPropertyHandle>& PropertyHandle, float BaseDuration);

	// ========== VARIABLES ==========
	TSharedPtr<IPropertyHandle> PropertyHandle;
	TSharedPtr<IPropertyUtilities> PropertyUtils;
	
	TSharedPtr<FDetailMessageRow> DetailMessage;

protected:
	FItemRow* Handle = nullptr;
	FGameplayTagContainer CurrentFlags;
	EItemType CurrentType = EItemType::E_None;
	
	TSharedPtr<IPropertyHandle> NameHandle;
	TSharedPtr<IPropertyHandle> ShortDescHandle;
	TSharedPtr<IPropertyHandle> LongDescHandle;
	TSharedPtr<IPropertyHandle> TypeHandle;
	TSharedPtr<IPropertyHandle> VisualItemClassHandle;
	TSharedPtr<IPropertyHandle> DropMeshHandle;
	TSharedPtr<IPropertyHandle> FlagsHandle;
	TSharedPtr<IPropertyHandle> ThumbnailHandle;
	TSharedPtr<IPropertyHandle> BindRuleHandle;
	TSharedPtr<IPropertyHandle> QualityHandle;
	TSharedPtr<IPropertyHandle> MaxStackSizeHandle;

	TSharedPtr<IPropertyHandle> IngredientsTypeHandle;
	TSharedPtr<IPropertyHandle> MetaModifiersHandle;
	
	TSharedPtr<IPropertyHandle> IsPocketHandle;
	TSharedPtr<IPropertyHandle> BagHandle;
	TSharedPtr<IPropertyHandle> PocketHandle;
	
	TSharedPtr<IPropertyHandle> InteractionDurationHandle;
	TSharedPtr<IPropertyHandle> MinInteractionDistanceHandle;
	TSharedPtr<IPropertyHandle> IsUsingOutlinesHandle;
	TSharedPtr<IPropertyHandle> OutlinesColorHandle;

	TSharedPtr<IPropertyHandle> WeightConfigHandle;

	TSharedPtr<IPropertyHandle> MeshHandle;
	TSharedPtr<IPropertyHandle> SkeletalMeshHandle;
	
	TSharedPtr<IPropertyHandle> WeaponTypeHandle;
	TSharedPtr<IPropertyHandle> HandlingHandle;
	TSharedPtr<IPropertyHandle> CompatibleAmmunitionHandle;

	TSharedPtr<IPropertyHandle> WeaponDamageHandle;
	TSharedPtr<IPropertyHandle> ArmorPenetrationHandle;
	TSharedPtr<IPropertyHandle> FleshDamageHandle;
	TSharedPtr<IPropertyHandle> RessourceCostHandle;
	TSharedPtr<IPropertyHandle> MinConeAngleHandle;
	TSharedPtr<IPropertyHandle> MaxConeAngleHandle;
	TSharedPtr<IPropertyHandle> SpeedMultiplierHandle;
	TSharedPtr<IPropertyHandle> MaxRangeHandle;
	TSharedPtr<IPropertyHandle> AimingSpeedHandle;
	TSharedPtr<IPropertyHandle> ReloadTimeMultiplierHandle;

	TSharedPtr<IPropertyHandle> ArmorTypeHandle;
	TSharedPtr<IPropertyHandle> ArmorSlotHandle;
	TSharedPtr<IPropertyHandle> ArmorValueHandle;
	TSharedPtr<IPropertyHandle> MaleHandle;
	TSharedPtr<IPropertyHandle> FemaleHandle;

	TSharedPtr<IPropertyHandle> ToolTypeHandle;
	TSharedPtr<IPropertyHandle> HarvestMultiplierHandle;
	
	TSharedPtr<IPropertyHandle> AmmunitionTypeHandle;
	TSharedPtr<IPropertyHandle> DamageHandle;
	TSharedPtr<IPropertyHandle> ProjectileSpeedMultiplierHandle;
	
	TSharedPtr<IPropertyHandle> FoodValueHandle;
	TSharedPtr<IPropertyHandle> WaterValueHandle;
	
	TSharedPtr<IPropertyHandle> PerishRateHandle;
	TSharedPtr<IPropertyHandle> PerishToHandle;
	TSharedPtr<IPropertyHandle> MaxDurabilityHandle;
	TSharedPtr<IPropertyHandle> MaxWearHandle;
	TSharedPtr<IPropertyHandle> RepairTimeMultiplierHandle;
	TSharedPtr<IPropertyHandle> RepairDataHandle;

	TSharedPtr<IPropertyHandle> ModifiersHandle;
};

USTRUCT(BlueprintType)
struct WARFALLCORE_API FItemRowDetail : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FItemRow Details;

	FItemRowDetail() :
	 Details(FItemRow())
	{}
	
};

namespace HlpItem
{
	inline bool GetItemRow(const FName RowID, FItemRow& OutItemRow)
	{
		const UDataTable* Table = UTables::GetTable(ETablePath::ItemsTable);
		if (!Table) { return false; }

		const FItemRowDetail* RowDetail = Table->FindRow<FItemRowDetail>(RowID, TEXT("Finding"));
		if (!RowDetail) { return false; }
		OutItemRow = RowDetail->Details;
		return true;
	}
	inline bool IsPerishable(const FItemRow& Row) { return Row.CanPerish();	}
	inline int32 GetPerishTime(const FItemRow& Row) { return Row.GetPerishTime(); }
	inline float GetDurabilityMax(const FItemRow& Row) { return Row.MaxDurability; }
	inline float GetWearMax(const FItemRow& Row) { return Row.GetMaxWear(); }
	inline FIntPoint GetFootprint(const FItemRow& Row) { return Row.Thumbnail.GetFixedDimensions(); }
	inline float GetMass(const FItemRow& Row) { return Row.WeightConfig.GetMass(); }
	
}
