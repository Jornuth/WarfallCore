#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "IDetailGroup.h"
#include "Custom/Variables/ChildsHandle.h"
#include "Custom/Variables/Thumbnail.h"
#include "Custom/Variables/ColorPicker.h"
#include "Custom/Variables/MassCalculator.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "InventoryTypes.generated.h"

// ===============================[ Enumerations ]============================

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
	E_Resource 		= 1 << 5 UMETA(DisplayName = "Resource"),
	E_Component 	= 1 << 6 UMETA(DisplayName = "Component"),
	E_Other			= 1 << 7 UMETA(DisplayName = "Other")
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

constexpr uint8 DefaultItemFlags()
{
	return static_cast<uint8>(
		  EItemFlags::E_CanLooted
		| EItemFlags::E_CanPerished
		| EItemFlags::E_CanTraded
		| EItemFlags::E_CanInteract
		| EItemFlags::E_CanRepair
	);
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
	 PerishRateMultiplier(1.0f)
	{}
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = "/Script/WarfallCore.EItemFlags"))
	uint8 Flags;
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
	TArray<FName> Tags;

	FItemRow();

	bool HasFlag(const EItemFlags Flag) const;
	bool HasTag(const FName Tag) const { return Tags.Contains(Tag); };
	void AddTag(const FName Tag, const int32 Index)
	{
		if (Tags.Num() > 0 && Tags[0] == Tag)
			return;
		if (!Tags.IsValidIndex(Index))
		{
			Tags.SetNum(Index + 1);
		}
		Tags[Index] = Tag;
	};
	void RemoveTag(const int32 Index)
	{
		if (!Tags.IsValidIndex(Index))
			return;
		Tags[Index] = NAME_None;
	};

	bool GetInteractionVisible() const { return HasFlag(EItemFlags::E_CanInteract); }
	
	bool GetEquipeableVisible() const { return GetWeaponVisible() || GetArmorVisible() || GetToolVisible() || GetAmmunitionVisible(); }
	bool GetWeaponVisible() const { return Type == EItemType::E_Weapon; }
	bool GetArmorVisible() const { return Type == EItemType::E_Armor; }
	bool GetToolVisible() const { return Type == EItemType::E_Tool; }
	bool GetAmmunitionVisible() const { return Type == EItemType::E_Ammunition; }
	
	bool GetConsumableVisible() const { return Type == EItemType::E_Consumable; }

	bool HasContainer() const { return HasFlag(EItemFlags::E_HasContainer); }
	
	bool CanPerish() const { return GetConsumableVisible() && HasFlag(EItemFlags::E_CanPerished) && PerishRate > 0.f; }
	bool HasDurability() const { return GetWeaponVisible() || GetArmorVisible() || GetToolVisible(); }
	float GetMaxWear () const { return MaxDurability * MaxWear; }
	
	bool IsUsingAmmunition() const
	{
		const bool IsValidType = WeaponType == EWeaponType::E_Bow || WeaponType == EWeaponType::E_Crossbow;
		return GetWeaponVisible() && IsValidType;
	}
	bool CanBeRepair() const
	{
		return HasDurability() && HasFlag(EItemFlags::E_CanRepair);
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
	void SetupTag() const;
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
	uint8 CurrentFlags = 0;
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

// ---------- Perish (prochain à périmer, 0 ou 2 dates max) ----------
USTRUCT(BlueprintType)
struct FPerishBuckets
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Perish")
    int64 NextPerishEpoch = 0;   // 0 = non périssable

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Perish")
    int32 NextPerishCount = 0;   // combien expirent exactement à NextPerishEpoch

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Perish")
    int64 LaterPerishEpoch = 0;  // bucket #2 optionnel

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Perish")
    int32 LaterPerishCount = 0;

    bool HasNext()  const { return NextPerishEpoch  > 0 && NextPerishCount  > 0; }
    bool HasLater() const { return LaterPerishEpoch > 0 && LaterPerishCount > 0; }
    void PromoteLaterToNext(){ NextPerishEpoch=LaterPerishEpoch; NextPerishCount=LaterPerishCount; LaterPerishEpoch=0; LaterPerishCount=0; }
};

// ---------- L’instance d’item (un “stack” d’items) ----------
USTRUCT(BlueprintType)
struct FItemInstance
{
    GENERATED_BODY()

    // NOTE: Garde l'ordre de déclaration = ordre d'initialisation pour éviter C5038
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Instance")
    FGuid InstanceGuid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Instance")
    FName RowID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Instance")
    int32 Count = 0;

    // Si Row.BindRule != None : owner effectif. Remplace par FGuid si pas d'OSS.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bind")
    FUniqueNetIdRepl BoundOwnerId;

    // État d’instance (actif seulement si Count==1 et Row a de la durabilité)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State")
    float CurrentDurability = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State")
    float CurrentWear = 0.f; // = Row.WearMax * Row.DurabilityMax (0 => Unbreakable)

    // Péremption directe
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Perish")
    FPerishBuckets Perish;

    // ctor simple (évite le warning d'ordre C5038)
    FItemInstance()
        :
		RowID(NAME_None)
        , Count(0)
        , BoundOwnerId()
        , CurrentDurability(0.f)
        , CurrentWear(0.f)
        , Perish()
    {}
};

// ---------- Containers (grille + FastArray) ----------
USTRUCT(BlueprintType)
struct FContainerId
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Container") FName Name = NAME_None; // "Backpack", "Pocket_0"...
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Container") FGuid Guid;
};

USTRUCT(BlueprintType)
struct FGridCoord { GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid") int32 X = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid") int32 Y = 0;
};

USTRUCT(BlueprintType)
struct FGridSize { GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid") int32 Width  = 6;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid") int32 Height = 6;
};

USTRUCT(BlueprintType)
struct FContainerEntry : public FFastArraySerializerItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item")
    FItemInstance Instance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
    FGridCoord TopLeft;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
    bool bRotated90 = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
    FIntPoint FootprintPlaced = FIntPoint(1,1);
};

USTRUCT(BlueprintType)
struct FContainerData : public FFastArraySerializer
{
    GENERATED_BODY()

    UPROPERTY() TArray<FContainerEntry> Entries;

    bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
    { return FFastArraySerializer::FastArrayDeltaSerialize<FContainerEntry, FContainerData>(Entries, DeltaParms, *this); }
};
template<> struct TStructOpsTypeTraits<FContainerData> : public TStructOpsTypeTraitsBase2<FContainerData>
{ enum { WithNetDeltaSerializer = true }; };

USTRUCT(BlueprintType)
struct FContainerMeta
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid") FGridSize Grid = {6,6};
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rules") float PerishRateMultiplier = 1.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rules") FGameplayTagContainer Whitelist;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rules") FGameplayTagContainer Blacklist;
};

// ---------- Utils compacts (inline, pas de BP lib) ----------
namespace WfInv
{
    // ⚠️ Adapte ces accès aux *noms exacts* de TES champs FItemRow
    inline bool  RowIsPerishable(const FItemRow& R)  { return R.CanPerish(); }     // adapte
    inline int32 RowPerishSeconds(const FItemRow& R) { return R.GetPerishTime(); }         // adapte
    inline float RowDurabilityMax(const FItemRow& R) { return R.MaxDurability; }         // adapte
    inline float RowWearMax(const FItemRow& R)       { return R.MaxWear; }               // adapte
    inline FIntPoint RowFootprint(const FItemRow& R) { return R.Thumbnail.GetFixedDimensions(); }  // adapte
    inline float RowUnitMass(const FItemRow& R)      { return R.WeightConfig.GetMass(); }                  // ou MassCalculator

    inline bool UsesDurability(const FItemRow& R, const FItemInstance& I)
    { return (I.Count == 1) && (RowDurabilityMax(R) > 0.f); }

    inline bool CreateInstance(const FItemRow Row, FName RowID, int32 Count, float PerishRateMultiplier, FItemInstance& Out)
    {
        if (!RowID.IsValid() || Count <= 0) return false;
        Out = FItemInstance{};
        Out.InstanceGuid = FGuid::NewGuid();
        Out.RowID = RowID;
        Out.Count = Count;

        const float DurMax = RowDurabilityMax(Row);
        if (Count == 1 && DurMax > 0.f)
        {
            Out.CurrentDurability = DurMax;
            const float WearMax = RowWearMax(Row);
            Out.CurrentWear = (WearMax <= 0.f) ? 0.f : WearMax * DurMax; // 0 => Unbreakable
        }

        if (RowIsPerishable(Row))
        {
            const int64 Now   = FDateTime::Now().ToUnixTimestamp();
            const int64 Base  = (int64)RowPerishSeconds(Row);
            const int64 Delay = (int64)(Base * FMath::Max(0.01f, PerishRateMultiplier));
            Out.Perish.NextPerishEpoch  = Now + Delay;
            Out.Perish.NextPerishCount  = FMath::Min(Count, 1);
            Out.Perish.LaterPerishEpoch = 0;
            Out.Perish.LaterPerishCount = 0;
        }
        else
        {
            Out.Perish = FPerishBuckets{};
        }
        return true;
    }

    inline float Mass(const FItemRow& R, const FItemInstance& I)
    { return RowUnitMass(R) * FMath::Max(0, I.Count); }

    inline bool Merge(FItemInstance& A, const FItemInstance& B)
    {
        if (A.RowID != B.RowID) return false;
        const bool AHasOwner = A.BoundOwnerId.IsValid();
        const bool BHasOwner = B.BoundOwnerId.IsValid();
        if (AHasOwner || BHasOwner)
        {
            if (!(AHasOwner && BHasOwner && A.BoundOwnerId == B.BoundOwnerId))
                return false;
        }
        A.Count += B.Count;

        auto Acc = [](int64 E, int32 C, int64& NE, int32& NC, int64& LE, int32& LC)
        {
            if (E <= 0 || C <= 0) return;
            if (NE == 0 || E < NE) { if (NE>0&&NC>0){ if (LE==0||NE<LE){LE=NE;LC=NC;} else if (NE==LE){LC+=NC;} } NE=E; NC=C; }
            else if (E == NE) { NC += C; }
            else { if (LE==0||E<LE){LE=E;LC=C;} else if (E==LE){LC+=C;} }
        };
        Acc(B.Perish.NextPerishEpoch , B.Perish.NextPerishCount , A.Perish.NextPerishEpoch , A.Perish.NextPerishCount , A.Perish.LaterPerishEpoch , A.Perish.LaterPerishCount);
        Acc(B.Perish.LaterPerishEpoch, B.Perish.LaterPerishCount, A.Perish.NextPerishEpoch , A.Perish.NextPerishCount , A.Perish.LaterPerishEpoch , A.Perish.LaterPerishCount);

        return true;
    }

    inline bool Split(FItemInstance& A, int32 Amount, FItemInstance& OutNew)
    {
        if (Amount <= 0 || Amount >= A.Count) return false;
        OutNew = A;
        OutNew.InstanceGuid = FGuid::NewGuid();
        OutNew.Count = Amount;
        A.Count -= Amount;

        if (A.Count > 1)      { A.CurrentDurability = 0.f; A.CurrentWear = 0.f; }
        if (OutNew.Count > 1) { OutNew.CurrentDurability = 0.f; OutNew.CurrentWear = 0.f; }
        return true;
    }
}