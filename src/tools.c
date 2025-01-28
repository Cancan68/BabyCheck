/*
 * BabyDrawList
 * ------------
 *
 * Draws a baby in the list
 *
 * Parameters: see plam sdk LstSetDrawFunction
 *
 * Returns: Nothing
 */
static void BabyDrawList(Int16 babynum, RectangleType *bounds, Char **listbabies)
{
  MemHandle recordHB;
  BabyDBType *recordPB;
  Boolean FitInWidth;
  Char            horzEllipsis;
  UInt16  len, listWidth;
  FontID  curFont;
  Char szTitle[30];

  /* Read the Database and fill the list */

  recordHB = DmQueryRecord(BabyDBRef, babynum);
  if (recordHB)
  {
    recordPB = (BabyDBType *) MemHandleLock(recordHB);
    StrCopy(szTitle, recordPB->name);
    for (len = 0; len < StrLen(szTitle); len++)
    {
      if (szTitle[len] == '\n')
        szTitle[len] = ' ';
    }

    len = StrLen(szTitle);
    curFont = FntSetFont(stdFont );
    listWidth = (UInt16) bounds->extent.x - 10;
    FntCharsInWidth(szTitle, &listWidth, &len, &FitInWidth );
    FntSetFont(curFont);
    if(!FitInWidth)
    {
      ChrHorizEllipsis( &horzEllipsis );
      szTitle[len-1] = horzEllipsis;
      szTitle[len] = '\0';
    }

    WinDrawChars(szTitle, StrLen(szTitle), bounds->topLeft.x + 1, bounds->topLeft.y );

    MemHandleUnlock(recordHB);
  }
}

/*
 * DisplayError
 * ------------
 *
 * Display an error in an Error Form
 *
 * Parameters: Error string ID
 *
 * Returns: Nothing
 */
void DisplayError(UInt16 errorID, UInt16 formID)
{
  Char   *s;
  MemHandle  error;
  EventType e;

  error = DmGetResource(strRsc, errorID);
  s = MemHandleLock(error);
  FrmCustomAlert(formID, s, NULL, NULL);
  MemHandleUnlock(error);
  DmReleaseResource(error);
  MemSet( &e, sizeof( EventType ), 0 );
  e.eType = winDisplayChangedEvent;
  EvtAddUniqueEventToQueue( &e, 0, true );

  return;
}

/*
 * GetIntFromFieldTxt
 * ------------------
 *  
 * Get a string and convert into an integer
 *  
 * Parameters: From pointer, Field ID, Indice
 *  
 * Returns: Converted Interger in UInt16
 */
UInt16 GetIntFromFieldTxt(FormPtr frm, UInt16 fieldID, UInt16 indice)
{
  UInt32 decimals = 0;
  UInt32 places;
  UInt32 i = 0;
  UInt32 haddecimal = 0;
  Char *buffer;

  MemHandle hnd = (MemHandle) FldGetTextHandle(FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, fieldID)));
  buffer = MemHandleLock(hnd);
 
  if (indice == 100)
    places = 2;
  else
    if (indice == 10)
      places = 1;
    else
      places = 0;

  while (*buffer)
  {
    if (*buffer ==  '.'  || *buffer == ',')
    {
      haddecimal = 1;
      decimals = 0;
    }
    else
    {
      if (*buffer >= '0' || *buffer <= '9')
      {
        if (haddecimal == 1)
        {
          if (decimals < places)
          {
            decimals++;
            i = i * 10 + *buffer - '0';
          }
        }
        else
          i = i * 10 + *buffer - '0';
      } 
    }
    buffer++;
  }

  MemHandleUnlock(hnd);
  while(decimals < places)
  {
    decimals++;
    i *= 10;
  }
 
  if (i > 99999)
    i = 99999;
  return(i);
}

/*
 * SetFieldTextFromInt
 * -------------------
 *
 * Put an integer in a field
 *
 * Parameters: Field ID, Integer, Indice
 *
 * Returns: Nothing
 */
void SetFieldTextFromInt(UInt16 fieldID, UInt16 numP, UInt16 indice)
{
  MemHandle txtH, oldTxtH;
  FormPtr frm = FrmGetActiveForm();
  FieldPtr fldP;
  Char *s;
  UInt16 partEnt, partDec;

  txtH = MemHandleNew(10);
  if (txtH)
  {
    s = MemHandleLock(txtH);
/*
    if (numP > 0)
    {
      if (indice == 1)
        StrPrintF(s, "%d", numP);
      else
      {
        partEnt = numP / indice;
        partDec = numP - (partEnt * indice);
        if (indice == 10)
          StrPrintF(s, "%d%c%d", partEnt, decSep, partDec);
        else
        {
          if (partDec < 10)
            StrPrintF(s, "%d%c0%d", partEnt, decSep, partDec);
          else
            StrPrintF(s, "%d%c%d", partEnt, decSep, partDec);
        } 
      }
    }
    else
    {
      if (indice == 1)
        StrPrintF(s, "%d", numP);
      else
        StrPrintF(s, "%d%c%d", numP, decSep, numP);
    }
*/
    if (indice == 1)
      StrPrintF(s, "%d", numP);
    else
    {
      if (numP > 0)
      {
        partEnt = numP / indice;
        partDec = numP - (partEnt * indice);
      }
      else
        partDec = partEnt = 0;
      if (indice == 10)
        StrPrintF(s, "%d%c%d", partEnt, decSep, partDec);
      else
        StrPrintF(s, "%d%c%02d", partEnt, decSep, partDec);
    }
    MemHandleUnlock(txtH);
    fldP = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, fieldID));
    oldTxtH = FldGetTextHandle(fldP);
    FldSetTextHandle(fldP, txtH);
    FldDrawField(fldP);
    if (oldTxtH)
      MemHandleFree(oldTxtH);
  }
}

/*
 * SetFieldTextFromStr
 * -------------------
 *  
 * Put a string in a field
 *  
 * Parameters: Field ID, String
 *
 * Returns: Nothing
 */
void SetFieldTextFromStr(UInt16 fieldID, Char *strP)
{
  MemHandle txtH, oldTxtH;
  FormPtr frm = FrmGetActiveForm();
  FieldPtr fldP;
        
  txtH = MemHandleNew(StrLen(strP) + 1);
  if (txtH)
  {
    StrCopy(MemHandleLock(txtH), strP);
    MemHandleUnlock(txtH);
    fldP = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, fieldID));
    oldTxtH = FldGetTextHandle(fldP);
    FldSetTextHandle(fldP, txtH);
    FldDrawField(fldP);
    if (oldTxtH)
      MemHandleFree(oldTxtH);
  }
}

/*
 * GetStrFromFieldTxt
 * ------------------
 *
 * Get a string from a field
 *
 * Parameters: Field ID, String, Char
 *
 * Returns: Char *
 */
void GetStrFromFieldTxt(FormPtr frm, UInt16 fieldID, Char *t)
{
  MemHandle txtH;
  FieldPtr fldP;
  Char *s;
         
  fldP = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, fieldID));
  txtH = MemHandleNew(FldGetTextLength(fldP) + 1);
  StrCopy(MemHandleLock(txtH), FldGetTextPtr(fldP));
  s = MemHandleLock(txtH);
  StrCopy(t, s);

  MemHandleUnlock(txtH);
  MemHandleFree(txtH);

  return;
}

/*  
 * OutilsSelectDate
 * ----------------
 *
 * Select a date
 *
 * Parameters: Date, Control, TitleID
 *
 * Returns: Nothing
 */
void OutilsSelectDate(DateType *date, ControlType *ctlP, UInt16 titleID)
{
  Boolean  apply;
  Int16  month, day, year;
  MemHandle  titleH;
  Char *labelP;

  month = date->month;
  day = date->day;
  year = date->year + firstYear;

  titleH = DmGetResource(strRsc, titleID);
  ErrFatalDisplayIf( !titleH, "resource not found" );
  apply = SelectDay(selectDayByDay, &month, &day, &year, MemHandleLock(titleH));
  MemHandleUnlock(titleH);
  DmReleaseResource(titleH);

  if(apply)
  {
    date->month = month;
    date->day = day;
    date->year = year - firstYear;

    labelP = (Char *)CtlGetLabel(ctlP);
    DateToDOWDMFormat(month, day, year, dateFormat, labelP);
    CtlSetLabel(ctlP, labelP);
  }

  return;
}

/*
 * OutilsSelectTime
 * ----------------
 *
 * Select a Time
 *
 * Parameters: Time, Control, TitleID
 *
 * Returns: Nothing
 */
void OutilsSelectTime(TimeType *time, ControlType *ctlP, UInt16 titleID)
{
  Boolean  apply;
  Int16  hour, minute;
  MemHandle  titleH;
  Char *labelP;

  hour = time->hours;
  minute = time->minutes;

  titleH = DmGetResource(strRsc, titleID);
  ErrFatalDisplayIf( !titleH, "resource not found" );
  apply = SelectOneTime(&hour, &minute, MemHandleLock(titleH));
  MemHandleUnlock(titleH);
  DmReleaseResource(titleH);

  if(apply)
  {
    time->hours = hour;
    time->minutes = minute;

    labelP = (Char *)CtlGetLabel(ctlP);
    TimeToAscii(hour, minute, timeFormat, labelP);
    CtlSetLabel(ctlP, labelP);
  }

  return;
}

/*
 * UpdateScrollbar
 * ---------------
 *
 * Set the Scroll bar value
 *
 * Parameters: Event
 *
 * Returns: void
 */
void UpdateScrollbar(UInt16 scrollID)
{
  FormPtr frm = FrmGetActiveForm();
  UInt16 scrollbar;
  ScrollBarPtr scroll;

  scrollbar = FrmGetObjectIndex(frm, scrollID);
  scroll = FrmGetObjectPtr(frm, scrollbar);

  if (NBRecs <= NBRows)
  {
    SclSetScrollBar(scroll, 0, 0, 0, (NBRows - 1));
  }
  else
  {
    SclSetScrollBar(scroll, TopRecord, 0, (NBRecs - NBRows), (NBRows - 1));
  }
}

/*
 * GetRomVersion
 * -------------
 *
 * Get the OS version
 *
 * Parameters: Nothing
 *
 * Returns: Error
 */
Err GetRomVersion(UInt16 launchFlags)
{
  Char *s;
  MemHandle h;

  FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);

  // See if we're on in minimum required version of the ROM or later
  if (romVersion < 0x03503000)
  {
    if ((launchFlags & (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) == (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp))
    {
      h = DmGetResource(strRsc, stringID_romversion);
      s = MemHandleLock(h);
      FrmCustomAlert(formID_error, s, NULL, NULL);
      MemHandleUnlock(h);
      DmReleaseResource(h);
      // Palm OS 1.0 will continuously relaunch this app unless we switch to another safe one
      if (romVersion < 0x02000000)
      {
        AppLaunchWithCommand(sysFileCDefaultApp, sysAppLaunchCmdNormalLaunch, NULL);
      }
    }
  return sysErrRomIncompatible;
  }

  IsOS4 = true;
  if (romVersion >= 0x05100000)
  {
    appPrefVersion = preferenceDataVer11;
    IsOS5 = true;
  }
  else
  {
    if (romVersion >= 0x05000000)
    {
      appPrefVersion = preferenceDataVer10;
      IsOS5 = true;
    }
    else 
    {
      IsOS5 = false;
      if (romVersion >= 0x04000000)
        appPrefVersion = preferenceDataVer9;
      else 
      {
        IsOS4 = false;
        appPrefVersion = preferenceDataVer8;
      }
    }
  }

  return errNone;
}
