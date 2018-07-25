/*
 *  iOSUtils.h
 *  Created by Seth Robinson on 3/6/09.
 *  For license info, check the license.txt file that should have come with this.
 *
 */

#pragma once

#if defined __cplusplus || defined __OBJC__

#include "../PlatformEnums.h"

void InitDeviceScreenInfoEx(int width, int height, int orientation);
int ConvertOSXKeycodeToProtonVirtualKey(int c);
#endif

