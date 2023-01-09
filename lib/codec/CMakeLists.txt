if(CONFIG_EXAMPLE_JSON_CODEC_ENABLE)
  zephyr_library()
  zephyr_library_sources(example_json_codec.c)
endif()