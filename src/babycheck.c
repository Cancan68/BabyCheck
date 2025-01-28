#include <PalmOS.h>				
#include "babycheck.h"
#include "lang.h"
#include <SonyCLIE.h>

UInt16  sonyLibRefNum;

extern Boolean frmAbout_HandleEvent(EventPtr event) SECTION1;
void WordsExportMemo(void) SECTION1;
void MeasurExportMemo(void) SECTION1;
void SuckExportMemo(void) SECTION2;
Int16 SuckCompare(SuckDBType *r1, SuckDBType *r2, Int16 sortOrder, SortRecordInfoPtr info1, SortRecordInfoPtr info2, MemHandle appInfoH) SECTION2;
void SuckDetailFormInit(void) SECTION2;
Boolean SuckDetailFormHandleEvent(EventPtr event) SECTION2;
void SuckDrawDate(void *table, Int16 row, Int16 column, RectanglePtr bounds) SECTION2;
void SuckDrawTime(void *table, Int16 row, Int16 column, RectanglePtr bounds) SECTION2;
void SuckDrawSuck(void *table, Int16 row, Int16 column, RectanglePtr bounds) SECTION2;
void SuckDrawBreast(void *table, Int16 row, Int16 column, RectanglePtr bounds) SECTION2;
void SuckLoadTable(void) SECTION2;
void SuckCreate(void) SECTION2;
void SuckFormInit(FormType *frmP) SECTION2;
Boolean SuckFormHandleEvent(EventPtr event) SECTION2;

// #include "DateDB.c"
#include "CollapseUtils.c"
#include "prefs.c"
#include "tools.c"
#include "database.c"
#include "measurements.c"
#include "words.c"
#include "graph.c"
#include "symbol.c"
#include "shots.c"
#include "sucks.c"

/*
 * ConvertV1toV2
 * -------------
 *
 * Convert main database form version 1 to version 2
 *
 * Parameters: Nothing
 *
 * Returns: Nothing
 */
static void ConvertV1toV2(void)
{
  UInt16  i;
  MemHandle     recordH;
  BabyDBType    *recordPB;
  BabyDBTypeV1    *recordPBV1;

  for (i = 0; i < DmNumRecords(BabyDBRef); i++)
  {
    recordH = DmQueryRecord(BabyDBRef, i);
    if (recordH)
    {
      recordPBV1 = (BabyDBTypeV1 *) MemHandleLock(recordH);

      StrCopy(WRecord.name, recordPBV1->name);
      WRecord.time = recordPBV1->time;
      WRecord.sexe = 'M';
      MemHandleUnlock(recordH);
    }

    recordH = DmGetRecord(BabyDBRef, i);
    if (recordH)
    {
      recordH = DmResizeRecord(BabyDBRef, i, sizeof(BabyDBType));
      recordPB = (BabyDBType *) MemHandleLock(recordH);

      DmWrite(recordPB, 0, &WRecord, sizeof(BabyDBType));
      MemHandleUnlock(recordH);
    }
    DmReleaseRecord(BabyDBRef, i, true);
  }

  if (Prefs.MyVersion == 1)
    Prefs.Munit += 10;
  Prefs.MyVersion = 2;
  /* Save the App preferences */
  PrefSetAppPreferences (appFileCreator, appPrefID, appPrefVersion, &Prefs, prefsSize, true);
}

/*
 * BirthInfoBabyDBRecord
 * ---------------------
 * 
 * Update the birth information
 * 
 * Parameters: Nothing
 * 
 * Returns: Nothing
 */
static void BirthInfoBabyDBRecord(void)
{
  MemHandle recordHB, recordHM, txtHname;
  BabyDBType *recordPB;
  MeasurDBType *recordPM;
  Char *s;
  UInt16 indexB;
  EventType e;
  FormPtr frm = FrmGetActiveForm();

  txtHname = MemHandleNew(26);
  s = MemHandleLock(txtHname);
  GetStrFromFieldTxt(frm, fieldID_firstname, s);
  StrCopy(WRecord.name, s);
  WRecord.name[25] = '\0';   

  /* New record or update ? */

  if (newBaby)
  {
    indexB = num_baby;
    recordHB = DmNewRecord(BabyDBRef, &indexB, sizeof(BabyDBType));
    DmRecordInfo (BabyDBRef, indexB, NULL, &uniqueID, NULL);
    recordPB = (BabyDBType *) MemHandleLock(recordHB);

    /* Save the new record in the Database */

    DmWrite(recordPB, 0, &WRecord, sizeof(BabyDBType));
    MemHandleUnlock(recordHB);
    DmReleaseRecord(BabyDBRef, indexB, true);

    RecordNum = 0;
    recordHM = DmNewRecord(MeasurDBRef, &RecordNum, sizeof(MeasurDBType));
    recordPM = (MeasurDBType *) MemHandleLock(recordHM);

    WMRecord.weight = GetIntFromFieldTxt(frm, fieldID_weight, 100);
    WMRecord.height = GetIntFromFieldTxt(frm, fieldID_height, 10);
    WMRecord.number = uniqueID;

    DmWrite(recordPM, 0, &WMRecord, sizeof(MeasurDBType));
    MemHandleUnlock(recordHM);
    DmReleaseRecord(MeasurDBRef, RecordNum, true);
  }
  else 
  {
    indexB = LstGetSelection(listbaby);
    recordHB = DmGetRecord(BabyDBRef, indexB);
    recordPB = (BabyDBType *) MemHandleLock(recordHB);

    /* Save the new record in the Database */

    DmWrite(recordPB, 0, &WRecord, sizeof(BabyDBType));
    MemHandleUnlock(recordHB);
    DmReleaseRecord(BabyDBRef, indexB, true);

    recordHM = DmGetRecord(MeasurDBRef, RecordNum);
    recordPM = (MeasurDBType *) MemHandleLock(recordHM);

    WMRecord.weight = GetIntFromFieldTxt(frm, fieldID_weight, 100);
    WMRecord.height = GetIntFromFieldTxt(frm, fieldID_height, 10);

    DmWrite(recordPM, 0, &WMRecord, sizeof(MeasurDBType));
    MemHandleUnlock(recordHM);
    DmReleaseRecord(MeasurDBRef, RecordNum, true);
  }

  MemHandleUnlock(txtHname);
  MemHandleFree(txtHname);

  /* Update the list of babies */

  FrmReturnToForm(0);

  MemSet( &e, sizeof( EventType ), 0 );
  e.eType = winDisplayChangedEvent;
  EvtAddUniqueEventToQueue( &e, 0, true );

  LstEraseList(listbaby);

  if (newBaby)
    num_baby++;

  LstSetListChoices(listbaby, NULL, (Int16) num_baby);
  LstSetSelection(listbaby, (Int16) indexB);
  LstDrawList(listbaby);

  return;
}

/*
 * BirthFormInit
 * -------------
 * 
 * Update the birth information
 *
 * Parameters: Integer
 * 
 * Returns: Nothing
 */
static void BirthFormInit(FormType *frmP)
{ 
  ControlType *ctlP, *ctl2P;
  Char *labelP, *label2P;
  Int16 len;
 
  ctlP = FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, triggerID_birdate));
  labelP = (Char *)CtlGetLabel(ctlP);
  DateToDOWDMFormat(WMRecord.date.month, WMRecord.date.day, WMRecord.date.year + firstYear, dateFormat, labelP);
  CtlSetLabel(ctlP, labelP);

  ctl2P = FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, triggerID_birtime));
  label2P = (Char *)CtlGetLabel(ctl2P);
  TimeToAscii(WRecord.time.hours, WRecord.time.minutes, timeFormat, label2P);
  CtlSetLabel(ctl2P, label2P);

  SetFieldTextFromInt(fieldID_weight, WMRecord.weight, 100);
  SetFieldTextFromInt(fieldID_height, WMRecord.height, 10);

  SetFieldTextFromStr(fieldID_firstname, WRecord.name);

  if (Prefs.Munit == 0 || Prefs.Munit == 10)
    SetFieldTextFromStr(fieldID_weightu, unit1);
  else
    SetFieldTextFromStr(fieldID_weightu, unit3);

  if (Prefs.Munit < 9)
    SetFieldTextFromStr(fieldID_heightu, unit2);
  else
    SetFieldTextFromStr(fieldID_heightu, unit4);

  if (WRecord.sexe == 'M')
    CtlSetLabel(Item(listID_sexet), male);
  else
    CtlSetLabel(Item(listID_sexet), female);

  FrmSetFocus(frmP, FrmGetObjectIndex(frmP, fieldID_firstname));

  if (newBaby)
  {
    len = StrLen(WRecord.name);
    FldSetSelection(Item(fieldID_firstname), 0, len);
  }
}

/* 
 * StartApplication
 * ----------------
 *
 * Starts the Application
 *
 * Parameters: Nothing
 *
 * Returns: Err
 */
static Err StartApplication(void)
{
  Err 		error = 0;
  LocalID 	dbID;
  UInt16 attr, card;
  LocalID lid;

  /* Get the preferences */

  BoolPrefs = false;
  GetPreferences();

  /* Open the databases */

  error = DmCreateDatabase(0, BabyDBName, appFileCreator, appDBType, false);
  dbID  = DmFindDatabase(0, BabyDBName);
  BabyDBRef = DmOpenDatabase(0, dbID, dmModeReadWrite);

  if (Prefs.MyVersion < 2  || BoolPrefs)
    ConvertV1toV2();

  DmOpenDatabaseInfo(BabyDBRef, &lid, NULL, NULL, &card, NULL);
  DmDatabaseInfo(card, lid, NULL, &attr, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
  attr |= dmHdrAttrBackup;
  DmSetDatabaseInfo(card, lid, NULL, &attr, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

  error = DmCreateDatabase(0, MeasurDBName, appFileCreator, appDBType, false);
  dbID  = DmFindDatabase(0, MeasurDBName);
  MeasurDBRef = DmOpenDatabase(0, dbID, dmModeReadWrite);

  DmOpenDatabaseInfo(MeasurDBRef, &lid, NULL, NULL, &card, NULL);
  DmDatabaseInfo(card, lid, NULL, &attr, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
  attr |= dmHdrAttrBackup;
  DmSetDatabaseInfo(card, lid, NULL, &attr, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

  error = DmCreateDatabase(0, WordsDBName, appFileCreator, appDBType, false);
  dbID  = DmFindDatabase(0, WordsDBName);
  WordsDBRef = DmOpenDatabase(0, dbID, dmModeReadWrite);

  DmOpenDatabaseInfo(WordsDBRef, &lid, NULL, NULL, &card, NULL);
  DmDatabaseInfo(card, lid, NULL, &attr, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
  attr |= dmHdrAttrBackup;
  DmSetDatabaseInfo(card, lid, NULL, &attr, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

  error = DmCreateDatabase(0, SuckDBName, appFileCreator, appDBType, false);
  dbID  = DmFindDatabase(0, SuckDBName);
  SuckDBRef = DmOpenDatabase(0, dbID, dmModeReadWrite);

  DmOpenDatabaseInfo(SuckDBRef, &lid, NULL, NULL, &card, NULL);
  DmDatabaseInfo(card, lid, NULL, &attr, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
  attr |= dmHdrAttrBackup;
  DmSetDatabaseInfo(card, lid, NULL, &attr, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

  CollapseAppStart();

  FrmPopupForm(formID_main);

  return (0);
}

/*  
 * StopApplication
 * ---------------
 *
 * Close the application and all forms
 *
 * Parameters: Nothing
 *
 * Returns: Nothing
 */
static void StopApplication(void)
{
  CollapseAppStop();

  FrmCloseAllForms();

  if(sonyLibRefNum)
  {
    HRWinScreenMode(sonyLibRefNum, winScreenModeSetToDefaults, NULL, NULL, NULL,
 NULL);
    HRClose(sonyLibRefNum);
    SysLibRemove(sonyLibRefNum);
  }

  if (BabyDBRef)
    DmCloseDatabase(BabyDBRef);

  if (WordsDBRef)
    DmCloseDatabase(WordsDBRef);

  if (MeasurDBRef)
    DmCloseDatabase(MeasurDBRef);

  if (SuckDBRef)
    DmCloseDatabase(SuckDBRef);
}

/*
 * NewBaby
 * -------
 *
 * Create a new baby in the list
 *
 * Parameters: Nothing
 *
 * Returns: Nothing
 */
static void NewBaby(void)
{
  DateTimeType dateTime;

  StrCopy(WRecord.name, BabyNewName);

  /* Get today's Date */

  TimSecondsToDateTime(TimGetSeconds(), &dateTime);
  WRecord.time.minutes = dateTime.minute;
  WRecord.time.hours = dateTime.hour;
  WRecord.sexe = 'M';

  WMRecord.date.year = dateTime.year - firstYear;
  WMRecord.date.month = dateTime.month;
  WMRecord.date.day = dateTime.day;
  WMRecord.weight = 0;
  WMRecord.height = 0;

  /* Set Flag for New Baby */

  newBaby = true;

  FrmPopupForm(formID_birth);

  return;
}

/*
 * UpdateBaby
 * ----------
 *
 * Update a baby in the list
 *
 * Parameters: Nothing
 *
 * Returns: Nothing
 */
static void UpdateBaby(void)
{
  ReadBabyRecord();
  ReadBabyMeasur();

  /* Set Flag for Update Baby */

  newBaby = false;

  FrmPopupForm(formID_birth);

  return;
}

/*
 * DeleteBaby
 * ----------
 *
 * Delete a Baby in the list
 *  
 * Parameters: Nothing
 *
 * Returns: Nothing
 */
static void DeleteBaby(void)
{ 
  Int16		index;
  UInt16	i;
  Err		err;
  MemHandle	recordH;
  WordsDBType	*recordPW;
  MeasurDBType	*recordPM;
  
  index = LstGetSelection(listbaby);

  /* Read the record to find the baby unique number */

  ReadBabyRecord();

  /* Delete the records in the Words Database */

  for (i = 0; i < DmNumRecords(WordsDBRef); i++)
  {
    recordH = DmQueryRecord(WordsDBRef, i);
    if (recordH)
    {
      recordPW = (WordsDBType *) MemHandleLock(recordH);
      if (recordPW->number == uniqueID)
      {
        MemHandleUnlock(recordH);
        DmRemoveRecord(WordsDBRef, i);
      }
      else
        MemHandleUnlock(recordH);
    }
  }

  /* Delete the records in the Measurements Database */

  for (i = 0; i < DmNumRecords(MeasurDBRef); i++)
  {
    recordH = DmQueryRecord(MeasurDBRef, i);
    if (recordH)
    {
      recordPM = (MeasurDBType *) MemHandleLock(recordH);
      if (recordPM->number == uniqueID)
      {
        MemHandleUnlock(recordH);
        DmRemoveRecord(MeasurDBRef, i);
      }
      else
        MemHandleUnlock(recordH);
    }
  }

  /* Delete the record in the Database */

  err = DmRemoveRecord(BabyDBRef, index);

  num_baby--;
  LstSetSelection(listbaby, 0);
  LstSetListChoices(listbaby, NULL, (Int16) num_baby);
  LstDrawList(listbaby);

  return;
}

/*
 * MainFormInit
 * ------------
 *
 * Initialize the main form
 *
 * Parameters: Nothing
 *
 * Returns: Nothing
 */
static void MainFormInit(void)
{
  FormType *frmP;
  SonySysFtrSysInfoP sonySysFtrSysInfoP;
  Err error = 0;
  MemHandle recordHB;
  Int16 i;
  Int16 offx, offy;

  frmP = FrmGetActiveForm();

  CollapseSetState(frmP, collapseStateUser);

  if (romVersion >= 0x04000000)
  {
    if (CollapseResizeForm(frmP, false, &offx, &offy))
    {
      if (offy > 0)
        offy += 2;
      if (offy < 0)
        offy -= 2;
      CollapseMoveResizeFormObject(frmP, listID_baby, 0, 0, offx, offy);
      CollapseMoveResizeFormObject(frmP, buttonID_new, 0, offy, 0, 0);
      CollapseMoveResizeFormObject(frmP, buttonID_ren, 0, offy, 0, 0);
      CollapseMoveResizeFormObject(frmP, buttonID_sup, 0, offy, 0, 0); 
      CollapseMoveResizeFormObject(frmP, buttonID_mes, offx, 0, 0, 0);
      CollapseMoveResizeFormObject(frmP, buttonID_wor, offx, 0, 0, 0);
      CollapseMoveResizeFormObject(frmP, buttonID_action, offx, 0, 0, 0);
      CollapseMoveResizeFormObject(frmP, buttonID_shots, offx, 0, 0, 0);
      CollapseMoveResizeFormObject(frmP, buttonID_sym, offx, 0, 0, 0);
      CollapseMoveResizeFormObject(frmP, buttonID_app, offx, 0, 0, 0);
      CollapseMoveResizeFormObject(frmP, buttonID_suc, offx, 0, 0, 0);
    }
  }

  /* find out whether we are running on a sony high-resolution screen  */
  if (IsOS5)
    sonyLibRefNum = 0;
  else
  {
    if((error = FtrGet(sonySysFtrCreator, sonySysFtrNumSysInfoP, (UInt32 *)&sonySysFtrSysInfoP)))
    {
    }
    else
    {
      if(sonySysFtrSysInfoP->libr & sonySysFtrSysInfoLibrHR)
      { 
        if ((error = SysLibFind(sonySysLibNameHR, &sonyLibRefNum)))
        {
          if(error == sysErrLibNotFound)
          {
            error = SysLibLoad( 'libr', sonySysFileCHRLib, &sonyLibRefNum);
          }
        }
  
        if (!error)
        {
          HROpen(sonyLibRefNum);
          Resolx = Resoly = Resolution = 320;
          HRWinScreenMode(sonyLibRefNum, winScreenModeGetSupportedDepths, NULL, NULL, &Depth, NULL);
          HRWinScreenMode(sonyLibRefNum, winScreenModeGetSupportsColor, NULL, NULL, NULL, &useColor);
          error = HRWinScreenMode(sonyLibRefNum, winScreenModeSet, &Resolx, &Resoly, NULL, NULL);
        }
      }
    }
  }
 
  listbaby = FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, listID_baby));

  /* Read the Database and count the number of Item
     Can't use DmNumRecords because the deleted records are counted */

  num_baby = 0; 
  for (i = 0; i < DmNumRecords(BabyDBRef); i++)
  {
    recordHB = DmQueryRecord(BabyDBRef, i);
    if (recordHB)
    {
      num_baby++;
    }
  }

  LstSetListChoices(listbaby, NULL, (Int16) num_baby);
  LstSetDrawFunction(listbaby, BabyDrawList);

  FrmDrawForm(frmP); 

  return;
}

/*
 * MainFormHandleEvent
 * -------------------
 *
 * Hanlde the main form
 *
 * Parameters: Event
 *
 * Returns: Boolean
 */
static Boolean MainFormHandleEvent(EventPtr event)
{
  Boolean handled = false;
  
  switch (event->eType)
  {

  case frmOpenEvent:	
    MainFormInit();
    break;

  case winDisplayChangedEvent:
  {
    Int16 offx, offy;
    FormType *frmP;

    frmP = FrmGetActiveForm();

    if (romVersion >= 0x04000000)
    {
      if (CollapseResizeForm(frmP, false, &offx, &offy))
      {
        if (offy > 0)
          offy += 2;
        if (offy < 0)
          offy -= 2;
        CollapseMoveResizeFormObject(frmP, listID_baby, 0, 0, offx, offy);
        CollapseMoveResizeFormObject(frmP, buttonID_new, 0, offy, 0, 0);
        CollapseMoveResizeFormObject(frmP, buttonID_ren, 0, offy, 0, 0);
        CollapseMoveResizeFormObject(frmP, buttonID_sup, 0, offy, 0, 0); 
        CollapseMoveResizeFormObject(frmP, buttonID_mes, offx, 0, 0, 0);
        CollapseMoveResizeFormObject(frmP, buttonID_wor, offx, 0, 0, 0);
        CollapseMoveResizeFormObject(frmP, buttonID_action, offx, 0, 0, 0);
        CollapseMoveResizeFormObject(frmP, buttonID_shots, offx, 0, 0, 0);
        CollapseMoveResizeFormObject(frmP, buttonID_sym, offx, 0, 0, 0);
        CollapseMoveResizeFormObject(frmP, buttonID_app, offx, 0, 0, 0);
        CollapseMoveResizeFormObject(frmP, buttonID_suc, offx, 0, 0, 0);
      }
    }
    FrmDrawForm(frmP); 
    handled = true;
    break;
  }

  case menuEvent:
    MenuEraseStatus(0);
    if(event->data.menu.itemID == menuitemID_new)
    {
      NewBaby();
      handled = true;
    }
    if(event->data.menu.itemID == menuitemID_ren)
    {
      if (num_baby > 0)
        UpdateBaby();
      else
        DisplayError(stringID_selbaby, formID_error);
      handled = true;
    }
    if (event->data.menu.itemID == menuitemID_sup)
    {
      if (num_baby > 0)
      {
        if (FrmAlert(formID_delete) == 0)
        {
          EventType e;
          DeleteBaby();

          MemSet( &e, sizeof( EventType ), 0 );
          e.eType = winDisplayChangedEvent;
          EvtAddUniqueEventToQueue( &e, 0, true );
        }
      }
      else
        DisplayError(stringID_selbaby, formID_error);
      handled = true;
    }
    if(event->data.menu.itemID == menuitemID_words)
    {
      if (num_baby > 0)
      {
        WordsCode = 'W';
        FrmPopupForm(formID_words);
      }
      else
        DisplayError(stringID_selbaby, formID_error);
      handled = true;
    }
    if(event->data.menu.itemID == menuitemID_action)
    {
      if (num_baby > 0)
      {
        WordsCode = 'A';
        FrmPopupForm(formID_words);
      }
      else
        DisplayError(stringID_selbaby, formID_error);
      handled = true;
    }
    if(event->data.menu.itemID == menuitemID_shots)
    {
      if (num_baby > 0)
      {
        WordsCode = 'S';
        FrmPopupForm(formID_shots);
      }
      else
        DisplayError(stringID_selbaby, formID_error);
      handled = true;
    }
    if(event->data.menu.itemID == menuitemID_symb)
    {
      if (num_baby > 0)
        FrmPopupForm(formID_symbol);
      else
        DisplayError(stringID_selbaby, formID_error);
      handled = true;
    }
    if(event->data.menu.itemID == menuitemID_pref)
    {
      FrmPopupForm(formID_pref);
      handled = true;
    }
    if(event->data.menu.itemID == menuitemID_mes)
    {
      if (num_baby > 0)
        FrmPopupForm(formID_mes);
      else
        DisplayError(stringID_selbaby, formID_error);
      handled = true;
    }
    if (event->data.menu.itemID == menuitemID_info)
    {
      EventType e;
      FrmHelp(stringID_info);
      MemSet( &e, sizeof( EventType ), 0 );
      e.eType = winDisplayChangedEvent;
      EvtAddUniqueEventToQueue( &e, 0, true );
      handled = true;
    }
    if(event->data.menu.itemID == menuitemID_about)
    {
      FrmPopupForm(formID_about);
      handled = true;
    }
    break;

  case ctlSelectEvent:
    if (event->data.ctlEnter.controlID == buttonID_new)
    {
      NewBaby();
      handled = true;
    }
    if (event->data.ctlEnter.controlID == buttonID_ren)
    {
      if (num_baby > 0)
        UpdateBaby();
      else
        DisplayError(stringID_selbaby, formID_error);
      handled = true;
    }
    if (event->data.ctlEnter.controlID == buttonID_sup)
    {
      if (num_baby > 0)
      {
        if (FrmAlert(formID_delete) == 0)
        {
          EventType e;
          DeleteBaby();

          MemSet( &e, sizeof( EventType ), 0 );
          e.eType = winDisplayChangedEvent;
          EvtAddUniqueEventToQueue( &e, 0, true );
        }
      }
      else
        DisplayError(stringID_selbaby, formID_error);
      handled = true;
    }
    if (event->data.ctlEnter.controlID == buttonID_wor)
    {
      if (num_baby > 0)
      {
        WordsCode = 'W';
        FrmPopupForm(formID_words);
      }
      else
        DisplayError(stringID_selbaby, formID_error);
      handled = true;
    }
    if(event->data.ctlEnter.controlID == buttonID_action)
    {
      if (num_baby > 0)
      {
        WordsCode = 'A';
        FrmPopupForm(formID_words);
      }
      else
        DisplayError(stringID_selbaby, formID_error);
      handled = true;
    }
    if(event->data.ctlEnter.controlID == buttonID_shots)
    {
      if (num_baby > 0)
      {
        WordsCode = 'S';
        FrmPopupForm(formID_shots);
      }
      else
        DisplayError(stringID_selbaby, formID_error);
      handled = true;
    }
    if (event->data.ctlEnter.controlID == buttonID_sym)
    {
      if (num_baby > 0)
        FrmPopupForm(formID_symbol);
      else
        DisplayError(stringID_selbaby, formID_error);
      handled = true;
    } 
    if (event->data.ctlEnter.controlID == buttonID_suc)
    {
      if (num_baby > 0)
        FrmPopupForm(formID_suc);
      else
        DisplayError(stringID_selbaby, formID_error);
      handled = true;
    } 
    if (event->data.ctlEnter.controlID == buttonID_mes)
    {
      if (num_baby > 0)
        FrmPopupForm(formID_mes);
      else
        DisplayError(stringID_selbaby, formID_error);
      handled = true;
    }
    if(event->data.ctlEnter.controlID == buttonID_app)
    {
      FrmPopupForm(formID_about);
      handled = true;
    }
    break;

  }			
  return(handled);
}

/*
 * BirthFormHandleEvent
 * --------------------
 *
 * Handle the birth information form
 *
 * Parameters: Event
 *
 * Returns: Boolean
 */
static Boolean BirthFormHandleEvent(EventPtr event)
{
  Boolean handled = false;
  FormType *frmP = FrmGetActiveForm();

  switch (event->eType)
  {

  case frmOpenEvent:
    CollapseSetState(frmP, collapseStateUser);
    FrmDrawForm(frmP);
    BirthFormInit(frmP);
    handled = true;
    break;

  case winDisplayChangedEvent:
    FrmDrawForm(frmP);  
    handled = true;
    break;

  case ctlSelectEvent:
    if (event->data.ctlEnter.controlID == buttonID_birok)
    {
       BirthInfoBabyDBRecord();
       handled = true;
    }
    if (event->data.ctlEnter.controlID == buttonID_bircancel)
    {
      EventType e;
      MemSet( &e, sizeof( EventType ), 0 );
      e.eType = winDisplayChangedEvent;
      EvtAddUniqueEventToQueue( &e, 0, true );
      FrmReturnToForm(0);
      handled = true;
    }
    if (event->data.ctlEnter.controlID == triggerID_birdate)
    {
        OutilsSelectDate(&WMRecord.date, event->data.ctlSelect.pControl, stringID_birdate);
        handled = true;
    }
    if (event->data.ctlEnter.controlID == triggerID_birtime)
    {
        OutilsSelectTime(&WRecord.time, event->data.ctlSelect.pControl, stringID_birtime);
        handled = true;
    }
    if (event->data.ctlEnter.controlID == listID_sexet)
    {
      ListPtr sexew = FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, listID_sexe));
      short choice;

      if (WRecord.sexe == 'F')
        LstSetSelection(sexew, 1);
      else
        LstSetSelection(sexew, 0);
      choice = LstPopupList(sexew);
      if (choice == 0)
      {
        WRecord.sexe = 'M';
        CtlSetLabel(Item(listID_sexet), male);
      }
      else
      {
        if (choice == 1)
        {
          WRecord.sexe = 'F';
          CtlSetLabel(Item(listID_sexet), female);
        }
      }
      handled = true;
    }
    break;

  }
  return(handled);
}

/*
 * ApplicationHandleEvent
 * ----------------------
 *
 * Hanlde the application
 *
 * Parameters: Event
 *
 * Returns: Boolean
 */
static Boolean ApplicationHandleEvent(EventPtr event)
{
  FormPtr frm;
  Int16	formId;
  Boolean handled = false;

  if (event->eType == frmLoadEvent)
  {
    // Load the form resource specified in the event then activate the form.
    formId = event->data.frmLoad.formID;
    frm = FrmInitForm(formId);
    FrmSetActiveForm(frm);

    /*
     * Set the event handler for the form.  The handler of the currently 
     * active form is called by FrmDispatchEvent each time it receives an event
     */
    switch (formId)
    {
      case formID_main:
        FrmSetEventHandler(frm, MainFormHandleEvent);
        handled = true;
        break;
      case formID_birth:
        FrmSetEventHandler(frm, BirthFormHandleEvent);
        handled = true;
        break;
      case formID_symbol:
        FrmSetEventHandler(frm, SymbolFormHandleEvent);
        handled = true;
        break;
      case formID_words:
        FrmSetEventHandler(frm, WordsFormHandleEvent);
        handled = true;
        break;
      case formID_graph:
        FrmSetEventHandler(frm, GraphFormHandleEvent);
        handled = true;
        break;
      case formID_mes:
        FrmSetEventHandler(frm, MeasurementsFormHandleEvent);
        handled = true;
        break;
      case formID_mesdet:
        FrmSetEventHandler(frm, MeasurementsDetailFormHandleEvent);
        handled = true;
        break;
      case formID_shots:
        FrmSetEventHandler(frm, ShotsFormHandleEvent);
        handled = true;
        break;
      case formID_pref:
        FrmSetEventHandler(frm, PrefsFormHandleEvent);
        handled = true;
        break;
      case formID_suc:
        FrmSetEventHandler(frm, SuckFormHandleEvent);
        handled = true;
        break;
      case formID_sucdet:
        FrmSetEventHandler(frm, SuckDetailFormHandleEvent);
        handled = true;
        break;
      case formID_about:
        FrmSetEventHandler(frm, frmAbout_HandleEvent);
        handled = true;
        break;
    }
  }
  return handled;
}

/*
 * EventLoop
 * ---------
 *
 * Main application loop
 *
 * Parameters: Nothing
 *
 * Returns: Nothing
 */
static void EventLoop(void)
{
  EventType event;
  UInt16 error;
	
  do {
    EvtGetEvent(&event, evtWaitForever);
    if (!SysHandleEvent(&event))
      if (!MenuHandleEvent(0, &event, &error))
	if (!ApplicationHandleEvent(&event))
	  FrmDispatchEvent(&event);
  }
  while (event.eType != appStopEvent);
}

/*
 * PilotMain
 * ---------
 *
 * Main Function
 *
 */
UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
  Err error = 0;		
    
  if (cmd == sysAppLaunchCmdNormalLaunch)
  {
    error = GetRomVersion(launchFlags);
    if (error)
    {
      return error;
    }
    if ((error = StartApplication()) == 0)
    {   
      EventLoop();
      StopApplication();
    }
  }
  return error;
}
