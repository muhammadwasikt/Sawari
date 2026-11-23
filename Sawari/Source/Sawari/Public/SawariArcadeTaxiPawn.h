#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "SawariArcadeTaxiPawn.generated.h"

class UBoxComponent;
class UCameraComponent;
class UStaticMeshComponent;
class USpringArmComponent;
class USawariRideSubsystem;

/**
 * Self-contained Phase 0 taxi pawn. Uses a physics-driven chassis and visible
 * modular body parts so the prototype can be played before a licensed vehicle
 * skeletal mesh and Chaos wheel rig are available.
 */
UCLASS()
class SAWARI_API ASawariArcadeTaxiPawn : public APawn
{
    GENERATED_BODY()

public:
    ASawariArcadeTaxiPawn();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    UFUNCTION(BlueprintPure, Category = "Sawari|Driving")
    float GetSpeedKmh() const;

    UFUNCTION(BlueprintCallable, Category = "Sawari|Camera")
    void CycleCamera();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sawari|Vehicle")
    TObjectPtr<UBoxComponent> PhysicsBody;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sawari|Vehicle")
    TObjectPtr<UStaticMeshComponent> ChassisVisual;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sawari|Vehicle")
    TObjectPtr<UStaticMeshComponent> RoofVisual;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sawari|Camera")
    TObjectPtr<USpringArmComponent> ChaseArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sawari|Camera")
    TObjectPtr<UCameraComponent> ChaseCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sawari|Camera")
    TObjectPtr<UCameraComponent> BonnetCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sawari|Camera")
    TObjectPtr<UCameraComponent> CockpitCamera;

private:
    UPROPERTY()
    TArray<TObjectPtr<UStaticMeshComponent>> WheelVisuals;

    float DriveInput = 0.0f;
    float SteeringInput = 0.0f;
    bool bHandbrake = false;
    float WheelSpinDegrees = 0.0f;
    uint8 CameraIndex = 0;

    void SetDriveInput(float Value);
    void SetSteeringInput(float Value);
    void SetHandbrakePressed();
    void SetHandbrakeReleased();
    void RequestRide();
    void AcceptRide();
    void DeclineRide();
    void ApplyCameraMode();
    void BuildTaxiVisuals();
    UMaterialInterface* MakeTaxiMaterial(const FLinearColor& Color) const;
};
