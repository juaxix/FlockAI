// Flock AI - Steering Behaviors for Unreal - juaxix

#pragma once

#include "GameFramework/Actor.h"
#include "Agent.generated.h"

//forwards
class UBoid;

UCLASS()
class FLOCKAI_API AAgent : public AActor
{
	GENERATED_BODY()

	/* The instanced mesh component */
	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UHierarchicalInstancedStaticMeshComponent* HierarchicalInstancedStaticMeshComponent;


public:
	AAgent();

	UFUNCTION(BlueprintCallable, Category = "AI")
	void SpawnBoid(const FVector& Location, const FRotator& Rotation);

	// Begin Actor Interface
	virtual void Tick(float DeltaSeconds) override;
	// End Actor Interface

public:
	// The class of the Boid to spawn
	UPROPERTY(Category = Spawn, EditDefaultsOnly)
	TSubclassOf<UBoid> BoidBP;

	//This instance
	static AAgent* Instance;
protected:
	UFUNCTION(BlueprintCallable, Category = "AI")
	void UpdateBoidNeighbourhood(UBoid* Boid);

	//All the agents are now boids inside this Agents Manager
	UPROPERTY(Category = AI, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TMap<int32, UBoid*> m_Boids;

	//protect the use of the boids
	FCriticalSection m_MutexBoid;
};
