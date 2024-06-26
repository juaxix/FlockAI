// Flock AI - Steering Behaviors for Unreal - juaxix

#include "Agent.h"
#include "Boid.h"
#include "Stimulus.h"
#include "Misc/ScopeLock.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

AAgent::AAgent()
{
	PrimaryActorTick.bCanEverTick = true;
	HierarchicalInstancedStaticMeshComponent = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("ShipMeshInstances"));
	RootComponent = HierarchicalInstancedStaticMeshComponent;
	HierarchicalInstancedStaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AAgent::SpawnBoid(const FVector& Location, const FRotator& Rotation)
{
	check(BoidBP);
	
	//Create new instanced mesh in location and rotation
	const int32 MeshInstanceIndex = HierarchicalInstancedStaticMeshComponent->AddInstance(
		FTransform(Rotation.Quaternion(), Location, FVector::OneVector));
	UBoid* Boid = NewObject<UBoid>(this, BoidBP);
	Boid->Init(Location, Rotation, MeshInstanceIndex);
	{
		FScopeLock ScopeLock(&MutexBoid);
		Boids.Add(MeshInstanceIndex, Boid);
	}
}

void AAgent::RemoveBoid(UBoid* Boid)
{
	if (IsValid(Boid))
	{
		FScopeLock ScopeLock(&MutexBoid);
		PendingBoidRemovals.Add(Boid->MeshIndex);
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
	for (const TTuple<int, UBoid*>& PairBoid : Boids)
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

	Boid->Neighbourhood.Empty(Boid->Neighbourhood.Num());

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

void AAgent::UpdateBoids(float DeltaTime)
{
	FScopeLock ScopeLock(&MutexBoid);
	const int32 LastKey = Boids.end().Key();
	
	for (const TTuple<int, UBoid*>& PairBoid : Boids)
	{
		UBoid* Boid = PairBoid.Value;
		UpdateBoidNeighbourhood(Boid);
		Boid->Update(DeltaTime, this);

		HierarchicalInstancedStaticMeshComponent->UpdateInstanceTransform(
			Boid->MeshIndex,
			Boid->Transform,
			PairBoid.Key == LastKey
		);
	}
}

void AAgent::ApplyPendingBoidRemovals()
{
	if (PendingBoidRemovals.IsEmpty())
	{
		return;
	}

	FScopeLock ScopeLock(&MutexBoid);
	PendingBoidRemovals.Sort();

	for (int32 i = PendingBoidRemovals.Num() - 1; i >= 0; i--)
	{
		const int32 MeshIndexToRemove = PendingBoidRemovals[i];
		Boids.Remove(MeshIndexToRemove);
		if (!HierarchicalInstancedStaticMeshComponent->RemoveInstance(MeshIndexToRemove))
		{
			break;
		}

		// Update MeshIndex values for the remaining Boids
		FTransform InstanceTransform;
		if (HierarchicalInstancedStaticMeshComponent->GetInstanceTransform(MeshIndexToRemove, InstanceTransform, true))
		{
			TMap<int32, UBoid*> UpdatedBoids;
			for (const TTuple<int32, UBoid*>& PairBoid : Boids)
			{
				if (IsValid(PairBoid.Value) && PairBoid.Value->Transform.Equals(InstanceTransform))
				{
					PairBoid.Value->MeshIndex = MeshIndexToRemove;
					UpdatedBoids.Add(MeshIndexToRemove, PairBoid.Value);
				}
				else
				{
					UpdatedBoids.Add(PairBoid.Key, PairBoid.Value);
				}
			}
			Boids = UpdatedBoids;
		}
	}

	PendingBoidRemovals.Empty();
}

void AAgent::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (Boids.Num() == 0)
	{
		return;
	}

	UpdateBoids(DeltaSeconds);
	ApplyPendingBoidRemovals();
}
