// Copyright Epic Games, Inc. All Rights Reserved.

#include "WarfallCore.h"

#include "Custom/Variables/ChildsHandle.h"
#include "Custom/Variables/ColorPicker.h"
#include "Inventory/InventoryTypes.h"
#include "Custom/Variables/MassCalculator.h"
#include "Custom/Variables/Thumbnail.h"
#include "Engine/RendererSettings.h"

#define LOCTEXT_NAMESPACE "FWarfallCoreModule"

FCollisionResponseTemplate FWarfallCoreModule::CreateNewCollisionTemplate(const FName& Name, const FName& ObjectTypeName,
	const TEnumAsByte<ECollisionEnabled::Type> CollisionEnabled, FChannels Collisions, FString HelpMessage)
{
	FCollisionResponseTemplate NewTemplate;
	NewTemplate.Name = Name;
	NewTemplate.ObjectTypeName = ObjectTypeName;
	NewTemplate.bCanModify = false;
	NewTemplate.CollisionEnabled = CollisionEnabled;
	NewTemplate.CustomResponses = Collisions.GetChannels();
	NewTemplate.HelpMessage = HelpMessage;
	return NewTemplate;
}


void FWarfallCoreModule::StartupModule()
{
	LaunchCollisionsSystems();
	LaunchCustomSystems();
	
	// Enable custom depth stencil buffer with stencil support 
	URendererSettings* StencilSettings = GetMutableDefault<URendererSettings>();
	StencilSettings->CustomDepthStencil = ECustomDepthStencil::EnabledWithStencil;
	StencilSettings->SaveConfig();

	if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.CustomDepth")))
	{
		CVar->Set(ECustomDepthStencil::EnabledWithStencil, ECVF_SetBySystemSettingsIni);
	}
}

void FWarfallCoreModule::ShutdownModule()
{
	ShutdownCustomSystems();
}

void FWarfallCoreModule::LaunchCollisionsSystems()
{
	// NEWS COLLISIONS CHANNELS
	AddNewCollisionChannel("Interaction", ECR_Ignore);
	AddNewCollisionChannel("Combat", ECR_Ignore);
	AddNewCollisionChannel("Interactable_Object", ECR_Ignore, true);

	// NEWS COLLISIONS PROFILES
	SetupCollisionChannels();
	
	FChannels Interactable;
	Interactable.ResponsesToAllChannels(ECR_Ignore);
	Interactable.ResponseToChannel("Interaction", ECR_Block);
	AddNewCollisionProfile(CreateNewCollisionTemplate(
		"Interactable",
		"Interactable_Object",
		ECollisionEnabled::QueryOnly,
		Interactable,
		"For interactable elements in game!"));

	// RESTART [ONLY IF NEW CHANNELS AND PROFILES CREATED]
	RestartForNewChannels();
}

void FWarfallCoreModule::LaunchCustomSystems()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	
	PropertyModule.RegisterCustomPropertyTypeLayout(
	"ColorPicker", 
	FOnGetPropertyTypeCustomizationInstance::CreateStatic(FCustomColorPicker::MakeInstance)
	);
	PropertyModule.RegisterCustomPropertyTypeLayout(
	"ItemRowHandle",
	FOnGetPropertyTypeCustomizationInstance::CreateStatic(FCustomItemRowHandle::MakeInstance)
	);
	PropertyModule.RegisterCustomPropertyTypeLayout(
	"Thumbnail", 
	FOnGetPropertyTypeCustomizationInstance::CreateStatic(FCustomThumbnail::MakeInstance)
	);
	PropertyModule.RegisterCustomPropertyTypeLayout(
	"MassRatio", 
	FOnGetPropertyTypeCustomizationInstance::CreateStatic(FCustomMassRatio::MakeInstance)
	);
	PropertyModule.RegisterCustomPropertyTypeLayout(
	"MassObject", 
	FOnGetPropertyTypeCustomizationInstance::CreateStatic(FCustomMassObject::MakeInstance)
	);
	PropertyModule.RegisterCustomPropertyTypeLayout(
	"ItemRow", 
	FOnGetPropertyTypeCustomizationInstance::CreateStatic(FCustomItemRow::MakeInstance)
	);
}

void FWarfallCoreModule::ShutdownCustomSystems()
{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
    	PropertyModule.UnregisterCustomPropertyTypeLayout("StatsHandle");
    	PropertyModule.UnregisterCustomPropertyTypeLayout("AttributesHandle");
    	PropertyModule.UnregisterCustomPropertyTypeLayout("SkillsHandle");
    	PropertyModule.UnregisterCustomPropertyTypeLayout("ColorHandle");
    	PropertyModule.UnregisterCustomPropertyTypeLayout("ItemRowHandle");
    	PropertyModule.UnregisterCustomPropertyTypeLayout("Thumbnail");
    	PropertyModule.UnregisterCustomPropertyTypeLayout("MassRatio");
    	PropertyModule.UnregisterCustomPropertyTypeLayout("MassObject");
    	PropertyModule.UnregisterCustomPropertyTypeLayout("SoundHandle");
    	PropertyModule.UnregisterCustomPropertyTypeLayout("ItemRow");
    	PropertyModule.UnregisterCustomPropertyTypeLayout("InventoryDetails");
    	PropertyModule.UnregisterCustomClassLayout("InteractableComponent");
}

void FWarfallCoreModule::RestartForNewChannels()
{
	bool bRequiredRestart = false;
	int32 LoopIndex = 0;
	int32 Count = 0;
	FString AllProfiles;

	for (auto It = ProfileNames.CreateConstIterator(); It; ++It)
	{
		const FString& Name = It.Key();
		if (It.Value())
		{
			AllProfiles += FString::Printf(TEXT("- %s"), *Name);
			if (LoopIndex < ProfileNames.Num() - 1)
			{
				AllProfiles += TEXT("\n");
			}
			bRequiredRestart = true;
			Count ++;
		}
		LoopIndex ++;
	}
	if  (!bRequiredRestart) return;

	RestartLog = FString::Printf(TEXT("%s\n\n%s\n\nL'éditeur va redémarrer pour appliquer les changements."),
		 Count > 1 ? TEXT("Les Channels et/ou Profiles suivants ont été ajoutés au fichier DefaultEngine.ini:") : TEXT("Le Channel et/ou Profile suivant a été ajouté au fichier DefaultEngine.ini:")
		,*AllProfiles);

	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this](float DeltaTime) mutable -> bool
	{
		if (GEditor)
		{
			FPlatformMisc::MessageBoxExt(EAppMsgType::Ok, *RestartLog, TEXT("Redémarrage requis"));

			constexpr bool bWarnBeforeRestart = false;
			FUnrealEdMisc::Get().RestartEditor(bWarnBeforeRestart);
			return false;
		}
		return true;
	}), 0.2);
}

void FWarfallCoreModule::SetupCollisionChannels()
{
	const UEnum* Enum = StaticEnum<ECollisionChannel>();
	const int32 NumEnum = Enum->NumEnums();
	const FString KeyName = TEXT("DisplayName");
	const FString TraceType = TEXT("TraceQuery");

	for (int32 EnumIndex = 0; EnumIndex < NumEnum; EnumIndex++)
	{
		if (const FString& EnumMetaData = Enum->GetMetaData(*KeyName, EnumIndex); EnumMetaData.Len() > 0)
		{
			FCollisionChannelInfo Info;
			Info.DisplayName = EnumMetaData;
			Info.CollisionChannel = static_cast<ECollisionChannel>(EnumIndex);
			if (Enum->GetMetaData(*TraceType, EnumIndex) == TEXT("1"))
			{
				Info.TraceType = true;
			}
			else
			{
				Info.TraceType = false;
			}

			ValidCollisionChannels.Add(Info);
		}
	}

	for (auto& ValidCollision : ValidCollisionChannels)
	{
		const FName ChannelName = *ValidCollision.DisplayName;
		ResponsesChannels.Add(FResponseChannel(ChannelName, ECR_Ignore));
	}

	const FString IniPath = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");
	const FString SectionName = TEXT("/Script/Engine.CollisionProfile");

	TArray<FString> ChannelLines;
	if (GConfig->GetArray(*SectionName, TEXT("+DefaultChannelResponses"), ChannelLines, IniPath))
	{
		for (const FString& Line : ChannelLines)
		{
			FString NameValue;
			if (Line.Split(TEXT("Name=\""), nullptr, &NameValue))
			{
				int32 EndIndex;
				if (NameValue.FindChar(TEXT('\"'), EndIndex))
				{
					FString ExtractedName = NameValue.Left(EndIndex);
					FName ChannelName(*ExtractedName);

					// Check si le channel est déjà dans la liste
					bool bAlreadyExists = ResponsesChannels.ContainsByPredicate(
						[&](const FResponseChannel& Existing)
						{
							return Existing.Channel == ChannelName;
						}
					);
					if (!bAlreadyExists)
					{
						ResponsesChannels.Add(FResponseChannel(ChannelName, ECR_Ignore));
						UE_LOG(LogTemp, Warning, TEXT("Ajouté depuis INI : %s"), *ChannelName.ToString());
					}
				}
			}
		}
	}
}

void FWarfallCoreModule::AddNewCollisionChannel(const FString& ChannelName, const ECollisionResponse Response, const bool bIsObject)
{
	ProfileNames.Add(CreateCollisionChannel(ChannelName, Response, bIsObject));
}

void FWarfallCoreModule::AddNewCollisionProfile(const FCollisionResponseTemplate& Template)
{
	ProfileNames.Add(CreateCollisionProfile(Template));
}

TPair<FString, bool> FWarfallCoreModule::CreateCollisionChannel(const FString& ChannelName, const ECollisionResponse Response, const bool bIsObject)
{
	const FString IniPath = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");
    const FString SectionHeader = TEXT("[/Script/Engine.CollisionProfile]");
    const FString TargetName = ChannelName;
    const FString DefaultResponse = StaticEnum<ECollisionResponse>()->GetNameStringByValue((int64)Response);

    TArray<FString> Lines;
    if (!FFileHelper::LoadFileToStringArray(Lines, *IniPath))
    {
        UE_LOG(LogTemp, Error, TEXT("Impossible de lire %s"), *IniPath);
        return TPair<FString, bool>(ChannelName, false);
    }

    TSet<int32> UsedIndices;
    bool bAlreadyExists = false;
    bool bInSection = false;
    int32 InsertIndex = Lines.Num();

    for (int32 i = 0; i < Lines.Num(); ++i)
    {
        FString Line = Lines[i].TrimStartAndEnd();

        if (Line.Equals(SectionHeader, ESearchCase::IgnoreCase))
        {
            bInSection = true;
            InsertIndex = i + 1;
            continue;
        }
        if (bInSection && Line.StartsWith(TEXT("[")) && Line != SectionHeader) break;

        if (bInSection && Line.StartsWith(TEXT("+DefaultChannelResponses=")))
        {
            if (Line.Contains(FString::Printf(TEXT("Name=\"%s\""), *TargetName)))
            {
                bAlreadyExists = true;
                break;
            }
        	FString ChannelText;
        	if (Line.Split(TEXT("Channel=ECC_GameTraceChannel"), nullptr, &ChannelText))
        	{
        		if (int32 CommaIndex; ChannelText.FindChar(TEXT(','), CommaIndex))
        		{
        			FString IndexStr = ChannelText.Left(CommaIndex);
        			int32 Index = FCString::Atoi(*IndexStr);
        			if (Index >= 1 && Index <= 18)
        			{
        				UsedIndices.Add(Index);
        			}
        		}
        	}
        }
    }
    if (bAlreadyExists)
    {
        UE_LOG(LogTemp, Warning, TEXT("Channel '%s' déjà présent dans le fichier INI."), *TargetName);
        return TPair<FString, bool>(ChannelName, false);
    }

    int32 AvailableIndex = 0;
    for (int32 i = 1; i <= 18; ++i)
    {
        if (!UsedIndices.Contains(i))
        {
            AvailableIndex = i;
            break;
        }
    }

    if (AvailableIndex == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Aucun ECC_GameTraceChannel disponible !"));
        return TPair<FString, bool>(ChannelName, false);
    }
    FString NewLine = FString::Printf(
        TEXT("+DefaultChannelResponses=(Channel=ECC_GameTraceChannel%d,DefaultResponse=ECR_Ignore,bTraceType=%s,bStaticObject=False,Name=\"%s\")"),
        AvailableIndex,
        !bIsObject ? TEXT("True") : TEXT("False"),
        *TargetName
    );
    if (!bInSection)
    {
        Lines.Add(TEXT(""));
        Lines.Add(SectionHeader);
        InsertIndex = Lines.Num();
    }

    Lines.Insert(NewLine, InsertIndex);
    if (FFileHelper::SaveStringArrayToFile(Lines, *IniPath))
    {
        UE_LOG(LogTemp, Warning, TEXT("New Collision Channel: %s"), *NewLine);
        return TPair<FString, bool>(ChannelName, true);
    }

    return TPair<FString, bool>(ChannelName, false);
}

TPair<FString, bool> FWarfallCoreModule::CreateCollisionProfile(const FCollisionResponseTemplate& Template)
{
	const FString IniPath = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");
    const FString SectionHeader = TEXT("[/Script/Engine.CollisionProfile]");
    
    const FString Name = Template.Name.ToString();
    const FString CollisionEnabled = StaticEnum<ECollisionEnabled::Type>()->GetNameStringByValue((int64)Template.CollisionEnabled.GetValue());
    const FString bCanBeModify = Template.bCanModify ? TEXT("true") : TEXT("false");
    const FString ObjectTypeName = Template.ObjectTypeName.ToString();
    FString CustomResponses;
    for (int32 i = 0; i < Template.CustomResponses.Num(); i++)
    {
        FResponseChannel Response = Template.CustomResponses[i];
        FString DefaultResponse = StaticEnum<ECollisionResponse>()->GetNameStringByValue((int64)Response.Response);
        CustomResponses += FString::Printf(TEXT("(Channel=\"%s\",Response=%s)%s"), 
                                     *Response.Channel.ToString(), 
                                     *DefaultResponse, i < Template.CustomResponses.Num() - 1 ? TEXT(",") : TEXT(""));
    }
    const FString HelpMessage = Template.HelpMessage;

    TArray<FString> Lines;
    if (!FFileHelper::LoadFileToStringArray(Lines, *IniPath))
    {
        UE_LOG(LogTemp, Error, TEXT("Impossible de lire %s"), *IniPath);
        return TPair<FString, bool>(Name, false);
    }

    TSet<int32> UsedIndices;
    bool bAlreadyExists = false;
    bool bInSection = false;
    int32 InsertIndex = Lines.Num();

    for (int32 i = 0; i < Lines.Num(); ++i)
    {
        FString Line = Lines[i].TrimStartAndEnd();

        if (Line.Equals(SectionHeader, ESearchCase::IgnoreCase))
        {
            bInSection = true;
            InsertIndex = i + 1;
            continue;
        }
        if (bInSection && Line.StartsWith(TEXT("[")) && Line != SectionHeader)
        {
            break;
        }
        if (bInSection && Line.StartsWith(TEXT("+Profiles=")))
        {
            if (Line.Contains(FString::Printf(TEXT("Name=\"%s\""), *Name)))
            {
                bAlreadyExists = true;
                break;
            }
        }
    }

    if (bAlreadyExists)
    {
        UE_LOG(LogTemp, Warning, TEXT("Profile '%s' déjà présent dans le fichier INI."), *Name);
        return TPair<FString, bool>(Name, false);
    }

    FString NewLine = FString::Printf(
        TEXT("+Profiles=(Name=\"%s\",CollisionEnabled=%s,bCanModify=%s,ObjectTypeName=\"%s\",CustomResponses=(%s),HelpMessage=\"%s\")"),
        *Name,
        *CollisionEnabled,
        *bCanBeModify,
        *ObjectTypeName,
        *CustomResponses,
        *HelpMessage
    );
	
    if (!bInSection)
    {
        Lines.Add(TEXT(""));
        Lines.Add(SectionHeader);
        InsertIndex = Lines.Num();
    }

    Lines.Insert(NewLine, InsertIndex);
    if (FFileHelper::SaveStringArrayToFile(Lines, *IniPath))
    {
        UE_LOG(LogTemp, Warning, TEXT("New Collision Profile: %s"), *NewLine);
        return TPair<FString, bool>(Name, true);
    }

    return TPair<FString, bool>(Name, false);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FWarfallCoreModule, WarfallCore)