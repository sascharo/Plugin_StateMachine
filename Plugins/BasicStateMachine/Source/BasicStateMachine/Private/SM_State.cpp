#include "SM_State.h"

USM_State* USM_Transition::TryTransition(const UObject* RefObject,
										 const TArray<USM_InputAtom*>& DataSource,
										 const int32 DataIndex,
										 int32 &OutDataIndex)
{
	OutDataIndex = DataIndex;

	if (!AcceptableInputs.Num() ||
		DataSource.IsValidIndex(DataIndex) && AcceptableInputs.Contains(DataSource[DataIndex]))
	{
		++OutDataIndex;
		return bReverseInputTest ? nullptr : DestinationState;
	}

	return bReverseInputTest ? DestinationState : nullptr;
	
}

FStateMachineResult USM_State::RunState(const UObject* RefObject,
										const TArray<USM_InputAtom*>& DataSource,
										const int32 DataIndex /*=0*/,
										int32 RemainingSteps /*=-1*/)
{
	bool bMustEndNow = bTerminateImmediately || !DataSource.IsValidIndex(DataIndex);

	// If we're still running, see where our branches lead.
	if (RemainingSteps && !bMustEndNow)
	{
		USM_State* DestinationState = nullptr;
		int32 DestinationDataIndex = DataIndex;

		for (int32 i = 0; i < InstancedTransitions.Num(); ++i)
		{
			// There shouldn't be a null transitions in the list.
			// This could be a check.
			//check(InstancedTransitions[i]);
			//ensure(InstancedTransitions[i]);
			if (InstancedTransitions[i])
			{
				DestinationState = InstancedTransitions[i]->TryTransition(RefObject, DataSource, DataIndex, DestinationDataIndex);
				if (DestinationState)
				{
					return DestinationState->RunState(RefObject, DataSource, DestinationDataIndex, RemainingSteps - 1);
				}
			}
		}

		// We didn't find any acceptable transition, so we must end on this state unless we're told to loop.
		if (bLoopByDefault)
		{
			return LoopState(RefObject, DataSource, DataIndex, RemainingSteps);
		}
		bMustEndNow = true;
	}

	const EStateMachineCompletionType StateMachineCompletionType = bMustEndNow ? CompletionType : EStateMachineCompletionType::OutOfSteps;
	const FStateMachineResult SMR(StateMachineCompletionType, this, DataIndex);
	//SMR.FinalState = this;
	//SMR.DataIndex = DataIndex;
	//SMR.CompletionType = bMustEndNow ? CompletionType : EStateMachineCompletionType::OutOfSteps;

	return SMR;
	
}

FStateMachineResult USM_State::LoopState(const UObject* RefObject,
										 const TArray<USM_InputAtom*>& DataSource,
										 int32 DataIndex,
										 int32 RemainingSteps)
{
	// By default, increment DataIndex by 1 and decrement RemainingSteps by 1.
	return RunState(RefObject, DataSource, DataIndex + 1, RemainingSteps - 1);
	
}
