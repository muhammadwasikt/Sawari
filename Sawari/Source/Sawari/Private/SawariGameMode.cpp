#include "SawariGameMode.h"

#include "SawariArcadeTaxiPawn.h"
#include "SawariPassengerNPC.h"
#include "SawariPrototypeCity.h"
#include "SawariPrototypeHUD.h"
#include "SawariRideSubsystem.h"
#include "SawariSaddarOSMActor.h"
#include "SawariTaxiPawn.h"
#include "SawariTrafficManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameInstance.h"

ASawariGameMode::ASawariGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    DefaultPawnClass = nullptr;
    HUDClass = ASawariPrototypeHUD::StaticClass();
}

void ASawariGameMode::StartPlay()
{
    Super::StartPlay();
    SpawnPrototypeWorld();

    if (UGameInstance* GI = GetGameInstance())
    {
        if (USawariRideSubsystem* Rides = GI->GetSubsystem<USawariRideSubsystem>())
        {
            Rides->GenerateDemoRide();
        }
    }
}

void ASawariGameMode::SpawnPrototypeWorld()
{
    UWorld* World = GetWorld();
    if (!World) return;

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    RealCity = World->SpawnActor<ASawariSaddarOSMActor>(
        ASawariSaddarOSMActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);

    if (RealCity && RealCity->IsOSMDataLoaded())
    {
        TaxiStart = RealCity->GetTaxiStartLocation();
        PickupPoint = RealCity->GetPickupLocation();
        DropoffPoint = RealCity->GetDropoffLocation();

        TrafficManager = World->SpawnActor<ASawariTrafficManager>(
            ASawariTrafficManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);
        if (TrafficManager)
        {
            TrafficManager->InitializeTraffic(RealCity);
        }

        UE_LOG(LogTemp, Display, TEXT("Sawari is using the real Saddar-Clifton OSM corridor."));
    }
    else
    {
        if (RealCity)
        {
            RealCity->Destroy();
            RealCity = nullptr;
        }

        FallbackCity = World->SpawnActor<ASawariPrototypeCity>(
            ASawariPrototypeCity::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);
        UE_LOG(LogTemp, Warning, TEXT("OSM data not staged; using the prototype city blockout. Run Scripts/fetch_saddar_osm.py for real roads/buildings."));
    }

    const FVector PassengerLocation = PickupPoint + FVector(0.0f, 430.0f, 12.0f);
    Passenger = World->SpawnActor<ASawariPassengerNPC>(
        ASawariPassengerNPC::StaticClass(), PassengerLocation, FRotator(0.0f, 180.0f, 0.0f), Params);

    TrySpawnTaxi();
}

void ASawariGameMode::TrySpawnTaxi()
{
    if (Taxi || !GetWorld()) return;
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;

    if (APawn* OldPawn = PC->GetPawn())
    {
        OldPawn->Destroy();
    }

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    if (ASawariTaxiPawn::IsTaxiAssetInstalled())
    {
        ASawariTaxiPawn* ChaosTaxi = GetWorld()->SpawnActor<ASawariTaxiPawn>(
            ASawariTaxiPawn::StaticClass(), TaxiStart, FRotator::ZeroRotator, Params);
        if (ChaosTaxi && ChaosTaxi->IsChaosVehicleConfigured())
        {
            Taxi = ChaosTaxi;
            UE_LOG(LogTemp, Display, TEXT("Sawari Chaos four-wheel taxi selected."));
        }
        else if (ChaosTaxi)
        {
            UE_LOG(LogTemp, Error, TEXT("Chaos taxi asset is present but the four-wheel rig is not configured; using fallback pawn."));
            ChaosTaxi->Destroy();
        }
    }

    if (!Taxi)
    {
        Taxi = GetWorld()->SpawnActor<ASawariArcadeTaxiPawn>(
            ASawariArcadeTaxiPawn::StaticClass(), TaxiStart, FRotator::ZeroRotator, Params);
    }

    if (Taxi)
    {
        PC->Possess(Taxi);
    }
    PC->SetShowMouseCursor(false);
    PC->SetInputMode(FInputModeGameOnly());
}

void ASawariGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!Taxi) TrySpawnTaxi();
    UpdateRideCheckpoints();
}

void ASawariGameMode::UpdateRideCheckpoints()
{
    if (!Taxi) return;
    USawariRideSubsystem* Rides = GetGameInstance() ? GetGameInstance()->GetSubsystem<USawariRideSubsystem>() : nullptr;
    if (!Rides) return;

    const ESawariRideState State = Rides->RideState;
    const bool bShowPickup = State == ESawariRideState::Requested || State == ESawariRideState::Accepted;
    const bool bShowDropoff = State == ESawariRideState::PassengerOnboard;

    if (RealCity)
    {
        RealCity->SetRideMarkers(bShowPickup, bShowDropoff);
    }
    if (FallbackCity)
    {
        FallbackCity->SetRideMarkers(bShowPickup, bShowDropoff);
    }

    if ((State == ESawariRideState::Requested || State == ESawariRideState::Accepted)
        && Passenger && Passenger->IsHidden())
    {
        Passenger->SetActorHiddenInGame(false);
    }

    if (State == ESawariRideState::Accepted
        && FVector::Dist2D(Taxi->GetActorLocation(), PickupPoint) <= 1900.0f)
    {
        if (Rides->MarkPassengerPickedUp() && Passenger)
        {
            Passenger->SetActorHiddenInGame(true);
        }
    }
    else if (State == ESawariRideState::PassengerOnboard
        && FVector::Dist2D(Taxi->GetActorLocation(), DropoffPoint) <= 2200.0f)
    {
        Rides->CompleteRide();
    }
}

float ASawariGameMode::GetTaxiSpeedKmh() const
{
    if (const ASawariArcadeTaxiPawn* Arcade = Cast<ASawariArcadeTaxiPawn>(Taxi))
    {
        return Arcade->GetSpeedKmh();
    }
    if (const ASawariTaxiPawn* Chaos = Cast<ASawariTaxiPawn>(Taxi))
    {
        return Chaos->GetSpeedKmh();
    }
    return 0.0f;
}
