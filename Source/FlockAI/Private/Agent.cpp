// Flock AI - Steering Behaviors for Unreal - juaxix

#include "Agent.h"
#include "Boid.h"
#include "FlockAI.h"
#include "Misc/ScopeLock.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

AAgent::AAgent()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create the h.instanced mesh component
	HierarchicalInstancedStaticMeshComponent = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(
		TEXT("ShipMeshInstances"));
	RootComponent = HierarchicalInstancedStaticMeshComponent;
	HierarchicalInstancedStaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AAgent::SpawnBoid(const FVector& Location, const FRotator& Rotation)
{
	FScopeLock ScopeLock(&m_MutexBoid);
	//Create new instanced mesh in location and rotation
	const int32 MeshInstanceIndex = HierarchicalInstancedStaticMeshComponent->AddInstance(
		FTransform(Rotation.Quaternion(), Location, FVector::OneVector));
	UBoid* Boid = NewObject<UBoid>(this, UBoid::StaticClass());
	Boid->Init(Location, Rotation, MeshInstanceIndex);
	m_Boids.Add(MeshInstanceIndex, Boid);
}

void AAgent::UpdateBoidNeighbourhood(UBoid* Boid)
{
	check(HierarchicalInstancedStaticMeshComponent);
	check(Boid);
	TArray<int32> OverlappingInstances =
		HierarchicalInstancedStaticMeshComponent->GetInstancesOverlappingSphere(
			Boid->Transform.GetLocation(), Boid->BoidPhysicalRadius, false);

	Boid->Neighbourhood.Empty();
	for (int32& Index : OverlappingInstances)
	{
		if (Boid->MeshIndex == Index)
		{
			continue;
		}
		Boid->Neighbourhood.Add(Boid);
	}
}

void AAgent::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (m_Boids.Num() == 0)
	{
		return;
	}
	FScopeLock ScopeLock(&m_MutexBoid);
	for (auto& PairBoid : m_Boids)
	{
		UBoid* Boid = PairBoid.Value;
		UpdateBoidNeighbourhood(Boid);
		Boid->Update(DeltaSeconds);
		HierarchicalInstancedStaticMeshComponent->UpdateInstanceTransform(
			Boid->MeshIndex,
			Boid->Transform,
			true
		);
	}
}
