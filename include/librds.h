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

#ifndef LIBRDS_H
#define LIBRDS_H
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LIBRDS_AF_BUFFER_SIZE 26
#define LIBRDS_PS_LENGTH 8
#define LIBRDS_RT_LENGTH 64

typedef uint8_t librds_block_t;
enum librds_block
{
    LIBRDS_BLOCK_A = 0,
    LIBRDS_BLOCK_B = 1,
    LIBRDS_BLOCK_C = 2,
    LIBRDS_BLOCK_D = 3,
    LIBRDS_BLOCK_COUNT
};

typedef uint8_t librds_block_error_t;
enum librds_block_error
{
    LIBRDS_BLOCK_ERROR_NONE = 0,
    LIBRDS_BLOCK_ERROR_SMALL = 1,
    LIBRDS_BLOCK_ERROR_LARGE = 2,
    LIBRDS_BLOCK_ERROR_UNCORRECTABLE = 3,
};

typedef uint8_t librds_block_type_t;
enum librds_block_type
{
    LIBRDS_BLOCK_TYPE_INFO = 0,
    LIBRDS_BLOCK_TYPE_DATA = 1,
    LIBRDS_BLOCK_TYPE_COUNT
};

typedef uint8_t librds_text_t;
enum librds_text
{
    LIBRDS_TEXT_PS = 0,
    LIBRDS_TEXT_RT = 1,
    LIBRDS_TEXT_COUNT
};

typedef uint8_t librds_string_error_t;
enum librds_string_error
{
    LIBRDS_STRING_ERROR_NONE = 0,
    LIBRDS_STRING_ERROR_SMALLEST = 1,
    LIBRDS_STRING_ERROR_X_SMALL = 2,
    LIBRDS_STRING_ERROR_SMALL = 3,
    LIBRDS_STRING_ERROR_MEDIUM_SMALL = 4,
    LIBRDS_STRING_ERROR_MEDIUM = 5,
    LIBRDS_STRING_ERROR_MEDIUM_LARGE = 6,
    LIBRDS_STRING_ERROR_LARGE = 7,
    LIBRDS_STRING_ERROR_X_LARGE = 8,
    LIBRDS_STRING_ERROR_LARGEST = 9,
    LIBRDS_STRING_ERROR_UNCORRECTABLE
};

typedef uint8_t librds_rt_flag_t;
enum librds_rt_flag
{
    LIBRDS_RT_FLAG_A = 0,
    LIBRDS_RT_FLAG_B = 1,
    LIBRDS_RT_FLAG_COUNT
};

typedef struct librds librds_t;
typedef uint16_t librds_data_t[LIBRDS_BLOCK_COUNT];
typedef uint8_t librds_error_t[LIBRDS_BLOCK_COUNT];
typedef uint8_t librds_af_t[LIBRDS_AF_BUFFER_SIZE];
#ifndef LIBRDS_DISABLE_UNICODE
#include <wchar.h>
typedef wchar_t librds_string_char_t;
#else
typedef uint8_t librds_string_char_t;
#endif
typedef librds_string_char_t librds_string_t;

#ifndef LIBRDS_DISABLE_HEAP
librds_t* librds_new(void);
void librds_free(librds_t *context);
#else
#include "librds_private.h"
#endif

void librds_init(librds_t *context);
void librds_clear(librds_t *context);

void librds_parse(librds_t *context, librds_data_t data, librds_error_t errors);
bool librds_parse_string(librds_t *context, const char *input);

void librds_set_text_correction(librds_t *context, librds_text_t text, librds_block_type_t type, librds_block_error_t error);
librds_block_error_t librds_get_text_correction(const librds_t *context, librds_text_t text, librds_block_type_t type);

void librds_set_text_progressive(librds_t *context, librds_text_t string, bool state);
bool librds_get_text_progressive(const librds_t *context, librds_text_t string);

int32_t librds_get_pi(const librds_t *context);
int8_t librds_get_tp(const librds_t *context);
int8_t librds_get_ta(const librds_t *context);
int8_t librds_get_ms(const librds_t *context);
int8_t librds_get_pty(const librds_t *context);
int16_t librds_get_ecc(const librds_t *context);
const librds_af_t* librds_get_af(const librds_t *context);
const librds_string_t* librds_get_ps(const librds_t *context);
const librds_string_t* librds_get_rt(const librds_t *context, librds_rt_flag_t flag);

void librds_set_user_data(librds_t *context, void *user_data);

void librds_register_pi(librds_t *context, void (*callback_pi)(uint16_t, void*));
void librds_register_pty(librds_t *context, void (*callback_pty)(uint8_t, void*));
void librds_register_tp(librds_t *context, void (*callback_tp)(bool, void*));
void librds_register_ta(librds_t *context, void (*callback_ta)(bool, void*));
void librds_register_ms(librds_t *context, void (*callback_ms)(bool, void*));
void librds_register_ecc(librds_t *context, void (*callback_ecc)(uint8_t, void*));
void librds_register_af(librds_t *context, void (*callback_af)(uint8_t, void*));
void librds_register_ps(librds_t *context, void (*callback_ps)(const librds_string_t*, void*));
void librds_register_rt(librds_t *context, void (*callback_rt)(const librds_string_t*, librds_rt_flag_t, void*));

uint8_t librds_string_get_length(const librds_string_t *string);
bool librds_string_get_available(const librds_string_t *string);
const librds_string_char_t* librds_string_get_content(const librds_string_t *string);
const librds_string_error_t* librds_string_get_errors(const librds_string_t *string);

#ifdef __cplusplus
}
#endif

#endif