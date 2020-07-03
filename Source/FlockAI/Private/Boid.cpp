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
	m_Boid2PhysicalRadius = 2 * BoidPhysicalRadius;
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
	m_NewMoveVector = Rotation.Vector().GetSafeNormal();
}


void UBoid::Update(float DeltaSeconds)
{
	m_CurrentMoveVector = m_NewMoveVector;

	CalculateNewMoveVector();

	FVector NewDirection = (m_NewMoveVector * BaseMovementSpeed * DeltaSeconds).
		GetClampedToMaxSize(MaxMovementSpeed * DeltaSeconds);

	CorrectDirectionAgainstCollision(NewDirection);

	Transform.SetLocation(Transform.GetLocation() + NewDirection);
	Transform.SetRotation(FMath::Lerp(Transform.Rotator().Quaternion(),
									  UKismetMathLibrary::MakeRotFromXZ(NewDirection, FVector::UpVector)
									  .Quaternion(), DeltaSeconds * MaxRotationSpeed));
}

void UBoid::DebugDraw() const
{
	const UWorld* World = GetWorld();
	const FVector& Location = Transform.GetLocation();
	DrawDebugLine(World, Location,
				  Location + m_CurrentMoveVector * 300.0f,
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
	//Compute Alignment Component Vector
	for (UBoid* Boid : Neighbourhood)
	{
		AlignmentComponent += Boid->m_CurrentMoveVector.GetSafeNormal(DefaultNormalizeVectorTolerance);
	}

	AlignmentComponent = (m_CurrentMoveVector + AlignmentComponent).GetSafeNormal(DefaultNormalizeVectorTolerance);
}

void UBoid::CalculateCohesionComponentVector()
{
	const FVector& Location = Transform.GetLocation();
	for (UBoid* Boid : Neighbourhood)
	{
		CohesionComponent += Boid->Transform.GetLocation() - Location;
	}

	CohesionComponent = (CohesionComponent / Neighbourhood.Num()) / CohesionLerp;
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

	for (UBoid* Boid : Neighbourhood)
	{
		FVector Separation = Location - Boid->Transform.GetLocation();
		SeparationComponent += Separation.GetSafeNormal(DefaultNormalizeVectorTolerance)
			/ FMath::Abs(Separation.Size() - BoidPhysicalRadius);
	}

	const FVector SeparationForceComponent = SeparationComponent * SeparationForce;
	SeparationComponent += SeparationForceComponent + SeparationForceComponent *
		(SeparationLerp / Neighbourhood.Num());
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
			if (FVector::Dist(Stimulus->GetActorLocation(), Location) <= m_Boid2PhysicalRadius)
			{
				Stimulus->Consume(this);
			}
			else
			{
				CalculatePositiveStimuliComponentVector(Stimulus);
			}
		}
	}

	NegativeStimuliComponent = NegativeStimuliMaxFactor *
		NegativeStimuliComponent.GetSafeNormal(DefaultNormalizeVectorTolerance);
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
	NegativeStimuliMaxFactor = FMath::Max(NegativeStimuliComponentForce.Size(),
										  NegativeStimuliMaxFactor);
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
	m_NewMoveVector = (AlignmentComponent * AlignmentWeight)
		+ (CohesionComponent * CohesionWeight)
		+ (SeparationComponent * SeparationWeight)
		+ NegativeStimuliComponent
		+ PositiveStimuliComponent;
}

void UBoid::CorrectDirectionAgainstCollision(FVector& Direction)
{
	//check for a hit on movement
	const FVector& Location = Transform.GetLocation();
	static TArray<AActor*> IgnoreActor = {Cast<AActor>(AAgent::Instance)};
	FHitResult Hit(1.0f);
	if (UKismetSystemLibrary::LineTraceSingle(
		this, Location,
		Location + Direction.GetSafeNormal(DefaultNormalizeVectorTolerance) * CollisionDistanceLook,
		TraceTypeQuery1, false, IgnoreActor, EDrawDebugTrace::ForOneFrame, Hit, true))
	{
		if (Hit.IsValidBlockingHit())
		{
			const FVector Normal2D = Hit.Normal.GetSafeNormal2D();
			Direction = FVector::VectorPlaneProject(Direction, Normal2D) * (1.f - Hit.Time) * CollisionWeight;
		}
	}
}
