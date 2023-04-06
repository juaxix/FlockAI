// Flock AI - Steering Behaviors for Unreal - juaxix

#include "Boid.h"

#include "Agent.h"
#include "Stimulus.h"
#include "Engine/EngineTypes.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"

namespace
{
	constexpr const float MAX_TRACE_Z_FIND = 1000.0f;
}

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
	, AlignmentComponent(0.0f, 0.0f, 0.0f)
	, CohesionComponent(0.0f, 0.0f, 0.0f)
	, SeparationComponent(0.0f, 0.0f, 0.0f)
	, NegativeStimuliComponent(0.0f, 0.0f, 0.0f)
	, PositiveStimuliComponent(0.0f, 0.0f, 0.0f)
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

	FVector NewDirection = (NewMoveVector * BaseMovementSpeed * DeltaSeconds).GetClampedToMaxSize(MaxMovementSpeed * DeltaSeconds);
	if (CollisionWeight != 0.0f)
	{
		CorrectDirectionAgainstCollision(NewDirection, PI*10.0f);
	}
	FVector Location = Transform.GetLocation() + NewDirection;
	if (bFollowFloorZ)
	{
		FindGroundPosition(Location, MAX_TRACE_Z_FIND, GetWorld(), ECC_WorldStatic, 4.0f);
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
				  FColor::Red, false, DebugRayDuration, 0, 1.0f);

	DrawDebugLine(World, Location,
				  Location + CohesionComponent * CohesionWeight * 100.0f,
				  FColor::Orange, false, DebugRayDuration, 0, 1.0f);

	DrawDebugLine(World, Location,
				  Location + AlignmentComponent * AlignmentWeight * 100.0f,
				  FColor::Purple, false, DebugRayDuration, 0, 1.0f);

	DrawDebugLine(World, Location,
				  Location + (SeparationComponent * SeparationWeight * 100.0f),
				  FColor::Blue, false, DebugRayDuration, 0, 1.0f);
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

	ComputeStimuliComponentVector();
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

	AlignmentComponent = (CurrentMoveVector + AlignmentComponent).GetSafeNormal(DefaultNormalizeVectorTolerance);
}

void UBoid::CalculateCohesionComponentVector()
{
	const FVector& Location = Transform.GetLocation();
	for (const UBoid* Boid : Neighbourhood)
	{
		CohesionComponent += Boid->Transform.GetLocation() - Location;
	}

	CohesionComponent = CohesionComponent / Neighbourhood.Num() / CohesionLerp;
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
	SeparationComponent += SeparationForceComponent + SeparationForceComponent * (SeparationLerp / Neighbourhood.Num());
}

void UBoid::ComputeStimuliComponentVector()
{
	CheckStimulusVision();
	const FVector& Location = Transform.GetLocation();
	for (AActor* Actor : ActorsInVision)
	{
		AStimulus* Stimulus = Cast<AStimulus>(Actor);
		if (!IsValid(Stimulus))
		{
			continue;
		}

		if (Stimulus->Value < 0.0f)
		{
			CalculateNegativeStimuliComponentVector(Stimulus);
		}
		else
		{
			if (FVector::Dist(Stimulus->GetActorLocation(), Location) <= Boid2PhysicalRadius)
			{
				Stimulus->Consume(this);
			}
			else
			{
				CalculatePositiveStimuliComponentVector(Stimulus);
			}
		}
	}

	NegativeStimuliComponent = NegativeStimuliMaxFactor * NegativeStimuliComponent.GetSafeNormal(DefaultNormalizeVectorTolerance);
}

void UBoid::CalculateNegativeStimuliComponentVector(AStimulus* Stimulus)
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

void UBoid::CalculatePositiveStimuliComponentVector(AStimulus* Stimulus)
{
	check(Stimulus);
	const FVector Direction = Stimulus->GetActorLocation() - Transform.GetLocation();
	const float Svalue = Stimulus->Value / Direction.Size();
	if (Svalue > PositiveStimuliMaxFactor)
	{
		PositiveStimuliMaxFactor = Svalue;
		PositiveStimuliComponent += Stimulus->Value *
			Direction.GetSafeNormal(DefaultNormalizeVectorTolerance);
	}
}

void UBoid::ComputeAggregationOfComponents()
{
	NewMoveVector = (AlignmentComponent * AlignmentWeight)
		+ (CohesionComponent * CohesionWeight)
		+ (SeparationComponent * SeparationWeight)
		+ NegativeStimuliComponent
		+ PositiveStimuliComponent;
}

void UBoid::CorrectDirectionAgainstCollision(FVector& Direction, double DeviationHitAngle) const
{
	const FVector& Location = Transform.GetLocation();
	static TArray<AActor*> IgnoreActor = {AAgent::Instance};
	FHitResult OutHit(1.0f);
	static const FName LineTraceSingleName(TEXT("LineTraceSingle"));
	FVector End = Location + Direction.GetSafeNormal(DefaultNormalizeVectorTolerance) * CollisionDistanceLook;
	FCollisionQueryParams Params(LineTraceSingleName, false, AAgent::Instance);
	
	if (GetWorld()->LineTraceSingleByChannel(OutHit, Location, End,ECC_WorldStatic, Params))
	{
		if (OutHit.IsValidBlockingHit())
		{
			const FVector Normal2D = OutHit.Normal.GetSafeNormal2D();
			Direction = FVector::VectorPlaneProject(Direction, Normal2D) * (1.f - OutHit.Time)* CollisionWeight;
			
		}
	}
#if ENABLE_DRAW_DEBUG
	UKismetSystemLibrary::DrawDebugLine(GetWorld(), Location, End, FColor::Red, 4.0f);
#endif
}

void UBoid::FindGroundPosition(FVector& Position, float TraceDistance, UWorld* World, ECollisionChannel CollisionChannel, float DrawDebugDuration)
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
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(AAgent::Instance);
	bool bHit = World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, CollisionChannel, TraceParams);

	if (DrawDebugDuration > 0.0f)
	{
		DrawDebugLine(World, Position, TraceEnd, FColor::Red, false, DrawDebugDuration);
		DrawDebugPoint(World, HitResult.ImpactPoint, 10.0f, FColor::Yellow, false, DrawDebugDuration);
	}

	if (bHit)
	{
		Position = HitResult.ImpactPoint;
	}
}
