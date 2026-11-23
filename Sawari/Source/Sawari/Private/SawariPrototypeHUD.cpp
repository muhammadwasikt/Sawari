#include "SawariPrototypeHUD.h"

#include "SawariGameMode.h"
#include "SawariRideSubsystem.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

void ASawariPrototypeHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas || !GEngine || !PlayerOwner) return;

    const UFont* Font = GEngine->GetMediumFont();
    const FVector2D Shadow(1.5f, 1.5f);
    auto Draw = [this, Font, &Shadow](const FString& Text, float X, float Y, float Scale, const FLinearColor& Color)
    {
        Canvas->K2_DrawText(
            const_cast<UFont*>(Font), Text, FVector2D(X, Y), FVector2D(Scale, Scale), Color,
            0.0f, FLinearColor::Black, Shadow, false, false, true, FLinearColor::Black);
    };

    const ASawariGameMode* GameMode = GetWorld()
        ? Cast<ASawariGameMode>(GetWorld()->GetAuthGameMode())
        : nullptr;

    Draw(TEXT("SAWARI  /  KARACHI TAXI SIMULATOR"), 38.0f, 30.0f, 1.25f, FLinearColor(1.0f, 0.78f, 0.25f));
    Draw(TEXT("W accelerate  S reverse  A/D steer  CTRL brake  SPACE handbrake  V camera"), 40.0f, 66.0f, 0.74f, FLinearColor::White);

    if (GameMode)
    {
        Draw(FString::Printf(TEXT("SPEED  %03.0f km/h"), GameMode->GetTaxiSpeedKmh()),
            40.0f, 104.0f, 0.95f, FLinearColor(0.45f, 0.95f, 0.82f));
    }

    USawariRideSubsystem* Rides = GetGameInstance()
        ? GetGameInstance()->GetSubsystem<USawariRideSubsystem>()
        : nullptr;
    if (!Rides)
    {
        Draw(TEXT("Ride service initializing..."), 40.0f, 145.0f, 0.85f, FLinearColor::White);
        return;
    }

    Draw(FString::Printf(TEXT("WALLET  %d COINS"), Rides->WalletCoins),
        40.0f, 145.0f, 0.95f, FLinearColor(0.42f, 0.95f, 0.54f));
    Draw(TEXT("R request ride    E accept    Q decline"),
        40.0f, 181.0f, 0.82f, FLinearColor::White);

    const APawn* Taxi = PlayerOwner->GetPawn();
    if (Taxi && GameMode)
    {
        if (Rides->RideState == ESawariRideState::Requested || Rides->RideState == ESawariRideState::Accepted)
        {
            const float DistanceMeters = FVector::Dist2D(Taxi->GetActorLocation(), GameMode->GetPickupCheckpoint()) / 100.0f;
            Draw(FString::Printf(TEXT("PICKUP DISTANCE  %.0f m"), DistanceMeters),
                40.0f, 216.0f, 0.82f, FLinearColor(1.0f, 0.78f, 0.25f));
        }
        else if (Rides->RideState == ESawariRideState::PassengerOnboard)
        {
            const float DistanceMeters = FVector::Dist2D(Taxi->GetActorLocation(), GameMode->GetDropoffCheckpoint()) / 100.0f;
            Draw(FString::Printf(TEXT("DESTINATION DISTANCE  %.0f m"), DistanceMeters),
                40.0f, 216.0f, 0.82f, FLinearColor(0.25f, 0.95f, 0.62f));
        }
    }

    switch (Rides->RideState)
    {
        case ESawariRideState::Idle:
            Draw(TEXT("STATUS  DRIVER OFFLINE - PRESS R FOR A RIDE"), 40.0f, 254.0f, 0.90f, FLinearColor(0.80f, 0.83f, 0.88f));
            break;
        case ESawariRideState::Requested:
            Draw(TEXT("NEW REQUEST  /  AHMED K.  /  4.8 STARS"), 40.0f, 254.0f, 0.90f, FLinearColor(1.0f, 0.78f, 0.25f));
            Draw(FString::Printf(TEXT("SADDAR PICKUP  ->  CLIFTON  /  EST. %d COINS"), Rides->CurrentRide.EstimatedFareCoins), 40.0f, 290.0f, 0.78f, FLinearColor::White);
            Draw(TEXT("Press E to accept or Q to decline"), 40.0f, 324.0f, 0.78f, FLinearColor::White);
            break;
        case ESawariRideState::Accepted:
            Draw(TEXT("RIDE ACCEPTED  /  DRIVE TO THE YELLOW PICKUP BEACON"), 40.0f, 254.0f, 0.90f, FLinearColor(1.0f, 0.78f, 0.25f));
            Draw(TEXT("Stop close to the visible passenger to begin the trip"), 40.0f, 290.0f, 0.78f, FLinearColor::White);
            break;
        case ESawariRideState::PassengerOnboard:
            Draw(TEXT("PASSENGER ONBOARD  /  FOLLOW THE ROAD TO GREEN DESTINATION"), 40.0f, 254.0f, 0.90f, FLinearColor(0.25f, 0.95f, 0.62f));
            Draw(TEXT("Stop at the green destination beacon to complete the ride"), 40.0f, 290.0f, 0.78f, FLinearColor::White);
            break;
        case ESawariRideState::Completed:
            Draw(FString::Printf(TEXT("RIDE COMPLETED  /  DRIVER EARNED %d COINS"), Rides->GetLastDriverPayoutCoins()), 40.0f, 254.0f, 0.90f, FLinearColor(0.25f, 0.95f, 0.62f));
            Draw(FString::Printf(TEXT("PLATFORM FEE %d COINS  /  PRESS R FOR NEXT RIDE"), Rides->GetLastPlatformFeeCoins()), 40.0f, 290.0f, 0.78f, FLinearColor::White);
            break;
        default:
            break;
    }
}
