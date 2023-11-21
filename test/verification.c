/*  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  librds – Radio Data System parser library
 *  Copyright (C) 2023  Konrad Kosmatka
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 */

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "librds.h"
#include "parser.h"

typedef struct {
    librds_t rds;
    uint16_t pi;
    uint8_t pty;
    bool tp;
    bool ta;
    bool ms;
    uint8_t af;
    uint8_t af2;
    uint8_t ecc;
    char ps[9];
    char rt[2][65];
} test_context_t;

static int
group_setup(void **state)
{
    test_context_t *ctx = calloc(sizeof(test_context_t), 1);
    *state = ctx;
    return 0;
}

static int
group_teardown(void **state)
{
    test_context_t *ctx = *state;
    free(ctx);
    return 0;
}

static int
test_setup(void **state)
{
    test_context_t *ctx = *state;
    librds_init(&ctx->rds);
    librds_set_user_data(&ctx->rds, ctx);
    return 0;
}

static int
test_teardown(void **state)
{
    test_context_t *ctx = *state;
    (void)ctx;
    return 0;
}

static void
callback_pi(uint16_t  pi,
            void     *user_data)
{
    test_context_t *ctx = (test_context_t*)user_data;
    assert_int_equal(pi, ctx->pi);
    function_called();
}

static void
callback_pty(uint8_t  pty,
             void    *user_data)
{
    test_context_t *ctx = (test_context_t*)user_data;
    assert_int_equal(pty, ctx->pty);
    function_called();
}

static void
callback_tp(bool  tp,
            void *user_data)
{
    test_context_t *ctx = (test_context_t*)user_data;
    assert_int_equal(tp, ctx->tp);
    function_called();
}

static void
callback_ta(bool  ta,
            void *user_data)
{
    test_context_t *ctx = (test_context_t*)user_data;
    assert_int_equal(ta, ctx->ta);
    function_called();
}

static void
callback_ms(bool  ms,
            void *user_data)
{
    test_context_t *ctx = (test_context_t*)user_data;
    assert_int_equal(ms, ctx->ms);
    function_called();
}

static void
callback_af(uint8_t  af,
            void    *user_data)
{
    test_context_t *ctx = (test_context_t*)user_data;
    assert_int_equal(af, ctx->af);
    ctx->af = ctx->af2;
    function_called();
}

static void
callback_ecc(uint8_t  ecc,
             void    *user_data)
{
    test_context_t *ctx = (test_context_t*)user_data;
    assert_int_equal(ecc, ctx->ecc);
    function_called();
}

static void
callback_ps(const char                  *ps,
            const librds_string_error_t *errors,
            void                        *user_data)
{
    test_context_t *ctx = (test_context_t*)user_data;
    assert_string_equal(ps, ctx->ps);
    function_called();
}

static void
callback_rt(const char                  *rt,
            const librds_string_error_t *errors,
            librds_rt_flag_t             flag,
            void                        *user_data)
{
    test_context_t *ctx = (test_context_t*)user_data;
    assert_string_equal(rt, ctx->rt[flag]);
    function_called();
}

/* ------------------------------------------------ */

static void
verification_pi(void **state)
{
    test_context_t *ctx = *state;
    librds_register_pi(&ctx->rds, callback_pi);
    assert_int_equal(librds_get_pi(&ctx->rds), -1);

    expect_function_call(callback_pi);
    ctx->pi = 0x1234;
    assert_int_equal(librds_parse_string(&ctx->rds, "1234567890123458"), true);
    assert_int_equal(librds_get_pi(&ctx->rds), ctx->pi);

    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "1234567890123458"), true);

    librds_clear(&ctx->rds);
    assert_int_equal(librds_get_pi(&ctx->rds), -1);
}

static void
verification_pi_invalid(void **state)
{
    test_context_t *ctx = *state;
    librds_register_pi(&ctx->rds, callback_pi);

    assert_int_equal(librds_parse_string(&ctx->rds, "123456789012345840"), true);
    assert_int_equal(librds_get_pi(&ctx->rds), -1);
}

static void
verification_tp_true(void **state)
{
    test_context_t *ctx = *state;
    librds_register_tp(&ctx->rds, callback_tp);
    assert_int_equal(librds_get_tp(&ctx->rds), -1);

    expect_function_call(callback_tp);
    ctx->tp = 1;
    assert_int_equal(librds_parse_string(&ctx->rds, "1234567890123458"), true);
    assert_int_equal(librds_get_tp(&ctx->rds), ctx->tp);

    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "1234567890123458"), true);

    librds_clear(&ctx->rds);
    assert_int_equal(librds_get_tp(&ctx->rds), -1);
}

static void
verification_tp_false(void **state)
{
    test_context_t *ctx = *state;
    librds_register_tp(&ctx->rds, callback_tp);
    assert_int_equal(librds_get_tp(&ctx->rds), -1);

    expect_function_call(callback_tp);
    ctx->tp = 0;
    assert_int_equal(librds_parse_string(&ctx->rds, "1234000000000000"), true);
    assert_int_equal(librds_get_tp(&ctx->rds), ctx->tp);

    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "1234000000000000"), true);

    librds_clear(&ctx->rds);
    assert_int_equal(librds_get_tp(&ctx->rds), -1);
}

static void
verification_tp_invalid(void **state)
{
    test_context_t *ctx = *state;
    librds_register_tp(&ctx->rds, callback_tp);

    assert_int_equal(librds_parse_string(&ctx->rds, "123400000000000010"), true);
    assert_int_equal(librds_get_tp(&ctx->rds), -1);
}

static void
verification_ta_true(void **state)
{
    test_context_t *ctx = *state;
    librds_register_ta(&ctx->rds, callback_ta);
    assert_int_equal(librds_get_ta(&ctx->rds), -1);

    expect_function_call(callback_ta);
    ctx->ta = 1;
    assert_int_equal(librds_parse_string(&ctx->rds, "12340FFFFFFFFFFF"), true);
    assert_int_equal(librds_get_ta(&ctx->rds), ctx->ta);

    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "12340FFFFFFFFFFF"), true);

    librds_clear(&ctx->rds);
    assert_int_equal(librds_get_ta(&ctx->rds), -1);
}

static void
verification_ta_false(void **state)
{
    test_context_t *ctx = *state;
    librds_register_ta(&ctx->rds, callback_ta);
    assert_int_equal(librds_get_ta(&ctx->rds), -1);

    expect_function_call(callback_ta);
    ctx->ta = 0;
    assert_int_equal(librds_parse_string(&ctx->rds, "1234000090123458"), true);
    assert_int_equal(librds_get_ta(&ctx->rds), ctx->ta);

    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "1234000090123458"), true);

    librds_clear(&ctx->rds);
    assert_int_equal(librds_get_ta(&ctx->rds), -1);
}

static void
verification_ta_invalid(void **state)
{
    test_context_t *ctx = *state;
    librds_register_ta(&ctx->rds, callback_ta);

    assert_int_equal(librds_parse_string(&ctx->rds, "123400000000000010"), true);
    assert_int_equal(librds_get_ta(&ctx->rds), -1);
}

static void
verification_ms_true(void **state)
{
    test_context_t *ctx = *state;
    librds_register_ta(&ctx->rds, callback_ms);
    assert_int_equal(librds_get_ms(&ctx->rds), -1);
    
    expect_function_call(callback_ms);
    ctx->ms = 1;
    assert_int_equal(librds_parse_string(&ctx->rds, "12340FFFFFFFFFFF"), true);
    assert_int_equal(librds_get_ms(&ctx->rds), ctx->ms);

    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "12340FFFFFFFFFFF"), true);

    librds_clear(&ctx->rds);
    assert_int_equal(librds_get_ms(&ctx->rds), -1);
}

static void
verification_ms_false(void **state)
{
    test_context_t *ctx = *state;
    librds_register_ta(&ctx->rds, callback_ms);
    assert_int_equal(librds_get_ms(&ctx->rds), -1);
    
    expect_function_call(callback_ms);
    ctx->ms = 0;
    assert_int_equal(librds_parse_string(&ctx->rds, "1234000001230458"), true);
    assert_int_equal(librds_get_ms(&ctx->rds), ctx->ms);

    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "1234000001230458"), true);

    librds_clear(&ctx->rds);
    assert_int_equal(librds_get_ms(&ctx->rds), -1);
}

static void
verification_ms_invalid(void **state)
{
    test_context_t *ctx = *state;
    librds_register_ms(&ctx->rds, callback_ms);

    assert_int_equal(librds_parse_string(&ctx->rds, "123400000000000010"), true);
    assert_int_equal(librds_get_ms(&ctx->rds), -1);
}

static void
verification_pty(void **state)
{
    test_context_t *ctx = *state;
    librds_register_pty(&ctx->rds, callback_pty);
    assert_int_equal(librds_get_pty(&ctx->rds), -1);

    expect_function_call(callback_pty);
    ctx->pty = 19;
    assert_int_equal(librds_parse_string(&ctx->rds, "1234567890123458"), true);
    assert_int_equal(librds_get_pty(&ctx->rds), ctx->pty);

    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "1234567890123458"), true);

    librds_clear(&ctx->rds);
    assert_int_equal(librds_get_pty(&ctx->rds), -1);
}

static void
verification_pty_invalid(void **state)
{
    test_context_t *ctx = *state;
    librds_register_pty(&ctx->rds, callback_pty);

    assert_int_equal(librds_parse_string(&ctx->rds, "123400000000000010"), true);
    assert_int_equal(librds_get_pty(&ctx->rds), -1);
}

static void
verification_af(void **state)
{
    test_context_t *ctx = *state;
    librds_register_af(&ctx->rds, callback_af);
    uint8_t af = 0x90;
    uint8_t af2 = 0x01;

    const librds_af_t *af_buffer = librds_get_af(&ctx->rds);
    for (uint8_t i = 0; i < LIBRDS_AF_BUFFER_SIZE; i++)
    {
        assert_int_equal((*af_buffer)[i], 0);
    }

    expect_function_calls(callback_af, 2);
    ctx->af = af;
    ctx->af2 = af2;
    assert_int_equal(librds_parse_string(&ctx->rds, "1234007890013458"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "1234007890013458"), true);

    for (uint8_t i = 0; i < LIBRDS_AF_BUFFER_SIZE; i++)
    {
        if (i == (af / 8) ||
            i == (af2 / 8))
        {
            continue;
        }

        assert_int_equal((*af_buffer)[i], 0);
    }

    assert_int_equal((*af_buffer)[af / 8], 0x80 >> (af % 8));
    assert_int_equal((*af_buffer)[af2 / 8], 0x80 >> (af2 % 8));

    librds_clear(&ctx->rds);

    for (uint8_t i = 0; i < LIBRDS_AF_BUFFER_SIZE; i++)
    {
        assert_int_equal((*af_buffer)[i], 0);
    }
}

static void
verification_af_invalid(void **state)
{
    test_context_t *ctx = *state;
    librds_register_af(&ctx->rds, callback_af);

    assert_int_equal(librds_parse_string(&ctx->rds, "123400789001345810"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "123400789001345804"), true);

    const librds_af_t *af_buffer = librds_get_af(&ctx->rds);
    for (uint8_t i = 0; i < LIBRDS_AF_BUFFER_SIZE; i++)
    {
        assert_int_equal((*af_buffer)[i], 0);
    }
}

static void
verification_ecc(void **state)
{
    test_context_t *ctx = *state;
    librds_register_ecc(&ctx->rds, callback_ecc);
    assert_int_equal(librds_get_ecc(&ctx->rds), -1);
    
    expect_function_call(callback_ecc);
    ctx->ecc = 0xE2;
    assert_int_equal(librds_parse_string(&ctx->rds, "3566100000E20000"), true);
    assert_int_equal(librds_get_ecc(&ctx->rds), ctx->ecc);

    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "3566100000E20000"), true);

    librds_clear(&ctx->rds);
    assert_int_equal(librds_get_ecc(&ctx->rds), -1);
}

static void
verification_ecc_invalid(void **state)
{
    test_context_t *ctx = *state;
    librds_register_ecc(&ctx->rds, callback_ecc);

    assert_int_equal(librds_parse_string(&ctx->rds, "3566100000E2000010"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "3566100000E2000004"), true);
    assert_int_equal(librds_get_ecc(&ctx->rds), -1);
}

static void
verification_ps(void **state)
{
    test_context_t *ctx = *state;
    librds_register_ps(&ctx->rds, callback_ps);
    assert_string_equal(librds_get_ps(&ctx->rds), "        ");

    expect_function_call(callback_ps);
    snprintf(ctx->ps, sizeof(ctx->ps), ":;      ");
    assert_int_equal(librds_parse_string(&ctx->rds, "1234054C01203A3B"), true);
    assert_string_equal(librds_get_ps(&ctx->rds), ctx->ps);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "1234054C01203A3B"), true);

    expect_function_call(callback_ps);
    snprintf(ctx->ps, sizeof(ctx->ps), ":;<=    ");
    assert_int_equal(librds_parse_string(&ctx->rds, "1234054901203C3D"), true);
    assert_string_equal(librds_get_ps(&ctx->rds), ctx->ps);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "1234054901203C3D"), true);

    expect_function_call(callback_ps);
    snprintf(ctx->ps, sizeof(ctx->ps), ":;<=>?  ");
    assert_int_equal(librds_parse_string(&ctx->rds, "1234054A01203E3F"), true);
    assert_string_equal(librds_get_ps(&ctx->rds), ctx->ps);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "1234054A01203E3F"), true);

    expect_function_call(callback_ps);
    snprintf(ctx->ps, sizeof(ctx->ps), ":;<=>?JK");
    assert_int_equal(librds_parse_string(&ctx->rds, "1234054F01204A4B"), true);
    assert_string_equal(librds_get_ps(&ctx->rds), ctx->ps);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "1234054F01204A4B"), true);

    librds_clear(&ctx->rds);
    assert_string_equal(librds_get_ps(&ctx->rds), "        ");
}

static void
verification_ps_invalid(void **state)
{
    test_context_t *ctx = *state;
    librds_register_ps(&ctx->rds, callback_ps);

    librds_set_correction(&ctx->rds, LIBRDS_STRING_PS, LIBRDS_BLOCK_TYPE_INFO, LIBRDS_BLOCK_ERROR_LARGE);
    librds_set_correction(&ctx->rds, LIBRDS_STRING_PS, LIBRDS_BLOCK_TYPE_DATA, LIBRDS_BLOCK_ERROR_LARGE);

    assert_int_equal(librds_parse_string(&ctx->rds, "34DD054822756645FF"), true); /* "fE      " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD054921824449FF"), true); /* "  x9    " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD054AE3054F20FF"), true); /* "    O   " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD09833D9D4449FF"), true); /* "      DI" */

    assert_string_equal(librds_get_ps(&ctx->rds), "        ");
    assert_int_equal(librds_get_ps_available(&ctx->rds), false);
}

static void
verification_ps_invalid_pos(void **state)
{
    test_context_t *ctx = *state;
    librds_register_ps(&ctx->rds, callback_ps);

    librds_set_correction(&ctx->rds, LIBRDS_STRING_PS, LIBRDS_BLOCK_TYPE_INFO, LIBRDS_BLOCK_ERROR_LARGE);
    librds_set_correction(&ctx->rds, LIBRDS_STRING_PS, LIBRDS_BLOCK_TYPE_DATA, LIBRDS_BLOCK_ERROR_LARGE);

    assert_int_equal(librds_parse_string(&ctx->rds, "34DD05482275664530"), true); /* "fE      " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD05492182444930"), true); /* "  x9    " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD054AE3054F2030"), true); /* "    O   " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD09833D9D444930"), true); /* "      DI" */

    assert_string_equal(librds_get_ps(&ctx->rds), "        ");
    assert_int_equal(librds_get_ps_available(&ctx->rds), false);
}

static void
verification_ps_invalid_data(void **state)
{
    test_context_t *ctx = *state;
    librds_register_ps(&ctx->rds, callback_ps);

    librds_set_correction(&ctx->rds, LIBRDS_STRING_PS, LIBRDS_BLOCK_TYPE_INFO, LIBRDS_BLOCK_ERROR_LARGE);
    librds_set_correction(&ctx->rds, LIBRDS_STRING_PS, LIBRDS_BLOCK_TYPE_DATA, LIBRDS_BLOCK_ERROR_LARGE);

    assert_int_equal(librds_parse_string(&ctx->rds, "34DD05482275664503"), true); /* "fE      " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD05492182444903"), true); /* "  x9    " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD054AE3054F2003"), true); /* "    O   " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD09833D9D444903"), true); /* "      DI" */

    assert_string_equal(librds_get_ps(&ctx->rds), "        ");
    assert_int_equal(librds_get_ps_available(&ctx->rds), false);
}

static void
verification_ps_with_small_errors(void **state)
{
    test_context_t *ctx = *state;

    librds_set_correction(&ctx->rds, LIBRDS_STRING_PS, LIBRDS_BLOCK_TYPE_INFO, LIBRDS_BLOCK_ERROR_SMALL);
    librds_set_correction(&ctx->rds, LIBRDS_STRING_PS, LIBRDS_BLOCK_TYPE_DATA, LIBRDS_BLOCK_ERROR_SMALL);
    
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD04C0E305006473"), true); /* " d      " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD0548E305524100"), true); /* "RA      " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD0548E3054F350E"), true); /* "O5      " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD05492182444901"), true); /* "  x9    " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD05492182444901"), true); /* "  DI    " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD05492182C443CF"), true); /* "   C    " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD054822756645FF"), true); /* "fE      " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD054AE3054F2015"), true); /* "    O   " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD052E23B2372034"), true); /* "    7   " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD054F2182372000"), true); /* "      7 " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD09833D9D444901"), true); /* "      DI" */
    
    assert_string_equal(librds_get_ps(&ctx->rds), "RADIO DI");
}

static void
verification_ps_with_large_errors(void **state)
{
    test_context_t *ctx = *state;

    librds_set_correction(&ctx->rds, LIBRDS_STRING_PS, LIBRDS_BLOCK_TYPE_INFO, LIBRDS_BLOCK_ERROR_LARGE);
    librds_set_correction(&ctx->rds, LIBRDS_STRING_PS, LIBRDS_BLOCK_TYPE_DATA, LIBRDS_BLOCK_ERROR_LARGE);
    
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD04C0E305006473"), true); /* " d      " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD0548E305524100"), true); /* "RA      " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD0548E3054F350E"), true); /* "O5      " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD05492182444901"), true); /* "  x9    " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD05492182444901"), true); /* "  DI    " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD05492182C443CF"), true); /* "   C    " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD054822756645FF"), true); /* "fE      " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD054AE3054F2015"), true); /* "    O   " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD052E23B2372034"), true); /* "    7   " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD054F2182372000"), true); /* "      7 " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD09833D9D444901"), true); /* "      DI" */
    
    assert_string_equal(librds_get_ps(&ctx->rds), "O5DIO DI");
}

static void
verification_ps_progressive(void **state)
{
    test_context_t *ctx = *state;

    librds_set_correction(&ctx->rds, LIBRDS_STRING_PS, LIBRDS_BLOCK_TYPE_INFO, LIBRDS_BLOCK_ERROR_LARGE);
    librds_set_correction(&ctx->rds, LIBRDS_STRING_PS, LIBRDS_BLOCK_TYPE_DATA, LIBRDS_BLOCK_ERROR_LARGE);
    librds_set_progressive(&ctx->rds, LIBRDS_STRING_PS, true);
    
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD04C0E305006473"), true); /* " d      " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD0548E305524100"), true); /* "RA      " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD0548E3054F350E"), true); /* "O5      " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD05492182444901"), true); /* "  x9    " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD05492182444901"), true); /* "  DI    " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD05492182C443CF"), true); /* "   C    " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD054822756645FF"), true); /* "fE      " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD054AE3054F2015"), true); /* "    O   " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD052E23B2372034"), true); /* "    7   " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD054F2182372000"), true); /* "      7 " */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DD09833D9D444901"), true); /* "      DI" */
    
    assert_string_equal(librds_get_ps(&ctx->rds), "RADIO 7 ");
}

static void
verification_rt_a(void **state)
{
    test_context_t *ctx = *state;
    const char empty[] = "                                                                ";
    librds_register_rt(&ctx->rds, callback_rt);

    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_A), empty);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_B), empty);

    expect_function_call(callback_rt);
    snprintf(ctx->rt[0], sizeof(ctx->rt[0]), "KRDP                                                            ");
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25404B52445000"), true);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_A), ctx->rt[0]);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25404B52445000"), true);
    
    expect_function_call(callback_rt);
    snprintf(ctx->rt[0], sizeof(ctx->rt[0]), "KRDP Plo                                                        ");
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254120506C6F00"), true);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_A), ctx->rt[0]);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254120506C6F00"), true);
    
    expect_function_call(callback_rt);
    snprintf(ctx->rt[0], sizeof(ctx->rt[0]), "KRDP Plock u                                                    ");
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB2542636B207500"), true);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_A), ctx->rt[0]);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB2542636B207500"), true);
    
    expect_function_call(callback_rt);
    snprintf(ctx->rt[0], sizeof(ctx->rt[0]), "KRDP Plock ul. T                                                ");
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25436C2E205400"), true);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_A), ctx->rt[0]);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25436C2E205400"), true);
    
    expect_function_call(callback_rt);
    snprintf(ctx->rt[0], sizeof(ctx->rt[0]), "KRDP Plock ul. Tumsk                                            ");
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB2544756D736B00"), true);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_A), ctx->rt[0]);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB2544756D736B00"), true);
    
    expect_function_call(callback_rt);
    snprintf(ctx->rt[0], sizeof(ctx->rt[0]), "KRDP Plock ul. Tumska 3                                         ");
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25456120332000"), true);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_A), ctx->rt[0]);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25456120332000"), true);
    
    expect_function_call(callback_rt);
    snprintf(ctx->rt[0], sizeof(ctx->rt[0]), "KRDP Plock ul. Tumska 3 (I p                                    ");
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25462849207000"), true);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_A), ctx->rt[0]);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25462849207000"), true);
    
    expect_function_call(callback_rt);
    snprintf(ctx->rt[0], sizeof(ctx->rt[0]), "KRDP Plock ul. Tumska 3 (I pietr                                ");
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25476965747200"), true);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_A), ctx->rt[0]);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25476965747200"), true);
    
    expect_function_call(callback_rt);
    snprintf(ctx->rt[0], sizeof(ctx->rt[0]), "KRDP Plock ul. Tumska 3 (I pietro) T                            ");
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25486F29205400"), true);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_A), ctx->rt[0]);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25486F29205400"), true);
    
    expect_function_call(callback_rt);
    snprintf(ctx->rt[0], sizeof(ctx->rt[0]), "KRDP Plock ul. Tumska 3 (I pietro) Tel d                        ");
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB2549656C206400"), true);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_A), ctx->rt[0]);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB2549656C206400"), true);
    
    expect_function_call(callback_rt);
    snprintf(ctx->rt[0], sizeof(ctx->rt[0]), "KRDP Plock ul. Tumska 3 (I pietro) Tel do re                    ");
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254A6F20726500"), true);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_A), ctx->rt[0]);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254A6F20726500"), true);
    
    expect_function_call(callback_rt);
    snprintf(ctx->rt[0], sizeof(ctx->rt[0]), "KRDP Plock ul. Tumska 3 (I pietro) Tel do redakc                ");
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254B64616B6300"), true);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_A), ctx->rt[0]);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254B64616B6300"), true);
    
    expect_function_call(callback_rt);
    snprintf(ctx->rt[0], sizeof(ctx->rt[0]), "KRDP Plock ul. Tumska 3 (I pietro) Tel do redakcji:             ");
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254C6A693A2000"), true);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_A), ctx->rt[0]);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254C6A693A2000"), true);
    
    expect_function_call(callback_rt);
    snprintf(ctx->rt[0], sizeof(ctx->rt[0]), "KRDP Plock ul. Tumska 3 (I pietro) Tel do redakcji: 24 2        ");
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254D3234203200"), true);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_A), ctx->rt[0]);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254D3234203200"), true);
    
    expect_function_call(callback_rt);
    snprintf(ctx->rt[0], sizeof(ctx->rt[0]), "KRDP Plock ul. Tumska 3 (I pietro) Tel do redakcji: 24 264 6    ");
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254E3634203600"), true);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_A), ctx->rt[0]);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254E3634203600"), true);
    
    expect_function_call(callback_rt);
    snprintf(ctx->rt[0], sizeof(ctx->rt[0]), "KRDP Plock ul. Tumska 3 (I pietro) Tel do redakcji: 24 264 64 00");
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254F3420303000"), true);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_A), ctx->rt[0]);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254F3420303000"), true);
    
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_B), empty);

    librds_clear(&ctx->rds);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_A), empty);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_B), empty);
}

static void
verification_rt_b(void **state)
{
    test_context_t *ctx = *state;
    const char empty[] = "                                                                ";

    librds_register_rt(&ctx->rds, callback_rt);

    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_A), empty);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_B), empty);

    expect_function_call(callback_rt);
    snprintf(ctx->rt[1], sizeof(ctx->rt[1]), "KRDP                                                            ");
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25504B52445000"), true);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_B), ctx->rt[1]);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25504B52445000"), true);
    
    expect_function_call(callback_rt);
    snprintf(ctx->rt[1], sizeof(ctx->rt[1]), "KRDP Plo                                                        ");
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB255120506C6F00"), true);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_B), ctx->rt[1]);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB255120506C6F00"), true);
    
    expect_function_call(callback_rt);
    snprintf(ctx->rt[1], sizeof(ctx->rt[1]), "KRDP Plock u                                                    ");
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB2552636B207500"), true);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_B), ctx->rt[1]);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB2552636B207500"), true);
    
    expect_function_call(callback_rt);
    snprintf(ctx->rt[1], sizeof(ctx->rt[1]), "KRDP Plock ul. T                                                ");
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25536C2E205400"), true);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_B), ctx->rt[1]);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25536C2E205400"), true);
    
    expect_function_call(callback_rt);
    snprintf(ctx->rt[1], sizeof(ctx->rt[1]), "KRDP Plock ul. Tumsk                                            ");
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB2554756D736B00"), true);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_B), ctx->rt[1]);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB2554756D736B00"), true);
    
    expect_function_call(callback_rt);
    snprintf(ctx->rt[1], sizeof(ctx->rt[1]), "KRDP Plock ul. Tumska 3                                         ");
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25556120332000"), true);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_B), ctx->rt[1]);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25556120332000"), true);
    
    expect_function_call(callback_rt);
    snprintf(ctx->rt[1], sizeof(ctx->rt[1]), "KRDP Plock ul. Tumska 3 (I p                                    ");
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25562849207000"), true);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_B), ctx->rt[1]);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25562849207000"), true);
    
    expect_function_call(callback_rt);
    snprintf(ctx->rt[1], sizeof(ctx->rt[1]), "KRDP Plock ul. Tumska 3 (I pietr                                ");
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25576965747200"), true);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_B), ctx->rt[1]);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25576965747200"), true);
    
    expect_function_call(callback_rt);
    snprintf(ctx->rt[1], sizeof(ctx->rt[1]), "KRDP Plock ul. Tumska 3 (I pietro) T                            ");
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25586F29205400"), true);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_B), ctx->rt[1]);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25586F29205400"), true);
    
    expect_function_call(callback_rt);
    snprintf(ctx->rt[1], sizeof(ctx->rt[1]), "KRDP Plock ul. Tumska 3 (I pietro) Tel d                        ");
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB2559656C206400"), true);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_B), ctx->rt[1]);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB2559656C206400"), true);
    
    expect_function_call(callback_rt);
    snprintf(ctx->rt[1], sizeof(ctx->rt[1]), "KRDP Plock ul. Tumska 3 (I pietro) Tel do re                    ");
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB255A6F20726500"), true);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_B), ctx->rt[1]);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB255A6F20726500"), true);
    
    expect_function_call(callback_rt);
    snprintf(ctx->rt[1], sizeof(ctx->rt[1]), "KRDP Plock ul. Tumska 3 (I pietro) Tel do redakc                ");
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB255B64616B6300"), true);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_B), ctx->rt[1]);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB255B64616B6300"), true);
    
    expect_function_call(callback_rt);
    snprintf(ctx->rt[1], sizeof(ctx->rt[1]), "KRDP Plock ul. Tumska 3 (I pietro) Tel do redakcji:             ");
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB255C6A693A2000"), true);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_B), ctx->rt[1]);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB255C6A693A2000"), true);
    
    expect_function_call(callback_rt);
    snprintf(ctx->rt[1], sizeof(ctx->rt[1]), "KRDP Plock ul. Tumska 3 (I pietro) Tel do redakcji: 24 2        ");
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB255D3234203200"), true);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_B), ctx->rt[1]);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB255D3234203200"), true);
    
    expect_function_call(callback_rt);
    snprintf(ctx->rt[1], sizeof(ctx->rt[1]), "KRDP Plock ul. Tumska 3 (I pietro) Tel do redakcji: 24 264 6    ");
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB255E3634203600"), true);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_B), ctx->rt[1]);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB255E3634203600"), true);
    
    expect_function_call(callback_rt);
    snprintf(ctx->rt[1], sizeof(ctx->rt[1]), "KRDP Plock ul. Tumska 3 (I pietro) Tel do redakcji: 24 264 64 00");
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB255F3420303000"), true);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_B), ctx->rt[1]);
    /* Same value, no callback */
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB255F3420303000"), true);

    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_A), empty);

    librds_clear(&ctx->rds);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_A), empty);
    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_B), empty);
}

static void
verification_rt_invalid(void **state)
{
    test_context_t *ctx = *state;
    const char empty[] = "                                                                ";
    librds_register_rt(&ctx->rds, callback_rt);

    librds_set_correction(&ctx->rds, LIBRDS_STRING_RT, LIBRDS_BLOCK_TYPE_INFO, LIBRDS_BLOCK_ERROR_LARGE);
    librds_set_correction(&ctx->rds, LIBRDS_STRING_RT, LIBRDS_BLOCK_TYPE_DATA, LIBRDS_BLOCK_ERROR_LARGE);

    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25404B524450FF"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254120506C6FFF"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB2542636B2075FF"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25436C2E2054FF"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB2544756D736BFF"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254561203320FF"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254628492070FF"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254769657472FF"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25486F292054FF"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB2549656C2064FF"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254A6F207265FF"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254B64616B63FF"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254C6A693A20FF"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254D32342032FF"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254E36342036FF"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254F34203030FF"), true);

    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_A), empty);
    assert_int_equal(librds_get_rt_available(&ctx->rds, LIBRDS_RT_FLAG_A), false);
}

static void
verification_rt_invalid_pos(void **state)
{
    test_context_t *ctx = *state;
    const char empty[] = "                                                                ";
    librds_register_rt(&ctx->rds, callback_rt);

    librds_set_correction(&ctx->rds, LIBRDS_STRING_RT, LIBRDS_BLOCK_TYPE_INFO, LIBRDS_BLOCK_ERROR_LARGE);
    librds_set_correction(&ctx->rds, LIBRDS_STRING_RT, LIBRDS_BLOCK_TYPE_DATA, LIBRDS_BLOCK_ERROR_LARGE);

    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25404B52445030"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254120506C6F30"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB2542636B207530"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25436C2E205430"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB2544756D736B30"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25456120332030"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25462849207030"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25476965747230"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25486F29205430"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB2549656C206430"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254A6F20726530"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254B64616B6330"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254C6A693A2030"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254D3234203230"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254E3634203630"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254F3420303030"), true);

    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_A), empty);
    assert_int_equal(librds_get_rt_available(&ctx->rds, LIBRDS_RT_FLAG_A), false);
}

static void
verification_rt_invalid_data(void **state)
{
    test_context_t *ctx = *state;
    const char empty[] = "                                                                ";
    librds_register_rt(&ctx->rds, callback_rt);

    librds_set_correction(&ctx->rds, LIBRDS_STRING_RT, LIBRDS_BLOCK_TYPE_INFO, LIBRDS_BLOCK_ERROR_LARGE);
    librds_set_correction(&ctx->rds, LIBRDS_STRING_RT, LIBRDS_BLOCK_TYPE_DATA, LIBRDS_BLOCK_ERROR_LARGE);

    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25404B5244500F"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254120506C6F0F"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB2542636B20750F"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25436C2E20540F"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB2544756D736B0F"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB2545612033200F"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB2546284920700F"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB2547696574720F"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25486F2920540F"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB2549656C20640F"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254A6F2072650F"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254B64616B630F"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254C6A693A200F"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254D323420320F"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254E363420360F"), true);
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB254F342030300F"), true);

    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_A), empty);
    assert_int_equal(librds_get_rt_available(&ctx->rds, LIBRDS_RT_FLAG_A), false);
}

static void
verification_rt_empty(void **state)
{
    test_context_t *ctx = *state;
    librds_register_rt(&ctx->rds, callback_rt);

    expect_function_call(callback_rt);
#pragma GCC diagnostic ignored "-Wformat-zero-length"
    snprintf(ctx->rt[0], sizeof(ctx->rt[1]), "");
#pragma GCC diagnostic warning "-Wformat-zero-length"
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25000D202020"), true);

    assert_string_equal(librds_get_rt(&ctx->rds, LIBRDS_RT_FLAG_A), "");
    assert_int_equal(librds_get_rt_available(&ctx->rds, LIBRDS_RT_FLAG_A), true);
}

static void
verification_rt_empty_with_error(void **state)
{
    test_context_t *ctx = *state;
    librds_register_rt(&ctx->rds, callback_rt);

#pragma GCC diagnostic ignored "-Wformat-zero-length"
    snprintf(ctx->rt[0], sizeof(ctx->rt[1]), "");
#pragma GCC diagnostic warning "-Wformat-zero-length"
    assert_int_equal(librds_parse_string(&ctx->rds, "34DB25000D20202010"), true);

    assert_int_equal(librds_get_rt_available(&ctx->rds, LIBRDS_RT_FLAG_A), false);
}

const struct CMUnitTest tests[] =
{
    cmocka_unit_test_setup_teardown(verification_pi, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(verification_pi_invalid, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(verification_tp_true, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(verification_tp_false, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(verification_tp_invalid, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(verification_ta_true, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(verification_ta_false, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(verification_ta_invalid, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(verification_ms_true, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(verification_ms_false, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(verification_ms_invalid, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(verification_pty, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(verification_pty_invalid, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(verification_af, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(verification_af_invalid, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(verification_ecc, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(verification_ecc_invalid, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(verification_ps, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(verification_ps_invalid, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(verification_ps_invalid_pos, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(verification_ps_invalid_data, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(verification_ps_with_small_errors, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(verification_ps_with_large_errors, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(verification_ps_progressive, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(verification_rt_a, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(verification_rt_b, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(verification_rt_invalid, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(verification_rt_invalid_pos, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(verification_rt_invalid_data, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(verification_rt_empty, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(verification_rt_empty_with_error, test_setup, test_teardown)
};

int
main(void)
{
    return cmocka_run_group_tests(tests, group_setup, group_teardown);
}