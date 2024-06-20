/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "properties"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#include "properties.h"
#include "threads.h"

static mutex_t env_lock = MUTEX_INITIALIZER;

int property_get(const char *key, char *value, const char *default_value) {
    char ename[PROPERTY_KEY_MAX + 6];
    char *p;
    int len;

    len = strlen(key);
    if (len >= PROPERTY_KEY_MAX) return -1;
    memcpy(ename, "PROP_", 5);
    memcpy(ename + 5, key, len + 1);

    mutex_lock(&env_lock);

    p = getenv(ename);
    if (p == 0) p = "";
    len = strlen(p);
    if (len >= PROPERTY_VALUE_MAX) {
        len = PROPERTY_VALUE_MAX - 1;
    }

    if ((len == 0) && default_value) {
        len = strlen(default_value);
        memcpy(value, default_value, len + 1);
    } else {
        memcpy(value, p, len);
        value[len] = 0;
    }

    mutex_unlock(&env_lock);

    return len;
}


int property_set(const char *key, const char *value) {
    char ename[PROPERTY_KEY_MAX + 6];
    char *p;
    int len;
    int r;

    if (strlen(value) >= PROPERTY_VALUE_MAX) return -1;

    len = strlen(key);
    if (len >= PROPERTY_KEY_MAX) return -1;
    memcpy(ename, "PROP_", 5);
    memcpy(ename + 5, key, len + 1);

    mutex_lock(&env_lock);
    r = setenv(ename, value, 1);
    mutex_unlock(&env_lock);

    return r;
}

int property_list(void (*propfn)(const char *key, const char *value, void *cookie),
                  void *cookie) {
    return 0;
}
