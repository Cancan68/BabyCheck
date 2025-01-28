#include "CollapseUtils.h"
#include <SonyClie.h>


//globals
static 	UInt16 	g_sonySilkRefNum;
static  Int32   g_CollapseAPIVersion;
////////////////////////////////////////////
//
////////////////////////////////////////////
void CollapseAppStart()
{
	Err error = errNone; 
	UInt16 cardNo;
	LocalID dbID;	
	UInt32 version;
	
	//register for the notification sent when the the win display changes size
	error = SysCurAppDatabase( &cardNo, &dbID );
    ErrFatalDisplayIf( ( error != errNone ), "can't get app db info" );

	//first check for PalmSource PinMgr version
	error = FtrGet(pinCreator, pinFtrAPIVersion, &version);
	if (!error)
	{
		g_CollapseAPIVersion = version;	

		// OG+: Garmin iQue compatibility code
		if (g_CollapseAPIVersion == pinAPIVersion1_0) {
			//Set the constraints.
			WinSetConstraintsSize(WinGetDisplayWindow(),	160, 160, 0x7FFF, 160, 160, 160);
			//Set the dynamic input area policy.
			FrmSetDIAPolicyAttr(FrmGetActiveForm(), frmDIAPolicyCustom);
			//Enable the input trigger.
			PINSetInputTriggerState(pinInputTriggerEnabled);
				error = SysNotifyRegister( cardNo, dbID, sysNotifyDisplayResizedEvent, CollapseDisplayResizedEventCallback, 
    							sysNotifyNormalPriority, NULL );
    }
		// OG-

		return;
	}
	
	//see if Sony silk library is already loaded and load if not
	if ((error = SysLibFind(sonySysLibNameSilk, &g_sonySilkRefNum)))
	{ 
		if (error == sysErrLibNotFound) 
			{ /* couldn't find lib */ 
				error = SysLibLoad( 'libr', sonySysFileCSilkLib, &g_sonySilkRefNum );

				g_CollapseAPIVersion = 0;

				// OG+: no collapsible silkscreen compatibility code
				if (error) {
					return;
				}
				// OG-
				
				error = SysNotifyRegister( cardNo, dbID, sysNotifyDisplayChangeEvent, CollapseDisplayResizedEventCallback, 
    							sysNotifyNormalPriority, NULL );
			} 
	}

	if (!error ) 
	{ 
		error = FtrGet(sonySysFtrCreator, sonySysFtrNumVskVersion, &version); 
		
		if (error) 
		{ /* Version 1 is installed only resize is available */ 
			if (SilkLibOpen (g_sonySilkRefNum)==errNone) 
			{ 
				SilkLibEnableResize(g_sonySilkRefNum);
				g_CollapseAPIVersion = sonyVersion1;
				 
			}
			else 
				g_CollapseAPIVersion = 0; 
		} 
		else 
		{ /* Version 2 or up is installed Some added features is available */ 
			if (VskOpen (g_sonySilkRefNum) == errNone) 
			{ 
			
				if (VskGetAPIVersion(g_sonySilkRefNum) >= 0x00000003)
				{
					//support vertical and horizontal resize
					VskSetState(g_sonySilkRefNum, vskStateEnable, (vskResizeVertically|vskResizeHorizontally));
				}
				else
					VskSetState(g_sonySilkRefNum, vskStateEnable, 1);
					 
				g_CollapseAPIVersion = sonyVersion2;
			}
			else
				g_CollapseAPIVersion = 0;
		} 
		
		error = SysNotifyRegister( cardNo, dbID, sysNotifyDisplayChangeEvent, CollapseDisplayResizedEventCallback, 
    							sysNotifyNormalPriority, NULL );
	}
	
}

Int32 CollapseGetAPIVersion()
{
	return g_CollapseAPIVersion;
}

void CollapseAppStop()
{
	UInt16 cardNo;
	LocalID dbID;
	Err error;

	//unregister for the notification sent when the the win display changes size
	error = SysCurAppDatabase( &cardNo, &dbID );
    ErrFatalDisplayIf( ( error != errNone ), "can't get app db info" );

  if (g_CollapseAPIVersion > 0)
  {
		SysNotifyUnregister( cardNo, dbID, sysNotifyDisplayResizedEvent, sysNotifyNormalPriority);
	}
	else if (g_CollapseAPIVersion < 0)
	{
		if (g_CollapseAPIVersion == sonyVersion1)
			SilkLibResizeDispWin(g_sonySilkRefNum, silkResizeNormal);
			
		//disable resize
		SilkLibDisableResize(g_sonySilkRefNum);
		
		//close silk lib
		SilkLibClose(g_sonySilkRefNum);
		
		SysNotifyUnregister( cardNo, dbID, sysNotifyDisplayChangeEvent, sysNotifyNormalPriority);
	
	}	

}

/***********************************************************************
 *
 * FUNCTION:    CollapseSetState
 *
 *	The user has just opened or closed the Active Input area.
 *	Now we need to resize the form to fill the draw window
 *	.
 * Set form to handle collapsible input area and set initial 
 * input area and trigger state. 
 *	
 *
 *
 * DESCRIPTION: Resizes form for active input -- filling available space
 *
 * PARAMETERS:  frmP - a pointer to a FormType struct
 				newState - selector for initial state
 *
 * RETURNED:   void
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			EZE			5/15/03			initial 
 *
 ***********************************************************************/

void CollapseSetState(FormType* frmP, UInt8 newState)
{

	Err theErr;
	RectangleType frmBounds;
	Coord x, y; 
	
	//set the min constraint values to match the initial forms values.
	//Note that an application may want to use a smaller value if the form can 
	//go smaller
	FrmGetFormBounds(frmP, &frmBounds);
	y = frmBounds.extent.y;
	x = frmBounds.extent.x;
	
	if (g_CollapseAPIVersion > 0)
	{
	
		switch(newState)
		{

			case collapseStateUp:
				WinSetConstraintsSize(WinGetWindowHandle(frmP), y, y, pinMaxConstraintSize, x, x, pinMaxConstraintSize);		
				FrmSetDIAPolicyAttr( frmP, frmDIAPolicyCustom );		
				PINSetInputTriggerState(pinInputTriggerEnabled);
				PINSetInputAreaState(pinInputAreaOpen);
				
				break;
				
			case collapseStateUser:
				//This is the default behaivor of apps -- do not pull up or push down input area
				//just respect what the user setting is
				WinSetConstraintsSize(WinGetWindowHandle(frmP), y, pinMaxConstraintSize, pinMaxConstraintSize, x, x, pinMaxConstraintSize);
				FrmSetDIAPolicyAttr( frmP, frmDIAPolicyCustom );
				PINSetInputTriggerState(pinInputTriggerEnabled);
				theErr = PINSetInputAreaState(pinInputAreaUser); 
				break;		
				
			case collapseStateDown:
				WinSetConstraintsSize(WinGetWindowHandle(frmP), y, pinMaxConstraintSize, pinMaxConstraintSize, x, x, pinMaxConstraintSize);		
				FrmSetDIAPolicyAttr( frmP, frmDIAPolicyCustom );
				PINSetInputTriggerState(pinInputTriggerEnabled);
				PINSetInputAreaState(pinInputAreaClosed);
				break;
				
				
			case collapseStateLockDown:
				WinSetConstraintsSize(WinGetWindowHandle(frmP), y, pinMaxConstraintSize, pinMaxConstraintSize, x, x, pinMaxConstraintSize);		
				FrmSetDIAPolicyAttr( frmP, frmDIAPolicyCustom );
				PINSetInputTriggerState( pinInputTriggerDisabled);
				PINSetInputAreaState(pinInputAreaClosed);
				break;
				
			//doing nothing results in legacy mode (lock input area up)
			//which is also the default behavior
			case collapseStateLockUp:
			default:
				break;
		
		}//switch(newState)
		
		
	}//if (g_CollapseAPIVersion > 0)
	else if (g_CollapseAPIVersion < 0)
	{
		//do any default state stuff
		switch(newState)
		{

			case collapseStateUp:
				SilkLibResizeDispWin(g_sonySilkRefNum, silkResizeNormal);
				SilkLibEnableResize(g_sonySilkRefNum);
				break;		
				
			case collapseStateDown:
				SilkLibResizeDispWin(g_sonySilkRefNum, silkResizeToStatus);
				SilkLibEnableResize(g_sonySilkRefNum);
				
				break;
				
				
			case collapseStateLockDown:
				SilkLibResizeDispWin(g_sonySilkRefNum, silkResizeToStatus);
				SilkLibDisableResize(g_sonySilkRefNum);
				break;
				
			case collapseStateLockUp:
				SilkLibResizeDispWin(g_sonySilkRefNum, silkResizeNormal);
				SilkLibDisableResize(g_sonySilkRefNum);				
				break;
			
			case collapseStateUser:
				//This is the default behaivor of apps -- do not pull up or push down input area
				//just respect what the user setting is. On Sony, this is the same as doing nothing to
				//change the state because silk is on or off on an app by app basis not form by form
				//SilkLibEnableResize(g_sonySilkRefNum);
				//break;
			default:
				break;		
		}
	}

}

/***********************************************************************
 *
 * FUNCTION:    CollapseVertResizeForm
 *
 *	The user has just opened or closed the Active Input area.
 *	Now we need to resize the form to fill the draw window
 *	.
 *
 *	
 *
 *
 * DESCRIPTION: Resizes form for active input -- filling available space
 *
 * PARAMETERS:  frmP - a pointer to a FormType struct
 				isModal specifies that this is a modal form
 				returnOffsetX, returnOffsetY - returns change in size of form to
 				 be used in relocating/resizing form objects
 *
 * RETURNED:    Boolean - true if resize was needed, else false
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			EZE			9/10/02			initial 
 *
 ***********************************************************************/
Boolean CollapseResizeForm(FormType *frmP, Boolean isModal, Int16* returnOffsetX, Int16* returnOffsetY)
{
	Coord			drawAreaHeightCoords, drawAreaWidthCoords;
	Int16			offsetCoordsX, offsetCoordsY;
	RectangleType	formBounds;
	WinHandle		frmWinH;


	//------------------------------------------------------------------
	// 1. Get the height of the drawing area available to this app
	//    (by calling WinGetDisplayExtent()). There has been a change in window diminsions
	//    such as the Active Input area being either opened or closed
	//   , and the value of the display extent will reflect that.
	//------------------------------------------------------------------
	WinGetDisplayExtent( &drawAreaWidthCoords, &drawAreaHeightCoords );

	//------------------------------------------------------------------
	// 2. Get the height/width of the current form's draw window and compute the
	//    difference between the draw window and the 
	//    the current app's drawing area. At this point,
	//    the form's draw window has not changed
	//------------------------------------------------------------------
	frmWinH = FrmGetWindowHandle(frmP);
	WinGetBounds(frmWinH, &formBounds);

	// Example #1: The user just closed the Active input area,
	//     so the drawing area is now > form draw window
	// eg: offsetCoordsY = drawAreaHeightCoords - formBounds.extent.y - formBounds.topLeft.y;
	//     offsetPixelsY = 225 - 160 - 0 = +65.

	// Example #2: The user just opened the Active input area,
	//     so the drawing area is now < form draw window (drawing area has shrunk !).
	// eg:offsetCoordsY = drawAreaHeightCoords - formBounds.extent.y - formBounds.topLeft.y;
	//     offsetPixels = 160 - 225 = -65.

	//for full screen windows topLeft.y is simply 0, or 1 for modal dialogs
	offsetCoordsX = drawAreaWidthCoords - formBounds.extent.x - formBounds.topLeft.x;
	offsetCoordsY = drawAreaHeightCoords - formBounds.extent.y - formBounds.topLeft.y;

	//modal dialogs need room for line at the bottom
 	if (isModal)
 	{
 		offsetCoordsX -=2;
 		offsetCoordsY -=2;
 	}	
 	
 	//return these values
 	*returnOffsetX = offsetCoordsX;
 	*returnOffsetY = offsetCoordsY;
 	
	//------------------------------------------------------------------
	// 3. If the form draw window is now smaller than the draw area,
	//    then we need to increase the size of the form draw window
	//    to equal the drawing area to take advantage of this newly
	//    available  space. The user just closed the Active input area.
	//    ... or ...
	//    If the form draw window is now larger than the draw area,
	//    then we need to decrease the size of the form draw window
	//    to equal the drawing area so no part of the draw window
	//    is outside this app's drawing area (or it'll get clipped).
	//    The user just opened the Active input area.
	//
	//------------------------------------------------------------------

	//------------------------------------------------------------------
	// 3. a. Increase/Decrease form height and width
	//------------------------------------------------------------------
	if (offsetCoordsX || offsetCoordsY)
	{
		formBounds.extent.x += offsetCoordsX;
		formBounds.extent.y += offsetCoordsY;

		// Change current draw window to match the display extent (app's drawing area)
		WinSetBounds( frmWinH, &formBounds );
		
		//return that the form has changed size
		return true;
	}
	
	return false;

}

/***********************************************************************
 *
 * FUNCTION:    CollapseMoveResizeDialog
 *
 *	The user has just opened or closed the Active Input area.
 *	Now we need to move the draw window to keep the dialod on the bottom of the screen. 
 *	This will move the form down to the bottom
 *
 *	We adjust the dialog to fill the screen left to right
 *
 *
 * DESCRIPTION: Moves form for active input
 *
 * PARAMETERS:  frmP - a pointer to a FormType struct
 				isModal defaults to modal dialog
 				returnOffsetX, returnOffsetY - returns change in size of form to be used in relocating/resizing form objects
 *
 * RETURNED:    Boolean - true if resize was needed, else false
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			EZE			9/10/02			initial 
 *
 ***********************************************************************/
Boolean CollapseMoveResizeDialog(FormType* frmP, Boolean isModal, Int16* returnOffsetX, Int16* returnOffsetY)
{
	Coord			drawAreaHeightCoords, drawAreaWidthCoords;
	RectangleType	formDrawWindowRect;
	Int16			offsetCoordsX, offsetCoordsY;
	WinHandle		formDrawWindowH;
	
	WinGetDisplayExtent( &drawAreaWidthCoords, &drawAreaHeightCoords);
	

	
	formDrawWindowH = WinGetWindowHandle( frmP );
	WinGetBounds( formDrawWindowH, &formDrawWindowRect );
	
	
	offsetCoordsX = drawAreaWidthCoords - formDrawWindowRect.extent.x - formDrawWindowRect.topLeft.x;	
	offsetCoordsY = drawAreaHeightCoords - formDrawWindowRect.extent.y - formDrawWindowRect.topLeft.y;	
	
	
	if (isModal)
	{
		offsetCoordsX -=2;
		offsetCoordsY -=2;
	}
	
	*returnOffsetX = offsetCoordsX;
 	*returnOffsetY = offsetCoordsY;
	
	if (offsetCoordsX || offsetCoordsY)
	{

		
		formDrawWindowRect.topLeft.y += offsetCoordsY;
		formDrawWindowRect.extent.x += offsetCoordsX;

		// Change current draw window to match the display extent (app's drawing area)
		WinSetBounds( formDrawWindowH, &formDrawWindowRect );
		
		return true;
	}
	
	return false;
}

/***********************************************************************
 *
 * FUNCTION:    CollapseMoveResizeFormObject
 *
 *  
 *
 *
 *
 * DESCRIPTION: Move form object by offset
 *
 * PARAMETERS:  frmP - a pointer to a FormType struct
 				objectID - ID of object
 				moveOffsetCoordsX, moveOffsetCoordsY - offset to move by
 				resizeOffsetCoordsX, resizeOffsetCoordsY - offset to resize by
 *
 * RETURNED:    void
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			EZE			9/10/02			initial 
 *
 ***********************************************************************/
void CollapseMoveResizeFormObject(FormType* frmP, UInt16 objectID, 
										Int16 moveOffsetCoordsX, Int16 moveOffsetCoordsY,
										Int16 resizeOffsetCoordsX, Int16 resizeOffsetCoordsY)
{
	
	RectangleType objectRect;

	FrmGetObjectBounds(frmP, FrmGetObjectIndex(frmP, objectID), &objectRect);
	
	objectRect.topLeft.x += moveOffsetCoordsX;
	objectRect.topLeft.y += moveOffsetCoordsY;
	objectRect.extent.x += resizeOffsetCoordsX;
	objectRect.extent.y += resizeOffsetCoordsY;	
	
	FrmSetObjectBounds(frmP, FrmGetObjectIndex(frmP, objectID), &objectRect);

}



/***********************************************************************
 *
 * FUNCTION:    CollapseMoveResizeFormObject
 *	Form bitmap objects need to be handled differently then other form objects
 *  
 *
 *
 *
 * DESCRIPTION: Move form object by offset
 *
 * PARAMETERS:  frmP - a pointer to a FormType struct
 				objectID - ID of object
 				moveOffsetCoordsX, moveOffsetCoordsY offset to move object by
 *
 * RETURNED:    void
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			EZE			5/20/03			initial 
 *
 ***********************************************************************/
void CollapseMoveFormBitmapObject(FormType* frmP, UInt16 objectID, 
										Int16 moveOffsetCoordsX, Int16 moveOffsetCoordsY)
{
	//Form Bitmaps just need to be repositioned. 
	Coord x, y;

	FrmGetObjectPosition(frmP, FrmGetObjectIndex(frmP, objectID), &x, &y);
	
	x += moveOffsetCoordsX;
	y += moveOffsetCoordsY;
	
	FrmSetObjectPosition(frmP, FrmGetObjectIndex(frmP, objectID), x, y);

}

/************************
*
*************************/
Err CollapseDisplayResizedEventCallback( SysNotifyParamType *notifyParamsP )
{
	//uniquely post back a winDisplayChangedEvent in response to the notification
	//PinMgr 1.0 only posts the notification while PinMgr 1.1 posts back. By doing it this way 
	//your code need only handle the winDisplayChangedEvent
	EventType eventToAdd;
	
	MemSet(&eventToAdd, sizeof(EventType), 0);

	eventToAdd.eType = (eventsEnum) winDisplayChangedEvent;
	
	EvtAddUniqueEventToQueue(&eventToAdd, 0, true);
	
	return 0;
}


UInt16 CollapseSetTriggerStates(UInt16 newState)
{
	UInt16 oldPinTriggerState=pinInputTriggerEnabled;

	if (g_CollapseAPIVersion > 0)
	{
		oldPinTriggerState = PINGetInputTriggerState();
		PINSetInputTriggerState(newState);
		
		//SysSetOrientationTriggerState needs to be added her as well
	}
	else if (g_CollapseAPIVersion == sonyVersion1)
	{
		
		
		if (newState == pinInputTriggerDisabled)
		{
			SilkLibDisableResize(g_sonySilkRefNum);
			oldPinTriggerState = pinInputTriggerEnabled;
		}
		else
		{
			SilkLibEnableResize(g_sonySilkRefNum);
			oldPinTriggerState = pinInputTriggerDisabled;
		}
	}
	else if (g_CollapseAPIVersion < sonyVersion1)
	{
		if (VskGetAPIVersion(g_sonySilkRefNum) >= 0x00000003)
		{	
			if (newState == pinInputTriggerDisabled)
			{
				VskSetState(g_sonySilkRefNum, vskStateEnable, vskResizeDisable);
				oldPinTriggerState = pinInputTriggerEnabled;
			}
			else
			{
				VskSetState(g_sonySilkRefNum, vskStateEnable, (vskResizeVertically|vskResizeHorizontally));
				oldPinTriggerState = pinInputTriggerDisabled;
			}	
		
		}
		else
		{
			if (newState == pinInputTriggerDisabled)
			{
				VskSetState(g_sonySilkRefNum, vskStateEnable, vskResizeDisable);
				oldPinTriggerState = pinInputTriggerEnabled;
			}
			else
			{
				VskSetState(g_sonySilkRefNum, vskStateEnable, vskResizeVertically);
				oldPinTriggerState = pinInputTriggerDisabled;
			}		
		}
		
	
	}
		
	return oldPinTriggerState; 				 
	

}
