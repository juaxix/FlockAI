// Flock AI - Steering Behaviors for Unreal - juaxix

#include "Stimulus.h"


AStimulus::AStimulus()
{
	// Initialize default values
	Value = 0.0f;
}

void AStimulus::Consume_Implementation(UBoid* Boid)
{
	Destroy();
}
