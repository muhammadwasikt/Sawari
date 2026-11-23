#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SawariPrototypeHUD.generated.h"

/** Lightweight native HUD used by the first playable build (no art/UI assets required). */
UCLASS()
class SAWARI_API ASawariPrototypeHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
};
