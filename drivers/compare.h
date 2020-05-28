/*
 * compare.h: A method for comparing equal-sized buffers in constant time.
 *
 * Based on: https://github.com/jedisct1/libsodium
 *
 * LICENSE INFO:
 *
 * ISC License
 *
 * Copyright (c) 2013-2020
 * Frank Denis <j at pureftpd dot org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef COMPARE_H_INC
#define COMPARE_H_INC

/*
 * Compare buffers of given length to be equal, in constant time.
 * Returns zero iff buffers are equal. 
 *
 * NOTE: parameters and variables are annotated as volatile to
 * prevent compiler optimisation
 */
static inline __attribute__((always_inline)) int compare_ct_time(const unsigned char *a1_,
                volatile unsigned char *a2_, size_t len)
{
    const volatile unsigned char *volatile a1 =
            (const volatile unsigned char *volatile) a1_;
    const volatile unsigned char *volatile a2 =
            (const volatile unsigned char *volatile) a2_;

    volatile unsigned int result = 0x0;
    int i;

    for (i=0; i<len; i++)
    {
        result |= a1[i] ^ a2[i];
    }

    // Returns zero iff buffers are equal
    return ((1 & ((result-1)>>8)) - 1);
}

#endif
