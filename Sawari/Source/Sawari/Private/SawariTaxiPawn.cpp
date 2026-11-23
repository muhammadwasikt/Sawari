#include "SawariTaxiPawn.h"

#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/InputComponent.h"
#include "Curves/CurveFloat.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/SpringArmComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/SoftObjectPath.h"

USawariFrontWheel::USawariFrontWheel()
{
    WheelRadius = 34.0f;
    WheelWidth = 20.0f;
    WheelMass = 18.0f;
    bAffectedByBrake = true;
    bAffectedByEngine = true;
    bAffectedByHandbrake = false;
    bAffectedBySteering = true;
    MaxSteerAngle = 34.0f;
    SuspensionMaxRaise = 12.0f;
    SuspensionMaxDrop = 18.0f;
    SuspensionDampingRatio = 0.58f;
    SpringRate = 30000.0f;
    SpringPreload = 0.0f;
    SuspensionSmoothing = 4;
    WheelLoadRatio = 0.9f;
}

USawariRearWheel::USawariRearWheel()
{
    WheelRadius = 34.0f;
    WheelWidth = 20.0f;
    WheelMass = 18.0f;
    bAffectedByBrake = true;
    bAffectedByEngine = false;
    bAffectedByHandbrake = true;
    bAffectedBySteering = false;
    MaxSteerAngle = 0.0f;
    SuspensionMaxRaise = 12.0f;
    SuspensionMaxDrop = 18.0f;
    SuspensionDampingRatio = 0.62f;
    SpringRate = 32000.0f;
    SpringPreload = 0.0f;
    SuspensionSmoothing = 4;
    WheelLoadRatio = 0.9f;
}

bool ASawariTaxiPawn::IsTaxiAssetInstalled()
{
    static const FSoftObjectPath TaxiMeshPath(
        TEXT("/Game/Vehicles/Taxi/SK_SawariTaxi.SK_SawariTaxi"));
    return TaxiMeshPath.TryLoad() != nullptr;
}

ASawariTaxiPawn::ASawariTaxiPawn()
{
    PrimaryActorTick.bCanEverTick = false;

    static ConstructorHelpers::FObjectFinder<USkeletalMesh> TaxiMeshAsset(
        TEXT("/Game/Vehicles/Taxi/SK_SawariTaxi.SK_SawariTaxi"));
    if (TaxiMeshAsset.Succeeded())
    {
        GetMesh()->SetSkeletalMesh(TaxiMeshAsset.Object);
        GetMesh()->SetRelativeLocation(FVector::ZeroVector);
        GetMesh()->SetRelativeRotation(FRotator::ZeroRotator);
        GetMesh()->SetSimulatePhysics(true);
        GetMesh()->SetCollisionProfileName(TEXT("Vehicle"));

        static ConstructorHelpers::FClassFinder<UAnimInstance> TaxiAnimBP(
            TEXT("/Game/Vehicles/Taxi/ABP_SawariTaxi"));
        if (TaxiAnimBP.Succeeded())
        {
            GetMesh()->SetAnimInstanceClass(TaxiAnimBP.Class);
        }
    }

    ChaseArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("ChaseArm"));
    ChaseArm->SetupAttachment(GetMesh());
    ChaseArm->TargetArmLength = 720.0f;
    ChaseArm->SetRelativeLocation(FVector(-80.0f, 0.0f, 175.0f));
    ChaseArm->SetRelativeRotation(FRotator(-10.0f, 0.0f, 0.0f));
    ChaseArm->bEnableCameraLag = true;
    ChaseArm->CameraLagSpeed = 6.5f;
    ChaseArm->bUsePawnControlRotation = false;

    ChaseCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ChaseCamera"));
    ChaseCamera->SetupAttachment(ChaseArm, USpringArmComponent::SocketName);

    BonnetCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("BonnetCamera"));
    BonnetCamera->SetupAttachment(GetMesh());
    BonnetCamera->SetRelativeLocation(FVector(175.0f, 0.0f, 112.0f));
    BonnetCamera->SetRelativeRotation(FRotator(-2.0f, 0.0f, 0.0f));

    CockpitCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CockpitCamera"));
    CockpitCamera->SetupAttachment(GetMesh());
    CockpitCamera->SetRelativeLocation(FVector(15.0f, 0.0f, 132.0f));
    CockpitCamera->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

    if (UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement())
    {
        Movement->WheelSetups.SetNum(4);

        Movement->WheelSetups[0].WheelClass = USawariFrontWheel::StaticClass();
        Movement->WheelSetups[0].BoneName = TEXT("Wheel_Front_Left");
        Movement->WheelSetups[0].AdditionalOffset = FVector::ZeroVector;

        Movement->WheelSetups[1].WheelClass = USawariFrontWheel::StaticClass();
        Movement->WheelSetups[1].BoneName = TEXT("Wheel_Front_Right");
        Movement->WheelSetups[1].AdditionalOffset = FVector::ZeroVector;

        Movement->WheelSetups[2].WheelClass = USawariRearWheel::StaticClass();
        Movement->WheelSetups[2].BoneName = TEXT("Wheel_Rear_Left");
        Movement->WheelSetups[2].AdditionalOffset = FVector::ZeroVector;

        Movement->WheelSetups[3].WheelClass = USawariRearWheel::StaticClass();
        Movement->WheelSetups[3].BoneName = TEXT("Wheel_Rear_Right");
        Movement->WheelSetups[3].AdditionalOffset = FVector::ZeroVector;

        Movement->Mass = 1450.0f;
        Movement->ChassisWidth = 180.0f;
        Movement->ChassisHeight = 135.0f;
        Movement->bEnableCenterOfMassOverride = true;
        Movement->CenterOfMassOverride = FVector(0.0f, 0.0f, -28.0f);
        Movement->bReverseAsBrake = true;

        Movement->EngineSetup.MaxTorque = 310.0f;
        Movement->EngineSetup.MaxRPM = 6200.0f;
        Movement->EngineSetup.EngineIdleRPM = 850.0f;
        Movement->EngineSetup.EngineRevUpMOI = 5.0f;
        Movement->EngineSetup.EngineRevDownRate = 600.0f;
        if (FRichCurve* Torque = Movement->EngineSetup.TorqueCurve.GetRichCurve())
        {
            Torque->AddKey(0.0f, 0.45f);
            Torque->AddKey(1200.0f, 0.78f);
            Torque->AddKey(2800.0f, 1.0f);
            Torque->AddKey(4400.0f, 0.92f);
            Torque->AddKey(5600.0f, 0.72f);
            Torque->AddKey(6200.0f, 0.45f);
        }

        Movement->TransmissionSetup.bUseAutomaticGears = true;
        Movement->SetUseAutomaticGears(true);
        Movement->SetRequiresControllerForInputs(true);
    }
}

UChaosWheeledVehicleMovementComponent* ASawariTaxiPawn::GetChaosMovement() const
{
    return Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent());
}

void ASawariTaxiPawn::BeginPlay()
{
    Super::BeginPlay();
    if (GetMesh())
    {
        GetMesh()->SetSimulatePhysics(true);
        GetMesh()->WakeAllRigidBodies();
    }
    if (UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement())
    {
        Movement->SetRequiresControllerForInputs(true);
        Movement->SetUseAutomaticGears(true);
        Movement->SetTargetGear(1, true);
    }
    ApplyCameraMode();
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
    if (UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement())
    {
        if (Movement->GetTargetGear() < 0)
        {
            Movement->SetTargetGear(1, true);
        }
        Movement->SetBrakeInput(0.0f);
        Movement->SetThrottleInput(FMath::Clamp(Value, 0.0f, 1.0f));
    }
}

void ASawariTaxiPawn::SetBrake(float Value)
{
    if (UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement())
    {
        Movement->SetBrakeInput(FMath::Clamp(Value, 0.0f, 1.0f));
    }
}

void ASawariTaxiPawn::SetSteering(float Value)
{
    if (UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement())
    {
        Movement->SetSteeringInput(FMath::Clamp(Value, -1.0f, 1.0f));
    }
}

void ASawariTaxiPawn::SetHandbrakePressed()
{
    if (UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement())
    {
        Movement->SetHandbrakeInput(true);
    }
}

void ASawariTaxiPawn::SetHandbrakeReleased()
{
    if (UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement())
    {
        Movement->SetHandbrakeInput(false);
    }
}

void ASawariTaxiPawn::SetReversePressed()
{
    if (UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement())
    {
        Movement->SetThrottleInput(0.0f);
        if (Movement->GetForwardSpeed() > 80.0f)
        {
            Movement->SetBrakeInput(1.0f);
        }
        else
        {
            Movement->SetBrakeInput(0.0f);
            Movement->SetTargetGear(-1, true);
            Movement->SetThrottleInput(0.35f);
        }
    }
}

void ASawariTaxiPawn::SetReverseReleased()
{
    if (UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement())
    {
        Movement->SetThrottleInput(0.0f);
        Movement->SetBrakeInput(0.0f);
        Movement->SetTargetGear(1, true);
    }
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

float ASawariTaxiPawn::GetSpeedKmh() const
{
    if (const UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement())
    {
        return FMath::Abs(Movement->GetForwardSpeed()) * 0.036f;
    }
    return 0.0f;
}

bool ASawariTaxiPawn::IsChaosVehicleConfigured() const
{
    const UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement();
    return GetMesh() && GetMesh()->GetSkeletalMeshAsset()
        && GetMesh()->GetPhysicsAsset()
        && Movement && Movement->WheelSetups.Num() == 4;
}
