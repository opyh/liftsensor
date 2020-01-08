/******************************************************************************
  Copyright (c) 2015 Particle Industries, Inc.  All rights reserved.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this program; if not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/


#include "CellLocation.h"


bool CellLocation::displayed_once = false;
CellLocation::MDM_CELL_LOCATE CellLocation::_cellLocate;
volatile uint32_t CellLocation::cellTimeout;
volatile uint32_t CellLocation::cellTimeStart;


void CellLocation::setTimeout(uint32_t timeout_ms) {
  cellTimeout = timeout_ms;
  cellTimeStart = millis();
}

bool CellLocation::isTimedOut() {
  return (cellTimeout && ((millis()-cellTimeStart) > cellTimeout));
}

void CellLocation::clearTimeout() {
  cellTimeout = 0;
}

int CellLocation::locateCallback(int type, const char* buf, int len, MDM_CELL_LOCATE* data)
{
  if ((type == TYPE_PLUS) && data) {
    //+UULOC: <date>,<time>,<lat>,<long>,<alt>,<uncertainty>,<speed>,<direction>,
    //        <vertical_acc>,<sensor_used>,<SV_used>,<antenna_status>,<jamming_status>
    //+UULOC: 25/09/2013,10:13:29.000,45.7140971,13.7409172,266,17,0,0,18,1,6,3,9
    int count = 0;
    //
    // TODO: %f was not working for float on LAT/LONG, so opted for capturing strings for now
    const char *format = "\r\n+UULOC: %d/%d/%d,%d:%d:%d.%*d,%[^,],%[^,],%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n";

    if ( (count = sscanf(buf, format,
    &data->day,
    &data->month,
    &data->year,
    &data->hour,
    &data->minute,
    &data->second,
    data->lat,
    data->lng,
    &data->altitude,
    &data->uncertainty,
    &data->speed,
    &data->direction,
    &data->vertical_acc,
    &data->sensor_used,
    &data->sv_used,
    &data->antenna_status,
    &data->jamming_status) ) > 0 ) {
      // UULOC Matched
      data->count = count;
      data->ok = true;
    }
  }
  return WAIT;
}

int CellLocation::cellLocate(MDM_CELL_LOCATE& loc, uint32_t timeout_ms) {
  loc.count = 0;
  loc.ok = false;
  if (Cellular.command(5000, "AT+ULOCCELL=0\r\n") != RESP_OK) {
    Serial.println("Error! No Response from AT+ULOCCELL");
    return -1;
  }
  if (Cellular.command(locateCallback, &loc, timeout_ms, "AT+ULOC=2,2,1,%d,5000\r\n", timeout_ms/1000) != RESP_OK) {
    Serial.println("Error! No Response from AT+LOC");
    return -2;
  }
  setTimeout(timeout_ms);
  if (loc.count > 0) {
    return loc.count;
  }
  return 0;
}

bool CellLocation::isInProgress(MDM_CELL_LOCATE& loc) {
  if (!loc.ok && !isTimedOut()) {
    return true;
  }
  else {
    clearTimeout();
    return false;
  }
}

bool CellLocation::getResponse(MDM_CELL_LOCATE& loc) {
  // Send empty string to check for URCs that were slow
  Cellular.command(locateCallback, &loc, 1000, "");
  if (loc.count > 0) {
    return true;
  }
  return false;
}

void CellLocation::display(MDM_CELL_LOCATE& loc) {
  const char *format = "\r\n%2d/%2d/%2d,%2d:%2d:%2d,LAT:%s,LONG:%s,%d,UNCERTAINTY:%d,SPEED:%d,%d,%d,%d,%d,%d,%d,MATCHED_COUNT:%d";
  Serial.printlnf(format,
  loc.month,
  loc.day,
  loc.year,
  loc.hour,
  loc.minute,
  loc.second,
  loc.lat,
  loc.lng,
  loc.altitude,
  loc.uncertainty,
  loc.speed,
  loc.direction,
  loc.vertical_acc,
  loc.sensor_used,
  loc.sv_used,
  loc.antenna_status,
  loc.jamming_status,
  loc.count);

  /* A nice map URL */
  Serial.printlnf("\r\nhttps://www.google.com/maps?q=%s,%s\r\n", loc.lat, loc.lng);
}

void CellLocation::startLocating() {
  Serial.println("run Cell Locate...");
  uint32_t timeout = 60000;
  displayed_once = false;
  int ret = cellLocate(_cellLocate, timeout);

  if (ret >= 8) {
    /* Got the response immediately */
    display(_cellLocate);
    displayed_once = true;
    return;
  }

  if (ret != 0) {
    Serial.println("Cell Locate Error!");
    return;
  }
  /* ret == 0, still waiting for the URC
  * Check for cell locate response, and display it. */
  Serial.print("Waiting for URC ");

  while (isInProgress(_cellLocate)) {
    /* still waiting for URC */
    if (getResponse(_cellLocate)) {
      display(_cellLocate);
      displayed_once = true;
    }
    if (!displayed_once) Serial.print(".");
  }
  
  Serial.println();

  /* We timed out, but maybe we have a response that includes LAT/LONG coords */
  if (!displayed_once && _cellLocate.count >= 8) {
    display(_cellLocate);
    displayed_once = true;
  }
}

void CellLocation::reprogramAssistNowServer() {
  Serial.print("Reprogram the AssistNow server to u-blox factory default: ");
  if (RESP_OK == Cellular.command(5000, "AT+UGAOP=\"eval1-les.services.u-blox.com\",46434,1000,0\r\n")) {
    Serial.println("OK!");
  }
  else {
    Serial.println("Error!");
  }
}
