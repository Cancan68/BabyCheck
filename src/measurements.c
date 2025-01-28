static void MeasurLoadTable(void);

/*
 * MeasurExportMemo
 * ----------------
 *
 * Export the measurements in the Memo Database
 *
 * Parameters: weight, height
 *
 * Returns: BMI
 */
static UInt32 CalculateBMI(UInt32 weight, UInt32 height)
{
  UInt32 result;

  if (weight > 0 && height > 0)
  {
    if  (Prefs.Munit == 0  || Prefs.Munit ==  10)
      weight = weight * 10000;
    else
      weight = weight * 4536;
    if (Prefs.Munit > 9)
      height = (height) * 254 / 100;

    result = (((weight / height) * 100) /  height);
  }
  else
    result = 0;

  return(result);
}

/*
 * MeasurExportMemo
 * ----------------
 *
 * Export the measurements in the Memo Database
 *
 * Parameters: Nothing
 *
 * Returns: Nothing
 */
void MeasurExportMemo(void)
{
  DmOpenRef	MemoDB;
  UInt32	size = 0, linelen, recValue;
  MemHandle	h, recordH;
  void		*memorec;
  Char		outline[120];
  Char		sbuffer[10];
  Char		*s;
  DateType	date;
  Char		dateBuffer[dateStringLength];
  Char		*dateStr;
  MeasurDBType  *recordP;
  UInt16	index;
  UInt16        rec, recNum;
  UInt16	partEnt, partDec;

  MemoDB = DmOpenDatabaseByTypeCreator('DATA', sysFileCMemo, dmModeWrite);
  if (!MemoDB)
    return;

  index = DmNumRecords(MemoDB);
  recordH = DmNewRecord(MemoDB, &index, 4096);
  memorec = MemHandleLock(recordH);

  // Write a header line
  StrCopy(outline, memo1);
  StrCat(outline, WRecord.name);
  StrCat(outline, memo2);
  DateSecondsToDate(TimGetSeconds(), &date);
  DateToAscii(date.month, date.day, date.year + firstYear, dateFormat, dateBuffer);
  dateStr = dateBuffer;
  StrCat(outline, dateStr); 
  StrCat(outline, "\n");
  linelen = StrLen(outline);
  DmWrite(memorec, size, outline, linelen);
  size += linelen;

  StrCopy(outline, memo3);
  linelen = StrLen(outline);
  DmWrite(memorec, size, outline, linelen + 1);
  size += linelen;

  recNum = DmNumRecords(MeasurDBRef);

  for (rec = 0; rec < recNum; rec++)
  {
    h = DmQueryRecord(MeasurDBRef, rec);
    if (h)
    {
      recordP = (MeasurDBType *) MemHandleLock(h);
      if (recordP->number == uniqueID)
      {
          DateToAscii(recordP->date.month, recordP->date.day, recordP->date.year + firstYear, dateFormat, dateBuffer);
          dateStr = dateBuffer;
          StrCopy(outline, dateStr);

          StrCat(outline,";");

          s = sbuffer;
          if (recordP->weight > 0)
          {
            partEnt = recordP->weight / 100;
            partDec = recordP->weight - (partEnt * 100);
            StrPrintF(s, "%d%c%02d", partEnt, decSep, partDec);
          }
          else
              StrPrintF(s, "%d%c%d", 0, decSep, 0);

          StrCat(outline, s);
          StrCat(outline,";");

          s = sbuffer;
          if (recordP->height > 0)
          {
            partEnt = recordP->height / 10;
            partDec = recordP->height - (partEnt * 10);
            StrPrintF(s, "%d%c%d", partEnt, decSep, partDec);
          }
          else
              StrPrintF(s, "%d%c%d", 0, decSep, 0);

          StrCat(outline, s);
          StrCat(outline,";");

          s = sbuffer;
          recValue = CalculateBMI((UInt32) recordP->weight, (UInt32) recordP->height);
          if (recValue > 0)
          {
            partEnt = (UInt16) (recValue / 100);
            partDec = ((UInt16) (recValue) - (partEnt * 100));
            StrPrintF(s, "%d%c%02d", partEnt, decSep, partDec);
          }
          else
              StrPrintF(s, "%d%c%d", 0, decSep, 0);

          StrCat(outline, s);
          StrCat(outline,";\n");

          linelen = StrLen(outline);
          DmWrite(memorec, size, outline, linelen + 1);
          size += linelen; 
      }
      MemHandleUnlock(h);
    }
  }

  MemHandleUnlock(recordH);
  DmResizeRecord(MemoDB, index, size + 1);
  DmReleaseRecord(MemoDB, index, true);

  DmCloseDatabase(MemoDB);

  DisplayError(stringID_transmemo, formID_info);
}

/*
 * DetermineMonth
 * --------------
 *
 * Determine the number of month to be drawn
 *
 * Parameters: Nothing
 *
 * Returns: Nothing
 */
static void DetermineMonth(void)
{
  graphMonth = 0;

  if (dateLast.day == 31 || (dateLast.day > 27 && dateLast.month == 2))
    dateLast.day = 30;
  if (dateBirth.day == 31 || (dateBirth.day > 27 && dateBirth.month == 2))
    dateBirth.day = 30;
  
  if (dateLast.day > dateBirth.day)
    graphMonth++;
  
  if (dateLast.year > (dateBirth.year + 1))
    graphMonth += (dateLast.year - (dateBirth.year + 1)) * 12;
  
  if (dateLast.year > dateBirth.year)
    graphMonth += dateLast.month + 12 - dateBirth.month;
  else
    graphMonth += dateLast.month - dateBirth.month;
  
  if (graphMonth > 30)
    graphMonth = 36;
  else if (graphMonth > 24)
         graphMonth = 30;
       else if (graphMonth > 18)
              graphMonth = 24;
            else if (graphMonth > 12)
                   graphMonth = 18;
                 else if (graphMonth > 6)
                        graphMonth = 12;
                      else if (graphMonth > 3)
                             graphMonth = 6;
                           else
                             graphMonth = 3;

  if (dateBirth.month == 2 && dateBirth.day == 30)
    dateBirth.day = 28;
}

/*
 * MeasurCompare
 * ------------
 *
 * Compare two records
 *
 * Parameters: Table, Row, Column, Rectangle
 *
 * Returns: Nothing
 */
static Int16 MeasurCompare(MeasurDBType *r1, MeasurDBType *r2, Int16 sortOrder, SortRecordInfoPtr info1, SortRecordInfoPtr info2, MemHandle appInfoH)
{
  Int16 result;

  result = r1->date.year - r2->date.year;
  if (result != 0)
  {
    if (DateToInt(r1->date) == 0xffff)
      return 1;
    if (DateToInt(r2->date) == 0xffff)
      return -1;
    return result;
  }

  result = r1->date.month - r2->date.month;
  if (result != 0)
    return result;

  result = r1->date.day - r2->date.day;

  return result;
}

/*
 * MeasurDetailFormInit
 * --------------------
 * 
 * Initialize the measurement detail form
 *
 * Parameters: Nothing
 * 
 * Returns: Nothing
 */
static void MeasurDetailFormInit(void)
{ 
  ControlType *ctlP;
  Char *labelP;
  FormType *frmP = FrmGetActiveForm();
  UInt32  recValue;

  ctlP = FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, triggerID_birdate));
  labelP = (Char *)CtlGetLabel(ctlP);
  DateToDOWDMFormat(WMRecord.date.month, WMRecord.date.day, WMRecord.date.year + firstYear, dateFormat, labelP);
  CtlSetLabel(ctlP, labelP);

  SetFieldTextFromInt(fieldID_weightm, WMRecord.weight, 100);
  SetFieldTextFromInt(fieldID_heightm, WMRecord.height, 10);

  recValue = CalculateBMI((UInt32) WMRecord.weight, (UInt32) WMRecord.height);
  if (recValue > 0)
    SetFieldTextFromInt(fieldID_bmi, recValue, 100);

  if (Prefs.Munit == 0 || Prefs.Munit == 10)
    SetFieldTextFromStr(fieldID_weightu, unit1);
  else
    SetFieldTextFromStr(fieldID_weightu, unit3);

  if (Prefs.Munit < 9)
    SetFieldTextFromStr(fieldID_heightu, unit2);
  else
    SetFieldTextFromStr(fieldID_heightu, unit4);

  FrmSetFocus(frmP, FrmGetObjectIndex(frmP, fieldID_weightm));
}

/*
 * MeasurementsDetailFormHandleEvent
 * ---------------------------------
 *
 * Handle the measurement detail form
 *
 * Parameters: Event
 *
 * Returns: Boolean
 */
static Boolean MeasurementsDetailFormHandleEvent(EventPtr event)
{
  Boolean handled = false;
  FormType *frmP;
  UInt16 zWeight, zHeight;

  switch (event->eType)
  {

  case frmOpenEvent:
    frmP = FrmGetActiveForm();
    CollapseSetState(frmP, collapseStateUser);
    FrmDrawForm(frmP);
    MeasurDetailFormInit();
    handled = true;
    break;

  case keyDownEvent:
    {
      UInt32 recValue;
      UInt16  fldno;
      FieldType *fldP;
      frmP = FrmGetActiveForm();
      fldno = FrmGetFocus(frmP);

      switch (FrmGetObjectId(frmP, fldno))
      {
        case fieldID_weightm:
        fldP = Item(fieldID_weightm);
        handled = FldHandleEvent(fldP, event);
        break;
        case fieldID_heightm:
        fldP = Item(fieldID_heightm);
        handled = FldHandleEvent(fldP, event);
        break;
      }

      zWeight = GetIntFromFieldTxt(frmP, fieldID_weightm, 100);
      zHeight = GetIntFromFieldTxt(frmP, fieldID_heightm, 10);

      recValue = CalculateBMI((UInt32) zWeight, (UInt32) zHeight);
      if (recValue > 0)
        SetFieldTextFromInt(fieldID_bmi, recValue, 100);
    }
    break;

  case ctlSelectEvent:
    if (event->data.ctlEnter.controlID == triggerID_birdate)
    {
        OutilsSelectDate(&WMRecord.date, event->data.ctlSelect.pControl, stringID_mesdate);
        handled = true;
    }
    if (event->data.ctlEnter.controlID == buttonID_mesdok)
    {
      MeasurDBType      *p;
      MemHandle         dataH;
      EventType e;
  
      dataH = DmGetRecord(MeasurDBRef, RecordNum);
      p = (MeasurDBType *) MemHandleLock(dataH);
  
      frmP = FrmGetActiveForm();

      WMRecord.weight = GetIntFromFieldTxt(frmP, fieldID_weightm, 100);
      WMRecord.height = GetIntFromFieldTxt(frmP, fieldID_heightm, 10);

      DmWrite(p, 0, &WMRecord, sizeof(MeasurDBType));
      DmReleaseRecord(MeasurDBRef, RecordNum, true);
      MemHandleUnlock(dataH);
  
      FrmReturnToForm(0);
      MemSet( &e, sizeof( EventType ), 0 );
      e.eType = winDisplayChangedEvent;
      EvtAddUniqueEventToQueue( &e, 0, true );
      DmQuickSort(MeasurDBRef, (DmComparF *) &MeasurCompare, 0);
      MeasurLoadTable();
      handled = true;
    }
    if (event->data.ctlEnter.controlID == buttonID_mesddel)
    {
      EventType e;

      // Don't delete the first record, it's the baby birth info
      if (NBRecs != 1)
        DmRemoveRecord(MeasurDBRef, RecordNum);

      if (TopRecord != 0)
        TopRecord--;

      FrmReturnToForm(0);
      MemSet( &e, sizeof( EventType ), 0 );
      e.eType = winDisplayChangedEvent;
      EvtAddUniqueEventToQueue( &e, 0, true );
      MeasurLoadTable();
      handled = true;
    }
    if (event->data.ctlEnter.controlID == buttonID_mesdcan)
    {
      EventType e;

      if (newBaby)
      {
        DmRemoveRecord(MeasurDBRef, RecordNum);

        if (TopRecord != 0)
          TopRecord--;
      }

      FrmReturnToForm(0);
      MemSet( &e, sizeof( EventType ), 0 );
      e.eType = winDisplayChangedEvent;
      EvtAddUniqueEventToQueue( &e, 0, true );
      MeasurLoadTable();
      handled = true;
    }
    break;

  }
  return(handled);
}

/*
 * MeasurDrawDate
 * -------------
 *
 * Draw the date in a row
 *
 * Parameters: Table, Row, Column, Rectangle
 *
 * Returns: Nothing
 */
static void MeasurDrawDate(void *table, Int16 row, Int16 column, RectanglePtr bounds)
{
  DateType date;
  Char dateBuffer[dateStringLength];
  Char* dateStr;
  Int16 drawX, drawY;
  UInt16 dateStrLen;
  FontID curFont;
  FontID oldFont = stdFont;

  // Get the due date to the item being drawn. 
  *((Int16 *) (&date)) = TblGetItemInt(table, row, 0);

  curFont = FntSetFont(oldFont);
  FntSetFont(stdFont);
  DateToAscii(date.month, date.day, date.year + firstYear, dateFormat, dateBuffer);
  dateStr = dateBuffer;

  dateStrLen = StrLen(dateStr);
  drawX = bounds->topLeft.x + bounds->extent.x - FntCharsWidth(dateStr, dateStrLen);
  drawY = bounds->topLeft.y;
  WinDrawChars(dateStr, dateStrLen, drawX, drawY);
  FntSetFont(curFont);
}

/*
 * MeasurDrawMeasur
 * ----------------
 *
 * Draw the weight or height in a row
 *
 * Parameters: Table, Row, Column, Rectangle
 *
 * Returns: Nothing
 */
static void MeasurDrawMeasur(void *table, Int16 row, Int16 column, RectanglePtr bounds)
{
  UInt16   weight;
  Char sbuffer[10];
  Char *s;
  UInt16 partEnt, partDec, indice;
  UInt16 weightStrLen;
  FontID oldFont = stdFont;
  FontID curFont;
  Int16 drawX, drawY;
  
  weight = (UInt16) TblGetItemInt(table, row, column);
  if (column == 1)
    indice = 100;
  else
    indice = 10;
  s = sbuffer;

  if (weight > 0)
  {
    partEnt = weight / indice;
    partDec = weight - (partEnt * indice);
    if (indice == 10)
      StrPrintF(s, "%d%c%d", partEnt, decSep, partDec);
    else
      StrPrintF(s, "%d%c%02d", partEnt, decSep, partDec);
  }
  else
      StrPrintF(s, "%d%c%d", weight, decSep, weight);

  curFont = FntSetFont(oldFont);
  FntSetFont(stdFont);
  weightStrLen = StrLen(s);
  drawX = bounds->topLeft.x + bounds->extent.x - FntCharsWidth(s, weightStrLen);
  drawY = bounds->topLeft.y;
  WinDrawChars(s, weightStrLen, drawX, drawY);
  FntSetFont(curFont);
}

/*
 * MeasurLoadTable
 * --------------
 *
 * Load the Measurement Table
 *
 * Parameters: Nothing
 *
 * Returns: Nothing
 */
static void MeasurLoadTable(void)
{
  MemHandle	h;
  MeasurDBType	*recordP;
  TablePtr	table;
  Int16		row, rec;
  UInt16	recNum;
  FormPtr	form;

  form = FrmGetActiveForm();
  table = (TablePtr) FrmGetObjectPtr(form, FrmGetObjectIndex(form, tableID_measur));

  row = 0;
  NBRecs = 0;
  recNum = DmNumRecords(MeasurDBRef);

  for (rec = 0; rec < recNum; rec++) 
  {
    h = DmQueryRecord(MeasurDBRef, (UInt16) rec);
    if (h)
    {
      recordP = (MeasurDBType *) MemHandleLock(h);
      if (recordP->number == uniqueID)
      {
        if (NBRecs == 0)
        {
          dateBirth = recordP->date;
          Wmin = (UInt32) recordP->weight;
          Hmin = (UInt32) recordP->height;  
        }

        if (NBRecs >= TopRecord && row < NBRows)
        {
          TblSetRowUsable(table, row, true);

          TblSetItemInt(table, row, 0, (*(Int16 *) &recordP->date));
          TblSetItemInt(table, row, 1, (Int16) recordP->weight);
          TblSetItemInt(table, row, 2, (Int16) recordP->height);

          TblSetRowID(table, row, (UInt16) rec);

          row++;
        }
        dateLast = recordP->date;
        NBRecs++;
      }
      MemHandleUnlock(h);
    }
  }

  // Hide the rows without data
  while (row < TNBRows)
  {
    TblSetRowUsable(table, row, false);
    row++;
  }

  UpdateScrollbar(scrollID_measur);
  TblEraseTable(table);
  TblMarkTableInvalid(table);
  TblDrawTable(table);
}

/*
 * MeasurCreate
 * -----------
 *
 * Create a measurement
 *
 * Parameters: Nothing
 *
 * Returns: Nothing
 */
static void MeasurCreate(void)
{
  TablePtr	table;
  UInt16	recNum;
  FormPtr	form;
  MemHandle	h;
  MeasurDBType	*recordP;
  DateTimeType	dateTime;

  form = FrmGetActiveForm();
  table = (TablePtr) FrmGetObjectPtr(form, FrmGetObjectIndex(form, tableID_measur));

  TblReleaseFocus(table);

  recNum = 0;
  h = DmNewRecord(MeasurDBRef, &recNum, sizeof(MeasurDBType));
  recordP = (MeasurDBType *) MemHandleLock(h);

  TimSecondsToDateTime(TimGetSeconds(), &dateTime);
  WMRecord.date.year = dateTime.year - firstYear;
  WMRecord.date.month = dateTime.month;
  WMRecord.date.day = dateTime.day;
  WMRecord.number = uniqueID;
  WMRecord.weight = 0;
  WMRecord.height = 0;
  
  DmWrite(recordP, 0, &WMRecord, sizeof(MeasurDBType));
  MemHandleUnlock(h);
  DmReleaseRecord(MeasurDBRef, recNum, true);
  
  NBRecs++;

  if (NBRecs > NBRows)
    TopRecord = NBRecs - NBRows;

  RecordNum = recNum;
  newBaby = true;
  FrmPopupForm(formID_mesdet);
}

/*
 * MeasurFormInit
 * -------------
 *
 * Initialize the Measur Form
 *
 * Parameters: FormType
 *
 * Returns: Nothing
 */
static void MeasurFormInit(FormType *frmP)
{
  TablePtr  table;
  Int16     row, col;
  RectangleType	r;
  FontID curFont;
  FontID oldFont = stdFont;

  ReadBabyRecord();

  TopRecord = 0;

  if (Prefs.Munit == 0 || Prefs.Munit == 10)
    SetFieldTextFromStr(fieldID_weightu, unit1);
  else
    SetFieldTextFromStr(fieldID_weightu, unit3);

  if (Prefs.Munit < 9)
    SetFieldTextFromStr(fieldID_heightu, unit2);
  else
    SetFieldTextFromStr(fieldID_heightu, unit4);

  table = (TablePtr) FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, tableID_measur));
  TblGetBounds(table, &r);

  TNBRows = TblGetNumberOfRows(table);

  for (row = 0; row < TNBRows; row++)
  {
    TblSetItemStyle(table, row, 0, customTableItem);
    TblSetItemStyle(table, row, 1, customTableItem);
    TblSetItemStyle(table, row, 2, customTableItem);
  }
  
  curFont = FntSetFont(oldFont);
  FntSetFont(stdFont);
  NBRows = (Int16) (r.extent.y /FntLineHeight());

  for (col = 0; col < 3; col++)
    TblSetColumnUsable(table, col, true);

  TblSetColumnSpacing(table, 1, 3);
  TblSetColumnSpacing(table, 2, 3);

  TblSetCustomDrawProcedure(table, 0, MeasurDrawDate);
  TblSetCustomDrawProcedure(table, 1, MeasurDrawMeasur);
  TblSetCustomDrawProcedure(table, 2, MeasurDrawMeasur);

  MeasurLoadTable();

  return;
}

/*
 * MeasurFormHandleEvent
 * --------------------
 *  
 * Handle the first measurement information form
 *  
 * Parameters: Event
 *
 * Returns: Boolean
 */
static Boolean MeasurementsFormHandleEvent(EventPtr event)
{
  Boolean handled = false;
  FormType *frmP;

  switch (event->eType)
  {

  case frmOpenEvent:
  {
    Int16 offx, offy;
    TablePtr	table;
    RectangleType	r;
    FontID curFont;
    FontID oldFont = stdFont;

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
      CollapseMoveResizeFormObject(frmP, tableID_measur, 0, 0, 0, offy);
      CollapseMoveResizeFormObject(frmP, buttonID_mesback, 0, offy, 0, 0);
      CollapseMoveResizeFormObject(frmP, buttonID_mesnew, 0, offy, 0, 0);
      CollapseMoveResizeFormObject(frmP, buttonID_mesmem, 0, offy, 0, 0);
      CollapseMoveResizeFormObject(frmP, scrollID_measur, 0, 0, 0, offy);
    }
}
    table = (TablePtr) FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, tableID_measur));
    TblGetBounds(table, &r);

    curFont = FntSetFont(oldFont);
    FntSetFont(stdFont);
    NBRows = (Int16) (r.extent.y /FntLineHeight());

    FrmDrawForm(frmP);
    MeasurFormInit(frmP);
    handled = true;
    break;
  }

  case winDisplayChangedEvent:
  {
    Int16 offx, offy;
    FormType *frmP;
    TablePtr	table;
    RectangleType	r;
    FontID curFont;
    FontID oldFont = stdFont;

    frmP = FrmGetActiveForm();

  if (romVersion >= 0x04000000)
  {
    if (CollapseResizeForm(frmP, false, &offx, &offy))
    {
      if (offy > 0)
        offy += 2;
      if (offy < 0)
        offy -= 2;
      CollapseMoveResizeFormObject(frmP, tableID_measur, 0, 0, 0, offy);
      CollapseMoveResizeFormObject(frmP, buttonID_mesback, 0, offy, 0, 0);
      CollapseMoveResizeFormObject(frmP, buttonID_mesnew, 0, offy, 0, 0);
      CollapseMoveResizeFormObject(frmP, buttonID_mesmem, 0, offy, 0, 0);
      CollapseMoveResizeFormObject(frmP, scrollID_measur, 0, 0, 0, offy);
    }
}
    table = (TablePtr) FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, tableID_measur));
    TblGetBounds(table, &r);

    curFont = FntSetFont(oldFont);
    FntSetFont(stdFont);
    NBRows = (Int16) (r.extent.y /FntLineHeight());

    MeasurLoadTable();
    FrmDrawForm(frmP); 
    handled = true;
    break;
  }

/*
    If the system fast enough 
    case sclRepeatEvent:
    TopRecord = event->data.sclRepeat.newValue - 1;
    MeasurLoadTable();
    handled = true;
    break;
 */

  case sclExitEvent:
    TopRecord = event->data.sclExit.newValue;
    MeasurLoadTable();
    handled = true;
    break;

  case keyDownEvent:
    switch (event->data.keyDown.chr)
    {
      case pageUpChr:
        if (TopRecord < NBRows)
          TopRecord = 0;
        else
          TopRecord -= NBRows;
        MeasurLoadTable();
        handled = true;
        break;

      case pageDownChr:
        TopRecord += NBRows;
        if ((TopRecord + NBRows) > NBRecs)
          TopRecord = NBRecs - NBRows;
        MeasurLoadTable();
        handled = true;
        break;
    }
    break;

  case ctlSelectEvent:
    if (event->data.ctlEnter.controlID == buttonID_mesback)
    {
      EventType e;
      FrmReturnToForm(0);
      MemSet( &e, sizeof( EventType ), 0 );
      e.eType = winDisplayChangedEvent;
      EvtAddUniqueEventToQueue( &e, 0, true );
      handled = true;
    }
    if (event->data.ctlEnter.controlID == buttonID_mesnew)
    {
      MeasurCreate();
      handled = true;
    }
    if (event->data.ctlEnter.controlID == buttonID_mesmem)
    {
      EventType e;
      MeasurExportMemo();
      MemSet( &e, sizeof( EventType ), 0 );
      e.eType = winDisplayChangedEvent;
      EvtAddUniqueEventToQueue( &e, 0, true );
      handled = true;
    }
    if (event->data.ctlEnter.controlID == buttonID_mesw)
    {
      GraphType = 1;
      DetermineMonth();
      FrmPopupForm(formID_graph);
      uniqueID2 = 0;
      handled = true;
    }
    if (event->data.ctlEnter.controlID == buttonID_mesh)
    {
      GraphType = 2;
      DetermineMonth();
      FrmPopupForm(formID_graph);
      uniqueID2 = 0;
      handled = true;
    }
    if (event->data.ctlEnter.controlID == buttonID_mesbmi)
    {
      GraphType = 3;
      DetermineMonth();
      FrmPopupForm(formID_graph);
      uniqueID2 = 0;
      handled = true;
    }
    break;

  case tblSelectEvent:
    // handle successful tap on a cell
    // for a checkbox or popup, tblExitEvent will be called instead of 
    // tblSelectEvent if the user cancels the control
    break;
      
  case tblEnterEvent:
  {
    Int16  row = event->data.tblEnter.row;
        
    TablePtr		table = event->data.tblEnter.pTable;
    MeasurDBType	*p;
    MemHandle		dataH;

    RecordNum = (UInt16) TblGetRowID(table, row);
    dataH = DmQueryRecord(MeasurDBRef, RecordNum);
    p = (MeasurDBType *) MemHandleLock(dataH);

    WMRecord.number = p->number;
    WMRecord.date = p->date;
    WMRecord.weight = p->weight;
    WMRecord.height = p->height;

    MemHandleUnlock(dataH);

    newBaby = false;
    FrmPopupForm(formID_mesdet);
    handled = true;

    break;
  }
  }
  return(handled);
}