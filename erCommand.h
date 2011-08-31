/* ----------------------------------------------------------------------------
 *
 * erCommand.h
 *
 * Copyright (C) xxxx-2011
 *		Mike Markowski, AB3AP
 * Copyright (C) 2011-
 *		Pierfrancesco Caci, IK5PVX
 *
 * This file is part of k3mem
 *
 * k3mem is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * k3mem is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with k3mem.  If not, see <http://www.gnu.org/licenses/>.
 * ----------------------------------------------------------------------------
 */
#ifndef __ER_COMMAND_H__
#define __ER_COMMAND_H__

#include "erCommandInfo.h"

#define BASE_ADDR    0x0C00
#define FREQ_MEM_LEN 0x40

k3FreqMemInfo * newK3FreqMemInfo ( void );
int checksum ( char asciiHex[], int from, int to, char * checksumAscii );
void spaceHexString ( char * hexStr, int interval );
int calcFreq ( k3VfoFreq f, char xvtrFlags );

#endif /* __ER_COMMAND_H__ */
