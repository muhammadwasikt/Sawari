#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SawariRideSubsystem.generated.h"

UENUM(BlueprintType)
enum class ESawariRideState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Requested UMETA(DisplayName = "Requested"),
    Accepted UMETA(DisplayName = "Accepted"),
    PassengerOnboard UMETA(DisplayName = "Passenger Onboard"),
    Completed UMETA(DisplayName = "Completed")
};
USTRUCT(BlueprintType)
struct FSawariRideRequest
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly, Category = "Sawari|Ride")
    FString PassengerName = TEXT("Ahmed K.");
    UPROPERTY(BlueprintReadOnly, Category = "Sawari|Ride")
    float PassengerRating = 4.8f;
    UPROPERTY(BlueprintReadOnly, Category = "Sawari|Ride")
    FString PickupArea = TEXT("Saddar");
    UPROPERTY(BlueprintReadOnly, Category = "Sawari|Ride")
    FString DestinationArea = TEXT("Clifton");
    UPROPERTY(BlueprintReadOnly, Category = "Sawari|Ride")
    float PickupDistanceKm = 1.2f;
    UPROPERTY(BlueprintReadOnly, Category = "Sawari|Ride")
    float TripDistanceKm = 5.0f;
    UPROPERTY(BlueprintReadOnly, Category = "Sawari|Ride")
    float EstimatedMinutes = 18.0f;
    UPROPERTY(BlueprintReadOnly, Category = "Sawari|Ride")
    float SurgeMultiplier = 1.0f;
    UPROPERTY(BlueprintReadOnly, Category = "Sawari|Ride")
    int32 EstimatedFareCoins = 0;
};
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSawariRideStateChanged, ESawariRideState, NewState, const FSawariRideRequest&, Ride);

/** Single-player ride flow. HUD and navigation widgets can bind to OnRideStateChanged. */
UCLASS()
class SAWARI_API USawariRideSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadOnly, Category = "Sawari|Ride")
    ESawariRideState RideState = ESawariRideState::Idle;
    UPROPERTY(BlueprintReadOnly, Category = "Sawari|Ride")
    FSawariRideRequest CurrentRide;
    UPROPERTY(BlueprintReadOnly, Category = "Sawari|Wallet")
    int32 WalletCoins = 0;
    UPROPERTY(BlueprintAssignable, Category = "Sawari|Ride")
    FSawariRideStateChanged OnRideStateChanged;
    UFUNCTION(BlueprintCallable, Category = "Sawari|Ride")
    bool GenerateDemoRide();
    UFUNCTION(BlueprintCallable, Category = "Sawari|Ride")
    bool AcceptRide();
    UFUNCTION(BlueprintCallable, Category = "Sawari|Ride")
    bool MarkPassengerPickedUp();
    UFUNCTION(BlueprintCallable, Category = "Sawari|Ride")
    bool CompleteRide(int32 TipCoins = 0);
    UFUNCTION(BlueprintCallable, Category = "Sawari|Ride")
    bool DeclineRide();
    UFUNCTION(BlueprintPure, Category = "Sawari|Wallet")
    int32 GetLastDriverPayoutCoins() const { return LastDriverPayoutCoins; }
    UFUNCTION(BlueprintPure, Category = "Sawari|Wallet")
    int32 GetLastPlatformFeeCoins() const { return LastPlatformFeeCoins; }
private:
    UPROPERTY()
    int32 LastDriverPayoutCoins = 0;
    UPROPERTY()
    int32 LastPlatformFeeCoins = 0;
    int32 CalculateFareCoins(float DistanceKm, float Minutes, float Surge, int32 TipCoins) const;
    void BroadcastState();
};
