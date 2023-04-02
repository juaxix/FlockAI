// Flock AI - Steering Behaviors for Unreal - juaxix

#include "FlockAIPlayer.h"
#include "FlockAI.h"


// Sets default values
AFlockAIPlayer::AFlockAIPlayer()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Default values
	ZoomedOutMoveSpeed = 3500.0f;
	ZoomedInMoveSpeed = 1750.0f;
	ZoomedOutDistance = 3000.0f;
	ZoomedInDistance = 1400.0f;
	bZoomingIn = true;
	bWantToSpawn = false;
	CurrentGamemode = EGM_SpawnNewAgents;

	// Create the root component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	CameraBoom->TargetArmLength = ZoomedInDistance;
	CameraBoom->SetRelativeRotation(FRotator(-80.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 3.0f;

	// Create a camera...
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	CameraComponent->AttachToComponent(CameraBoom, FAttachmentTransformRules::KeepRelativeTransform,
									   USpringArmComponent::SocketName);

	// Create the spawning preview mesh
	PreviewMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMesh"));
	PreviewMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PreviewMeshComponent->SetCollisionObjectType(ECC_Destructible);
	// Ignored by VisionSphere in Agents
	PreviewMeshComponent->SetUsingAbsoluteLocation(true);
	PreviewMeshComponent->SetUsingAbsoluteRotation(true);

	Agent = nullptr;
}

// Called when the game starts or when spawned
void AFlockAIPlayer::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AFlockAIPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Find movement direction
	const float ForwardValue = GetInputAxisValue("MoveForward");
	const float RightValue = GetInputAxisValue("MoveRight");

	// Clamp max size so that (X=1, Y=1) doesn't cause faster movement in diagonal directions
	FVector MoveDirection = FVector(ForwardValue, RightValue, 0.f).GetClampedToMaxSize(1.0f);

	// If non-zero size, move this actor
	if (!MoveDirection.IsZero())
	{
		const float MoveSpeed = bZoomingIn ? ZoomedInMoveSpeed : ZoomedOutMoveSpeed;

		MoveDirection = MoveDirection.GetSafeNormal() * MoveSpeed * DeltaTime;

		FVector NewLocation = GetActorLocation();
		NewLocation += GetActorForwardVector() * MoveDirection.X;
		NewLocation += GetActorRightVector() * MoveDirection.Y;

		SetActorLocation(NewLocation);
	}

	// Handle zoom camera movement
	{
		if (bZoomingIn)
		{
			ZoomFactor -= DeltaTime / 0.5f;
		}
		else
		{
			ZoomFactor += DeltaTime / 0.5f;
		}

		ZoomFactor = FMath::Clamp<float>(ZoomFactor, 0.0f, 1.0f);

		CameraBoom->TargetArmLength = FMath::Lerp(ZoomedInDistance, ZoomedOutDistance, ZoomFactor);
	}

	// Show oriented preview of Agent when spawning
	if (bWantToSpawn)
	{
		if (CurrentGamemode == EGM_SpawnNewAgents)
		{
			MouseLocation = GetCursorPositionInActionLayer();

			PreviewMeshComponent->SetWorldRotation((MouseLocation - SpawningLocation).Rotation());
		}
	}
}

// Called to bind functionality to input
void AFlockAIPlayer::SetupPlayerInputComponent(class UInputComponent* InInputComponent)
{
	Super::SetupPlayerInputComponent(InInputComponent);

	check(InInputComponent);

	// set up key bindings
	InInputComponent->BindAxis("MoveForward");
	InInputComponent->BindAxis("MoveRight");
	InInputComponent->BindAction("ZoomIn", IE_Pressed, this, &AFlockAIPlayer::ZoomIn);
	InInputComponent->BindAction("ZoomOut", IE_Pressed, this, &AFlockAIPlayer::ZoomOut);
	InInputComponent->BindAction("Spawn", IE_Pressed, this, &AFlockAIPlayer::BeginSpawning);
	InInputComponent->BindAction("Spawn", IE_Released, this, &AFlockAIPlayer::DoSpawning);
	InInputComponent->BindAction("CancelSpawning", IE_Pressed, this, &AFlockAIPlayer::CancelSpawning);
	InInputComponent->BindAction("Gamemode1", IE_Pressed, this,
								 &AFlockAIPlayer::ChangeGamemode<EGM_SpawnNewAgents>);
	InInputComponent->BindAction("Gamemode2", IE_Pressed, this,
								 &AFlockAIPlayer::ChangeGamemode<EGM_SpawnPositiveStimuli>);
	InInputComponent->BindAction("Gamemode3", IE_Pressed, this,
								 &AFlockAIPlayer::ChangeGamemode<EGM_SpawnNegativeStimuli>);
}

void AFlockAIPlayer::ZoomIn()
{
	bZoomingIn = true;
}

void AFlockAIPlayer::ZoomOut()
{
	bZoomingIn = false;
}

void AFlockAIPlayer::BeginSpawning()
{
	bWantToSpawn = true;

	SpawningLocation = GetCursorPositionInActionLayer();

	PreviewMeshComponent->SetWorldLocation(SpawningLocation);

	switch (CurrentGamemode)
	{
	case EGM_SpawnNewAgents:
		{
			PreviewMeshComponent->SetStaticMesh(PreviewMeshAgent);
			break;
		}
	case EGM_SpawnPositiveStimuli:
		{
			PreviewMeshComponent->SetStaticMesh(PreviewMeshPositiveStimulus);
			break;
		}
	case EGM_SpawnNegativeStimuli:
		{
			PreviewMeshComponent->SetStaticMesh(PreviewMeshNegativeStimulus);
			break;
		}
	}
}

void AFlockAIPlayer::DoSpawning()
{
	if (bWantToSpawn)
	{
		if (CurrentGamemode == EGM_SpawnNewAgents)
		{
			//Only one Agent (it is a manager now)
			if (!IsValid(Agent))
			{
				Agent = Cast<AAgent>(GetWorld()->SpawnActor<AAgent>(
					AgentBP, FVector::ZeroVector, FRotator::ZeroRotator));
			}
			Agent->SpawnBoid(SpawningLocation, (MouseLocation - SpawningLocation).Rotation());
		}
		else if (CurrentGamemode == EGM_SpawnPositiveStimuli)
		{
			GetWorld()->SpawnActor<AStimulus>(PositiveStimulusBP, SpawningLocation, FRotator::ZeroRotator);
		}
		else if (CurrentGamemode == EGM_SpawnNegativeStimuli)
		{
			GetWorld()->SpawnActor<AStimulus>(NegativeStimulusBP, SpawningLocation, FRotator::ZeroRotator);
		}

		CancelSpawning();
	}
}

void AFlockAIPlayer::CancelSpawning()
{
	bWantToSpawn = false;

	PreviewMeshComponent->SetStaticMesh(nullptr);
}

FVector AFlockAIPlayer::GetCursorPositionInActionLayer()
{
	FVector MousePosition;
	FVector MouseDirection;
	FVector Result;

	GetWorld()->GetFirstPlayerController()->DeprojectMousePositionToWorld(MousePosition, MouseDirection);

	Result.X = (- MousePosition.Z / MouseDirection.Z) * MouseDirection.X + MousePosition.X;
	Result.Y = (- MousePosition.Z / MouseDirection.Z) * MouseDirection.Y + MousePosition.Y;
	Result.Z = 0.0F;

	return Result;
}

template <EFlockAIGamemode Gamemode>
void AFlockAIPlayer::ChangeGamemode()
{
	if (CurrentGamemode != Gamemode)
	{
		CancelSpawning();

		CurrentGamemode = Gamemode;

		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
										 FString::Printf(TEXT("Set Gamemode: %d"), static_cast<int>(Gamemode)));
	}
}
