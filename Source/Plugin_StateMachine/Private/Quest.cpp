#include "Quest.h"

bool FQuestInProgress::UpdateQuest(const UObject* ObjectRef, USM_InputAtom* QuestActivity)
{
	// Only log activity to valid, in-progress quests.
	// Check the blacklist/whitelist before logging.
	if (Quest &&
		QuestProgress == EQuestCompletion::EQC_Started &&
		Quest->bInputBlackList != Quest->InputList.Contains(QuestActivity))
	{
		QuestActivities.Add(QuestActivity);
		const FStateMachineResult QuestResult = Quest->QuestStateMachine->RunState(ObjectRef, QuestActivities);

		switch (QuestResult.CompletionType)
		{
		case EStateMachineCompletionType::Accepted:
			QuestProgress = EQuestCompletion::EQC_Succeeded;
			return true;
		case EStateMachineCompletionType::Rejected:
			QuestProgress = EQuestCompletion::EQC_Failed;
			return true;
		// case NotAccepted: // Still in progress, no update needed.
		default:
			return false;
		}
	}

	return false;
}

void UQuestStatus::UpdateQuests(USM_InputAtom* QuestActivity)
{
	// Update the master list of everything we've ever done.
	QuestActivities.Add(QuestActivity);

	// Update individual quests (if they care about this activity) and see if they are complete.
	for (int32 i = QuestList.Num() - 1; i >= 0; --i)
	{
		if (QuestList[i].UpdateQuest(this, QuestActivity))
		{
			UE_LOG(LogTemp, Warning, TEXT("Quest \"%s\" completed: %s"),
				*QuestList[i].Quest->QuestName.ToString(),
				QuestList[i].QuestProgress == EQuestCompletion::EQC_Succeeded ? TEXT("Success") : TEXT("Failure"));
		}
	}
}
