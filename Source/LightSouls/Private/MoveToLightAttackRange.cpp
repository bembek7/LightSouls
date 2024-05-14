// Fill out your copyright notice in the Description page of Project Settings.


#include "MoveToLightAttackRange.h"
#include "LightAttackTask.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnemyBase.h"

UMoveToLightAttackRange::UMoveToLightAttackRange(const FObjectInitializer& objectInitializer)
	: Super(objectInitializer)
{
	INIT_TASK_NODE_NOTIFY_FLAGS();
}

EBTNodeResult::Type UMoveToLightAttackRange::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	AAIController* const OwningController = Cast<AAIController>(OwnerComp.GetOwner());
	if (OwningController)
	{
		if (UBlackboardComponent* const BlackboardComp = OwningController->GetBlackboardComponent())
		{
			AActor* const Target = Cast<AActor>(BlackboardComp->GetValueAsObject(FName("Target")));
			if (Target)
			{
				if (AEnemyBase* const ControlledPawn = Cast<AEnemyBase>(OwningController->GetPawn()))
				{
					const float LightAttackRange = ControlledPawn->GetLightAttackRange();
					if (FVector::Distance(Target->GetActorLocation(), ControlledPawn->GetActorLocation()) <= LightAttackRange + 100.f)
					{
						return EBTNodeResult::Succeeded;
					}
					else
					{
						OwningController->MoveToActor(Target, LightAttackRange);
						OwningController->GetPathFollowingComponent()->OnRequestFinished.AddUFunction(this, FName("FinishMove"));
					}
				}
			}
		}
	}

	return EBTNodeResult::InProgress;
}

void UMoveToLightAttackRange::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	// I have to use such a stupid workaurond cause there is no way to pass Owner comp to some function so that it would work, it's pretty weird
	if (bTaskFinished)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		AAIController* const OwningController = Cast<AAIController>(OwnerComp.GetOwner());
		if (OwningController)
		{
			OwningController->GetPathFollowingComponent()->OnRequestFinished.Clear();
		}
		bTaskFinished = false;
	}
}

void UMoveToLightAttackRange::FinishMove()
{
	bTaskFinished = true;
}