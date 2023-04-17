// Flock AI Game - Steering Behaviors for Unreal - juaxix

#pragma once

#include "GameFramework/Pawn.h"
#include "Agent.h"
#include "Stimulus.h"
#include "FlockAIGamePawn.generated.h"

UENUM()
enum class EFlockAIGamemode : uint8
{
	EGM_SpawnNewAgents,
	EGM_SpawnPositiveStimuli,
	EGM_SpawnNegativeStimuli,
};

UCLASS()
class FLOCKAIGAME_API AFlockAIGamePawn : public APawn
{
	GENERATED_BODY()

public:
	AFlockAIGamePawn();
	
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* InInputComponent) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "AI")
	void OnAgentSpawned(AAgent* Agent);

	UFUNCTION(BlueprintPure, Category = "AI")
	TArray<class AAgent*>& Agents() { return AgentInstances; }

	// CAMERA VARIABLES

	/* The speed the camera moves around the level, when zoomed out */
	UPROPERTY(Category = Camera, EditAnywhere, BlueprintReadWrite)
	float ZoomedOutMoveSpeed;

	/* The speed the camera moves around the level, when zoomed in */
	UPROPERTY(Category = Camera, EditAnywhere, BlueprintReadWrite)
	float ZoomedInMoveSpeed;

	/* The distance the camera is from the action layer, when zoomed out */
	UPROPERTY(Category = Camera, EditAnywhere, BlueprintReadWrite)
	float ZoomedOutDistance;

	/* The distance the camera is from the action layer, when zoomed in */
	UPROPERTY(Category = Camera, EditAnywhere, BlueprintReadWrite)
	float ZoomedInDistance;


	//SPAWN REFERENCES

	// The preview mesh of the Agent when its is able to spawn
	UPROPERTY(Category = Spawn, EditDefaultsOnly)
	class UStaticMesh* PreviewMeshAgent;

	// The preview mesh of the Positive Stimulus
	UPROPERTY(Category = Spawn, EditDefaultsOnly)
	class UStaticMesh* PreviewMeshPositiveStimulus;

	// The preview mesh of the Negative Stimulus
	UPROPERTY(Category = Spawn, EditDefaultsOnly)
	class UStaticMesh* PreviewMeshNegativeStimulus;

	/* The mesh component */
	UPROPERTY(Category = Spawn, EditDefaultsOnly)
	class UStaticMeshComponent* PreviewMeshComponent;

	// The class of the Agent to spawn
	UPROPERTY(Category = Spawn, EditDefaultsOnly)
	TSubclassOf<AAgent> AgentBP;

	// The class of the Negative Stimulus to spawn
	UPROPERTY(Category = Spawn, EditDefaultsOnly)
	TSubclassOf<AStimulus> NegativeStimulusBP;

	// The class of the Positive Stimulus to spawn
	UPROPERTY(Category = Spawn, EditDefaultsOnly)
	TSubclassOf<AStimulus> PositiveStimulusBP;

protected:
	/* The camera */
	UPROPERTY(Category = Camera, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	class UCameraComponent* CameraComponent = nullptr;

	/* Camera boom positioning the camera above the character */
	UPROPERTY(Category = Camera, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	class USpringArmComponent* CameraBoom = nullptr;

	UPROPERTY(Transient)
	TArray<class AAgent*> AgentInstances;

	// Binding functions for input actions
	void ZoomIn();
	void ZoomOut();
	void BeginSpawning();
	void DoSpawning();
	void CancelSpawning();
	void SpawnAgent();
	template <EFlockAIGamemode Gamemode>
	void ChangeGamemode();

	// Input variables
	bool bZoomingIn = true;
	float ZoomFactor = 0.0f;
	bool bWantToSpawn = false;
	FVector SpawningLocation;
	FVector MouseLocation;
	FVector MouseDirection;
	EFlockAIGamemode CurrentGamemode;
	int32 CurrentAgentIndex = -1;
	/* Returns the cursor position inside the game action layer */
	FVector GetCursorPositionInActionLayer();
	
public:
	/** Returns CameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetCameraComponent() const { return CameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
};
