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
    UpdateNavigation(DeltaSeconds);
}

void ASawariGameMode::UpdateNavigation(float DeltaSeconds)
{
    if (!Taxi)
    {
        bNavigationRouteValid = false;
        NavigationInstruction = TEXT("Taxi is not ready");
        return;
    }

    USawariRideSubsystem* Rides = GetGameInstance()
        ? GetGameInstance()->GetSubsystem<USawariRideSubsystem>()
        : nullptr;
    if (!Rides)
    {
        NavigationInstruction = TEXT("Ride service initializing");
        return;
    }

    const bool bToPickup = Rides->RideState == ESawariRideState::Requested
        || Rides->RideState == ESawariRideState::Accepted;
    const bool bToDropoff = Rides->RideState == ESawariRideState::PassengerOnboard;
    if (!bToPickup && !bToDropoff)
    {
        bNavigationTargetSet = false;
        bNavigationRouteValid = false;
        NavigationRoute.Reset();
        NavigationDistanceMeters = 0.0f;
        NavigationInstruction = TEXT("Accept a ride to start navigation");
        return;
    }

    const FVector Target = bToDropoff ? DropoffPoint : PickupPoint;
    const FVector TaxiLocation = Taxi->GetActorLocation();
    const bool bTargetChanged = !bNavigationTargetSet
        || bNavigationTargetIsDropoff != bToDropoff
        || FVector::DistSquared2D(NavigationTarget, Target) > 1.0;

    NavigationRetryTimer += DeltaSeconds;

    int32 ClosestSegment = INDEX_NONE;
    float ClosestAlpha = 0.0f;
    FVector ClosestPoint = FVector::ZeroVector;
    double ClosestDistanceSquared = TNumericLimits<double>::Max();

    if (bNavigationRouteValid && NavigationRoute.Num() >= 2)
    {
        for (int32 Index = 0; Index < NavigationRoute.Num() - 1; ++Index)
        {
            const FVector A(NavigationRoute[Index].X, NavigationRoute[Index].Y, 0.0);
            const FVector B(NavigationRoute[Index + 1].X, NavigationRoute[Index + 1].Y, 0.0);
            const FVector Segment = B - A;
            const double LengthSquared = Segment.SizeSquared2D();
            if (LengthSquared <= 1.0) continue;

            const float Alpha = static_cast<float>(FMath::Clamp(
                FVector::DotProduct(FVector(TaxiLocation.X, TaxiLocation.Y, 0.0) - A, Segment)
                    / LengthSquared,
                0.0, 1.0));
            const FVector Candidate = A + Segment * Alpha;
            const double DistanceSquared = FVector::DistSquared2D(Candidate, TaxiLocation);
            if (DistanceSquared < ClosestDistanceSquared)
            {
                ClosestDistanceSquared = DistanceSquared;
                ClosestSegment = Index;
                ClosestAlpha = Alpha;
                ClosestPoint = Candidate;
            }
        }
    }

    const bool bMovedFarOffRoute = bNavigationRouteValid
        && ClosestDistanceSquared > FMath::Square(15000.0)
        && FVector::DistSquared2D(TaxiLocation, LastNavigationBuildLocation) > FMath::Square(5000.0);
    const bool bRetryFailedRoute = !bNavigationRouteValid && NavigationRetryTimer >= 5.0f;
    const bool bRebuildRoute = bTargetChanged || bMovedFarOffRoute || bRetryFailedRoute;

    if (bRebuildRoute)
    {
        NavigationTarget = Target;
        bNavigationTargetIsDropoff = bToDropoff;
        bNavigationTargetSet = true;
        LastNavigationBuildLocation = TaxiLocation;
        NavigationRetryTimer = 0.0f;
        NavigationRoute.Reset();
        bNavigationRouteValid = RealCity && RealCity->IsOSMDataLoaded()
            && RealCity->FindDrivingRoute(TaxiLocation, Target, NavigationRoute);
    }

    if (!bNavigationRouteValid || ClosestSegment == INDEX_NONE)
    {
        NavigationDistanceMeters = FVector::Dist2D(TaxiLocation, Target) / 100.0f;
        NavigationInstruction = RealCity && RealCity->IsOSMDataLoaded()
            ? TEXT("No connected road route; follow the destination beacon")
            : TEXT("OSM road data not loaded; follow the destination beacon");
        return;
    }

    // Remaining path length begins at the taxi's closest point on the route
    // and includes every road segment still ahead.
    double RemainingCm = FVector::Dist2D(TaxiLocation, ClosestPoint)
        + FVector::Dist2D(ClosestPoint, NavigationRoute[ClosestSegment + 1]);
    for (int32 Index = ClosestSegment + 1; Index < NavigationRoute.Num() - 1; ++Index)
    {
        RemainingCm += FVector::Dist2D(NavigationRoute[Index], NavigationRoute[Index + 1]);
    }
    NavigationDistanceMeters = static_cast<float>(RemainingCm / 100.0);

    NavigationInstruction = bToDropoff
        ? TEXT("Continue to the passenger destination")
        : TEXT("Continue to the passenger pickup");

    if (NavigationDistanceMeters <= 80.0f)
    {
        NavigationInstruction = bToDropoff ? TEXT("Arrive at the passenger destination")
            : TEXT("Arrive at the passenger pickup");
        return;
    }

    // Inspect upcoming connected road bends and announce the first meaningful turn.
    double DistanceToBendCm = FVector::Dist2D(ClosestPoint, NavigationRoute[ClosestSegment + 1]);
    for (int32 Index = ClosestSegment + 1; Index < NavigationRoute.Num() - 1; ++Index)
    {
        const FVector Incoming = (NavigationRoute[Index] - NavigationRoute[Index - 1]).GetSafeNormal2D();
        const FVector Outgoing = (NavigationRoute[Index + 1] - NavigationRoute[Index]).GetSafeNormal2D();
        const float Cross = Incoming.X * Outgoing.Y - Incoming.Y * Outgoing.X;
        if (FMath::Abs(Cross) >= 0.38f && DistanceToBendCm >= 250.0)
        {
            const float DistanceToBendMeters = static_cast<float>(DistanceToBendCm / 100.0);
            NavigationInstruction = Cross > 0.0f
                ? FString::Printf(TEXT("Turn right in %.0f m"), DistanceToBendMeters)
                : FString::Printf(TEXT("Turn left in %.0f m"), DistanceToBendMeters);
            break;
        }
        DistanceToBendCm += FVector::Dist2D(NavigationRoute[Index], NavigationRoute[Index + 1]);
    }
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
