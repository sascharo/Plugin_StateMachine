#pragma once

#include "CoreMinimal.h"

class FBasicStateMachineModule : public IModuleInterface
{
public:
	// IModuleInterface implementation.
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
};
