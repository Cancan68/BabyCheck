static Int16		selectedrow;

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
void WordsExportMemo(void)
{
  DmOpenRef     MemoDB;
  UInt32        size = 0, linelen;
  MemHandle     h, recordH;
  void          *memorec;
  Char          outline[120];
  DateType      date;
  Char          dateBuffer[dateStringLength];
  Char          *dateStr;
  WordsDBType	*recordP;
  UInt16        index;
  UInt16        i, rec, recNum;

  MemoDB = DmOpenDatabaseByTypeCreator('DATA', sysFileCMemo, dmModeWrite);
  if (!MemoDB)
    return;

  index = DmNumRecords(MemoDB);
  recordH = DmNewRecord(MemoDB, &index, 4096);
  memorec = MemHandleLock(recordH);

  // Write a header line
  if (WordsCode == 'W')
    StrCopy(outline, memo4);
  else
    StrCopy(outline, memo5);
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

  if (WordsCode == 'W')
    StrCopy(outline, memo6);
  else
    StrCopy(outline, memo7);
  linelen = StrLen(outline);
  DmWrite(memorec, size, outline, linelen + 1);
  size += linelen;

  recNum = DmNumRecords(WordsDBRef);

  for (rec = 0; rec < recNum; rec++)
  {
    h = DmQueryRecord(WordsDBRef, rec);
    if (h)
    {
      recordP = (WordsDBType *) MemHandleLock(h);
      if (recordP->number == uniqueID && recordP->code == WordsCode)
      {
          DateToAscii(recordP->date.month, recordP->date.day, recordP->date.year
 + firstYear, dateFormat, dateBuffer);
          dateStr = dateBuffer;
          StrCopy(outline, dateStr);

          StrCat(outline,";\"");

          StrCat(outline, &recordP->word);

          for (i = 0; i < StrLen(outline); i++)
          {
            if (outline[i] == '\n')
              outline[i] = ' ';
            else
            {
              if (outline[i] == ';')
                outline[i] = ',';
            }
          }

          StrCat(outline,"\";\n");
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
 * WordsCompare
 * ------------
 *
 * Compare two records
 *
 * Parameters: Table, Row, Column, Rectangle
 *
 * Returns: Nothing
 */
Int16 WordsCompare(WordsDBType *r1, WordsDBType *r2, Int16 sortOrder, SortRecordInfoPtr info1, SortRecordInfoPtr info2, MemHandle appInfoH)
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
 * WordsDrawDate
 * -------------
 *
 * Draw the date in a row
 *
 * Parameters: Table, Row, Column, Rectangle
 *
 * Returns: Nothing
 */
void WordsDrawDate(void *table, Int16 row, Int16 column, RectanglePtr bounds)
{
  DateType date;
  Char dateBuffer[dateStringLength];
  Char* dateStr;
  Int16 drawX, drawY;
  UInt16 dateStrLen;
  FontID curFont;
  FontID oldFont = stdFont;

  // Get the due date to the item being drawn. 
  *((Int16 *) (&date)) = TblGetItemInt(table, row, 1);

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
 * WordsLoadTable
 * --------------
 *
 * Load the Words Table
 *
 * Parameters: Do we need to draw the table
 *
 * Returns: Nothing
 */
void WordsLoadTable(void)
{
  MemHandle	h;
  WordsDBType	*recordP;
  TablePtr	table;
  Int16		row, rec;
  UInt16	recNum, nbrec;
  UInt32	WuniqueID;
  FormPtr	form;
  UInt16	wordHeight, lineHeight, columnWidth, tableHeight, dataHeight;
  RectangleType	r;
  FontID curFont;
  FontID oldFont = stdFont;

  form = FrmGetActiveForm();
  table = (TablePtr) FrmGetObjectPtr(form, FrmGetObjectIndex(form, tableID_words));
  TblGetBounds(table, &r);
  TblEraseTable(table);

  curFont = FntSetFont(oldFont);
  FntSetFont(stdFont);
  tableHeight = r.extent.y;
  columnWidth = TblGetColumnWidth(table , 2);
  lineHeight = FntLineHeight();

  row = 0;
  NBRecs = 0;
  nbrec = 1;
  dataHeight = 0;
  recNum = DmNumRecords(WordsDBRef);
  rec = 0;

  while (rec < recNum)
  {
    h = DmQueryRecord(WordsDBRef, (UInt16) rec);
    if (h)
    {
      DmRecordInfo (WordsDBRef, (UInt16) rec, NULL, &WuniqueID, NULL);
      recordP = (WordsDBType *) MemHandleLock(h);
      if (recordP->number == uniqueID && recordP->code == WordsCode)
      {
        wordHeight = FldCalcFieldHeight(&recordP->word, columnWidth);
        if (wordHeight == 0)
          wordHeight = 1;

        if (NBRecs >= TopRecord)
        {
          dataHeight += (wordHeight * lineHeight);
          if (dataHeight < tableHeight)
          {
            TblSetRowUsable(table, row, true);
            TblSetRowHeight(table, row, (wordHeight * lineHeight));

            TblSetItemInt(table, row, 0, nbrec);
            TblSetItemInt(table, row, 1, (*(Int16 *) &recordP->date));

            TblSetRowData(table, row, WuniqueID);
            TblSetRowID(table, row, (UInt16) rec);

            TblMarkRowInvalid(table, row);
            row++;

            if (row == 1)
              TopRecord = NBRecs;
          }
        }
        NBRecs += wordHeight;
        nbrec++;
      }
      MemHandleUnlock(h);
    }
    rec++;
  }

  /* Hide the rows without data */
  while (row < TNBRows)
  {
    TblSetRowUsable(table, row, false);
    row++;
  }

  FntSetFont(curFont);

  UpdateScrollbar(scrollID_words);
  TblMarkTableInvalid(table);
  TblDrawTable(table);
}

/*
 * WordsDelete
 * -----------
 *
 * Delete a word
 *
 * Parameters: Nothing
 *
 * Returns: Nothing
 */
void WordsDelete(Boolean loadTable)
{
  TablePtr            table;
  UInt16              recordNum;
  FormPtr             form;

  form = FrmGetActiveForm();
  table = (TablePtr) FrmGetObjectPtr(form, FrmGetObjectIndex(form, tableID_words));

  if (selectedrow > -1)
  {
    TblReleaseFocus(table);

    recordNum = TblGetRowID(table, selectedrow);

    DmRemoveRecord(WordsDBRef, recordNum);

    if (TopRecord > 0)
      TopRecord--;

    if (loadTable)
      WordsLoadTable();

    selectedrow = -1;
  }
  else
  {
    DisplayError(stringID_row, formID_error);
  }
}

/*
 * WordsCreate
 * -----------
 *
 * Create a word
 *
 * Parameters: Nothing
 *
 * Returns: Nothing
 */
void WordsCreate(void)
{
  TablePtr	table;
  Int16		row;
  UInt16	recNum, attr;
  UInt32	WuniqueID;
  FormPtr	form;
  MemHandle	h;
  WordsDBType	*recordP;
  DateTimeType	dateTime;
  Char  null = 0;

  form = FrmGetActiveForm();
  table = (TablePtr) FrmGetObjectPtr(form, FrmGetObjectIndex(form, tableID_words));

  TblReleaseFocus(table);

  recNum = 0;
  h = DmNewRecord(WordsDBRef, &recNum, sizeof(WordsDBType) + 1);
  DmRecordInfo (WordsDBRef, recNum, NULL, &WuniqueID, NULL);
  recordP = MemHandleLock(h);

  TimSecondsToDateTime(TimGetSeconds(), &dateTime);
  WWRecord.date.year = dateTime.year - firstYear;
  WWRecord.date.month = dateTime.month;
  WWRecord.date.day = dateTime.day;
  WWRecord.number = uniqueID;
  WWRecord.code = WordsCode;
  
  DmWrite(recordP, 0, &WWRecord.number, sizeof(WWRecord.number));
  DmWrite(recordP, OffsetOf(WordsDBType, date), &WWRecord.date, sizeof(DateType));
  DmWrite(recordP, OffsetOf(WordsDBType, code), &WWRecord.code, sizeof(Char));
  DmWrite(recordP, OffsetOf(WordsDBType, word), &null, 1);

  MemHandleUnlock(h);
  DmRecordInfo(WordsDBRef, recNum, &attr, NULL, NULL);
  attr |= dmRecAttrDirty;
  DmSetRecordInfo(WordsDBRef, recNum, &attr, NULL);

  DmReleaseRecord(WordsDBRef, recNum, true);

  NBRecs++;

  if (NBRecs > NBRows)
    TopRecord = NBRecs - NBRows - 1;

  for (row = 0; row < TNBRows; row++)
    TblSetRowUsable (table, row, false);

  DmQuickSort(WordsDBRef, (DmComparF *) &WordsCompare, 0);

  WordsLoadTable();

  if (TblFindRowData(table, WuniqueID, &row))
  {
    FrmSetFocus(form, FrmGetObjectIndex(form, tableID_words));
    TblGrabFocus(table, row, 2);
    FldGrabFocus(TblGetCurrentField(table));
    selectedrow = row;
  }
}

/*
 * WordsLoadItem
 * -------------
 *
 * Load the first word in the table
 *
 * Parameters: Event
 *
 * Returns: Boolean
 */
Err WordsLoadItem(void *tbl, Int16 row, Int16 column,
  Boolean editable, MemHandle * dataH, Int16 *dataOffset,
  Int16 *dataSize, FieldPtr fld)
{ 
  TablePtr table = (TablePtr) tbl;
  MemHandle recordH;
  UInt16 recordNum;
  WordsDBType *p;
  FieldAttrType attr;
  
  recordNum = (UInt16) TblGetRowID(table, row);
  recordH = DmQueryRecord(WordsDBRef, recordNum);
 
  p = (WordsDBType *) MemHandleLock(recordH);

  *dataOffset = &p->word - ((Char *) p);
  *dataSize = StrLen(&p->word) + 1;  // one for null terminator
  *dataH = recordH;

  MemHandleUnlock(recordH);

  // Set the auto-shift to true
  if (fld)
  {
    FldGetAttributes(fld, &attr);
    attr.autoShift = true;
    FldSetAttributes(fld, &attr);
    FldSetMaxChars(fld, 80);
  }

  return(0);

}

/*
 * WordsSaveItem
 * -------------
 *
 * Save the first word in the table
 *
 * Parameters: Event
 *
 * Returns: Boolean
 */
Boolean WordsSaveItem(void *table, Int16 row, Int16 column)
{
  UInt16 attr;
  UInt16 recordNumber;

  recordNumber = (UInt16) TblGetRowID(table, row);

  DmRecordInfo(WordsDBRef, recordNumber, &attr, NULL, NULL);
  attr |= dmRecAttrDirty;
  DmSetRecordInfo(WordsDBRef, recordNumber, &attr, NULL);

  return false;  // the table should not be redrawn
}

/*
 * WordsFormInit
 * -------------
 *
 * Initialize the Words Form
 *
 * Parameters: FormType
 *
 * Returns: Nothing
 */
void WordsFormInit(FormType *frmP)
{
  TablePtr	table;
  Int16		row, col;
  RectangleType	r;
  FontID curFont;
  FontID oldFont = stdFont;

  ReadBabyRecord();

  if (WordsCode == 'W')
    FrmSetTitle(frmP, titleWo);
  else if (WordsCode == 'A')
         FrmSetTitle(frmP, titleAc);

  TopRecord = 0;

  table = (TablePtr) FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, tableID_words));
  TblGetBounds(table, &r);

  TNBRows = TblGetNumberOfRows(table);
  for (row = 0; row < TNBRows; row++)
  {
    TblSetItemStyle(table, row, 0, numericTableItem);
    TblSetItemStyle(table, row, 1, customTableItem);
    TblSetItemStyle(table, row, 2, textTableItem);
  }
  curFont = FntSetFont(oldFont);
  FntSetFont(stdFont);
  NBRows = (Int16) (r.extent.y /FntLineHeight());
  
  for (col = 0; col < 3; col++)
    TblSetColumnUsable(table, col, true);

  TblSetColumnSpacing(table, 0, 0);
  TblSetColumnSpacing(table, 1, 3);

  TblSetCustomDrawProcedure(table, 1, WordsDrawDate);

  TblSetLoadDataProcedure(table, 2, WordsLoadItem);
  TblSetSaveDataProcedure(table, 2, WordsSaveItem);
  
  WordsLoadTable();

  return;
}

/*
 * WordsTableResizeWord
 * --------------------
 *
 * Resize a word's row
 *
 * Parameters: Event
 *
 * Returns: void
 */ 
void WordsTableResizeWord(EventPtr event)
{
  TablePtr	table;

  table = (TablePtr) FrmGetObjectPtr(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), tableID_words));
  TblHandleEvent(table, event);

  WordsLoadTable();

  return;
}

/*
 * WordsFormHandleEvent
 * --------------------
 *  
 * Handle the first words information form
 *  
 * Parameters: Event
 *
 * Returns: Boolean
 */
Boolean WordsFormHandleEvent(EventPtr event)
{
  Boolean 	handled = false;
  FormType 	*frmP = FrmGetActiveForm();
  TablePtr	table = (TablePtr) FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, tableID_words));

  switch (event->eType)
  {

  case frmOpenEvent:
  {
    Int16 offx, offy;
    RectangleType	r;
    FontID curFont;
    FontID oldFont = stdFont;

    CollapseSetState(frmP, collapseStateUser);

    if (romVersion >= 0x04000000)
    {
      if (CollapseResizeForm(frmP, false, &offx, &offy))
      {
        if (offy > 0)
          offy += 2;
        if (offy < 0)
          offy -= 2;
        CollapseMoveResizeFormObject(frmP, tableID_words, 0, 0, offx, offy);
        CollapseMoveResizeFormObject(frmP, buttonID_worback, 0, offy, 0, 0);
        CollapseMoveResizeFormObject(frmP, buttonID_wornew, 0, offy, 0, 0);
        CollapseMoveResizeFormObject(frmP, buttonID_wordel, 0, offy, 0, 0);
        CollapseMoveResizeFormObject(frmP, buttonID_wormem, 0, offy, 0, 0);
        CollapseMoveResizeFormObject(frmP, scrollID_words, offx, 0, 0, offy);
      }
    }
    else
      offx = 0;

    TblGetBounds(table, &r);

    TblSetColumnWidth(table, 2, TblGetColumnWidth(table, 2) + offx);

    curFont = FntSetFont(oldFont);
    FntSetFont(stdFont);
    NBRows = (Int16) (r.extent.y /FntLineHeight());

    selectedrow = -1;

    FrmDrawForm(frmP);
    WordsFormInit(frmP);
    handled = true;
    break;
  }

  case winDisplayChangedEvent:
  {
    Int16 offx, offy;
    RectangleType	r;
    FontID curFont;
    FontID oldFont = stdFont;

    if (romVersion >= 0x04000000)
    {
      if (CollapseResizeForm(frmP, false, &offx, &offy))
      {
        if (offy > 0)
          offy += 2;
        if (offy < 0)
          offy -= 2;
        CollapseMoveResizeFormObject(frmP, tableID_words, 0, 0, offx, offy);
        CollapseMoveResizeFormObject(frmP, buttonID_worback, 0, offy, 0, 0);
        CollapseMoveResizeFormObject(frmP, buttonID_wornew, 0, offy, 0, 0);
        CollapseMoveResizeFormObject(frmP, buttonID_wordel, 0, offy, 0, 0);
        CollapseMoveResizeFormObject(frmP, buttonID_wormem, 0, offy, 0, 0);
        CollapseMoveResizeFormObject(frmP, scrollID_words, offx, 0, 0, offy);
      }
    }
    else
      offx = 0;

    TblGetBounds(table, &r);

    TblSetColumnWidth(table, 2, TblGetColumnWidth(table, 2) + offx);

    curFont = FntSetFont(oldFont);
    FntSetFont(stdFont);
    NBRows = (Int16) (r.extent.y /FntLineHeight());

    WordsLoadTable();
    FrmDrawForm(frmP);
    handled = true;
    break;
  }

  case sclExitEvent:
    TopRecord = event->data.sclExit.newValue;
    WordsLoadTable();
    selectedrow = -1;
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
        WordsLoadTable();
        handled = true;
        selectedrow = -1;
        break;

      case pageDownChr:
        TopRecord += NBRows;
        if ((TopRecord + NBRows) > NBRecs)
          TopRecord = NBRecs - NBRows - 1;
        WordsLoadTable();
        selectedrow = -1;
        handled = true;
        break;
    }
    break;

  case tblSelectEvent:
    /* handle successful tap on a cell
       for a checkbox or popup, tblExitEvent will be called instead of 
       tblSelectEvent if the user cancels the control */
 {
  /*  WordsDBType	*p;
    MemHandle	dataH;
    UInt16		recordNum;
    Int16         previousrow; */
    //Int16  		row = event->data.tblSelect.row;
/*
    Int16  		column = event->data.tblSelect.column;

    if (selectedrow > -1)
    {
      recordNum = (UInt16) TblGetRowID(table, selectedrow);
      dataH = DmGetRecord(WordsDBRef, recordNum);
      p = (WordsDBType *) MemHandleLock(dataH);

      if (StrLen(&p->word) < 1)
      {
        MemHandleUnlock(dataH); 
        DmReleaseRecord(WordsDBRef, recordNum, true);
        previousrow = selectedrow;
        WordsDelete(true);
        table = (TablePtr) FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, tableID_words));
        if (previousrow < row)
        {
          row = row - 1;
        }
        TblSetSelection(table, row, column);
        TblGrabFocus(table, row, column);
        FldGrabFocus(TblGetCurrentField(table));
        TblSetColumnEditIndicator(table, row, column);
      }
      else
      {
        MemHandleUnlock(dataH); 
        DmReleaseRecord(WordsDBRef, recordNum, true);
      }
    }
    selectedrow = row;*/
    break;
  }

  case tblExitEvent:
    WordsLoadTable();
    break;
      
  case tblEnterEvent:
  {
    WordsDBType	*p;
    MemHandle	dataH, titleH;
    UInt16		recordNum;
    UInt32		WuniqueID;
    Int16		month, day, year;
    DateType	date;
    Boolean		apply;
    Int16  		row = event->data.tblEnter.row;
    Int16  		column = event->data.tblEnter.column;
  //  Int16         previousrow;

 /*   if (selectedrow > -1)
    {
      WuniqueID = TblGetRowData(table, row);
      recordNum = (UInt16) TblGetRowID(table, selectedrow);
      dataH = DmGetRecord(WordsDBRef, recordNum);
      p = (WordsDBType *) MemHandleLock(dataH);

      if (StrLen(&p->word) < 1)
      {
        MemHandleUnlock(dataH); 
        DmReleaseRecord(WordsDBRef, recordNum, true);
        previousrow = selectedrow;
        WordsDelete(true);
        if (row > selectedrow)
        {
          row--;*/
   //     table = (TablePtr) FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, tableID_words));
   //     if (TblFindRowData(table, WuniqueID, &row))
   //     {
/*          FrmSetFocus(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), tableID_words));
          TblGrabFocus(table, row, 2);
          FldGrabFocus(TblGetCurrentField(table));
        }
      }
      else
      {
        MemHandleUnlock(dataH); 
        DmReleaseRecord(WordsDBRef, recordNum, true);
      }
   }*/
    selectedrow = row;

    if (column == 1)
    {
      recordNum = (UInt16) TblGetRowID(table, row);
      WuniqueID = TblGetRowData(table, row);
      dataH = DmGetRecord(WordsDBRef, recordNum);
      p = (WordsDBType *) MemHandleLock(dataH);

      month = p->date.month;
      day = p->date.day;
      year = p->date.year + firstYear;

      if (WordsCode == 'W')
        titleH = DmGetResource(strRsc, stringID_wordate);
      else if (WordsCode == 'A')
             titleH = DmGetResource(strRsc, stringID_actiondate);
           else
             titleH = DmGetResource(strRsc, stringID_shotsdate);

      apply = SelectDay(selectDayByDay, &month, &day, &year, MemHandleLock(titleH));
      MemHandleUnlock(titleH);
      DmReleaseResource(titleH);
      if(apply)
      {
        date.month = month;
        date.day = day;
        date.year = year - firstYear;

        DmWrite(p, OffsetOf(WordsDBType, date), &date, sizeof(DateType));
      } 
      DmReleaseRecord(WordsDBRef, recordNum, true);
      MemHandleUnlock(dataH);        

      DmQuickSort(WordsDBRef, (DmComparF *) &WordsCompare, 0);

      WordsLoadTable();

      if (TblFindRowData(table, WuniqueID, &row))
      {
        FrmSetFocus(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), tableID_words));
        TblGrabFocus(table, row, 2);
        FldGrabFocus(TblGetCurrentField(table));
      }
      handled = true;
    }

    break;
  }

  case ctlSelectEvent:
  {
    if ((event->data.ctlEnter.controlID == buttonID_worback) ||
        (event->data.ctlEnter.controlID == buttonID_wornew) ||
        (event->data.ctlEnter.controlID == buttonID_wordel) ||
        (event->data.ctlEnter.controlID == buttonID_wormem))
    {
      WordsDBType	*p;
      MemHandle	dataH;
      UInt16	recordNum;
    
      if (selectedrow > -1)
      {
        recordNum = (UInt16) TblGetRowID(table, selectedrow);
        dataH = DmGetRecord(WordsDBRef, recordNum);
        p = (WordsDBType *) MemHandleLock(dataH);

        if (StrLen(&p->word) < 1)
        {
          MemHandleUnlock(dataH); 
          DmReleaseRecord(WordsDBRef, recordNum, true);
          WordsDelete(true);
        }
        else
        {
          MemHandleUnlock(dataH); 
          DmReleaseRecord(WordsDBRef, recordNum, true);
        }
      }
      selectedrow = -1;
    }

    if (event->data.ctlEnter.controlID == buttonID_worback)
    {
      EventType e;
      
      FrmReturnToForm(0);
      MemSet( &e, sizeof( EventType ), 0 );
      e.eType = winDisplayChangedEvent;
      EvtAddUniqueEventToQueue( &e, 0, true );
      handled = true;
    }
    if (event->data.ctlEnter.controlID == buttonID_wornew)
    {
      WordsCreate();
      handled = true;
    }
    if (event->data.ctlEnter.controlID == buttonID_wordel)
    {
      WordsDelete(true);
      handled = true;
    }
    if (event->data.ctlEnter.controlID == buttonID_wormem)
    {
      EventType e;
      WordsExportMemo();
      MemSet( &e, sizeof( EventType ), 0 );
      e.eType = winDisplayChangedEvent;
      EvtAddUniqueEventToQueue( &e, 0, true );
      handled = true;
    }
    break;
  }


  case fldHeightChangedEvent:
    WordsTableResizeWord(event);
    handled = true;
    break;

  }
  return(handled);
}
