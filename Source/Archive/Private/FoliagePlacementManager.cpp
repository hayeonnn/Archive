// Fill out your copyright notice in the Description page of Project Settings.


#include "FoliagePlacementManager.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "LandscapeStreamingProxy.h"
#include "Engine/StaticMesh.h"
#include "LandscapeProxy.h"
#include "Engine/World.h"
#include "Engine/Level.h" // ULevel 헤더 추가
#include "Kismet/KismetMathLibrary.h" //랜덤 함수를 위해 필요

// Sets default values
AFoliagePlacementManager::AFoliagePlacementManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	FoliageComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("FoliageISM"));
	FoliageComponent->SetupAttachment(RootComponent);
	FoliageComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}

// Called when the game starts or when spawned
void AFoliagePlacementManager::BeginPlay()
{
	Super::BeginPlay();

	if (!FoliageMesh) {
		UE_LOG(LogTemp, Warning, TEXT("PlaceFoliageInstances: FoliageMesh를 찾을 수 없습니다!"));
		return;
	}

	FoliageComponent->SetStaticMesh(FoliageMesh);

	// 월드가 유효할 때, 델리게이트 구독을 시작합니다.
	UWorld* World = GetWorld();
	if (World)
	{
		//World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateUObject(this, &AFoliagePlacementManager::OnActorSpawned));

		FWorldDelegates::LevelAddedToWorld.AddUObject(this, &AFoliagePlacementManager::OnLevelAdded);
		FWorldDelegates::LevelRemovedFromWorld.AddUObject(this, &AFoliagePlacementManager::OnLevelRemovedFromWorld);
	}
	UE_LOG(LogTemp, Log, TEXT("FoliagePlacementManager가 이벤트를 감지하기 시작합니다."));
}

void AFoliagePlacementManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{ 
	// 액터가 파괴될 때, 모든 폴리지 컴포넌트를 정리합니다.
	for (auto& Elem : FoliageComponentsMap)
	{
		if (Elem.Value && !Elem.Value->IsBeingDestroyed())
		{
			Elem.Value->DestroyComponent();
		}
	}
	FoliageComponentsMap.Empty();
	Super::EndPlay(EndPlayReason);
}

// Called every frame
void AFoliagePlacementManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AFoliagePlacementManager::OnLevelAdded(ULevel* InLevel, UWorld* InWorld)
{

	if (!InLevel) return;

	UE_LOG(LogTemp, Log, TEXT("OnLevelAdded: InLevel!!!!!!!!!!!!!!!!"));
	// 로드된 레벨 안에 있는 모든 액터를 순회합니다.
	for (AActor* Actor : InLevel->Actors)
	{
		// 액터가 유효하고, ALandscapeProxy 타입인지 확인합니다.
		if (Actor)
		{
			if (ALandscapeProxy* LandscapeProxy = Cast<ALandscapeProxy>(Actor))
			{
				UE_LOG(LogTemp, Log, TEXT("OnLevelAdded: LandscapeProxy!!!!!!!!!!!!!!!!"));
				// 스트리밍되는 랜드스케이프인지 다시 한번 확인합니다.
				if (LandscapeProxy->IsActorBeginningPlayFromLevelStreaming())
				{
				UE_LOG(LogTemp, Log, TEXT("OnLevelAdded: LandscapeProxy!!!!!!!!!!!!!!!!"));
					// 드디어 정확한 시점에, 정확한 액터를 찾았습니다!
					UE_LOG(LogTemp, Warning, TEXT("LevelAdded 감지: %s 레벨의 랜드스케이프 프록시 %s"), *InLevel->GetName(), *LandscapeProxy->GetName());

					// ALandscapeStreamingProxy로 캐스팅하여 PlaceFoliageForProxy 함수를 호출합니다.
					if (ALandscapeStreamingProxy* StreamingProxy = Cast<ALandscapeStreamingProxy>(LandscapeProxy))
					{
						PlaceFoliageForProxy(StreamingProxy);
					}
				}
			}
		}
	}
}

void AFoliagePlacementManager::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	if (InLevel)
	{
		RemoveFoliageForLevel(InLevel);
	}
}

void AFoliagePlacementManager::PlaceFoliageForProxy(ALandscapeStreamingProxy* LandscapeProxy)
{
	// FoliageMesh가 에디터에서 지정되었는지 확인
	if (!LandscapeProxy || !FoliageMesh) return;

	ULevel* ProxyLevel = LandscapeProxy->GetLevel();
	if (!ProxyLevel || FoliageComponentsMap.Contains(ProxyLevel)) return;

	// 1. 새 InstancedStaticMeshComponent(ISMC)를 동적으로 생성
	UInstancedStaticMeshComponent* NewFoliageComponent = NewObject<UInstancedStaticMeshComponent>(this);
	NewFoliageComponent->SetStaticMesh(FoliageMesh);
	NewFoliageComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NewFoliageComponent->RegisterComponentWithWorld(GetWorld());

	// 2. 랜드스케이프 프록시의 경계(Bounds) 안에서만 폴리지 배치
	FBoxSphereBounds ProxyBounds = LandscapeProxy->GetStreamingBounds();
	FVector Origin = ProxyBounds.Origin;
	FVector Extent = ProxyBounds.BoxExtent;

	TArray<FTransform> InstanceTransforms;
	for (int32 i = 0; i < InstanceCount; ++i)
	{
		float RandX = FMath::RandRange(Origin.X - Extent.X, Origin.X + Extent.X);
		float RandY = FMath::RandRange(Origin.Y - Extent.Y, Origin.Y + Extent.Y);

		FVector StartPoint(RandX, RandY, Origin.Z + Extent.Z + 100.f); // 조금 위에서 시작
		FVector EndPoint(RandX, RandY, Origin.Z - Extent.Z - 100.f);   // 조금 아래까지

		FHitResult HitResult;
		if (GetWorld()->LineTraceSingleByChannel(HitResult, StartPoint, EndPoint, ECC_Visibility))
		{
			if (HitResult.bBlockingHit && HitResult.GetActor() == LandscapeProxy)
			{
				FRotator Rotation = FRotationMatrix::MakeFromZ(HitResult.ImpactNormal).Rotator();
				Rotation.Yaw += FMath::RandRange(0.f, 360.f);
				float Scale = FMath::FRandRange(MinScale, MaxScale);

				InstanceTransforms.Add(FTransform(Rotation, HitResult.Location, FVector(Scale)));
			}
		}
	}

	// 3. 계산된 모든 트랜스폼을 한 번에 추가
	if (InstanceTransforms.Num() > 0)
	{
		NewFoliageComponent->AddInstances(InstanceTransforms, false);
	}

	// 4. 생성된 컴포넌트를 맵에 저장하여 관리
	FoliageComponentsMap.Add(ProxyLevel, NewFoliageComponent);
}

void AFoliagePlacementManager::RemoveFoliageForLevel(ULevel* LevelToRemove)
{
	// 맵에서 제거할 ISMC를 찾아 파괴
	if (TObjectPtr<UInstancedStaticMeshComponent>* FoundComponent = FoliageComponentsMap.Find(LevelToRemove))
	{
		if (*FoundComponent && !(*FoundComponent)->IsBeingDestroyed())
		{
			(*FoundComponent)->DestroyComponent();
		}
		FoliageComponentsMap.Remove(LevelToRemove);
	}
}
