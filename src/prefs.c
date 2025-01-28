//#include "prefs.h"

/*
 * GetPreferences
 * --------------
 *  
 * Handle the preferences screen
 *  
 * Parameters: Event
 *
 * Returns: Boolean
 */
void GetPreferences(void)
{
  /* Get the decimal separator */
  PrefGetPreferences(&syspref);
  LocGetNumberSeparators((NumberFormatType) PrefGetPreference(prefNumberFormat), &thoSep, &decSep);

  /* Get the date and time format */
  timeFormat = (TimeFormatType)PrefGetPreference(prefTimeFormat);
  dateFormat = (DateFormatType)PrefGetPreference(prefDateFormat);

  /* Get the App preferences */
  if (PrefGetAppPreferences(appFileCreator, appPrefID, &Prefs, &prefsSize, true) == noPreferenceFound)
  {
    /* No preferences found so initialize them */
    Prefs.Munit = 0;
    Prefs.MyVersion = 2;
    PrefSetAppPreferences (appFileCreator, appPrefID, appPrefVersion, &Prefs, prefsSize, true);
    BoolPrefs = true;
  }
}

/*
 * PrefsFormHandleEvent
 * --------------------
 *  
 * Handle the preferences screen
 *  
 * Parameters: Event
 *
 * Returns: Boolean
 */
Boolean PrefsFormHandleEvent(EventPtr event)
{
  Boolean handled = false;
  FormType *frmP = FrmGetActiveForm();

  switch (event->eType)
  {

  case frmOpenEvent:
    CollapseSetState(frmP, collapseStateUser);
    if (Prefs.Munit == 0 || Prefs.Munit == 10)
      CtlSetLabel(Item(listID_preftw), unit1);
    else
      CtlSetLabel(Item(listID_preftw), unit3);
    if (Prefs.Munit == 0 || Prefs.Munit == 1)
      CtlSetLabel(Item(listID_prefth), unit2);
    else
      CtlSetLabel(Item(listID_prefth), unit4);
    oldMunit = Prefs.Munit;
    FrmDrawForm(frmP);
    handled = true;
    break;

  case ctlSelectEvent:
    if (event->data.ctlEnter.controlID == listID_preftw)
    {
      ListPtr unitsw = FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, listID_prefw));
      short choice;
      if (Prefs.Munit == 1  || Prefs.Munit == 11)
        LstSetSelection(unitsw, 1);
      else
        LstSetSelection(unitsw, 0);
      choice = LstPopupList(unitsw);
      if (choice == 0)
      {
        if (Prefs.Munit == 1  || Prefs.Munit == 11)
          Prefs.Munit = Prefs.Munit - 1;
        CtlSetLabel(Item(listID_preftw), unit1);
      }
      if (choice == 1)
      {
        if (Prefs.Munit == 0  || Prefs.Munit == 10)
          Prefs.Munit = Prefs.Munit + 1;
        CtlSetLabel(Item(listID_preftw), unit3);
      }
      handled = true;
    }
    if (event->data.ctlEnter.controlID == listID_prefth)
    {
      ListPtr unitsh = FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, listID_prefh));
      short choice;
      if (Prefs.Munit > 9)
        LstSetSelection(unitsh, 1);
      else
        LstSetSelection(unitsh, 0);
      choice = LstPopupList(unitsh);
      if (choice == 0)
      {
        if (Prefs.Munit > 9)
          Prefs.Munit = Prefs.Munit - 10;
        CtlSetLabel(Item(listID_prefth), unit2);
      }
      if (choice == 1)
      {
        if (Prefs.Munit < 10)
          Prefs.Munit = Prefs.Munit + 10;
        CtlSetLabel(Item(listID_prefth), unit4);
      }
      handled = true;
    }
    if (event->data.ctlEnter.controlID == buttonID_prefok)
    {
        EventType e;

       /* Save the App preferences */
       PrefSetAppPreferences (appFileCreator, appPrefID, appPrefVersion, &Prefs, prefsSize, true);
       FrmReturnToForm(0);
        MemSet( &e, sizeof( EventType ), 0 );
        e.eType = winDisplayChangedEvent;
        EvtAddUniqueEventToQueue( &e, 0, true );
       handled = true;
    }
    if (event->data.ctlEnter.controlID == buttonID_prefcancel)
    {
        EventType e;

       Prefs.Munit = oldMunit;
       FrmReturnToForm(0);
        MemSet( &e, sizeof( EventType ), 0 );
        e.eType = winDisplayChangedEvent;
        EvtAddUniqueEventToQueue( &e, 0, true );
       handled = true;
    }
    break;
  }
  return(handled);
}
