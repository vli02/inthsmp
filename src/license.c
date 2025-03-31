/*
 *  Copyright (C) 2015, 2016  Victor Li (vli02us@gmail.com)
 *
 *  This file is part of inthsmp/hsmp.
 *
 *  inthsmp/hsmp is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "license.h"
#include "util.h"

#define LIC_FILE "/etc/hsmp.conf"

#define LIC_KEY      "Key"
#define LIC_COMPANY  "Company"
#define LIC_USERID   "Userid"
#define LIC_USERNAME "Username"

#define LIC_CORRUPT \
do {\
    if (o_i == 0) {\
        fprintf(stderr, "\nLicense file seems corrupted!\n");\
    }\
    goto check_done;\
} while (0)

#define RM_LEADING_SPACE(P) \
while ((*(P)) && isspace(*(P))) {\
    (P) ++;\
}

/* no longer need a license ^_^ */
int  lic_valid = 1;

unsigned char lic_key[KEY_LEN]           = { 0 };
char lic_company[COMPANY_LEN]   = { 0 };
char lic_userid[USERID_LEN]     = { 0 };
char lic_username[USERNAME_LEN] = { 0 };
int company_len;
int userid_len;
