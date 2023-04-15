// Flock AI - Steering Behaviors for Unreal - juaxix

#include "Stimulus.h"

#include "Agent.h"

void AStimulus::Consume_Implementation(UBoid* Boid)
{
	for (AAgent* Agent : AAgent::Instances)
	{
		Agent->RemoveGlobalStimulus(this);
	}

	Destroy();
}
