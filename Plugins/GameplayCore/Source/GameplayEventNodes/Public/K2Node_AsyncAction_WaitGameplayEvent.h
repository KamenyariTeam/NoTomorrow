#pragma once

#include "K2Node_AsyncAction.h"

#include "K2Node_AsyncAction_WaitGameplayEvent.generated.h"

class FBlueprintActionDatabaseRegistrar;
class FKismetCompilerContext;
class FMulticastDelegateProperty;
class FString;
class UEdGraph;
class UEdGraphPin;
class UObject;

/**
 * Blueprint node which is spawned to handle the async logic for UAsyncAction_WaitGameplayEvent
 */
UCLASS()
class UK2Node_AsyncAction_WaitGameplayEvent: public UK2Node_AsyncAction
{
	GENERATED_BODY()
public:
	//~UEdGraphNode interface
	virtual void PostReconstructNode() override;
	virtual void PinDefaultValueChanged(UEdGraphPin* ChangedPin) override;
	virtual void GetPinHoverText(const UEdGraphPin& Pin, FString& HoverTextOut) const override;
	//~End of UEdGraphNode interface
	
	//~UK2Node interface
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual void AllocateDefaultPins() override;
	//~End of UK2Node interface

	protected:
	virtual bool HandleDelegates(
		const TArray<FBaseAsyncTaskHelper::FOutputPinAndLocalVariable>& VariableOutputs, UEdGraphPin* ProxyObjectPin,
		UEdGraphPin*& InOutLastThenPin, UEdGraph* SourceGraph, FKismetCompilerContext& CompilerContext) override;

private:
	
	// Add the GetEventData flow to the end of the delegate handler's logic chain
	bool HandleEventImplementation(
		FMulticastDelegateProperty* CurrentProperty,
		const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& ProxyObjectVar,
		const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& PayloadVar,
		const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& ActualChannelVar,
		UEdGraphPin*& InOutLastActivatedThenPin, UEdGraph* SourceGraph, FKismetCompilerContext& CompilerContext);

	// Make sure the output Event wildcard matches the input EventType 
	void RefreshOutputEventType();

	UEdGraphPin* GetEventPin() const;
	UEdGraphPin* GetEventTypePin() const;
	UEdGraphPin* GetActualChannelPin() const;
};
