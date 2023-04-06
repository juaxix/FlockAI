// Flock AI - Steering Behaviors for Unreal - juaxix

#include "FlockAIGamePawn.h"
#include "FlockAI.h"
#include "FlockAIGame.h"

namespace
{
	constexpr const float MAX_CLICK_DISTANCE = 100000.0f;
}

// Sets default values
AFlockAIGamePawn::AFlockAIGamePawn()
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
	CurrentGamemode = EFlockAIGamemode::EGM_SpawnNewAgents;

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

// Called every frame
void AFlockAIGamePawn::Tick(float DeltaTime)
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
		if (CurrentGamemode == EFlockAIGamemode::EGM_SpawnNewAgents)
		{
			SpawningLocation = GetCursorPositionInActionLayer();
			if (!SpawningLocation.IsZero())
			{
				FVector Direction =  SpawningLocation - PreviewMeshComponent->GetComponentLocation();
				Direction.Z = 0.0;
				PreviewMeshComponent->SetWorldLocation(SpawningLocation);
				PreviewMeshComponent->SetWorldRotation(Direction.Rotation());
			}
		}
	}
}

// Called to bind functionality to input
void AFlockAIGamePawn::SetupPlayerInputComponent(class UInputComponent* InInputComponent)
{
	Super::SetupPlayerInputComponent(InInputComponent);

	check(InInputComponent);

	// set up key bindings
	InInputComponent->BindAxis("MoveForward");
	InInputComponent->BindAxis("MoveRight");
	InInputComponent->BindAction("ZoomIn", IE_Pressed, this, &AFlockAIGamePawn::ZoomIn);
	InInputComponent->BindAction("ZoomOut", IE_Pressed, this, &AFlockAIGamePawn::ZoomOut);
	InInputComponent->BindAction("Spawn", IE_Pressed, this, &AFlockAIGamePawn::BeginSpawning);
	InInputComponent->BindAction("Spawn", IE_Released, this, &AFlockAIGamePawn::DoSpawning);
	InInputComponent->BindAction("CancelSpawning", IE_Pressed, this, &AFlockAIGamePawn::CancelSpawning);
	InInputComponent->BindAction("Gamemode1", IE_Pressed, this,
								 &AFlockAIGamePawn::ChangeGamemode<EFlockAIGamemode::EGM_SpawnNewAgents>);
	InInputComponent->BindAction("Gamemode2", IE_Pressed, this,
								 &AFlockAIGamePawn::ChangeGamemode<EFlockAIGamemode::EGM_SpawnPositiveStimuli>);
	InInputComponent->BindAction("Gamemode3", IE_Pressed, this,
								 &AFlockAIGamePawn::ChangeGamemode<EFlockAIGamemode::EGM_SpawnNegativeStimuli>);
}

void AFlockAIGamePawn::ZoomIn()
{
	bZoomingIn = true;
}

void AFlockAIGamePawn::ZoomOut()
{
	bZoomingIn = false;
}

void AFlockAIGamePawn::BeginSpawning()
{
	bWantToSpawn = true;
	SpawningLocation = GetCursorPositionInActionLayer();
	if (SpawningLocation.IsZero())
	{
		CancelSpawning();
		return;
	}
	PreviewMeshComponent->SetWorldLocation(SpawningLocation);
	switch (CurrentGamemode)
	{
		case EFlockAIGamemode::EGM_SpawnNewAgents:
		{
			PreviewMeshComponent->SetStaticMesh(PreviewMeshAgent);
			break;
		}
		case EFlockAIGamemode::EGM_SpawnPositiveStimuli:
		{
			PreviewMeshComponent->SetStaticMesh(PreviewMeshPositiveStimulus);
			break;
		}
		case EFlockAIGamemode::EGM_SpawnNegativeStimuli:
		{
			PreviewMeshComponent->SetStaticMesh(PreviewMeshNegativeStimulus);
			break;
		}
	}
}

void AFlockAIGamePawn::DoSpawning()
{
	if (bWantToSpawn)
	{
		if (CurrentGamemode == EFlockAIGamemode::EGM_SpawnNewAgents)
		{
			// Only one Agent (it is a manager now)
			if (!IsValid(Agent))
			{
				Agent = GetWorld()->SpawnActor<AAgent>(AgentBP, FVector::ZeroVector, FRotator::ZeroRotator);
			}
			
			Agent->SpawnBoid(PreviewMeshComponent->GetComponentLocation(), PreviewMeshComponent->GetComponentRotation());
		}
		else if (CurrentGamemode == EFlockAIGamemode::EGM_SpawnPositiveStimuli)
		{
			GetWorld()->SpawnActor<AStimulus>(PositiveStimulusBP, SpawningLocation, FRotator::ZeroRotator);
		}
		else if (CurrentGamemode == EFlockAIGamemode::EGM_SpawnNegativeStimuli)
		{
			GetWorld()->SpawnActor<AStimulus>(NegativeStimulusBP, SpawningLocation, FRotator::ZeroRotator);
		}

		CancelSpawning();
	}
}

void AFlockAIGamePawn::CancelSpawning()
{
	bWantToSpawn = false;
	PreviewMeshComponent->SetStaticMesh(nullptr);
}

FVector AFlockAIGamePawn::GetCursorPositionInActionLayer()
{
	UWorld* World = GetWorld();
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
    PlayerController->DeprojectMousePositionToWorld(MouseLocation, MouseDirection);
	FVector EndLocation = MouseLocation - (MouseDirection * (MouseLocation.Z / MouseDirection.Z) * MAX_CLICK_DISTANCE);
	FHitResult HitResult;
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(this);
	CollisionQueryParams.AddIgnoredActor(AAgent::Instance);
	DrawDebugLine(World, MouseLocation, EndLocation, FColor::Purple, false, 15.0f, 1, 0.21f);
	if (World->LineTraceSingleByChannel(HitResult, MouseLocation, EndLocation, ECollisionChannel::ECC_Visibility, CollisionQueryParams))
	{
    	DrawDebugCone(World, HitResult.Location, FVector::UpVector, 300.0f, 35.0f, 35.0f, 12, FColor::Cyan, false, 2.0f);
		return HitResult.Location;
    }
	
	return FVector::Zero();
}

template <EFlockAIGamemode Gamemode>
void AFlockAIGamePawn::ChangeGamemode()
{
	if (CurrentGamemode != Gamemode)
	{
		CancelSpawning();
		CurrentGamemode = Gamemode;
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
			FString::Printf(TEXT("Set Gamemode: %d"), static_cast<int32>(Gamemode)));
	}
}
