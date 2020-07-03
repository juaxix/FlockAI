// Flock AI - Steering Behaviors for Unreal - juaxix
#pragma once

#include <CoreMinimal.h>
#include <UObject/Object.h>

#include "Boid.generated.h"

UCLASS(BlueprintType, Blueprintable)
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
	void CorrectDirectionAgainstCollision(FVector& Direction);
public:
	/* The weight of the Alignment vector component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	float AlignmentWeight;

	/* The weight of the Cohesion vector component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	float CohesionWeight;

	/* The damping of the cohesion force after sum */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	float CohesionLerp;

	/* The weight of the Collision vector component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	float CollisionWeight;

	float SeparationLerp;
	float SeparationForce;
	float StimuliLerp;

	/* The weight of the Separation vector component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	float SeparationWeight;

	/* The base movement speed for the Agents */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	float BaseMovementSpeed;

	/* The maximum movement speed the Agents can have */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	float MaxMovementSpeed;

	/* The maximum radius at which the Agent can detect other Agents */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	float VisionRadius;

	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	float CollisionDistanceLook;

	/* Speed to look at direction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	float MaxRotationSpeed;

	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	int32 MeshIndex;

	UPROPERTY(EditAnywhere , BlueprintReadOnly, Category = "AI|Steering Behavior Component")
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

	UPROPERTY(VisibleAnywhere , BlueprintReadOnly, Category = "AI|Steering Behavior Component")
	TArray<class AActor*> ActorsInVision;

	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component", meta = (Tooltip=
		"If enable components forces will be visible"))
	bool bEnableDebugDraw;

	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component", meta = (ClampMin=0.1f,
		ClampMax=10.0f))
	float DebugRayDuration;

	const float DefaultNormalizeVectorTolerance = 0.0001f;

protected:
	/* The movement vector (in local) this agent should move this tick. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	FVector m_NewMoveVector;

	/* The movement vector (in local) this agent had last tick. */
	UPROPERTY(EditAnywhere , BlueprintReadOnly, Category = "AI|Steering Behavior Component")
	FVector m_CurrentMoveVector;

	//2 * PhysicalRadius
	float m_Boid2PhysicalRadius;
};
