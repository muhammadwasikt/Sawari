#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SawariTrafficManager.generated.h"

class ASawariSaddarOSMActor;
class ASawariTrafficCar;

/** Spawns a capped number of moving AI traffic vehicles on imported OSM roads. */
UCLASS()
class SAWARI_API ASawariTrafficManager : public AActor
{
    GENERATED_BODY()
public:
    ASawariTrafficManager();
    void InitializeTraffic(ASawariSaddarOSMActor* City);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sawari|Traffic")
    int32 MaximumTrafficCars = 18;

private:
    UPROPERTY()
    TArray<TObjectPtr<ASawariTrafficCar>> SpawnedCars;
};
