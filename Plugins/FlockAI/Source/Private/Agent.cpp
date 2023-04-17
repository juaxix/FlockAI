// Flock AI - Steering Behaviors for Unreal - juaxix

#include "Agent.h"
#include "Boid.h"
#include "Stimulus.h"
#include "Misc/ScopeLock.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

AAgent::AAgent()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create the h.instanced mesh component
	HierarchicalInstancedStaticMeshComponent = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("ShipMeshInstances"));
	RootComponent = HierarchicalInstancedStaticMeshComponent;
	HierarchicalInstancedStaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AAgent::SpawnBoid(const FVector& Location, const FRotator& Rotation)
{
	check(BoidBP);
	FScopeLock ScopeLock(&MutexBoid);
	//Create new instanced mesh in location and rotation
	const int32 MeshInstanceIndex = HierarchicalInstancedStaticMeshComponent->AddInstance(
		FTransform(Rotation.Quaternion(), Location, FVector::OneVector));
	UBoid* Boid = NewObject<UBoid>(this, BoidBP);
	Boid->Init(Location, Rotation, MeshInstanceIndex);
	Boids.Add(MeshInstanceIndex, Boid);
}

void AAgent::RemoveBoid(UBoid* Boid)
{
	if (IsValid(Boid))
	{
		FScopeLock ScopeLock(&MutexBoid);
		Boids.Remove(Boid->MeshIndex);
		HierarchicalInstancedStaticMeshComponent->RemoveInstance(Boid->MeshIndex);
	}
}

void AAgent::AddGlobalStimulus(AStimulus* Stimulus)
{
	if (IsValid(Stimulus))
	{
		GlobalStimuli.AddUnique(Stimulus);
	}
}

void AAgent::RemoveGlobalStimulus(AStimulus* Stimulus)
{
	GlobalStimuli.Remove(Stimulus);
	for (const auto& PairBoid : Boids)
	{
		check(IsValid(PairBoid.Value));
		PairBoid.Value->RemovePrivateGlobalStimulus(Stimulus);
	}
}

void AAgent::UpdateBoidNeighbourhood(UBoid* Boid)
{
	check(HierarchicalInstancedStaticMeshComponent);
	check(Boid);
	TArray<int32> OverlappingInstances =
		HierarchicalInstancedStaticMeshComponent->GetInstancesOverlappingSphere(
			Boid->Transform.GetLocation(), Boid->VisionRadius, false);

	Boid->Neighbourhood.Empty();

	for (const int32& Index : OverlappingInstances)
	{
		if (Boid->MeshIndex == Index)
		{
			continue;
		}
		UBoid** OverlappingBoid = Boids.Find(Index);
		if (OverlappingBoid != nullptr && IsValid(*OverlappingBoid))
		{
			Boid->Neighbourhood.Add(*OverlappingBoid);
		}
	}
}

void AAgent::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (Boids.Num() == 0)
	{
		return;
	}
	FScopeLock ScopeLock(&MutexBoid);
	const int32 LastKey = Boids.end().Key();

	for (const auto& PairBoid : Boids)
	{
		UBoid* Boid = PairBoid.Value;
		UpdateBoidNeighbourhood(Boid);
		Boid->Update(DeltaSeconds, this);

		HierarchicalInstancedStaticMeshComponent->UpdateInstanceTransform(
			Boid->MeshIndex,
			Boid->Transform,
			PairBoid.Key == LastKey
		);
	}
}
