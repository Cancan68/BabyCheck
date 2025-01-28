//CollapseUtils.h
#ifndef COLLAPSE_UTILS_H
#define COLLAPSE_UTILS_H

#include <PalmOS.h>

//CollapseUtil constants
#define collapseStateLockUp 		0
#define collapseStateUp				1
#define collapseStateUser			2
#define collapseStateDown			3
#define collapseStateLockDown		4									
				
				
#define 	sonyVersion1	-1
#define		sonyVersion2	-2	

#ifndef SetBits
#define SetBits( b, len )      ( ( ( 1U << ( ( len ) - 1 ) ) - 1U + ( 1U << ( ( len ) - 1 ) ) ) << ( b ) )
#endif

//pinMaxConstraintSize is set to the max value of a Coord such that it can grow as big as the 
//hardware supports
#ifndef pinMaxConstraintSize
#define pinMaxConstraintSize 	SetBits( 0, ( sizeof( Coord) * 8 ) - 1 )
#endif


//Collapse Util func declare
void CollapseAppStart();
Int32 CollapseGetAPIVersion();
void CollapseAppStop();
void CollapseSetState(FormType* frmP, UInt8 newState);
Boolean CollapseResizeForm(FormType *frmP, Boolean isModal, Int16* returnOffsetX, Int16* returnOffsetY);
Boolean CollapseMoveResizeDialog(FormType* frmP, Boolean isModal, Int16* returnOffsetX, Int16* returnOffsetY);
void CollapseMoveResizeFormObject(FormType* frmP, UInt16 objectID, 
										Int16 moveOffsetCoordsX, Int16 moveOffsetCoordsY,
										Int16 resizeOffsetCoordsX, Int16 resizeOffsetCoordsY);
void CollapseMoveFormBitmapObject(FormType* frmP, UInt16 objectID, 
										Int16 moveOffsetCoordsX, Int16 moveOffsetCoordsY);
Err CollapseDisplayResizedEventCallback( SysNotifyParamType *notifyParamsP );	
UInt16 CollapseSetTriggerStates(UInt16 newState);



#endif // COLLAPSE_UTILS_H