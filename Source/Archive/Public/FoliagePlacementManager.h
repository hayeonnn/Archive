// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FoliagePlacementManager.generated.h"

class UInstancedStaticMeshComponent;
class ALandscapeStreamingProxy;
UCLASS()
class ARCHIVE_API AFoliagePlacementManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFoliagePlacementManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(VisibleAnywhere)
	USceneComponent* SceneRoot;
	
	//폴리지를 렌더링할 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foliage")
	UInstancedStaticMeshComponent* FoliageComponent;

	//Mesh
	UPROPERTY(EditAnywhere, Category = "Foliage Settings")
	UStaticMesh* FoliageMesh;

	//Radius
	UPROPERTY(EditAnywhere, Category = "Foliage Settings")
	float PlacementRadius = 2500.0f; //25m

	//Count
	UPROPERTY(EditAnywhere, Category = "Foliage Settings")
	int32 InstanceCount = 500;

	//Min
	UPROPERTY(EditAnywhere, Category = "Foliage Settings")
	float MinScale = 0.8f;
		
	//Max
	UPROPERTY(EditAnywhere, Category = "Foliage Settings")
	float MaxScale = 1.5f;

private: 
	// 월드에 액터가 스폰될 때 호출될 함수
	UFUNCTION()
	void OnLevelAdded(ULevel* InLevel, UWorld* InWorld);

	// 레벨이 언로드될 때 호출될 함수
	UFUNCTION()
	void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld);

	// 주어진 랜드스케이프 프록시 영역 내에 폴리지를 배치하는 함수
	void PlaceFoliageForProxy(ALandscapeStreamingProxy* LandscapeProxy);

	// 특정 ULevel에 연결된 폴리지를 제거하는 함수
	void RemoveFoliageForLevel(ULevel* LevelToRemove);

	// 각 스트리밍 레벨별로 생성된 폴리지 컴포넌트들을 관리하기 위한 맵
	UPROPERTY()
	TMap<TObjectPtr<ULevel>, TObjectPtr<UInstancedStaticMeshComponent>> FoliageComponentsMap;

};
