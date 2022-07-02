#pragma once

#include "SM_State.h"
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Quest.generated.h"

class UQuest;
class UQuestWithResult;
class UQuestStatus;

UENUM()
enum class EQuestCompletion : uint8
{
	EQC_NotStarted,
	EQC_Started,
	EQC_Succeeded,
	EQC_Failed
	
};

USTRUCT()
struct FQuestInProgress
{
	GENERATED_USTRUCT_BODY()

public:
	bool UpdateQuest(const UObject* ObjectRef, USM_InputAtom* QuestActivity);

	static FQuestInProgress NewQuestInProgress(const UQuest* Quest)
	{
		FQuestInProgress QIP;
		QIP.Quest = Quest;
		QIP.QuestProgress = EQuestCompletion::EQC_Started;
		
		return QIP;
	}

public:
	// Quest data asset.
	UPROPERTY(EditAnywhere)
	const UQuest* Quest;

	// Current progress in the quest.
	UPROPERTY(EditAnywhere)
	EQuestCompletion QuestProgress;

protected:
	// All input for this quest, filtered by the quest's blacklist/whitelist.
	UPROPERTY(EditAnywhere)
	TArray<USM_InputAtom*> QuestActivities;
	
};

UCLASS()
class PLUGIN_STATEMACHINE_API UQuest : public UDataAsset
{
	GENERATED_BODY()

public:
	virtual void OnSucceeded(class UQuestStatus* QuestStatus) const;
	virtual void OnFailed(class UQuestStatus* QuestStatus) const;
	
public:
	// The name of the quest.
	UPROPERTY(EditAnywhere)
	FText QuestName;

	// If this machine accepts our QuestActivities log, the quest is successful.
	UPROPERTY(EditAnywhere)
	USM_State* QuestStateMachine;

	// If true, the InputList is a blacklist; otherwise, it's a whitelist.
	UPROPERTY(EditAnywhere)
	uint32 bInputBlackList : 1;

	// The blacklist/whitelist (depending on bBlackList) used to filter InputAtoms this Quest recognizes.
	UPROPERTY(EditAnywhere)
	TArray<USM_InputAtom*> InputList;
	
};

UCLASS()
class PLUGIN_STATEMACHINE_API UQuestWithResult : public UQuest
{
	GENERATED_BODY()

public:
	virtual void OnSucceeded(UQuestStatus* QuestStatus) const override;
	virtual void OnFailed(UQuestStatus* QuestStatus) const override;
	
protected:
	// The quests in this list will go from NotStarted to Started if the current quest succeeds.
	UPROPERTY(EditAnywhere)
	TArray<UQuest*> SuccessQuests;

	// Input atoms to add if the quest succeeds.
	UPROPERTY(EditAnywhere)
	TArray<USM_InputAtom*> SuccessInputs;

	// The quests in this list will go from NotStarted to Started if the current quest fails.
	UPROPERTY(EditAnywhere)
	TArray<UQuest*> FailureQuests;

	// Input atoms to add if the quest fails.
	UPROPERTY(EditAnywhere)
	TArray<USM_InputAtom*> FailureInputs;
	
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PLUGIN_STATEMACHINE_API UQuestStatus : public UActorComponent
{
	GENERATED_BODY()
	
public:
	// Sets default values for this component's properties.
	UQuestStatus()
	{
		// Set this component to be initialized when the game starts, and to be ticked every frame.
		// You can turn these features off to improve performance if you don't need them.
		//bWantsBeginPlay = true; // Deprecated with UE 4.14.
		PrimaryComponentTick.bCanEverTick = true;
	}

	// Called when the game starts.
	virtual void BeginPlay() override
	{
		Super::BeginPlay();
	}

	// Called every frame.
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override
	{
		Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	}

	UFUNCTION(BlueprintCallable, Category="Quests")
	void LogLast();

	// Add to our quest activity log!
	// This also automatically checks to see if any unfinished quests are now complete.
	UFUNCTION(BlueprintCallable, Category="Quests")
	void UpdateQuests(USM_InputAtom* QuestActivity);

	// Add a new quest-in-progress entry, or begin the quest provided if it's already on the list and hasn't been started yet.
	UFUNCTION(BlueprintCallable, Category="Quests")
	bool BeginQuest(const UQuest* Quest);

protected:
	// The master list of all quest-related things we've down.
	UPROPERTY(EditAnywhere)
	TArray<USM_InputAtom*> QuestActivities;

	// The list of quests in our current game or area.
	UPROPERTY(EditAnywhere)
	TArray<FQuestInProgress> QuestList;
	
};
