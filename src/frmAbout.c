/*
 *  Project:    TxtMemo
 *  Author:     Laurent Campredon <laurent@campredon.net>
 *
 *  File:       frmAbout.c
 */

#include "frmAbout.h"   

/*
 * frmAbout_HandleEvent
 * --------------------
 *  
 * Handle the about screen
 *  
 * Parameters: Event
 *
 * Returns:    Boolean
 */
Boolean frmAbout_HandleEvent(EventPtr event)
{
  Boolean handled = false;
  FormType *frmP = FrmGetActiveForm();

  switch (event->eType)
  {
    case frmOpenEvent:
    {
      Int16 offx, offy, offy2;
      frmP = FrmGetActiveForm(); 
  
      if (romVersion >= 0x04000000)
      {
        CollapseSetState(frmP, collapseStateUser);
        if (CollapseResizeForm(frmP, true, &offx, &offy))
        {
          offx = offx / 2;
          CollapseMoveFormBitmapObject(frmP, bitmapID_app, offx, 0);
          CollapseMoveResizeFormObject(frmP, labelID_version, offx, 0, 0, 0);
          CollapseMoveResizeFormObject(frmP, labelID_copyright, offx, 0, 0, 0);
          CollapseMoveResizeFormObject(frmP, labelID_freeware, offx, offy, 0, 0);
          CollapseMoveResizeFormObject(frmP, buttonID_aboutok, offx, offy, 0, 0);
          offy2 = offy / 2;
          CollapseMoveResizeFormObject(frmP, labelID_site, offx, offy2, 0, 0);
          CollapseMoveResizeFormObject(frmP, labelID_email, offx, offy2, 0, 0);
        }
      }
      FrmDrawForm(frmP);
      handled = true;
      break;
    }

    case winDisplayChangedEvent:
    {
      Int16 offx, offy, offy2;
      frmP = FrmGetActiveForm();

      if (romVersion >= 0x04000000)
      {
        if (CollapseResizeForm(frmP, true, &offx, &offy))
        {
          offx = offx / 2;
          CollapseMoveFormBitmapObject(frmP, bitmapID_app, offx, 0);
          CollapseMoveResizeFormObject(frmP, labelID_version, offx, 0, 0, 0);
          CollapseMoveResizeFormObject(frmP, labelID_copyright, offx, 0, 0, 0);
          CollapseMoveResizeFormObject(frmP, labelID_freeware, offx, offy, 0, 0);
          CollapseMoveResizeFormObject(frmP, buttonID_aboutok, offx, offy, 0, 0);
          offy2 = offy / 2;
          CollapseMoveResizeFormObject(frmP, labelID_site, offx, offy2, 0, 0);
          CollapseMoveResizeFormObject(frmP, labelID_email, offx, offy2, 0, 0);
        }
      }
      FrmDrawForm(frmP);
      handled = true;
      break;
    }

    case ctlSelectEvent:
    {
      EventType e;

      MemSet( &e, sizeof( EventType ), 0 );
      e.eType = winDisplayChangedEvent;
      EvtAddUniqueEventToQueue( &e, 0, true );
      FrmReturnToForm(0);
      handled = true;
      break;
    }
    
    break;
  }
  return handled;
}
