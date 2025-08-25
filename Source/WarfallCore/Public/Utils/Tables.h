#pragma once

#include "CoreMinimal.h"
#include "Paths.h"
#include "Tables.generated.h"

/**
 * @enum ETablePath
 *
 * Represents various predefined paths to different tables within the application.
 * This enumeration is used to identify and reference specific tables in the system.
 *
 * Possible values:
 * - None: Represents an uninitialized or default state.
 * - InputsTable: Refers to the table storing input-related data.
 * - ItemsTable: Refers to the table containing information about items.
 * - ModulesTable: Refers to the table managing module-related data.
 * - StatsTable: Represents the table with statistical information.
 * - CharProfilesTable: Refers to the table containing character profile data.
 */
enum class ETablePath
{
	None,
	InputsTable,
	ItemsTable,
	ModulesTable,
	StatsTable,
	CharProfilesTable,
};

enum class EAssetsDataPath
{
	None,
	AttributesTree,
	SkillsTree,
};

class UInputMappingContext;
class URootModule;

UCLASS(Abstract)
class WARFALLCORE_API UTables : public UObject
{
	GENERATED_BODY()

	public:
	/**
	 * Loads and returns a DataTable based on the provided name.
	 *
	 * @return A pointer to the loaded UDataTable or nullptr if not found.
	 */
	static UDataTable* GetTable(const ETablePath& Table)
	{
		static const TMap<ETablePath, FString> TableMap =
			{
			{ETablePath::InputsTable, INPUT_TABLE_PATH},
			{ETablePath::ItemsTable, ITEM_TABLE_PATH},
			{ETablePath::ModulesTable, MODULE_TABLE_PATH},
			{ETablePath::StatsTable, STATS_TABLE_PATH},
			{ETablePath::CharProfilesTable, CHAR_PROFILE_TABLE_PATH},
			};
		
		const FString* PathPtr = TableMap.Find(Table);
		if (!PathPtr)
		{
			return nullptr;
		}
		return Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, **PathPtr));
	}

	/**
	 * Loads and returns a DataAsset based on the provided name.
	 *
	 * @param DataName One of: "AttributesTree", "SkillsTree".
	 * @return A pointer to the loaded UDataAsset or nullptr if not found.
	 */
	static UDataAsset* GetDataAsset(const EAssetsDataPath& DataName)
	{
		static const TMap<EAssetsDataPath, FString> DataMap =
			{
			{EAssetsDataPath::AttributesTree, ATTRIBUTES_TREE_DATA_PATH},
			{EAssetsDataPath::SkillsTree, SKILLS_TREE_DATA_PATH}
			};
		const FString* PathPtr = DataMap.Find(DataName);
		if (!PathPtr)
		{
			return nullptr;
		}
		return Cast<UDataAsset>(StaticLoadObject(UDataAsset::StaticClass(), nullptr, **PathPtr));
	}
	
	/**
	* Checks if the given DataTableRowHandle is valid (table exists and row is named).
	*
	* @param Handle The handle to validate.
	* @param ErrorResult The message if an error occurs.
	* @return True if valid; false otherwise.
	*/
	static bool IsValidHandle(const FDataTableRowHandle& Handle, FString& ErrorResult)
	{
		if (!Handle.DataTable || Handle.RowName.IsNone())
		{
			ErrorResult = !Handle.DataTable ? TEXT("DataTable is null") : TEXT("RowName is null");
			return false;
		}
		const bool Result = Handle.DataTable->GetRowNames().Contains(Handle.RowName);
		ErrorResult = Result ? TEXT("") : TEXT("Row not found");
		return Result;
	}
};