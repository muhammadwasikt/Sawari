#include "SawariArcadeTaxiPawn.h"

#include "SawariRideSubsystem.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/InputComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/GameInstance.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ASawariArcadeTaxiPawn::ASawariArcadeTaxiPawn()
{
    PrimaryActorTick.bCanEverTick = true;
    AutoPossessPlayer = EAutoReceiveInput::Disabled;
    bUseControllerRotationYaw = false;

    PhysicsBody = CreateDefaultSubobject<UBoxComponent>(TEXT("TaxiPhysicsBody"));
    SetRootComponent(PhysicsBody);
    PhysicsBody->SetBoxExtent(FVector(220.0f, 95.0f, 55.0f));
    PhysicsBody->SetCollisionProfileName(TEXT("BlockAll"));
    PhysicsBody->SetSimulatePhysics(false);
    PhysicsBody->SetLinearDamping(0.12f);
    PhysicsBody->SetAngularDamping(3.2f);
    PhysicsBody->SetMassOverrideInKg(NAME_None, 1450.0f, true);
    PhysicsBody->SetCenterOfMass(FVector(0.0f, 0.0f, -32.0f), NAME_None);

    ChaseArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("ChaseArm"));
    ChaseArm->SetupAttachment(PhysicsBody);
    ChaseArm->TargetArmLength = 720.0f;
    ChaseArm->SetRelativeLocation(FVector(-55.0f, 0.0f, 180.0f));
    ChaseArm->SetRelativeRotation(FRotator(-12.0f, 0.0f, 0.0f));
    ChaseArm->bEnableCameraLag = true;
    ChaseArm->CameraLagSpeed = 5.5f;
    ChaseArm->bUsePawnControlRotation = false;

    ChaseCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ChaseCamera"));
    ChaseCamera->SetupAttachment(ChaseArm, USpringArmComponent::SocketName);
    BonnetCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("BonnetCamera"));
    BonnetCamera->SetupAttachment(PhysicsBody);
    BonnetCamera->SetRelativeLocation(FVector(205.0f, 0.0f, 112.0f));
    BonnetCamera->SetRelativeRotation(FRotator(-2.0f, 0.0f, 0.0f));
    CockpitCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CockpitCamera"));
    CockpitCamera->SetupAttachment(PhysicsBody);
    CockpitCamera->SetRelativeLocation(FVector(20.0f, 0.0f, 142.0f));

    BuildTaxiVisuals();
}

UMaterialInterface* ASawariArcadeTaxiPawn::MakeTaxiMaterial(const FLinearColor& Color) const
{
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> BasicMaterial(
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!BasicMaterial.Succeeded()) return nullptr;

    UMaterialInstanceDynamic* Dynamic = UMaterialInstanceDynamic::Create(
        BasicMaterial.Object, const_cast<ASawariArcadeTaxiPawn*>(this));
    if (Dynamic)
    {
        Dynamic->SetVectorParameterValue(TEXT("Color"), Color);
        Dynamic->SetVectorParameterValue(TEXT("BaseColor"), Color);
    }
    return Dynamic;
}

void ASawariArcadeTaxiPawn::BuildTaxiVisuals()
{
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (!CubeMesh.Succeeded() || !CylinderMesh.Succeeded()) return;

    UStaticMesh* CubeAsset = CubeMesh.Object;
    UStaticMesh* CylinderAsset = CylinderMesh.Object;
    auto MakePart = [this, CubeAsset](const TCHAR* Name, const FVector& Location, const FVector& Scale, UMaterialInterface* Material)
    {
        UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(FName(Name));
        Part->SetupAttachment(PhysicsBody);
        Part->SetStaticMesh(CubeAsset);
        Part->SetRelativeLocation(Location);
        Part->SetRelativeScale3D(Scale);
        Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        if (Material) Part->SetMaterial(0, Material);
        return Part;
    };

    UMaterialInterface* TaxiYellow = MakeTaxiMaterial(FLinearColor(0.95f, 0.64f, 0.06f));
    UMaterialInterface* WindowDark = MakeTaxiMaterial(FLinearColor(0.08f, 0.16f, 0.20f));
    UMaterialInterface* Rubber = MakeTaxiMaterial(FLinearColor(0.025f, 0.03f, 0.035f));
    UMaterialInterface* Bumper = MakeTaxiMaterial(FLinearColor(0.82f, 0.84f, 0.85f));
    ChassisVisual = MakePart(TEXT("TaxiBodyVisual"), FVector(0, 0, 0), FVector(4.35f, 1.88f, 0.58f), TaxiYellow);
    RoofVisual = MakePart(TEXT("TaxiRoofVisual"), FVector(-5, 0, 62), FVector(2.15f, 1.55f, 0.54f), TaxiYellow);
    MakePart(TEXT("TaxiFrontGlass"), FVector(78, 0, 54), FVector(0.62f, 1.55f, 0.43f), WindowDark);
    MakePart(TEXT("TaxiRearGlass"), FVector(-87, 0, 54), FVector(0.56f, 1.55f, 0.42f), WindowDark);
    MakePart(TEXT("TaxiFrontBumper"), FVector(222, 0, -24), FVector(0.22f, 1.82f, 0.22f), Bumper);
    MakePart(TEXT("TaxiRearBumper"), FVector(-222, 0, -24), FVector(0.22f, 1.82f, 0.22f), Bumper);

    for (int32 Index = 0; Index < 4; ++Index)
    {
        const bool bFront = Index < 2;
        const float X = (Index % 2 == 0) ? 145.0f : -145.0f;
        const float Y = bFront ? 101.0f : -101.0f;
        UStaticMeshComponent* Wheel = CreateDefaultSubobject<UStaticMeshComponent>(
            *FString::Printf(TEXT("TaxiWheel_%d"), Index));
        Wheel->SetupAttachment(PhysicsBody);
        Wheel->SetStaticMesh(CylinderAsset);
        Wheel->SetRelativeLocation(FVector(X, Y, -13.0f));
        Wheel->SetRelativeRotation(FRotator(0.0f, 0.0f, 90.0f));
        Wheel->SetRelativeScale3D(FVector(0.42f, 0.42f, 0.24f));
        Wheel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        if (Rubber) Wheel->SetMaterial(0, Rubber);
        WheelVisuals.Add(Wheel);
    }
}

void ASawariArcadeTaxiPawn::BeginPlay()
{
    Super::BeginPlay();
    PhysicsBody->SetSimulatePhysics(true);
    PhysicsBody->WakeAllRigidBodies();
    ApplyCameraMode();
}

void ASawariArcadeTaxiPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    check(PlayerInputComponent);
    PlayerInputComponent->BindAxis(TEXT("SawariDrive"), this, &ASawariArcadeTaxiPawn::SetDriveInput);
    PlayerInputComponent->BindAxis(TEXT("SawariSteering"), this, &ASawariArcadeTaxiPawn::SetSteeringInput);
    PlayerInputComponent->BindAction(TEXT("SawariHandbrake"), IE_Pressed, this, &ASawariArcadeTaxiPawn::SetHandbrakePressed);
    PlayerInputComponent->BindAction(TEXT("SawariHandbrake"), IE_Released, this, &ASawariArcadeTaxiPawn::SetHandbrakeReleased);
    PlayerInputComponent->BindAction(TEXT("SawariCamera"), IE_Pressed, this, &ASawariArcadeTaxiPawn::CycleCamera);
    PlayerInputComponent->BindAction(TEXT("SawariRequestRide"), IE_Pressed, this, &ASawariArcadeTaxiPawn::RequestRide);
    PlayerInputComponent->BindAction(TEXT("SawariAcceptRide"), IE_Pressed, this, &ASawariArcadeTaxiPawn::AcceptRide);
    PlayerInputComponent->BindAction(TEXT("SawariDeclineRide"), IE_Pressed, this, &ASawariArcadeTaxiPawn::DeclineRide);
}

void ASawariArcadeTaxiPawn::SetDriveInput(float Value) { DriveInput = FMath::Clamp(Value, -1.0f, 1.0f); }
void ASawariArcadeTaxiPawn::SetSteeringInput(float Value) { SteeringInput = FMath::Clamp(Value, -1.0f, 1.0f); }
void ASawariArcadeTaxiPawn::SetHandbrakePressed() { bHandbrake = true; }
void ASawariArcadeTaxiPawn::SetHandbrakeReleased() { bHandbrake = false; }

void ASawariArcadeTaxiPawn::RequestRide()
{
    if (UGameInstance* GI = GetGameInstance())
        if (USawariRideSubsystem* Rides = GI->GetSubsystem<USawariRideSubsystem>())
            Rides->GenerateDemoRide();
}
void ASawariArcadeTaxiPawn::AcceptRide()
{
    if (UGameInstance* GI = GetGameInstance())
        if (USawariRideSubsystem* Rides = GI->GetSubsystem<USawariRideSubsystem>())
            Rides->AcceptRide();
}
void ASawariArcadeTaxiPawn::DeclineRide()
{
    if (UGameInstance* GI = GetGameInstance())
        if (USawariRideSubsystem* Rides = GI->GetSubsystem<USawariRideSubsystem>())
            Rides->DeclineRide();
}

void ASawariArcadeTaxiPawn::CycleCamera()
{
    CameraIndex = (CameraIndex + 1) % 3;
    ApplyCameraMode();
}
void ASawariArcadeTaxiPawn::ApplyCameraMode()
{
    if (ChaseCamera) ChaseCamera->SetActive(CameraIndex == 0);
    if (BonnetCamera) BonnetCamera->SetActive(CameraIndex == 1);
    if (CockpitCamera) CockpitCamera->SetActive(CameraIndex == 2);
}
float ASawariArcadeTaxiPawn::GetSpeedKmh() const
{
    return PhysicsBody ? PhysicsBody->GetPhysicsLinearVelocity().Size() * 0.036f : 0.0f;
}

void ASawariArcadeTaxiPawn::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!PhysicsBody || !PhysicsBody->IsSimulatingPhysics()) return;

    const FVector Velocity = PhysicsBody->GetPhysicsLinearVelocity();
    const FVector Forward = GetActorForwardVector();
    const FVector Right = GetActorRightVector();
    const float ForwardSpeed = FVector::DotProduct(Velocity, Forward);
    const float SideSpeed = FVector::DotProduct(Velocity, Right);
    const float Speed = Velocity.Size();

    if (DriveInput > 0.0f)
    {
        PhysicsBody->AddForce(Forward * DriveInput * 820000.0f);
    }
    else if (DriveInput < 0.0f)
    {
        if (ForwardSpeed > 180.0f)
            PhysicsBody->AddForce(-Velocity.GetSafeNormal() * (-DriveInput) * 980000.0f);
        else
            PhysicsBody->AddForce(-Forward * (-DriveInput) * 520000.0f);
    }

    if (bHandbrake && Speed > 1.0f)
        PhysicsBody->AddForce(-Velocity.GetSafeNormal() * 1050000.0f);

    const float SideGrip = bHandbrake ? 45.0f : 190.0f;
    PhysicsBody->AddForce(-Right * SideSpeed * SideGrip);
    PhysicsBody->AddForce(-Velocity * 0.18f);

    const float SpeedAlpha = FMath::Clamp(FMath::Abs(ForwardSpeed) / 2200.0f, 0.0f, 1.0f);
    if (FMath::Abs(ForwardSpeed) > 80.0f)
        PhysicsBody->AddTorqueInRadians(FVector::UpVector * SteeringInput * SpeedAlpha * 26000000.0f);

    if (Speed > 6200.0f)
        PhysicsBody->AddForce(-Velocity.GetSafeNormal() * (Speed - 6200.0f) * 850.0f);

    WheelSpinDegrees += (ForwardSpeed / 38.0f) * 57.2958f * DeltaSeconds;
    for (int32 Index = 0; Index < WheelVisuals.Num(); ++Index)
    {
        if (WheelVisuals[Index])
        {
            const float SteerYaw = Index < 2 ? SteeringInput * 22.0f : 0.0f;
            WheelVisuals[Index]->SetRelativeRotation(FRotator(WheelSpinDegrees, SteerYaw, 90.0f));
        }
    }
}
