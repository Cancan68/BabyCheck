/*
 *  Project:    TxtMemo
 *  Author:     Laurent Campredon <laurent@campredon.net>
 *
 *  File:       frmAbout.c
 */

#include <PalmOS.h>
#include "babycheck.rcp.h"   
#include "CollapseUtils.h"  
#include "sections.h"

extern UInt32 romVersion;

Boolean frmAbout_HandleEvent(EventPtr event) SECTION1;
