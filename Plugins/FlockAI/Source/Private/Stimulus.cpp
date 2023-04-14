// Flock AI - Steering Behaviors for Unreal - juaxix

#include "Stimulus.h"

#include "Agent.h"

void AStimulus::Consume_Implementation(UBoid* Boid)
{
	AAgent::Instance->RemoveGlobalStimulus(this);
	Destroy();
}
