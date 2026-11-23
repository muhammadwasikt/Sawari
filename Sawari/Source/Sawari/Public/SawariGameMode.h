#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SawariGameMode.generated.h"

class APawn;
class ASawariArcadeTaxiPawn;
class ASawariPassengerNPC;
class ASawariPrototypeCity;
class ASawariSaddarOSMActor;
class ASawariTrafficManager;

/** Starts real OSM Karachi when staged data exists; retains a blockout fallback. */
UCLASS()
class SAWARI_API ASawariGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ASawariGameMode();
    virtual void StartPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UFUNCTION(BlueprintPure, Category = "Sawari|Ride")
    FVector GetPickupCheckpoint() const { return PickupPoint; }

    UFUNCTION(BlueprintPure, Category = "Sawari|Ride")
    FVector GetDropoffCheckpoint() const { return DropoffPoint; }

    UFUNCTION(BlueprintPure, Category = "Sawari|Driving")
    float GetTaxiSpeedKmh() const;

private:
    UPROPERTY()
    TObjectPtr<APawn> Taxi;

    UPROPERTY()
    TObjectPtr<ASawariPrototypeCity> FallbackCity;

    UPROPERTY()
    TObjectPtr<ASawariSaddarOSMActor> RealCity;

    UPROPERTY()
    TObjectPtr<ASawariTrafficManager> TrafficManager;

    UPROPERTY()
    TObjectPtr<ASawariPassengerNPC> Passenger;

    FVector TaxiStart = FVector(-180000.0f, 0.0f, 170.0f);
    FVector PickupPoint = FVector(-60000.0f, 1450.0f, 0.0f);
    FVector DropoffPoint = FVector(440000.0f, 0.0f, 0.0f);

    void SpawnPrototypeWorld();
    void TrySpawnTaxi();
    void UpdateRideCheckpoints();
};
