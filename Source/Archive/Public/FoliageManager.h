// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "FoliageManager.generated.h"

class UInstancedStaticMeshComponent;
// 개별 폴리지 타입의 설정을 담을 구조체
USTRUCT(BlueprintType)
struct FMyFoliageType
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
	UStaticMesh* FoliageMesh;

	// 밀도 (제곱미터당 개수)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage", meta = (ClampMin = "0.01"))
	float Density = 1.0f;

	// 크기 범위
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
	FVector2D ScaleRange = FVector2D(0.8f, 1.2f);

	UPROPERTY()
	UInstancedStaticMeshComponent* InstanceComponent;
};

// 각 셀의 폴리지 정보를 담는 구조체
USTRUCT()
struct FCellFoliage
{
	GENERATED_BODY()

	// 이 셀에 속한 Instanced Static Mesh 컴포넌트들 (폴리지 타입별로 하나씩)
	UPROPERTY()
	TArray<UInstancedStaticMeshComponent*> InstanceComponents;

	// 컴포넌트들을 파괴하고 메모리에서 해제하는 함수
	void DestroyComponents()
	{
		for (UInstancedStaticMeshComponent* Comp : InstanceComponents)
		{
			if (Comp)
			{
				Comp->DestroyComponent();
			}
		}
		InstanceComponents.Empty();
	}
};

UCLASS()
class ARCHIVE_API AFoliageManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFoliageManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(EditAnywhere, Category = "Foliage Settings")
	TArray<FMyFoliageType> FoliageTypes;

private:
	// 플레이어 캐릭터의 참조
	UPROPERTY()
	APawn* PlayerPawn;

	// 폴리지를 스폰할 영역의 반경 (이제 그리드 탐색 반경으로 사용)
	UPROPERTY(EditAnywhere, Category = "Foliage Settings")
	float SpawnRadius = 5000.0f;

	// 그리드 셀의 크기 (cm 단위)
	UPROPERTY(EditAnywhere, Category = "Foliage Settings")
	float CellSize = 2000.0f;

	// 현재 활성화된 셀들의 정보 (좌표, 컴포넌트) 를 저장하는 맵
	TMap<FIntPoint, FCellFoliage> ActiveCells;

	// 폴리지 생성이 완료된 셀의 좌표를 저장
	//TSet<FIntPoint> PopulatedCells;

	// 플레이어의 현재 셀 위치
	FIntPoint CurrentPlayerCell;

	// 월드 위치를 그리드 셀 좌표로 변환하는 함수
	FIntPoint WorldToCell(const FVector& WorldLocation) const;

	// 폴리지를 스폰하는 함수
	void UpdateFoliage();

};
