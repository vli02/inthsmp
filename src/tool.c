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
#include <string.h>
#include "license.h"
#include "util.h"

int
main(int argc, char **argv)
{
    char lic_company[COMPANY_LEN]   = { 0 };
    char lic_userid[USERID_LEN]     = { 0 };
    int company_len;
    int userid_len;

    unsigned char hh[KEY_LEN];

    if (argc != 3) {
        fprintf(stderr, "usage: %s <company name> <user id>\n", argv[0]);
        return -1;
    }

    company_len = strlen(argv[1]);
    userid_len  = strlen(argv[2]);
    if (company_len <= 0 || company_len >= COMPANY_LEN ||
        userid_len  <= 0 || userid_len  >= USERID_LEN) {
        fprintf(stderr, "Invalid parameters.\n");
        return -1;
    }

    strcpy(lic_company, argv[1]);
    strcpy(lic_userid,  argv[2]);

    hmac_sha1((unsigned char *)&lic_company[0], company_len,
              (unsigned char *)&lic_userid[0], userid_len,
              &hh[0]);

    printf("%s\t%02X%02X%02X%02X%02X-"
               "%02X%02X%02X%02X%02X-"
               "%02X%02X%02X%02X%02X-"
               "%02X%02X%02X%02X%02X\n",
           lic_userid,
           hh[0],  hh[1],  hh[2],  hh[3],  hh[4],
           hh[5],  hh[6],  hh[7],  hh[8],  hh[9],
           hh[10], hh[11], hh[12], hh[13], hh[14],
           hh[15], hh[16], hh[17], hh[18], hh[19]);
 
    return 0;
}
