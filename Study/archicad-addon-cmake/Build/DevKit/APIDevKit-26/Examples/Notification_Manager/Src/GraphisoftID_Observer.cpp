// *****************************************************************************
// Description:     Source code for the Notification Manager Add-On
// *****************************************************************************
#include "APIEnvir.h"
#define _GRAPHISOFTID_OBSERVER_TRANSL_

// ------------------------------ Includes -------------------------------------

#include	"ACAPinc.h"
#include	"Notification_Manager.h"


GS::Optional<API_Guid> GraphisoftIDEventHandlerId;



GSErrCode	__ACENV_CALL RegisterGraphisoftIDEventHandlers ()
{
	class GraphisoftIDEventHandler : public API_IGraphisoftIDEventHandler {
	public:
		virtual void OnUserChanged () const override
		{
			ACAPI_WriteReport ("GraphisoftIDEventHandler::OnUserChanged event called", false);
		}
	};

	GraphisoftIDEventHandlerId.New ();
	const auto result = ACAPI_Notify_RegisterEventHandler (GS::NewOwned<GraphisoftIDEventHandler> (), *GraphisoftIDEventHandlerId);
	if (result != NoError) {
		GraphisoftIDEventHandlerId.Clear ();
	}

	return result;
}


GSErrCode	__ACENV_CALL UnregisterGraphisoftIDEventHandlers ()
{
	if (GraphisoftIDEventHandlerId.HasValue ()) {
		ACAPI_Notify_UnregisterEventHandler (*GraphisoftIDEventHandlerId);
		GraphisoftIDEventHandlerId.Clear ();
	}

	return NoError;
}


// ============================================================================
// Install Notification Handlers
//
//
// ============================================================================
void	Do_GraphisoftIDMonitor (bool switchOn)
{
	if (switchOn)
		RegisterGraphisoftIDEventHandlers ();
	else
		UnregisterGraphisoftIDEventHandlers ();
}
