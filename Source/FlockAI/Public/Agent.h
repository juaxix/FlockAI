// Flock AI - Steering Behaviors for Unreal - juaxix

#pragma once

#include "GameFramework/Actor.h"
#include "Agent.generated.h"


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

protected:
	UFUNCTION(BlueprintCallable, Category = "AI")
	void UpdateBoidNeighbourhood(class UBoid* Boid);

	//All the agents are now boids inside this Agents Manager
	UPROPERTY(Category = AI, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TMap<int32, class UBoid*> m_Boids;
	FCriticalSection m_MutexBoid;
};
