// Flock AI - Steering Behaviors for Unreal - juaxix

#include "Boid.h"

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
	  , SeparationLerp(5.0f)
	  , SeparationForce(100.0f)
	  , StimuliLerp(100.0f)
	  , SeparationWeight(60.0f)
	  , BaseMovementSpeed(50.0f)
	  , MaxMovementSpeed(100.0f)
	  , VisionRadius(400.0f)
	  , MaxRotationSpeed(60.0f)
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

	const FVector NewDirection = (m_NewMoveVector * BaseMovementSpeed * DeltaSeconds).
		GetClampedToMaxSize(MaxMovementSpeed * DeltaSeconds);

	//@todo check for a hit on movement
	//FHitResult Hit(1.f);
	//RootComponent->MoveComponent(NewDirection, NewRotation, true, &Hit);
	//if (Hit.IsValidBlockingHit())
	//{
	//const FVector Normal2D = Hit.Normal.GetSafeNormal2D();
	//const FVector Deflection = FVector::VectorPlaneProject(NewDirection, Normal2D) * (1.f - Hit.Time);
	//RootComponent->MoveComponent(Deflection, NewRotation, true);
	//}

	Transform.SetLocation(Transform.GetLocation() + NewDirection);
	Transform.SetRotation(UKismetMathLibrary::MakeRotFromXZ(NewDirection, FVector::UpVector).Quaternion());
}

void UBoid::DebugDraw() const
{
	DrawDebugLine(GetWorld(), Transform.GetLocation(),
				  Transform.GetLocation() + m_CurrentMoveVector * 300.0f, FColor::Red,
				  false, 0.12f, 0, 1.0f);

	DrawDebugLine(GetWorld(), Transform.GetLocation(),
				  Transform.GetLocation() + CohesionComponent * 300.0f, FColor::Orange,
				  false, 0.12f, 0, 1.0f);

	DrawDebugLine(GetWorld(), Transform.GetLocation(),
				  Transform.GetLocation() + AlignmentComponent * 300.0f, FColor::Purple,
				  false, 0.12f, 0, 1.0f);

	DrawDebugLine(GetWorld(), Transform.GetLocation(),
				  Transform.GetLocation() + SeparationComponent * 300.0f, FColor::Emerald,
				  false, 0.12f, 0, 1.0f);
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

	CheckStimulusVision();
	ComputeStimuliComponentVector();
	ComputeAggregationOfComponents();

	DebugDraw();
}

void UBoid::CalculateAlignmentComponentVector()
{
	//Compute Alignment Component Vector
	for (UBoid* Boid : Neighbourhood)
	{
		AlignmentComponent += Boid->m_CurrentMoveVector.GetSafeNormal(DefaultNormalizeVectorTolerance);
	}

	AlignmentComponent = (m_CurrentMoveVector+AlignmentComponent).GetSafeNormal(DefaultNormalizeVectorTolerance);
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
			/ FMath::Abs(Separation.Size() - m_Boid2PhysicalRadius);
	}

	const FVector SeparationForceComponent = SeparationComponent * SeparationForce;
	SeparationComponent += SeparationForceComponent + SeparationForceComponent *
		(SeparationLerp / Neighbourhood.Num());
}

void UBoid::ComputeStimuliComponentVector()
{
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
			CalculatePositiveStimuliComponentVector(Stimulus);
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
