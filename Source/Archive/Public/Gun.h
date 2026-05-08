// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Projectile.h"
#include "Gun.generated.h"

/**
 * 
 */
class AProjectile;

UCLASS()
class ARCHIVE_API AGun : public AWeapon
{
	GENERATED_BODY()
	
public:

	AGun();
	virtual void Attack() override;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, Category = "Effects")
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, Category = "Effects")
	UParticleSystem* ImpactEffect;

	UPROPERTY(EditAnywhere, Category = "Effects")
	USoundBase* MuzzleSound;

	UPROPERTY(EditAnywhere, Category = "Effects")
	USoundBase* ImpactSound;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float Damage = 25.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float Range = 10000.f;

	void PerformLineTrace(FHitResult& OutHit);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<AProjectile> ProjectileClass;
};
