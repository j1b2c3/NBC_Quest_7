#include "PC_PlayerController.h"
#include "EnhancedInputSubsystems.h"

APC_PlayerController::APC_PlayerController()
	: InputMappingContext(nullptr), IA_Move(nullptr), IA_Fly(nullptr), IA_Look(nullptr), IA_Boost(nullptr)
{
}

void APC_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (InputMappingContext)
			{
				Subsystem->AddMappingContext(InputMappingContext, 0);
			}
		}
	}
}
