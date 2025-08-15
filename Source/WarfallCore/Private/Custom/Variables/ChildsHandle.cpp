#include "Custom/Variables/ChildsHandle.h"

#include "Utils/Tables.h"

// ===============================[ Property Customization Factory ]============================
// ====================================== FITEMROWHANDLE =======================================
// =============================================================================================

bool FCustomItemRowHandle::GetFilterConditions(const FName RowName, const FName HandleTag) const
{
	const bool bFilterByTag = HandleTag != NAME_None;
	if (const FItemRowDetail* Row = DataTable->FindRow<FItemRowDetail>(RowName, TEXT("")))
	{
		if (!bFilterByTag)
		{
			return true;
		}
		for (const FGameplayTag& Tag : Row->Details.Tags)
		{
			if (Tag.GetTagName().ToString().Contains(HandleTag.ToString()))
			{
				return true;
			}
		}
	}
	return false;
}

ETablePath FCustomItemRowHandle::GetDataTable()
{
	return ETablePath::ItemsTable;
}

FString FCustomItemRowHandle::DefaultFilter() const
{
	return FCustomBaseHandle::DefaultFilter();
}
