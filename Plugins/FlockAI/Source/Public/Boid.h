// Flock AI - Steering Behaviors for Unreal - juaxix
#pragma once

#include <CoreMinimal.h>
#include <UObject/Object.h>

#include "Boid.generated.h"

class AStimulus;

UCLASS(BlueprintType, Blueprintable)
class FLOCKAI_API UBoid : public UObject
{
	GENERATED_BODY()

public:
	UBoid();

	void ResetComponents();

	void Init(const FVector& Location, const FRotator& Rotation, int32 MeshInstanceIndex);

	void Update(float DeltaSeconds, AAgent* Agent);
#if UE_ENABLE_DEBUG_DRAWING
	void DebugDraw() const;
#endif

	UFUNCTION(BlueprintCallable, Category = "AI")
	void AddPrivateGlobalStimulus(AStimulus* Stimulus);

	UFUNCTION(BlueprintCallable, Category = "AI")
	void RemovePrivateGlobalStimulus(AStimulus* Stimulus);

protected:
	void CalculateNewMoveVector(AAgent* Agent);
	void CalculateAlignmentComponentVector();
	void CalculateCohesionComponentVector();
	bool CheckStimulusVision();
	void CalculateSeparationComponentVector();
	void ComputeAllStimuliComponentVector(AAgent* Agent);
	void ComputeStimuliComponentVector(AAgent* Agent, AStimulus *Stimulus, const FVector& Location, bool bIsGlobal = false);
	void CalculateNegativeStimuliComponentVector(const AStimulus* Stimulus, bool bIsGlobal = false);
	void CalculatePositiveStimuliComponentVector(const AStimulus* Stimulus, bool bIsGlobal = false);
	void CalculateCollisionComponentVector(AAgent* Agent);
	void ComputeAggregationOfComponents();
	void FindGroundLocation(AAgent* Agent, float TraceDistance, ECollisionChannel CollisionChannel = ECC_WorldStatic, float HeightOffSet = 35.0f);
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	double CollisionDeviationHitAngle = PI * 10.0;

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
	FVector CollisionComponent;

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
	TArray<class AActor*> StimulusInVision;

	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component", meta = (Tooltip = "If enabled, set boid in the floor with a trace"))
	bool bFollowFloorZ = true;

	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component", meta = (Tooltip = "If enabled, set boid in the floor with a trace", EditCondition = "bFollowFloorZ"))
	float MaxFloorDistance = 1000.0f;

	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	float FloorHeightOffset = 23.0f;
	
	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component", meta = (Tooltip = "If enabled, components forces will be visible"))
	bool bEnableDebugDraw;

	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component", meta = (ClampMin=0.1f, ClampMax=10.0f))
	float DebugRayDuration = 4.0f;

	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component", meta = (ClampMin=0.1f, ClampMax=10.0f))
	float FloorRayDuration = 0.0f;


	const float DefaultNormalizeVectorTolerance = 0.0001f;

protected:
	/* The movement vector (in local) this agent should move this tick. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	FVector NewMoveVector;

	/* The movement vector (in local) this agent had last tick. */
	UPROPERTY(EditAnywhere , BlueprintReadOnly, Category = "AI|Steering Behavior Component")
	FVector CurrentMoveVector;

	// 2 * PhysicalRadius
	float Boid2PhysicalRadius;

	TArray<AStimulus*> PrivateGlobalStimulus;

	TSet<AStimulus*> ComputedStimulus;
};
