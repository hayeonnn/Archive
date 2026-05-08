// Fill out your copyright notice in the Description page of Project Settings.


#include "FoliageManager.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AFoliageManager::AFoliageManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
    // 루트 컴포넌트를 생성합니다.
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}


// Called when the game starts or when spawned
void AFoliageManager::BeginPlay()
{
	Super::BeginPlay();

    // 플레이어 폰을 가져옵니다.
    PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);

    // 초기 플레이어 셀 위치를 유효하지 않은 값으로 설정하여 처음 한번은 무조건 실행되도록 함
    CurrentPlayerCell = FIntPoint(INT_MAX, INT_MAX);

    // 폴리지 스폰 함수를 주기적으로 호출하기 위해 타이머 설정
    FTimerHandle FoliageSpawnTimer;
    GetWorldTimerManager().SetTimer(FoliageSpawnTimer, this, &AFoliageManager::UpdateFoliage, 1.0f, true, 1.0f);

}

// Called every frame
void AFoliageManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FIntPoint AFoliageManager::WorldToCell(const FVector& WorldLocation) const
{
    return FIntPoint(
        FMath::FloorToInt(WorldLocation.X / CellSize),
        FMath::FloorToInt(WorldLocation.Y / CellSize)
    );
}

void AFoliageManager::UpdateFoliage()
{
    if (!PlayerPawn) return;

    FIntPoint NewPlayerCell = WorldToCell(PlayerPawn->GetActorLocation());

    // 플레이어가 새로운 셀로 이동했을 때만 업데이트 진행
    if (NewPlayerCell == CurrentPlayerCell)
    {
        return;
    }

    CurrentPlayerCell = NewPlayerCell;

    TSet<FIntPoint> RequiredCells;
    int32 RadiusInCells = FMath::CeilToInt(SpawnRadius / CellSize);

    // 1. 현재 플레이어 주변에 '필요한' 셀 목록(RequiredCells)
    for (int32 y = -RadiusInCells; y <= RadiusInCells; ++y)
    {
        for (int32 x = -RadiusInCells; x <= RadiusInCells; ++x)
        {
            if (FVector2D(x, y).Size() <= RadiusInCells)
            {
                RequiredCells.Add(CurrentPlayerCell + FIntPoint(x, y));
            }
        }
    }

    // 2. '제거'할 셀 찾기
    TArray<FIntPoint> CellsToRemove;
    for (const auto& Pair : ActiveCells)
    {
        if (!RequiredCells.Contains(Pair.Key))
        {
            CellsToRemove.Add(Pair.Key);
        }
    }

    // 3. 찾아낸 셀들을 제거
    for (const FIntPoint& CellCoord : CellsToRemove)
    {
        if (ActiveCells.Contains(CellCoord))
        {
            ActiveCells[CellCoord].DestroyComponents();
            ActiveCells.Remove(CellCoord);
        }
    }

    // 4. '생성'할 셀을 찾아서 생성
    for (const FIntPoint& CellCoord : RequiredCells)
    {
        if (!ActiveCells.Contains(CellCoord))
        {
            FCellFoliage NewCellFoliage;
            // 이 새로운 셀에 폴리지를 생성
            for (const FMyFoliageType& FoliageType : FoliageTypes)
            {
                // 셀마다, 폴리지 타입마다 새로운 InstancedStaticMeshComponent를 생성
                UInstancedStaticMeshComponent* ISMComponent = NewObject<UInstancedStaticMeshComponent>(this);
                ISMComponent->RegisterComponent();
                ISMComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
                ISMComponent->SetStaticMesh(FoliageType.FoliageMesh);
                ISMComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

                // 셀 영역 내에 인스턴스들을 배치
                float CellArea = FMath::Square(CellSize / 100.0f);
                int32 NumToSpawn = FMath::RoundToInt(CellArea * FoliageType.Density);


                for (int32 i = 0; i < NumToSpawn; ++i)
                {
                    FVector CellBaseLocation(CellCoord.X * CellSize, CellCoord.Y * CellSize, PlayerPawn->GetActorLocation().Z);
                    FVector RandomOffset(FMath::FRandRange(0, CellSize), FMath::FRandRange(0, CellSize), 0);
                    FVector SpawnOrigin = CellBaseLocation + RandomOffset;

                    // 라인 트레이스로 바닥 찾기
                    FVector Start = SpawnOrigin + FVector(0, 0, 2000.0f);
                    FVector End = SpawnOrigin - FVector(0, 0, 2000.0f);
                    FHitResult HitResult;
                    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility))
                    {
                        FTransform InstanceTransform(
                            FRotator(0, FMath::RandRange(0.f, 360.f), 0),
                            HitResult.Location,
                            FVector(FMath::RandRange(FoliageType.ScaleRange.X, FoliageType.ScaleRange.Y))
                        );
                        ISMComponent->AddInstance(InstanceTransform);
                    }
                }
                NewCellFoliage.InstanceComponents.Add(ISMComponent);
            }
            // 생성된 셀 정보를 ActiveCells 맵에 추가
            ActiveCells.Add(CellCoord, NewCellFoliage);
        }
    }
}

