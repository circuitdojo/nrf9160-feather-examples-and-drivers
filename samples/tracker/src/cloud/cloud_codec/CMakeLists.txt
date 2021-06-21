#
# Copyright (c) 2021 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

target_sources_ifdef(CONFIG_PYRINAS_CLOUD_ENABLED app 
                     PRIVATE ${ASSET_TRACKER_PATH}/src/cloud/cloud_codec/aws_iot_codec.c)