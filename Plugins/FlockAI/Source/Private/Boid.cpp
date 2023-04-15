// Flock AI - Steering Behaviors for Unreal - juaxix

#include "Boid.h"

#include "Agent.h"
#include "Stimulus.h"
#include "Engine/EngineTypes.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"

UBoid::UBoid()
	: AlignmentWeight(1.0f)
	, CohesionWeight(1.0f)
	, CohesionLerp(100.0f)
	, CollisionWeight(1.0f)
	, SeparationLerp(5.0f)
	, SeparationForce(100.0f)
	, StimuliLerp(100.0f)
	, SeparationWeight(0.8f)
	, BaseMovementSpeed(150.0f)
	, MaxMovementSpeed(250.0f)
	, VisionRadius(400.0f)
	, CollisionDistanceLook(400.0f)
	, MaxRotationSpeed(6.0f)
	, MeshIndex(0)
	, Transform(FQuat::Identity, FVector::ZeroVector, FVector::OneVector)
	, AlignmentComponent(0.0)
	, CohesionComponent(0.0)
	, SeparationComponent(0.0)
	, NegativeStimuliComponent(0.0)
	, PositiveStimuliComponent(0.0)
	, CollisionComponent(0.0)
	, NegativeStimuliMaxFactor(0.0f)
	, PositiveStimuliMaxFactor(0.0f)
	, InertiaWeigh(0.0f)
	, BoidPhysicalRadius(45.0f)
	, bEnableDebugDraw(false)
	, DebugRayDuration(0.12f)
{
	Boid2PhysicalRadius = 2 * BoidPhysicalRadius;
}

void UBoid::ResetComponents()
{
	AlignmentComponent = FVector::ZeroVector;
	CohesionComponent = FVector::ZeroVector;
	SeparationComponent = FVector::ZeroVector;
	NegativeStimuliComponent = FVector::ZeroVector;
	PositiveStimuliComponent = FVector::ZeroVector;
	CollisionComponent = FVector::ZeroVector;
	NegativeStimuliMaxFactor = 0.0f;
	PositiveStimuliMaxFactor = 0.0f;
}

void UBoid::Init(const FVector& Location, const FRotator& Rotation, int32 MeshInstanceIndex)
{
	Transform.SetRotation(Rotation.Quaternion());
	Transform.SetLocation(Location);
	MeshIndex = MeshInstanceIndex;
	// Initialize move vector
	NewMoveVector = Rotation.Vector().GetSafeNormal();
}


void UBoid::Update(float DeltaSeconds)
{
	CurrentMoveVector = NewMoveVector;

	CalculateNewMoveVector();

	const FVector NewDirection = (NewMoveVector * BaseMovementSpeed * DeltaSeconds).GetClampedToMaxSize(MaxMovementSpeed * DeltaSeconds);
	FVector Location = Transform.GetLocation() + NewDirection;
	if (bFollowFloorZ)
	{
		FindGroundPosition(Location, MaxFloorDistance, GetWorld(), ECC_WorldStatic, 0.0f);
	}
	
	Transform.SetLocation(Location);
	Transform.SetRotation(FMath::Lerp(Transform.Rotator().Quaternion(),
									  UKismetMathLibrary::MakeRotFromXZ(NewDirection, FVector::UpVector)
									  .Quaternion(), DeltaSeconds * MaxRotationSpeed));
}

void UBoid::DebugDraw() const
{
	const UWorld* World = GetWorld();
	const FVector& Location = Transform.GetLocation();
	DrawDebugLine(World, Location,
				  Location + CurrentMoveVector * 300.0f,
				  FColor::Green, false, DebugRayDuration, 0, 1.0f);

	DrawDebugLine(World, Location,
				  Location + CohesionComponent * CohesionWeight * 100.0f,
				  FColor::Orange, false, DebugRayDuration, 0, 1.0f);

	DrawDebugLine(World, Location,
				  Location + AlignmentComponent * AlignmentWeight * 100.0f,
				  FColor::Purple, false, DebugRayDuration, 0, 1.0f);

	DrawDebugLine(World, Location,
				  Location + (SeparationComponent * SeparationWeight * 100.0f),
				  FColor::Blue, false, DebugRayDuration, 0, 1.0f);
	if (CollisionWeight > 0.0f)
	{
		DrawDebugLine(World, Location, 
				  Location + CollisionComponent * CollisionWeight  * 100.0f,
				  FColor::Red, false, DebugRayDuration, 0, 1.0f);
	}
}

void UBoid::CalculateNewMoveVector()
{
	ResetComponents();
	CalculateAlignmentComponentVector();

	if (Neighbourhood.Num() > 0)
	{
		CalculateCohesionComponentVector();
		CalculateSeparationComponentVector();
	}

	ComputeAllStimuliComponentVector();

	if (CollisionWeight != 0.0f)
	{
		CalculateCollisionComponentVector();
	}

	ComputeAggregationOfComponents();
	
	if (bEnableDebugDraw)
	{
		DebugDraw();
	}
}

void UBoid::CalculateAlignmentComponentVector()
{
	for (const UBoid* Boid : Neighbourhood)
	{
		AlignmentComponent += Boid->CurrentMoveVector.GetSafeNormal(DefaultNormalizeVectorTolerance);
	}

	AlignmentComponent = (CurrentMoveVector + AlignmentComponent).GetSafeNormal(DefaultNormalizeVectorTolerance) * AlignmentWeight;
}

void UBoid::CalculateCohesionComponentVector()
{
	const FVector& Location = Transform.GetLocation();
	for (const UBoid* Boid : Neighbourhood)
	{
		CohesionComponent += Boid->Transform.GetLocation() - Location;
	}

	CohesionComponent = (CohesionComponent / Neighbourhood.Num() / CohesionLerp) * CohesionWeight;
}

bool UBoid::CheckStimulusVision()
{
	static TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes{{UEngineTypes::ConvertToObjectType(ECC_Destructible)}};
	return UKismetSystemLibrary::SphereOverlapActors(
		this, Transform.GetLocation(),
		VisionRadius,
		ObjectTypes,
		AStimulus::StaticClass(),
		TArray<AActor*>(),
		ActorsInVision);
}

void UBoid::CalculateSeparationComponentVector()
{
	const FVector& Location = Transform.GetLocation();

	for (const UBoid* Boid : Neighbourhood)
	{
		FVector Separation = Location - Boid->Transform.GetLocation();
		SeparationComponent += Separation.GetSafeNormal(DefaultNormalizeVectorTolerance)
			/ FMath::Abs(Separation.Size() - BoidPhysicalRadius);
	}

	const FVector SeparationForceComponent = SeparationComponent * SeparationForce;
	SeparationComponent += (SeparationForceComponent + SeparationForceComponent * (SeparationLerp / Neighbourhood.Num())) * SeparationWeight;
}

void UBoid::ComputeAllStimuliComponentVector()
{
	CheckStimulusVision();
	const FVector& Location = Transform.GetLocation();
	for (AActor* Actor : ActorsInVision)
	{
		ComputeStimuliComponentVector(Cast<AStimulus>(Actor), Location);
	}

	for (AStimulus* Stimulus : AAgent::Instance->GetGlobalStimulus())
	{
		ComputeStimuliComponentVector(Stimulus, Location);
	}

	NegativeStimuliComponent = NegativeStimuliMaxFactor * NegativeStimuliComponent.GetSafeNormal(DefaultNormalizeVectorTolerance);
}

void UBoid::ComputeStimuliComponentVector(AStimulus* Stimulus, const FVector& Location, bool bIsGlobal)
{
	if (!IsValid(Stimulus))
	{
		return;
	}

	if (Stimulus->Value < 0.0f)
	{
		CalculateNegativeStimuliComponentVector(Stimulus, bIsGlobal);
	}
	else
	{
		if (FVector::Dist(Stimulus->GetActorLocation(), Location) <= Boid2PhysicalRadius)
		{
			Stimulus->Consume(this);
		}
		else
		{
			CalculatePositiveStimuliComponentVector(Stimulus, bIsGlobal);
		}
	}
}

void UBoid::CalculateNegativeStimuliComponentVector(const AStimulus* Stimulus, bool bIsGlobal)
{
	check(Stimulus);
	const FVector Direction = Stimulus->GetActorLocation() - Transform.GetLocation();
	const FVector NegativeStimuliComponentForce =
		(Direction.GetSafeNormal(DefaultNormalizeVectorTolerance)
			/ FMath::Abs(Direction.Size() - BoidPhysicalRadius))
		* StimuliLerp * Stimulus->Value;
	NegativeStimuliComponent += NegativeStimuliComponentForce;
	NegativeStimuliMaxFactor = FMath::Max(NegativeStimuliComponentForce.Size(), NegativeStimuliMaxFactor);
}

void UBoid::CalculatePositiveStimuliComponentVector(const AStimulus* Stimulus, bool bIsGlobal)
{
	check(Stimulus);
	const FVector Direction = Stimulus->GetActorLocation() - Transform.GetLocation();
	const float Svalue = bIsGlobal ? Stimulus->Value : Stimulus->Value / Direction.Size();
	if (Svalue > PositiveStimuliMaxFactor)
	{
		PositiveStimuliMaxFactor = Svalue;
		PositiveStimuliComponent += Stimulus->Value * Direction.GetSafeNormal(DefaultNormalizeVectorTolerance);
	}
}

void UBoid::CalculateCollisionComponentVector()
{
	FHitResult OutHit;
	const FVector& Location = Transform.GetLocation();
	static const FName LineTraceSingleName(TEXT("LineTraceSingle"));
	const FVector End = Location + Transform.GetRotation().GetForwardVector() * CollisionDistanceLook;
	const FCollisionQueryParams Params(LineTraceSingleName, false, AAgent::Instance);
	const static FCollisionShape SphereShape = FCollisionShape::MakeSphere(BoidPhysicalRadius);
	
	if (GetWorld()->SweepSingleByChannel(OutHit, Location, End, FQuat::Identity, ECC_WorldStatic, SphereShape, Params))
	{
		const FVector Direction = OutHit.ImpactPoint - Transform.GetLocation();
		CollisionComponent -= (Direction.GetSafeNormal(DefaultNormalizeVectorTolerance) / FMath::Abs(Direction.Size() - BoidPhysicalRadius))
							  .RotateAngleAxis(CollisionDeviationHitAngle, FVector::UpVector) * CollisionWeight;

#if ENABLE_DRAW_DEBUG
	if (bEnableDebugDraw)
	{
		UKismetSystemLibrary::DrawDebugArrow(GetWorld(), Location, OutHit.ImpactPoint,  Boid2PhysicalRadius, FColor::Red, 4.0f);
	}
#endif
	}
}

void UBoid::ComputeAggregationOfComponents()
{
	NewMoveVector = AlignmentComponent
		+ CohesionComponent
		+ SeparationComponent
		+ NegativeStimuliComponent
		+ PositiveStimuliComponent
		+ CollisionComponent;

	if (bFollowFloorZ)
	{
		NewMoveVector.Z = 0.0;
	}
}

void UBoid::FindGroundPosition(FVector& Position, float TraceDistance, UWorld* World, ECollisionChannel CollisionChannel, float DrawDebugDuration, float HeightOffSet)
{
	if (!World)
	{
		return;
	}

	FVector TraceEnd = Position;
	FVector TraceStart = Position;
	TraceStart.Z += TraceDistance;
	TraceEnd.Z -= TraceDistance;
	FHitResult HitResult;
	static const FName LineTraceSingleName(TEXT("LineTraceSingle"));
	FCollisionQueryParams TraceParams(LineTraceSingleName, false, AAgent::Instance);
	const bool bHit = World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, CollisionChannel, TraceParams);
	if (bHit)
	{
		Position = HitResult.ImpactPoint;
		Position.Z += HeightOffSet;
	}

	if (DrawDebugDuration > 0.0f)
	{
		UKismetSystemLibrary::DrawDebugArrow(World, TraceStart, bHit ? Position : TraceEnd, 25.0f, FColor::Red, DrawDebugDuration);
	}
}
