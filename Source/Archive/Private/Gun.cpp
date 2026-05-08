// Fill out your copyright notice in the Description page of Project Settings.


#include "Gun.h"
#include "DrawDebugHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "Engine/DamageEvents.h"


AGun::AGun()
{
}


void AGun::BeginPlay()
{
	Super::BeginPlay();
}

void AGun::Attack() {
	if (MuzzleFlash) {
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, Mesh->GetSocketLocation(TEXT("Muzzle")));

	}
	if (MuzzleSound) {
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), MuzzleSound, GetActorLocation());
	}

	FHitResult HitResult;
	PerformLineTrace(HitResult);

	if (HitResult.bBlockingHit) {
		if (ImpactEffect) {
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, 
				HitResult.Location, HitResult.ImpactNormal.Rotation());
		}
		if (ImpactSound) {
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactSound, HitResult.Location);
		}

		AActor* HitActor = HitResult.GetActor();
		if (HitActor) {
			APawn* OwnerPawn = Cast<APawn>(GetOwner());
			if (!OwnerPawn) return;

			AController* OwnerController = OwnerPawn->GetController();
			if (!OwnerController) return;

			FPointDamageEvent DamageEvent(Damage, HitResult, HitResult.ImpactNormal, nullptr);
			HitActor->TakeDamage(Damage, DamageEvent, OwnerController, this);
		}
	}
}

void AGun::PerformLineTrace(FHitResult& OutHit)
{
	FName MuzzleSocketName = TEXT("Muzzle");
	if (ProjectileClass != nullptr) {
		UWorld* const World = GetWorld();
		if (World != nullptr) {
			//총구의 위치와 회전값
			FVector SpawnLocation = Mesh->GetSocketLocation(MuzzleSocketName);
			FRotator SpawnRotation = Mesh->GetSocketRotation(MuzzleSocketName);

			//소환 옵션
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			//총알의 주인을 이 총을 들고있는 캐릭터로 설정
			ActorSpawnParams.Instigator = Cast<APawn>(GetOwner());
			ActorSpawnParams.Owner = GetOwner();

			//월드에 투사체 액터 생성
			World->SpawnActor<AProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
		}
	
	}

	

	/*
	//총구가 향하는 방향
	FVector ShotDirection =(-1.f)* MuzzleRotation.Vector();

	//총구 위치+ 총구방향*사거리
	FVector EndPoint = MuzzleLocation + (ShotDirection * Range);
		
		
	GetWorld()->LineTraceSingleByChannel(OutHit, MuzzleLocation,
		EndPoint, ECollisionChannel::ECC_Visibility);

	DrawDebugLine(GetWorld(), MuzzleLocation, EndPoint, FColor::Red, false, 2.0f, 0, 1.0f);
	*/
}
