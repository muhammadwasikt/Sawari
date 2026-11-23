#pragma once
#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "SawariTaxiPawn.generated.h"
class UCameraComponent;
class USpringArmComponent;
UENUM(BlueprintType)
enum class ESawariCameraMode : uint8
{
    Chase,
    Bonnet,
    Cockpit
};
/** Blueprint child requires a real vehicle mesh, Physics Asset and Chaos wheel setup. */
UCLASS(Abstract, Blueprintable)
class SAWARI_API ASawariTaxiPawn : public AWheeledVehiclePawn
{
    GENERATED_BODY()
public:
    ASawariTaxiPawn();
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    UFUNCTION(BlueprintCallable, Category = "Sawari|Camera")
    void CycleCamera();
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
};
