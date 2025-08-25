#pragma once

#include "Modules/ModuleManager.h"
#include "Engine/EngineTypes.h"
#include "Engine/CollisionProfile.h"
#include "Editor/DetailCustomizations/Private/BodyInstanceCustomization.h"

struct FChannels;

class FWarfallCoreModule : public IModuleInterface
{
public:
	// ========== FUNCTIONS ==========
	
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/**
	 * Provides access to the FGameCoreModule instance. Ensures the module is loaded and ready for use.
	 * @return A reference to the FGameCoreModule instance.
	 */
	static FWarfallCoreModule& Get()
	{
		return FModuleManager::GetModuleChecked<FWarfallCoreModule>("GameCore");
	}

	/**
	 * Retrieves all response channels.
	 * @return A constant reference to the array of FResponseChannel objects.
	 */
	const TArray<FResponseChannel>& GetAllResponseChannels() const { return ResponsesChannels; }

	/**
	 * Retrieves the collision channel associated with the specified channel name.
	 *
	 * @param ChannelName The name of the channel to retrieve the collision channel for.
	 * @return The corresponding collision channel if found; otherwise, returns ECC_Visibility.
	 */
	ECollisionChannel GetCollisionChannel(const FString& ChannelName) const
	{
		for (auto& ChannelInfo : ValidCollisionChannels)
		{
			if (ChannelInfo.DisplayName == ChannelName)
			{
				return ChannelInfo.CollisionChannel;
			}
		}
		return ECC_Visibility;
	}

private:
	
	/**
	 * Initializes and launches custom systems required for the module.
	 */
	static void LaunchCustomSystems();
	/**
	 * Shuts down and cleans up custom systems utilized by the module.
	 * This function ensures that any resources or processes launched by
	 * LaunchCustomSystems are properly terminated or released.
	 */
	static void ShutdownCustomSystems();
	
		// ========== COLLISIONS ==========

	/**
	 * Initializes and launches the systems required for handling collision behaviors.
	 * This function sets up and starts the necessary infrastructure to manage
	 * collision channels and profiles within the module.
	 */
	void LaunchCollisionsSystems();
	/**
	 * Restarts the module's systems to accommodate new collision channels.
	 * This function ensures that any updated or newly added collision
	 * channels are properly initialized and integrated into the module's workflow.
	 */
	void RestartForNewChannels();
	/**
	 * Configures and sets up the necessary collision channels for the module.
	 * This function initializes the collision channels that will be used
	 * throughout the system, ensuring they are properly defined and ready
	 * for interaction.
	 */
	void SetupCollisionChannels();
	/**
	 * Adds a new collision channel to the system with specified response and type.
	 *
	 * @param ChannelName The name of the new collision channel to be added.
	 * @param Response The collision response associated with the new channel.
	 * @param bIsObject Indicates whether the channel is an object channel (true) or a trace channel (false). Default is false.
	 */
	void AddNewCollisionChannel(const FString& ChannelName, const ECollisionResponse Response, const bool bIsObject = false);
	static TPair<FString, bool> CreateCollisionChannel(const FString& ChannelName, const ECollisionResponse Response, const bool bIsObject = false);
	/**
	 * Adds a new collision profile to the system using the provided collision response template.
	 *
	 * @param Template The collision response template that defines the settings for the new collision profile.
	 */
	void AddNewCollisionProfile(const FCollisionResponseTemplate& Template);
	/**
	 * Creates a new collision profile based on the provided collision response template.
	 *
	 * @param Template The collision response template used to define the settings for the new collision profile.
	 * @return A pair consisting of the unique name of the created collision profile as a string and a boolean value indicating success (true if the profile was successfully created, false otherwise).
	 */
	static TPair<FString, bool> CreateCollisionProfile(const FCollisionResponseTemplate& Template);
	/**
	 * Creates a new collision response template with the specified parameters.
	 *
	 * @param Name The name of the collision template to create.
	 * @param ObjectTypeName The name of the object type associated with the collision template.
	 * @param CollisionEnabled Specifies whether and how collision is enabled (e.g., query only, physics only, or both).
	 * @param Collisions The configuration of response channels for the collision template.
	 * @param HelpMessage A help message or description for the collision template.
	 * @return A configured FCollisionResponseTemplate object based on the provided parameters.
	 */
	static FCollisionResponseTemplate CreateNewCollisionTemplate(const FName& Name, const FName& ObjectTypeName, const TEnumAsByte<ECollisionEnabled::Type> CollisionEnabled, FChannels Collisions, FString HelpMessage);


	
public:
	FString RestartLog;
	
private:
	TArray<FCollisionChannelInfo>	ValidCollisionChannels;
	TArray<FResponseChannel>		ResponsesChannels;
	TMap<FString, bool>				ProfileNames;
};


/**
 * Represents a collection of response channels and provides
 * methods to manipulate their collision responses.
 */
struct FChannels
{
private:
	TArray<FResponseChannel> Channels;

public:
	FChannels() : Channels()
	{
		if (!FModuleManager::Get().IsModuleLoaded("GameCore")) return;
		Channels = FWarfallCoreModule::Get().GetAllResponseChannels();
	}

	TArray<FResponseChannel>& GetChannels() { return Channels; }

	/**
	 * Sets the specified collision response for all the channels in the collection.
	 *
	 * @param Response The collision response to set for all channels.
	 */
	void ResponsesToAllChannels(const ECollisionResponse Response)
	{
		for (auto& Channel : Channels)
		{
			Channel.Response = Response;
		}
	}

	/**
	 * Updates the collision response for a specific channel within the list of channels.
	 *
	 * @param ChannelName The name of the channel to update.
	 * @param Response The collision response value to assign to the specified channel.
	 */
	void ResponseToChannel(const FName& ChannelName, const ECollisionResponse Response)
	{
		for (auto& Channel : Channels)
		{
			if (Channel.Channel == ChannelName)
			{
				Channel.Response = Response;
				return;
			}
		}
	}
};