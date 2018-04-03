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

#ifndef CELL_LOCATION_H
#define CELL_LOCATION_H


#include "application.h"
#include "cellular_hal.h"

class CellLocation {
  struct MDM_CELL_LOCATE {
    int day;
    int month;
    int year;
    int hour;
    int minute;
    int second;
    char lat[14];
    char lng[14];
    int altitude; // only for GNSS positioning
    int uncertainty;
    int speed; // only for GNSS positioning
    int direction; // only for GNSS positioning
    int vertical_acc; // only for GNSS positioning
    int sensor_used; // 0: the last fix in the internal database, 2: CellLocate(R) location information
    int sv_used; // only for GNSS positioning
    int antenna_status; // only for GNSS positioning
    int jamming_status; // only for GNSS positioning
    int count;
    bool ok;
    int size;

    MDM_CELL_LOCATE() {
      memset(this, 0, sizeof(*this));
      size = sizeof(*this);
    }
  };

  static MDM_CELL_LOCATE _cellLocate;
  static bool displayed_once;
  static volatile uint32_t cellTimeout;
  static volatile uint32_t cellTimeStart;

  static void setTimeout(uint32_t timeout_ms);
  static bool isTimedOut();
  static void clearTimeout();
  static int locateCallback(int type, const char* buf, int len, MDM_CELL_LOCATE* data);
  static int cellLocate(MDM_CELL_LOCATE& loc, uint32_t timeout_ms);
  static bool isInProgress(MDM_CELL_LOCATE& loc);
  static bool getResponse(MDM_CELL_LOCATE& loc);
  static void display(MDM_CELL_LOCATE& loc);

public:
  static void startLocating();
  static void reprogramAssistNowServer();
};

#endif
