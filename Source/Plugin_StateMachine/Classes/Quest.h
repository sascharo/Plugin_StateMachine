#pragma once

#include "SM_State.h"
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Quest.generated.h"

class UQuest;

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

public:
	bool UpdateQuest(const UObject* ObjectRef, USM_InputAtom* QuestActivity);
	
};

UCLASS()
class PLUGIN_STATEMACHINE_API UQuest : public UDataAsset
{
	GENERATED_BODY()

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

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PLUGIN_STATEMACHINE_API UQuestStatus : public UActorComponent
{
	GENERATED_BODY()

protected:
	// The master list of all quest-related things we've down.
	UPROPERTY(EditAnywhere)
	TArray<USM_InputAtom*> QuestActivities;

	// The list of quests in our current game or area.
	UPROPERTY(EditAnywhere)
	TArray<FQuestInProgress> QuestList;
	
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
	//virtual void BeginPlay() override
	//{
	//	Super::BeginPlay();
	//}

	// Called every frame.
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override
	//{
	//	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	//}

	// Add to our quest activity log!
	// This also automatically checks to see if any unfinished quests are now complete.
	UFUNCTION(BlueprintCallable, Category="Quests")
	void UpdateQuests(USM_InputAtom* QuestActivity);
	
};
