/*
 *  Copyright (C) 2015, 2016  Victor Li (vli02@hotmail.com)
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

int  lic_valid = 0;

unsigned char lic_key[KEY_LEN]           = { 0 };
char lic_company[COMPANY_LEN]   = { 0 };
char lic_userid[USERID_LEN]     = { 0 };
char lic_username[USERNAME_LEN] = { 0 };
int company_len;
int userid_len;

static int
read_lic_input(int k)
{
    int ret = 0;
    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    char *val, *p;
    int len, i;
    char h, hv[2];
    int j;

    while ((linelen = getline(&line, &linecap, stdin)) > 0) {
        val = line;

        /* remove leading space */
        RM_LEADING_SPACE(val);
        if (!(*val)) continue;

        /* remove trailing space */
        p = &line[linelen - 1];
        while (p > val && isspace(*p)) {
            *p-- = 0;
        }

        /* remove quotes if there are any */
        if ((*p) == '\"') {
            *p-- = 0;
        }
        if ((*val) == '\"') {
            val ++;
        }
        break;
    }

    if (linelen <= 0) {
        ret = -1;
        goto input_done;
    }

    len = p - val + 1;
    if (len <= 0 || !(*val)) {
        ret = -2;
        goto input_done;
    }

    switch (k) {
        case 1:
            i = 0;
            j = 0;
            while (len > 0 && i < KEY_LEN) {
                if ((*val) != '-' &&
                    !isspace(*val)) {
                    h = (*val >= '0' && *val <= '9') ? *val - '0' + 0x0 :
                       ((*val >= 'A' && *val <= 'F') ? *val - 'A' + 0xA :
                       ((*val >= 'a' && *val <= 'f') ? *val - 'a' + 0xA : 0x10));
                    if (h == 0x10) {
                        ret = -2;
                        goto input_done;
                    }
                    hv[j ++] = h;
                }
                if (j == 2) {
                    lic_key[i ++] = (hv[0] << 4) | (hv[1] & 0xF);
                    j = 0;
                }
                val ++;
                len --;
            }
            if (len > 0 || i < KEY_LEN || j != 0) {
                ret = -2;
            }
            break;
        case 2:
            if (len < COMPANY_LEN) {
                company_len = len;
                strcpy(lic_company, val);
            } else {
                ret = -2;
            }
            break;
        case 3:
            if (len < USERID_LEN) {
                userid_len = len;
                strcpy(lic_userid, val);
            } else {
                ret = -2;
            }
            break;
        case 4:
            if (len < USERNAME_LEN) {
                strcpy(lic_username, val);
            } else {
                ret = -2;
            }
            break;
    }

input_done:
    if (line) {
        free(line);
    }
    if (ret == -2) {
        fprintf(stderr, "Invalid input.\n");
    }
    return ret;
}

static void
verify_license()
{
    int i = 0;
    unsigned char hh[KEY_LEN] = { 0 };
    hmac_sha1((unsigned char *)&lic_company[0], company_len,
              (unsigned char *)&lic_userid[0], userid_len,
              &hh[0]);
    while (i < KEY_LEN) {
        if (hh[i] != lic_key[i]) {
            lic_valid = 0;
            return;
        }
        i ++;
    }
    lic_valid = 1;
}

void
check_license(int o_i)
{
    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    char *key, *val, *p;
    int val_len, i;
    int num = 0;

    char h, hv[2];
    int j;

    FILE *fp = fopen(LIC_FILE, "r");
    if(fp == NULL) {
        goto check_done;
    }

    while ((linelen = getline(&line, &linecap, fp)) > 0) {
        key = line;
        /* remove leading space */
        RM_LEADING_SPACE(key);
        /* skip empty line or comment */
        if (!(*key) || (*key) == '#') {
            continue;
        }
        /* locate delimiter */
        p = strchr(key, ':');
        if (!p) {
            LIC_CORRUPT;
        }
        val = p + 1;
        *p-- = 0;
        /* remove trailing space */
        while (p > key && isspace(*p)) {
            *p-- = 0;
        }
        if (!*key) {
            LIC_CORRUPT;
        }
        /* remove leading space */
        RM_LEADING_SPACE(val);
        if (*val != '\"') {
            LIC_CORRUPT;
        }
        val ++;
        /* remove trailing space */
        p = &line[linelen - 1];
        while (p > val && isspace(*p)) {
            *p-- = 0;
        }
        if (*p != '\"') {
            LIC_CORRUPT;
        }
        *p-- = 0;
        val_len = p - val + 1;

        if (strcmp(key, LIC_KEY) == 0) {
            if (lic_key[0] != 0) {
                LIC_CORRUPT;
            }
            i = 0;
            j = 0;
            while (val_len > 0 && i < KEY_LEN) {
                if ((*val) != '-' &&
                    !isspace(*val)) {
                    h = (*val >= '0' && *val <= '9') ? *val - '0' + 0x0 :
                       ((*val >= 'A' && *val <= 'F') ? *val - 'A' + 0xA :
                       ((*val >= 'a' && *val <= 'f') ? *val - 'a' + 0xA : 0x10));
                    if (h == 0x10) {
                        LIC_CORRUPT;
                    }
                    hv[j ++] = h;
                }
                if (j == 2) {
                    lic_key[i ++] = (hv[0] << 4) | (hv[1] & 0xF);
                    j = 0;
                }
                val ++;
                val_len --;
            }
            if (val_len > 0 || i < KEY_LEN || j != 0) {
                LIC_CORRUPT;
            }
            num ++;
        } else if (strcmp(key, LIC_COMPANY) == 0) {
            if (lic_company[0] != 0 ||
                val_len <= 0 ||
                val_len >= COMPANY_LEN) {
                LIC_CORRUPT;
            }
            company_len = val_len;
            strcpy(lic_company, val);
            num ++;
        } else if (strcmp(key, LIC_USERID) == 0) {
            if (lic_userid[0] != 0 ||
                val_len <= 0 ||
                val_len >= USERID_LEN) {
                LIC_CORRUPT;
            }
            userid_len = val_len;
            strcpy(lic_userid, val);
            num ++;
        } else if (strcmp(key, LIC_USERNAME) == 0) {
            if (lic_username[0] != 0 ||
                val_len <= 0 ||
                val_len >= USERNAME_LEN) {
                LIC_CORRUPT;
            }
            strcpy(lic_username, val);
            num ++;
        } else {
            /* skip it */
        }
    }

    if (num < 4) {
        LIC_CORRUPT;
    }

    verify_license();
    if (lic_valid == 0 && o_i == 0) {
        fprintf(stderr, "\nLicense is invalid!\n");
    }

check_done:
    if (line) {
        free(line);
    }
    if (fp) {
        fclose(fp);
        fp = NULL;
    }
    if (lic_valid == 0 && o_i == 0) {
        printf("This software and its associated documentation are the intellectual property of the author. They are\n"
               "free for evaluation purpose or noncommercial use, a license is required for commercial production use.\n"
               "Contact vli02@hotmail.com or vli02us@gmail.com for commercial license information.\n\n");
    }
}

void
print_license(int opt)
{
    /* show license detail */
    if (lic_valid == 0) {
        printf("No a valid commercial license is found!\n");
        exit(0);
    }
    printf("License information found for the copy of this software is:\n");
    printf("%s:      %02X%02X%02X%02X%02X-"
                    "%02X%02X%02X%02X%02X-"
                    "%02X%02X%02X%02X%02X-"
                    "%02X%02X%02X%02X%02X\n",
           LIC_KEY,
           lic_key[0],  lic_key[1],  lic_key[2],  lic_key[3],  lic_key[4],
           lic_key[5],  lic_key[6],  lic_key[7],  lic_key[8],  lic_key[9],
           lic_key[10], lic_key[11], lic_key[12], lic_key[13], lic_key[14],
           lic_key[15], lic_key[16], lic_key[17], lic_key[18], lic_key[19]);
    printf("%s:  %s\n", LIC_COMPANY, lic_company);
    printf("%s:   %s\n", LIC_USERID, lic_userid);
    printf("%s: %s\n", LIC_USERNAME, lic_username);

    if (opt == 0) {
        return;
    }

    printf("\nTerms of Use, License Agreement:\n");
    printf("\nThe license is non-exclusive and non-transferable, it gives the right to use this software under"
           "\nthe conditions given below. The license remains valid for an unlimited period of time.\n"
           "\nEach licensed copy of the software may either be used by a single person who uses the software"
           "\npersonally on one or more computers, or installed on a single worksatation used non-simultaneously"
           "\nby multiple people, but not both. The software may be installed and accessed through a network,"
           "\nprovided that you have purchased the rights to use a licensed copy for each worksatation that will"
           "\naccess the software through the network. You may make copies of the software for backup purpose.\n");
    printf("\nThe licensee agrees not to translate, disassemble, de-compile or in any other way create derivative"
           "\nworks of the software.\n");
    printf("\nThe licensee agrees not to rent, sell, lend, license or otherwise distribute the softare to any"
           "\nthird party.\n");
    printf("\nThe software is provided \"as is\", without warranty of any kind, express of implied, including but"
           "\nnot limite to the warranties of merchantability, fitness for a particular purpose and non-infringement."
           "\nIn no event shall the author is liable for any claim, damage or other liability, whether in an action"
           "\nof contract, tort or otherwise, arising from, out of or in connection with the software or the use"
           "\nof other dealings in the software. In no event shall the author's liability arising out of the use"
           "\nor inability to use the software exceed the amount that you have actually paid for the software.\n");
}

void
add_license()
{
    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    char *yesno, *p;

    FILE *fp = NULL;

    if (lic_valid != 0) {
        print_license(0);
        printf("\ndo you want to replace it? (y/n): ");
        linelen = getline(&line, &linecap, stdin);
        if (linelen > 0) {
            yesno = line;
            RM_LEADING_SPACE(yesno);
            p = &line[linelen - 1];
            while (p > yesno && isspace(*p)) {
                *p-- = 0;
            }
            if (!*yesno ||
                (strcmp(yesno, "y") &&
                 strcmp(yesno, "ye") &&
                 strcmp(yesno, "yes"))) {
                goto add_done;
            }
        }
    }

    printf("\nEnter license key, company name and user id you have received;\n"
             "Enter your name as user name (no more than 32 characters);\n"
             "Enter ctrl-d to abort.\n");
    printf("Key      :");
    if (read_lic_input(1) != 0) {
        goto add_done;
    }
    printf("Company  :");
    if (read_lic_input(2) != 0) {
        goto add_done;
    }
    printf("Userid   :");
    if (read_lic_input(3) != 0) {
        goto add_done;
    }
    printf("Username :");
    if (read_lic_input(4) != 0) {
        goto add_done;
    }

    /* validate the license */
    verify_license();
    if (lic_valid == 0) {
        fprintf(stderr, "This is an invalid license!\n");
        goto add_done;
    }

    /* write to file */
    fp = fopen(LIC_FILE, "w");
    if(fp == NULL) {
        fprintf(stderr, "Failed to write to file: %s!\n", LIC_FILE);
        goto add_done;
    }

    fprintf(fp, "%s:      \"%02X%02X%02X%02X%02X-"
                           "%02X%02X%02X%02X%02X-"
                           "%02X%02X%02X%02X%02X-"
                           "%02X%02X%02X%02X%02X\"\n",
           LIC_KEY,
           lic_key[0],  lic_key[1],  lic_key[2],  lic_key[3],  lic_key[4],
           lic_key[5],  lic_key[6],  lic_key[7],  lic_key[8],  lic_key[9],
           lic_key[10], lic_key[11], lic_key[12], lic_key[13], lic_key[14],
           lic_key[15], lic_key[16], lic_key[17], lic_key[18], lic_key[19]);
    fprintf(fp, "%s:  \"%s\"\n", LIC_COMPANY, lic_company);
    fprintf(fp, "%s:   \"%s\"\n", LIC_USERID, lic_userid);
    fprintf(fp, "%s: \"%s\"\n", LIC_USERNAME, lic_username);

    printf("\nLicense is successfully added!\n");

add_done:
    if (line) {
        free(line);
    }
    if (fp) {
        fclose(fp);
    }

    exit(0);
}
