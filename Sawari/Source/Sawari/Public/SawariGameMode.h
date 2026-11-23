#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SawariGameMode.generated.h"

class ASawariArcadeTaxiPawn;
class ASawariPassengerNPC;
class ASawariPrototypeCity;

/** Boots the self-contained playable Phase 0 world without a hand-built level. */
UCLASS()
class SAWARI_API ASawariGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ASawariGameMode();
    virtual void StartPlay() override;
    virtual void Tick(float DeltaSeconds) override;

private:
    UPROPERTY()
    TObjectPtr<ASawariArcadeTaxiPawn> Taxi;
    UPROPERTY()
    TObjectPtr<ASawariPrototypeCity> City;
    UPROPERTY()
    TObjectPtr<ASawariPassengerNPC> Passenger;

    const FVector TaxiStart = FVector(-180000.0f, 0.0f, 170.0f);
    const FVector PickupPoint = FVector(-60000.0f, 1450.0f, 0.0f);
    const FVector DropoffPoint = FVector(440000.0f, 0.0f, 0.0f);

    void SpawnPrototypeWorld();
    void TrySpawnTaxi();
    void UpdateRideCheckpoints();
};
