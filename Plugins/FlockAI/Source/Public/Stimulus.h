// Flock AI - Steering Behaviors for Unreal - juaxix

#pragma once

#include "GameFramework/Actor.h"
#include "Stimulus.generated.h"

UCLASS(abstract)
class FLOCKAI_API AStimulus : public AActor
{
	GENERATED_BODY()

public:
	AStimulus() = default;

	UFUNCTION(BlueprintNativeEvent, Category = AI)
	void Consume(class UBoid* Boid);
	void Consume_Implementation(UBoid* Boid);

	// The value of the stimulus for the Agents.
	UPROPERTY(Category = AI, EditAnywhere, BlueprintReadWrite)
	float Value = 0.0f;
};
