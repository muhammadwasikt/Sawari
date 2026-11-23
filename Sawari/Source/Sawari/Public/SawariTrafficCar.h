#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SawariTrafficCar.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UStaticMesh;

/** Kinematic NPC traffic vehicle that follows an OSM road polyline. */
UCLASS()
class SAWARI_API ASawariTrafficCar : public AActor
{
    GENERATED_BODY()
public:
    ASawariTrafficCar();
    virtual void Tick(float DeltaSeconds) override;

    void InitializeRoute(const TArray<FVector>& InRoute, int32 StartIndex, float SpeedKmh);

protected:
    UPROPERTY(VisibleAnywhere, Category = "Sawari|Traffic")
    TObjectPtr<UBoxComponent> CollisionBody;
    UPROPERTY(VisibleAnywhere, Category = "Sawari|Traffic")
    TObjectPtr<UStaticMeshComponent> BodyVisual;
    UPROPERTY(VisibleAnywhere, Category = "Sawari|Traffic")
    TObjectPtr<UStaticMeshComponent> RoofVisual;

private:
    UPROPERTY()
    TArray<TObjectPtr<UStaticMeshComponent>> Wheels;

    TArray<FVector> RoutePoints;
    int32 CurrentPointIndex = 0;
    float SpeedCentimetersPerSecond = 1100.0f;
    float BlockedTime = 0.0f;

    void BuildVisuals();
};
