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

void UQuest::OnSucceeded(UQuestStatus* QuestStatus) const
{
	UE_LOG(LogTemp, Warning, TEXT("Quest \"%s\" succeeded!"), *QuestName.ToString());
}

void UQuest::OnFailed(UQuestStatus* QuestStatus) const
{
	UE_LOG(LogTemp, Error, TEXT("Quest \"%s\" failed!"), *QuestName.ToString());
}

void UQuestWithResult::OnSucceeded(UQuestStatus* QuestStatus) const
{
	Super::OnSucceeded(QuestStatus);

	for (const UQuest* SuccessQuest : SuccessQuests)
	{
		QuestStatus->BeginQuest(SuccessQuest);
	}

	for (int32 i = 0; i < SuccessInputs.Num(); ++i)
	{
		QuestStatus->UpdateQuests(SuccessInputs[i]);
	}
}

void UQuestWithResult::OnFailed(UQuestStatus* QuestStatus) const
{
	Super::OnFailed(QuestStatus);

	for (const UQuest* FailureQuest : FailureQuests)
	{
		QuestStatus->BeginQuest(FailureQuest);
	}

	for (int32 i = 0; i < FailureInputs.Num(); ++i)
	{
		QuestStatus->UpdateQuests(FailureInputs[i]);
	}
}

void UQuestStatus::LogLast()
{
	UE_LOG(LogTemp, Warning, TEXT("Quest \"%s\" was added."), *QuestActivities.Last()->Description.ToString());
}

void UQuestStatus::UpdateQuests(USM_InputAtom* QuestActivity)
{
	TArray<int32> RecentlyCompletedQuests;
	
	// Update the master list of everything we've ever done.
	QuestActivities.Add(QuestActivity);
	
	LogLast();

	// Update individual quests (if they care about this activity) and see if they are complete.
	for (int32 i = QuestList.Num() - 1; i >= 0; --i)
	{
		if (QuestList[i].UpdateQuest(this, QuestActivity))
		{
			//UE_LOG(LogTemp, Warning, TEXT("Quest \"%s\" completed: %s"),
			//	*QuestList[i].Quest->QuestName.ToString(),
			//	QuestList[i].QuestProgress == EQuestCompletion::EQC_Succeeded ? TEXT("Success") : TEXT("Failure"));

			RecentlyCompletedQuests.Add(i);
		}
	}

	// Process completed quests after updating all quests.
	//This way, a completed quest can't inject out-of-order input atoms into other quests.
	for (int32 i = RecentlyCompletedQuests.Num() - 1; i >= 0; --i)
	{
		FQuestInProgress& QIP = QuestList[RecentlyCompletedQuests[i]];
		
		if (QIP.QuestProgress == EQuestCompletion::EQC_Succeeded)
		{
			QIP.Quest->OnSucceeded(this);
		}
		else
		{
			QIP.Quest->OnFailed(this);
		}
		
		RecentlyCompletedQuests.RemoveAtSwap(i);
	}
}

bool UQuestStatus::BeginQuest(const UQuest* Quest)
{
	for (FQuestInProgress& QIP : QuestList)
	{
		if (QIP.Quest == Quest)
		{
			if (QIP.QuestProgress == EQuestCompletion::EQC_NotStarted)
			{
				UE_LOG(LogTemp, Warning, TEXT("Changing quest \"%s\" to Started status."), *QIP.Quest->QuestName.ToString());
				QIP.QuestProgress = EQuestCompletion::EQC_Started;
				
				return true;
			}
			UE_LOG(LogTemp, Warning, TEXT("Quest \"%s\" is already in the list."), *QIP.Quest->QuestName.ToString());
			return false;
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Adding quest \"%s\" to the list and starting it."), *Quest->QuestName.ToString());
	QuestList.Add(FQuestInProgress::NewQuestInProgress(Quest));
	
	return true;
}
