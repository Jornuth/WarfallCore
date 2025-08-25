#include "Core/Player/ProgressionComponent.h"

void UProgressionComponent::AddNewDiscovery(const FDataTableRowHandle& NewDiscovery)
{
	if (HasDiscovery(NewDiscovery)) return;

	Discovery.AddUnique(NewDiscovery);
	OnNewDiscovery.Broadcast(NewDiscovery);
}

void UProgressionComponent::RemoveDiscovery(const FDataTableRowHandle& DiscoveryToRemove)
{
	if (!HasDiscovery(DiscoveryToRemove)) return;

	Discovery.Remove(DiscoveryToRemove);
	OnForgetDiscovery.Broadcast(DiscoveryToRemove);
}

void UProgressionComponent::ClearDiscoveries()
{
	Discovery.Empty();
	ClearRecipes();
}

bool UProgressionComponent::HasDiscovery(const FDataTableRowHandle& DiscoveryToCheck) const
{
	return Discovery.Contains(DiscoveryToCheck);
}

void UProgressionComponent::LearnRecipe(const FDataTableRowHandle& Recipe)
{
	if (HasRecipe(Recipe)) return;
	OnLearnNewRecipe.Broadcast(Recipe);
}

void UProgressionComponent::ForgetRecipe(const FDataTableRowHandle& Recipe)
{
	if (!HasRecipe(Recipe)) return;
	ForgetRecipe(Recipe);
	OnForgetRecipe.Broadcast(Recipe);
}

bool UProgressionComponent::HasRecipe(const FDataTableRowHandle& Recipe) const
{
	return KnownRecipes.Contains(Recipe);
}

void UProgressionComponent::ClearRecipes()
{
	KnownRecipes.Empty();
}

