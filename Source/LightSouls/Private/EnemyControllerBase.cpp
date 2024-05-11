// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyControllerBase.h"
#include "BehaviorTree/BlackboardComponent.h"

AEnemyControllerBase::AEnemyControllerBase()
{
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
}

void AEnemyControllerBase::BeginPlay()
{
	Super::BeginPlay();

	if (BlackboardToUse)
	{
		UseBlackboard(BlackboardToUse, BlackboardComponent);
	}

	if (DefaultBehaviorTree)
	{
		RunBehaviorTree(DefaultBehaviorTree);
	}

	FScriptDelegate TargetPereceptionUpdatedDelegate;
	TargetPereceptionUpdatedDelegate.BindUFunction(this, FName("TargetPerceptionUpdated"));
	AIPerception->OnTargetPerceptionUpdated.AddUnique(TargetPereceptionUpdatedDelegate);
}

void AEnemyControllerBase::TargetPerceptionUpdated(AActor* const SpottedActor, const FAIStimulus& Stimulus)
{
	if (Stimulus.WasSuccessfullySensed() && SpottedActor && BlackboardComponent)
	{
		BlackboardComponent->SetValueAsObject(FName("Target"), SpottedActor);
	}
}
