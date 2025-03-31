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
#include "util.h"

static unsigned int
rotl(unsigned int x, int b)
{
    b = b % 32;
    return (x << b) | (x >> (32 - b));
}

static void
sha1(unsigned char *msg, int msg_len, unsigned char *hh)
{
    unsigned int h0;
    unsigned int h1;
    unsigned int h2;
    unsigned int h3;
    unsigned int h4;
    unsigned long long ml;
    unsigned int padded_len;
    unsigned int a, b, c, d, e, w[80];
    unsigned int f, k, temp;
    int i, j;

    union {
        int i;
        struct {
           char a;
           char b;
           char c;
           char d;
        } s;
    } u;
    u.i = 1;
#define BE (u.s.a == 0)

    if (BE) {
        h0 = 0x01234567;
        h1 = 0x89ABCDEF;
        h2 = 0xFEDCBA98;
        h3 = 0x76543210;
        h4 = 0xF0E1D2C3;
    } else {
        h0 = 0x67452301;
        h1 = 0xEFCDAB89;
        h2 = 0x98BADCFE;
        h3 = 0x10325476;
        h4 = 0xC3D2E1F0;
    }

    ml = msg_len * 8;

    msg[msg_len ++] = 0x80;
    padded_len = ((msg_len + 8 + 63) / 64) * 64;

    while (msg_len < (padded_len - 8)) {
        msg[msg_len ++] = 0;
    }

    msg[msg_len ++] = (ml >> 56) & 0xff;
    msg[msg_len ++] = (ml >> 48) & 0xff;
    msg[msg_len ++] = (ml >> 40) & 0xff;
    msg[msg_len ++] = (ml >> 32) & 0xff;
    msg[msg_len ++] = (ml >> 24) & 0xff;
    msg[msg_len ++] = (ml >> 16) & 0xff;
    msg[msg_len ++] = (ml >>  8) & 0xff;
    msg[msg_len ++] = (ml      ) & 0xff;

    j = 0;
    while (padded_len != 0) {
        padded_len -= 64;
        for (i = 0; i < 16; i ++) {
            w[i]  = ((unsigned int)msg[j ++]) << 24;
            w[i] |= ((unsigned int)msg[j ++]) << 16;
            w[i] |= ((unsigned int)msg[j ++]) << 8;
            w[i] |=  (unsigned int)msg[j ++];
        }

        for (; i < 80; i ++) {
            w[i] = rotl(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
        }

        a = h0;
        b = h1;
        c = h2;
        d = h3;
        e = h4;

        for (i = 0; i < 80; i ++) {
            if (i < 20) {
                f = (b & c) | ((~b) & d);
                k = (BE) ? 0x9979825A : 0x5A827999;
            } else if (i < 40) {
                f = b ^ c ^ d;
                k = (BE) ? 0xA1EBD96E : 0x6ED9EBA1;
            } else if (i < 60) {
                f = (b & c) | (b & d) | (c & d);
                k = (BE) ? 0xDCBC1B8F : 0x8F1BBCDC;
            } else {
                f = b ^ c ^ d;
                k = (BE) ? 0xD6C162CA : 0xCA62C1D6;
            }

            temp = rotl(a, 5) + f + e + k + w[i];
            e = d;
            d = c;
            c = rotl(b, 30);
            b = a;
            a = temp;
        }

        h0 = h0 + a;
        h1 = h1 + b;
        h2 = h2 + c;
        h3 = h3 + d;
        h4 = h4 + e;
    }

    hh[0]  = (h0 >> 24) & 0xff;
    hh[1]  = (h0 >> 16) & 0xff;
    hh[2]  = (h0 >> 8)  & 0xff;
    hh[3]  =  h0        & 0xff;
    hh[4]  = (h1 >> 24) & 0xff;
    hh[5]  = (h1 >> 16) & 0xff;
    hh[6]  = (h1 >> 8)  & 0xff;
    hh[7]  =  h1        & 0xff;
    hh[8]  = (h2 >> 24) & 0xff;
    hh[9]  = (h2 >> 16) & 0xff;
    hh[10] = (h2 >> 8)  & 0xff;
    hh[11] =  h2        & 0xff;
    hh[12] = (h3 >> 24) & 0xff;
    hh[13] = (h3 >> 16) & 0xff;
    hh[14] = (h3 >> 8)  & 0xff;
    hh[15] =  h3        & 0xff;
    hh[16] = (h4 >> 24) & 0xff;
    hh[17] = (h4 >> 16) & 0xff;
    hh[18] = (h4 >> 8)  & 0xff;
    hh[19] =  h4        & 0xff;
}

void
hmac_sha1(const unsigned char *company, int company_len,
          const unsigned char *userid, int userid_len,
          unsigned char *hh)
{
    int i;
    const unsigned char key[64] = {
    'H', 'o', 'n', 'g', 'h', 'u', 's', 'h', 'u', 'i', 'y', 'a', '~', 'l', 'a', 'n',
    'g', 'y', 'a', 'm', 'a', 'l', 'a', 'n', 'g', 'd', 'a', 'l', 'a', 'n', 'g', 'a',
    '~', 'H', 'o', 'n', 'g', 'h', 'u', '~', 'a', 'n', 'b', 'i', 'a', 'n', 's', 'i',
    'y', 'a', 'm', 'a', 's', 'i', 'j', 'i', 'a', 'x', 'i', 'a', 'n', 'g', 'a', '~'
    };

    unsigned char msg[256];
    int msg_len = 0;

    for (i = 0; i < 64; i++) {
        msg[i] = ((unsigned char)key[i]) ^ 0x36;
    }

    for (i = 0; i < company_len; i++) {
        msg[64 + i] = company[i];
    }

    for (i = 0; i < userid_len; i++) {
        msg[64 + company_len + i] = userid[i];
    }

    msg_len = 64 + company_len + userid_len;

    sha1(msg, msg_len, &hh[0]);

    for (i = 0; i < 64; i++) {
        msg[i] = ((unsigned char)key[i]) ^ 0x5c;
    }

    for (i = 0; i < 20; i++) {
        msg[64 + i] = hh[i];
    }

    msg_len = 64 + 20;

    sha1(msg, msg_len, &hh[0]);
}
