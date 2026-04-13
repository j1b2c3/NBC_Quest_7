#include "GM_Gamemode.h"

#include "Drone.h"
#include "PC_PlayerController.h"

AGM_Gamemode::AGM_Gamemode()
{
	DefaultPawnClass = ADrone::StaticClass();
	PlayerControllerClass = APC_PlayerController::StaticClass();
}
