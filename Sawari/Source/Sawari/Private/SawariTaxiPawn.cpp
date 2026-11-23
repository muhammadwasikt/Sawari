#include "SawariTaxiPawn.h"
#include "Camera/CameraComponent.h"
#include "ChaosVehicleMovementComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"

ASawariTaxiPawn::ASawariTaxiPawn()
{
    PrimaryActorTick.bCanEverTick = false;
    ChaseArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("ChaseArm"));
    ChaseArm->SetupAttachment(GetRootComponent());
    ChaseArm->TargetArmLength = 650.0f;
    ChaseArm->SetRelativeLocation(FVector(-40.0f, 0.0f, 125.0f));
    ChaseArm->SetRelativeRotation(FRotator(-10.0f, 0.0f, 0.0f));
    ChaseArm->bEnableCameraLag = true;
    ChaseArm->CameraLagSpeed = 8.0f;
    ChaseCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ChaseCamera"));
    ChaseCamera->SetupAttachment(ChaseArm, USpringArmComponent::SocketName);
    BonnetCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("BonnetCamera"));
    BonnetCamera->SetupAttachment(GetRootComponent());
    BonnetCamera->SetRelativeLocation(FVector(175.0f, 0.0f, 100.0f));
    CockpitCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CockpitCamera"));
    CockpitCamera->SetupAttachment(GetRootComponent());
    CockpitCamera->SetRelativeLocation(FVector(45.0f, 0.0f, 115.0f));
}
void ASawariTaxiPawn::BeginPlay()
{
    Super::BeginPlay();
    ApplyCameraMode();
    if (UChaosVehicleMovementComponent* Movement = FindComponentByClass<UChaosVehicleMovementComponent>())
    {
        Movement->SetUseAutomaticGears(false);
        Movement->SetTargetGear(1, true);
        Movement->SetRequiresControllerForInputs(true);
    }
}
void ASawariTaxiPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    check(PlayerInputComponent);
    PlayerInputComponent->BindAxis(TEXT("SawariThrottle"), this, &ASawariTaxiPawn::SetThrottle);
    PlayerInputComponent->BindAxis(TEXT("SawariBrake"), this, &ASawariTaxiPawn::SetBrake);
    PlayerInputComponent->BindAxis(TEXT("SawariSteering"), this, &ASawariTaxiPawn::SetSteering);
    PlayerInputComponent->BindAction(TEXT("SawariHandbrake"), IE_Pressed, this, &ASawariTaxiPawn::SetHandbrakePressed);
    PlayerInputComponent->BindAction(TEXT("SawariHandbrake"), IE_Released, this, &ASawariTaxiPawn::SetHandbrakeReleased);
    PlayerInputComponent->BindAction(TEXT("SawariReverse"), IE_Pressed, this, &ASawariTaxiPawn::SetReversePressed);
    PlayerInputComponent->BindAction(TEXT("SawariReverse"), IE_Released, this, &ASawariTaxiPawn::SetReverseReleased);
    PlayerInputComponent->BindAction(TEXT("SawariCamera"), IE_Pressed, this, &ASawariTaxiPawn::CycleCamera);
}
void ASawariTaxiPawn::SetThrottle(float Value)
{
    if (UChaosVehicleMovementComponent* M = FindComponentByClass<UChaosVehicleMovementComponent>())
        M->SetThrottleInput(FMath::Clamp(Value, 0.0f, 1.0f));
}
void ASawariTaxiPawn::SetBrake(float Value)
{
    if (UChaosVehicleMovementComponent* M = FindComponentByClass<UChaosVehicleMovementComponent>())
        M->SetBrakeInput(FMath::Clamp(Value, 0.0f, 1.0f));
}
void ASawariTaxiPawn::SetSteering(float Value)
{
    if (UChaosVehicleMovementComponent* M = FindComponentByClass<UChaosVehicleMovementComponent>())
        M->SetSteeringInput(FMath::Clamp(Value, -1.0f, 1.0f));
}
void ASawariTaxiPawn::SetHandbrakePressed()
{
    if (UChaosVehicleMovementComponent* M = FindComponentByClass<UChaosVehicleMovementComponent>())
        M->SetHandbrakeInput(true);
}
void ASawariTaxiPawn::SetHandbrakeReleased()
{
    if (UChaosVehicleMovementComponent* M = FindComponentByClass<UChaosVehicleMovementComponent>())
        M->SetHandbrakeInput(false);
}
void ASawariTaxiPawn::SetReversePressed()
{
    if (UChaosVehicleMovementComponent* M = FindComponentByClass<UChaosVehicleMovementComponent>())
        M->SetTargetGear(-1, true);
}
void ASawariTaxiPawn::SetReverseReleased()
{
    if (UChaosVehicleMovementComponent* M = FindComponentByClass<UChaosVehicleMovementComponent>())
        M->SetTargetGear(1, true);
}
void ASawariTaxiPawn::CycleCamera()
{
    switch (ActiveCameraMode)
    {
        case ESawariCameraMode::Chase: ActiveCameraMode = ESawariCameraMode::Bonnet; break;
        case ESawariCameraMode::Bonnet: ActiveCameraMode = ESawariCameraMode::Cockpit; break;
        default: ActiveCameraMode = ESawariCameraMode::Chase; break;
    }
    ApplyCameraMode();
}
void ASawariTaxiPawn::ApplyCameraMode()
{
    if (ChaseCamera) ChaseCamera->SetActive(ActiveCameraMode == ESawariCameraMode::Chase);
    if (BonnetCamera) BonnetCamera->SetActive(ActiveCameraMode == ESawariCameraMode::Bonnet);
    if (CockpitCamera) CockpitCamera->SetActive(ActiveCameraMode == ESawariCameraMode::Cockpit);
}
