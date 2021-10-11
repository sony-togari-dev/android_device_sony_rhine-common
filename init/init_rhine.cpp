/*
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (C) 2014 The CyanogenMod Project
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

#include <stdlib.h>

#include <android-base/logging.h>

#include <fcntl.h>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <android-base/properties.h>
#include <android-base/file.h>
#include <android-base/strings.h>
#include <sys/_system_properties.h>
#include <sys/types.h>

#include "vendor_init.h"
#include "property_service.h"
#include "util.h"

#define LTALABEL "/lta-label"

enum { C6802, C6806, C6833, C6843 };

void property_override(char const prop[], char const value[], bool add = true)
{
    auto pi = (prop_info *) __system_property_find(prop);

    if (pi != nullptr) {
        __system_property_update(pi, value, strlen(value));
    } else if (add) {
        __system_property_add(prop, strlen(prop), value, strlen(value));
    }
}

void property_override_dual(char const system_prop[], char const vendor_prop[], char const value[])
{
    property_override(system_prop, value);
    property_override(vendor_prop, value);
}

void property_override_triple(char const product_prop[], char const system_prop[], char const vendor_prop[], char const value[])
{
    property_override(product_prop, value);
    property_override(system_prop, value);
    property_override(vendor_prop, value);
}

static int model_number_from_ltalabel() {
    DIR* dir;
    struct dirent* dp;
    int rc = 0;

    // Open '/lta-label' (like 'cd /lta-label')
    dir = opendir(LTALABEL);
    if (dir) {
        // Show all files inside '/lta-label' (like 'ls')
        while ((dp = readdir(dir)) != NULL) {
            // Only show html files (like 'grep html')
            if (strstr(dp->d_name, ".html")) {
                // Check one of supported models (like 'grep *model*')
                if (strstr(dp->d_name, "C6802") || strstr(dp->d_name, "c6802")) {
                    rc = C6802;
                } else if (strstr(dp->d_name, "C6806") || strstr(dp->d_name, "c6806")) {
                    rc = C6806;
                } else if (strstr(dp->d_name, "C6833") || strstr(dp->d_name, "c6833")) {
                    rc = C6833;
                } else if (strstr(dp->d_name, "C6843") || strstr(dp->d_name, "c6843")) {
                    rc = C6843;
                };
            };
        };
        // Close '/lta-label' (like 'cd /')
        closedir(dir);
    };
    return rc;
}

void import_kernel_cmdline(const std::function<void(const std::string&, const std::string&)>& fn) {
    std::string cmdline;
    android::base::ReadFileToString("/proc/cmdline", &cmdline);

    for (const auto& entry : android::base::Split(android::base::Trim(cmdline), " ")) {
        std::vector<std::string> pieces = android::base::Split(entry, "=");
        if (pieces.size() == 2) {
            fn(pieces[0], pieces[1]);
        }
    }
}

static void import_kernel_nv(const std::string& key, const std::string& value)
{
    if (key.empty()) return;

    // We only want the bootloader version
    if (key == "oemandroidboot.s1boot") {
		android::base::SetProperty("ro.boot.oemandroidboot.s1boot", value.c_str());
    }
}

void vendor_load_properties()
{
    import_kernel_cmdline(import_kernel_nv);

    std::string model = "";

    if (model_number_from_ltalabel() != 0) {

        switch (model_number_from_ltalabel()) {
            case C6802:
                model = "C6802";
                break;
            case C6806:
                model = "C6806";
                break;
            case C6833:
                model = "C6833";
                break;
            case C6843:
                model = "C6843";
                break;
            default:
                break;
       };

        property_override("ro.semc.product.model", model.c_str());
        property_override_triple("ro.product.model", "ro.product.system.model", "ro.product.vendor.model", model.c_str());
        property_override_triple("ro.product.name", "ro.product.system.name", "ro.product.vendor.name", model.c_str());
    }
}
