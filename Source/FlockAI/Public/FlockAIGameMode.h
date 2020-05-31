// Flock AI - Steering Behaviors for Unreal - juaxix
#pragma once

#include "GameFramework/GameMode.h"
#include "FlockAIGameMode.generated.h"


UCLASS(minimalapi)
class AFlockAIGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AFlockAIGameMode(const FObjectInitializer& ObjectInitializer);
};
