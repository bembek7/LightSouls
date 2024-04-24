// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyBase.generated.h"

UCLASS()
class LIGHTSOULS_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyBase();

	float GetCurrentHealth() const;
	float GetMaxHealth() const;

	void Damage(const float Damage, const FVector& HitterLocation);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	void Die();
	void SpawnBlood(const FVector& HitterLocation);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget>HealthBarWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor>OnHitParticleEffectClass;

	UPROPERTY(EditDefaultsOnly)
	class UWidgetComponent* HealthBarInWorld;

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* HitImpactAnimMontage;

private:
	float MaxHealth = 100.f;
	float CurrentHealth;
};
