// Flock AI - Steering Behaviors for Unreal - juaxix

#pragma once

#include "GameFramework/Actor.h"
#include "Agent.generated.h"

class AStimulus;
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

	UFUNCTION(BlueprintCallable, Category = "AI")
	void RemoveBoid(UBoid* Boid);

	UFUNCTION(BlueprintCallable, Category = "AI")
	void AddGlobalStimulus(AStimulus* Stimulus);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "AI")
	void RemoveGlobalStimulus(AStimulus* Stimulus);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "AI")
	const TArray<AStimulus*>& GetGlobalStimulus() const { return GlobalStimuli; }

	// Begin Actor Interface
	virtual void Tick(float DeltaSeconds) override;
	// End Actor Interface

public:
	// The class of the Boid to spawn
	UPROPERTY(Category = Spawn, EditDefaultsOnly)
	TSubclassOf<UBoid> BoidBP;

protected:
	UFUNCTION(BlueprintCallable, Category = "AI")
	void UpdateBoidNeighbourhood(UBoid* Boid);

	void UpdateBoids(float DeltaTime);

	void ApplyPendingBoidRemovals();

	// All the agents are now boids inside this Agents Manager
	UPROPERTY(Category = AI, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TMap<int32, UBoid*> Boids;

	UPROPERTY(Category = AI, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TArray<int32> PendingBoidRemovals;

	// All the global tracked stimulus
	UPROPERTY(Category = AI, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TArray<AStimulus*> GlobalStimuli;

	//protect the use of the boids
	FCriticalSection MutexBoid;
};
