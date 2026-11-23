#pragma once
#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "ChaosVehicleWheel.h"
#include "SawariTaxiPawn.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UChaosWheeledVehicleMovementComponent;

UENUM(BlueprintType)
enum class ESawariCameraMode : uint8
{
    Chase,
    Bonnet,
    Cockpit
};

/** Front axle wheel: driven and steered, no handbrake lock. */
UCLASS()
class SAWARI_API USawariFrontWheel : public UChaosVehicleWheel
{
    GENERATED_BODY()
public:
    USawariFrontWheel();
};

/** Rear axle wheel: driven and handbrake-affected, not steered. */
UCLASS()
class SAWARI_API USawariRearWheel : public UChaosVehicleWheel
{
    GENERATED_BODY()
public:
    USawariRearWheel();
};

/**
 * Production taxi physics base for the imported SK_SawariTaxi skeletal mesh.
 * The mesh must include Chassis and four Wheel_* bones and an imported Physics Asset.
 */
UCLASS(Blueprintable)
class SAWARI_API ASawariTaxiPawn : public AWheeledVehiclePawn
{
    GENERATED_BODY()

public:
    ASawariTaxiPawn();

    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    UFUNCTION(BlueprintCallable, Category = "Sawari|Camera")
    void CycleCamera();

    UFUNCTION(BlueprintPure, Category = "Sawari|Driving")
    float GetSpeedKmh() const;

    UFUNCTION(BlueprintPure, Category = "Sawari|Vehicle")
    bool IsChaosVehicleConfigured() const;

    static bool IsTaxiAssetInstalled();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sawari|Camera")
    TObjectPtr<USpringArmComponent> ChaseArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sawari|Camera")
    TObjectPtr<UCameraComponent> ChaseCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sawari|Camera")
    TObjectPtr<UCameraComponent> BonnetCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sawari|Camera")
    TObjectPtr<UCameraComponent> CockpitCamera;

    UPROPERTY(BlueprintReadOnly, Category = "Sawari|Camera")
    ESawariCameraMode ActiveCameraMode = ESawariCameraMode::Chase;

private:
    void SetThrottle(float Value);
    void SetBrake(float Value);
    void SetSteering(float Value);
    void SetHandbrakePressed();
    void SetHandbrakeReleased();
    void SetReversePressed();
    void SetReverseReleased();
    void ApplyCameraMode();

    UChaosWheeledVehicleMovementComponent* GetChaosMovement() const;
};
