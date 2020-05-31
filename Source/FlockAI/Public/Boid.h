// Flock AI - Steering Behaviors for Unreal - juaxix
#pragma once

#include <CoreMinimal.h>
#include <UObject/Object.h>

#include "Boid.generated.h"

UCLASS(BlueprintType)
class FLOCKAI_API UBoid : public UObject
{
	GENERATED_BODY()

public:
	UBoid();

	void ResetComponents();

	void Init(const FVector& Location, const FRotator& Rotation, int32 MeshInstanceIndex);

	void Update(float DeltaSeconds);

	void DebugDraw() const;

protected:
	void CalculateNewMoveVector();
	void CalculateAlignmentComponentVector();
	void CalculateCohesionComponentVector();
	bool CheckStimulusVision();
	void CalculateSeparationComponentVector();
	void ComputeStimuliComponentVector();
	void CalculateNegativeStimuliComponentVector(class AStimulus* Stimulus);
	void CalculatePositiveStimuliComponentVector(class AStimulus* Stimulus);
	void ComputeAggregationOfComponents();

public:
	/* The weight of the Alignment vector component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
	float AlignmentWeight;

	/* The weight of the Cohesion vector component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
	float CohesionWeight;

	/* The damping of the cohesion force after sum */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
	float CohesionLerp;

	float SeparationLerp;
	float SeparationForce;
	float StimuliLerp;
	/* The weight of the Separation vector component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
	float SeparationWeight;

	/* The base movement speed for the Agents */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
	float BaseMovementSpeed;

	/* The maximum movement speed the Agents can have */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
	float MaxMovementSpeed;

	/* The maximum radius at which the Agent can detect other Agents */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
	float VisionRadius;

	/* Speed to look at direction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
	float MaxRotationSpeed;

	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = AI)
	int32 MeshIndex;

	UPROPERTY(EditAnywhere , BlueprintReadOnly, Category = AI)
	FTransform Transform;

	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	FVector AlignmentComponent;

	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	FVector CohesionComponent;

	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	FVector SeparationComponent;

	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	FVector NegativeStimuliComponent;

	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	FVector PositiveStimuliComponent;

	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	float NegativeStimuliMaxFactor;

	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	float PositiveStimuliMaxFactor;

	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	float InertiaWeigh;

	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	float BoidPhysicalRadius;

	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	TArray<UBoid*> Neighbourhood;

	UPROPERTY(VisibleAnywhere , BlueprintReadOnly, Category = "AI")
	TArray<class AActor*> ActorsInVision;

	const float DefaultNormalizeVectorTolerance = 0.0001f;
protected:
	/* The movement vector (in local) this agent should move this tick. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
	FVector m_NewMoveVector;

	/* The movement vector (in local) this agent had last tick. */
	UPROPERTY(EditAnywhere , BlueprintReadOnly, Category = AI)
	FVector m_CurrentMoveVector;

	float m_Boid2PhysicalRadius;
};
