#include "GameplayEventHandle.h"

#include "GameplayEventSubsystem.h"

bool FGameplayEventHandle::IsValid() const
{
	return HandleID != 0 && ::IsValid(Subsystem);
}

void FGameplayEventHandle::Invalidate() const
{
	if (IsValid() && ::IsValid(Subsystem))
	{
		// @todo: RemoveReceiver?
	}
		
	HandleID = 0;
	Subsystem = nullptr;
}
