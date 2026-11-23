#include "SawariRideSubsystem.h"

namespace SawariEconomy
{
    constexpr int32 BaseFareCoins = 70;
    constexpr int32 RatePerKmCoins = 15;
    constexpr int32 RatePerMinuteCoins = 2;
    constexpr float DriverShare = 0.85f;
}

bool USawariRideSubsystem::GenerateDemoRide()
{
    if (RideState == ESawariRideState::Accepted || RideState == ESawariRideState::PassengerOnboard)
        return false;

    CurrentRide = FSawariRideRequest();
    CurrentRide.EstimatedFareCoins = CalculateFareCoins(
        CurrentRide.TripDistanceKm, CurrentRide.EstimatedMinutes, CurrentRide.SurgeMultiplier, 0);
    RideState = ESawariRideState::Requested;
    LastDriverPayoutCoins = 0;
    LastPlatformFeeCoins = 0;
    BroadcastState();
    return true;
}

bool USawariRideSubsystem::AcceptRide()
{
    if (RideState != ESawariRideState::Requested) return false;
    RideState = ESawariRideState::Accepted;
    BroadcastState();
    return true;
}

bool USawariRideSubsystem::MarkPassengerPickedUp()
{
    if (RideState != ESawariRideState::Accepted) return false;
    RideState = ESawariRideState::PassengerOnboard;
    BroadcastState();
    return true;
}

bool USawariRideSubsystem::CompleteRide(int32 TipCoins)
{
    if (RideState != ESawariRideState::PassengerOnboard || TipCoins < 0) return false;

    const int32 FinalFare = CalculateFareCoins(
        CurrentRide.TripDistanceKm, CurrentRide.EstimatedMinutes, CurrentRide.SurgeMultiplier, TipCoins);
    LastDriverPayoutCoins = FMath::FloorToInt(FinalFare * SawariEconomy::DriverShare);
    LastPlatformFeeCoins = FinalFare - LastDriverPayoutCoins;
    WalletCoins += LastDriverPayoutCoins;
    RideState = ESawariRideState::Completed;
    BroadcastState();
    return true;
}

bool USawariRideSubsystem::DeclineRide()
{
    if (RideState != ESawariRideState::Requested) return false;
    RideState = ESawariRideState::Idle;
    BroadcastState();
    return true;
}

int32 USawariRideSubsystem::CalculateFareCoins(float DistanceKm, float Minutes, float Surge, int32 TipCoins) const
{
    const float SafeSurge = FMath::Clamp(Surge, 1.0f, 2.0f);
    const float BaseAndUsage = static_cast<float>(SawariEconomy::BaseFareCoins)
        + (SawariEconomy::RatePerKmCoins * FMath::Max(0.0f, DistanceKm))
        + (SawariEconomy::RatePerMinuteCoins * FMath::Max(0.0f, Minutes));
    return FMath::Max(0, FMath::RoundToInt(BaseAndUsage * SafeSurge) + TipCoins);
}

void USawariRideSubsystem::BroadcastState()
{
    OnRideStateChanged.Broadcast(RideState, CurrentRide);
}
