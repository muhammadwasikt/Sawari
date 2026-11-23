#include "SawariGameMode.h"

#include "SawariArcadeTaxiPawn.h"
#include "SawariPassengerNPC.h"
#include "SawariPrototypeCity.h"
#include "SawariPrototypeHUD.h"
#include "SawariRideSubsystem.h"
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
    if (!City)
    {
        City = World->SpawnActor<ASawariPrototypeCity>(
            ASawariPrototypeCity::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);
    }
    if (!Passenger)
    {
        Passenger = World->SpawnActor<ASawariPassengerNPC>(
            ASawariPassengerNPC::StaticClass(), PickupPoint, FRotator(0.0f, 180.0f, 0.0f), Params);
    }
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
    Taxi = GetWorld()->SpawnActor<ASawariArcadeTaxiPawn>(
        ASawariArcadeTaxiPawn::StaticClass(), TaxiStart, FRotator::ZeroRotator, Params);
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
    if (City)
    {
        City->SetRideMarkers(
            State == ESawariRideState::Requested || State == ESawariRideState::Accepted,
            State == ESawariRideState::PassengerOnboard);
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
